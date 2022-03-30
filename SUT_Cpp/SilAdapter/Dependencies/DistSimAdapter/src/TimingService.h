/*-----------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/
#pragma once

#include "AbstractService.h"
//#include "RemoteAmbassador.h"
#include "UserTimer.h"
#include <queue>
#include <chrono>
#include <atomic>

#include "RTI/time/HLAinteger64Time.h"

namespace NDistSimIB {

// Forward declaration
class RemoteAmbassador;

class TimingService : 
  public ITimingService, public AbstractService
{
public:
  TimingService(RemoteAmbassador& federate);
  ~TimingService();

  // =========================================
  // Public DistSimAPI methods

  // Timer handling
  std::shared_ptr<ITimingService::ITimer> CreateOneshotTimer(ITimingService::TimerCallback callback, std::chrono::duration<VTimeNS, std::nano> timeFromNow) override;
  std::shared_ptr<ITimingService::ITimer> CreateCyclicTimer(ITimingService::TimerCallback callback, std::chrono::duration<VTimeNS, std::nano> cycleTime) override;
  std::shared_ptr<ITimingService::ITimer> CreateCyclicTimer(ITimingService::TimerCallback callback, std::chrono::duration<VTimeNS, std::nano> offsetTime, std::chrono::duration<VTimeNS, std::nano> cycleTime) override;

  // Simulation control
  void RunTo(std::chrono::duration<VTimeNS, std::nano> finalTimestamp) override;
  std::chrono::nanoseconds GetSimTime() const override;
  void RequestSimulationTime(std::chrono::duration<VTimeNS, std::nano> newTime) override;
  void RequestSimulationTimeStep(std::chrono::duration<VTimeNS, std::nano> delta) override;
  
   //=========================================
  // Service Methods called by RemoteAbmassador
  void OnJoined() override;
  void Reset() override;
  bool ProcessBuffers() override;

  // =========================================
  // OpenRTI callbacks

  // Time management
  void timeRegulationEnabled(rti::LogicalTime const& theFederateTime);
  void timeConstrainedEnabled(rti::LogicalTime const& theFederateTime);
  void timeAdvanceGrant(rti::LogicalTime const& theTime);


  // Public
  void TimeRequest();

  struct CompareUserTimer
  {
    bool operator()(std::shared_ptr<UserTimer> const& t1, std::shared_ptr<UserTimer> const& t2) {
      return t1->GetNextAbsoluteDueTime() > t2->GetNextAbsoluteDueTime();
    }
  };
  typedef std::priority_queue<std::shared_ptr<UserTimer>, std::vector<std::shared_ptr<UserTimer>>, CompareUserTimer> UserTimerQueue;

  UserTimerQueue mTimerQueue;
  std::vector<std::shared_ptr<UserTimer>> mTimerPersistent;

  void SetNewTime(VTimeNS newTime);
  void SetInitialTime(VTimeNS newTime);
  void SetNewTimeFromEvent(VTimeNS newTime);
  bool IsTimeConstrained() const;
  bool IsTimeRegulating() const;
  bool IsFederateTooFast() const;
  std::chrono::nanoseconds GetNextTimerDueTime() const;
  double GetAnimationFactor() const;

  VTimeNS GetEarliestSendTime() const;

  // Process timer queue; respect due time, canceled timer, cyclic timer
  void ProcessUserTimers(); // Default is to process up to mLogicalTime
  void ProcessUserTimers(VTimeNS processUntil); // Used to process timers to requested time of SetSimulationTime 


private:
  // =========================================
  //The parent federate
  RemoteAmbassador& mRemoteAmbassador;

  // =========================================
  // Private TimingService methods

  // Common registration
  std::shared_ptr<ITimingService::ITimer> RegisterTimer(std::shared_ptr<UserTimer> userTimer);

  // =========================================
  // Private TimingService data types

  VTimeNS                                  mLookahead;
  VTimeNS                                  mLogicalTime; // The strictly monotonic requested and granted simulation time 
  VTimeNS                                  mGrantedTime; // The granted simulation time 
  VTimeNS                                  mInitialOffsetTime;
  
  bool                                     mProcessUserTimers;
  bool                                     mIsRegulating;
  bool                                     mIsConstrained;


  // Animation factor of 10 means: 10 real seconds for 1 simulated second
  double                                   mAnimationFactor = 1.0;
  std::chrono::system_clock::time_point    mLastLogicalTimeUpdate;
  VTimeNS                                  mLastLogicalTimeValue = 0;
  bool                                     mFederateTooFast = false;
  
  // Is a timeAdvanceRequest or nextMessgeRequest pending? (=unconfirmed by grant)
  // Atomic because mRequestPending is set both from 
  // SetSimulationTime to block the threaded execution of TimingService::TimeRequest in RemoteAmbassador::EvokeCallbacks,
  // also simultaneously from TimingService::TimeRequest
  std::atomic<bool>                        mRequestPending; 

  VTimeNS                                  mUserRequestedSimTime;

};



} // namespace NDistSimIB