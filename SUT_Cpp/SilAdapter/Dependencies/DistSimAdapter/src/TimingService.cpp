/*-----------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/

#include "TimingService.h"
#include "RemoteAmbassador.h"

#include <math.h>

namespace NDistSimIB {

TimingService::TimingService(RemoteAmbassador& remoteAmb)
  : mRemoteAmbassador(remoteAmb)
{
  mLookahead = mRemoteAmbassador.mConfig.Lookahead;
  mLogicalTime = 0;
  mInitialOffsetTime = 0;
  mUserRequestedSimTime = 0;
  mProcessUserTimers = false;
  mIsRegulating = false;
  mIsConstrained = false;
  mRequestPending = false;
}

TimingService::~TimingService()
{
  DebugPrintf(SDebug, "~TimingService\n");
}

//=========================================
// Public API methods
std::shared_ptr<ITimingService::ITimer> TimingService::CreateOneshotTimer(TimerCallback callback, std::chrono::duration<VTimeNS, std::nano> absolutTime)
{
  std::shared_ptr<UserTimer> userTimer(new UserTimer(callback, 0, absolutTime.count(), false));
  return RegisterTimer(userTimer);
}
std::shared_ptr<ITimingService::ITimer> TimingService::CreateCyclicTimer(TimerCallback callback, std::chrono::duration<VTimeNS, std::nano> cycleTime)
{
  std::shared_ptr<UserTimer> userTimer(new UserTimer(callback, 0, cycleTime.count(), true));
  return RegisterTimer(userTimer);
}
std::shared_ptr<ITimingService::ITimer> TimingService::CreateCyclicTimer(TimerCallback callback, std::chrono::duration<VTimeNS, std::nano> offsetTime,  std::chrono::duration<VTimeNS, std::nano> cycleTime)
{
  std::shared_ptr<UserTimer> userTimer(new UserTimer(callback, offsetTime.count(), cycleTime.count(), true));
  return RegisterTimer(userTimer);
}

std::shared_ptr<ITimingService::ITimer> TimingService::RegisterTimer(std::shared_ptr<UserTimer> userTimer)
{
  // mTimerPersistent is moved to mTimerQueue in OnJoined
  mTimerPersistent.push_back(userTimer);
  if (mRemoteAmbassador.mState == RemoteAmbassador::AmbassadorState::Online)
  {
    mTimerQueue.push(userTimer);
  }
  return std::shared_ptr<ITimingService::ITimer>(userTimer);
}


void TimingService::RunTo(std::chrono::duration<VTimeNS, std::nano> finalTimestamp)
{
  VTimeNS finalTimestampNS = finalTimestamp.count();
  DebugPrintf(SDebug, "RunTo %lld\n", finalTimestampNS);

  while ((finalTimestampNS == 0) || (mLogicalTime <= finalTimestampNS))
  {
    VTimeNS nextTimerDueTime = mLogicalTime + 1000 * 1000 * 1000;
    if (!mTimerQueue.empty())
    {
      nextTimerDueTime = mTimerQueue.top()->GetNextAbsoluteDueTime();
    }
    if (nextTimerDueTime <= mLogicalTime)
    {
      ProcessUserTimers();
    }
    else
    {      
      auto start = std::chrono::system_clock::now();
      double duration = mAnimationFactor * (nextTimerDueTime - mLogicalTime) * 1e-9;
      // Mkb std::cout << "WAIT for " << " TOO FAST=" << mFederateTooFast << ": "<< mAnimationFactor << "*(" << nextTimerDueTime*1e-9 << "-" << mLogicalTime*1e-9 << ")=" << duration << "...";
      mRemoteAmbassador.EvokeCallbacks(duration);
      if (!this->mIsConstrained && !this->mIsRegulating && !this->mFederateTooFast && mRemoteAmbassador.mConfig.TimingInterpolationEnabled)
      {
        std::chrono::nanoseconds spent = std::chrono::system_clock::now() - start;
        // MKb std::cout << "  .... spent=" << spent.count() * 1e-9 << std::endl;
        // FIXME Mkb: increase time only after measurement started
        this->SetNewTime((VTimeNS)(mLogicalTime + spent.count() / mAnimationFactor));
      }
    }
    if (mRemoteAmbassador.GetState() == RemoteAmbassador::AmbassadorState::ReceivedMeasurementStop)
    {
      return;
    }
  }
  DebugPrintf(SDebug, "RanTo %lld\n", this->GetSimTime());

}

std::chrono::nanoseconds TimingService::GetSimTime() const
{
  return std::chrono::nanoseconds(mLogicalTime);
}

// Convenience
void TimingService::RequestSimulationTimeStep(std::chrono::duration<VTimeNS, std::nano> delta)
{
  std::chrono::nanoseconds newTime = std::chrono::nanoseconds(mLogicalTime + delta.count());
  RequestSimulationTime(newTime);
}

void TimingService::RequestSimulationTime(std::chrono::duration<VTimeNS, std::nano> newTime)
{
  // Non-Regulating federates can't set the time
  if (!mIsRegulating)
  {
    // TODO/DISCUSS bkd: Same as RunTo()?  
    Error("%s for Non-TimeRegulating undefined\n", __FUNCTION__);
    return;
  }

  VTimeNS newTimeNS = newTime.count();
  DebugPrintf(SDebug, "%s: %lld\n", __FUNCTION__, newTimeNS);
  if (newTimeNS <= mLogicalTime)
  {
    DebugPrintf(SWarn, "%s: Requested simulation time (%lld) must be greater than the current simulation time (%lld)\n", __FUNCTION__, newTimeNS, mLogicalTime);
    return;
  }

  // Save the requested time, this is then used in the TimeRequest
  mUserRequestedSimTime = newTimeNS;

  // Trigger an event notification to run through evokeCallbacks to NMR in threaded mode
  mRemoteAmbassador.SendManualSignal();
}

//=========================================
// Methods called by RemoteAbmassador
void TimingService::OnJoined()
{
  if (!mRemoteAmbassador.mConfig.TimeConstrainedEnabled && !mRemoteAmbassador.mConfig.TimeRegulationEnabled)
  {
    // If we don't get the federation time by time policy callbacks, we query it
    rti::HLAinteger64Time queriedTime;
    // TODO bkd: LITS also calls getGALT internally, check if still valid with OpenRTI+TCList (CLBTS only for TC Federates)
    mRemoteAmbassador.mRTIAmb->queryLITS(queriedTime);
    SetInitialTime(convertTime(queriedTime));
  }
  else
  {
    if (mRemoteAmbassador.mConfig.TimeRegulationEnabled)
    {
      try
      {
        mRemoteAmbassador.mRTIAmb->enableTimeRegulation(rti::HLAinteger64Interval(mRemoteAmbassador.mConfig.Lookahead));
        // Wait for callback
        while (!mIsRegulating)
        {
          //DebugPrintf(SInfo, "---->mIsRegulating: Wait for callback \n");
          mRemoteAmbassador.mRTIAmb->evokeCallback(RTIConstants::kDefaultEvokeTimeout);
        }

      }
      catch (const rti::Exception & e)
      {
        Error("%s: enable time regulation: %ls\n", __FUNCTION__, e.what().c_str());
        (void)e;
        throw;
      }
    }
    if (mRemoteAmbassador.mConfig.TimeConstrainedEnabled)
    {
      try
      {
        mRemoteAmbassador.mRTIAmb->enableTimeConstrained();
        // Wait for callback
        while (!mIsConstrained)
        {
          //DebugPrintf(SInfo, "---->mIsConstrained: Wait for callback \n");
          mRemoteAmbassador.mRTIAmb->evokeCallback(RTIConstants::kDefaultEvokeTimeout);
        }
      }
      catch (const rti::Exception & e)
      {
        Error("%s: enable time constrained: %ls\n", __FUNCTION__, e.what().c_str());
        (void)e;
        throw;
      }
    }
  }

  assert(mTimerQueue.empty());

  // Move mTimerPersistent to mTimerQueue
  for (auto ut : mTimerPersistent)
  {
    ut->Restart();
    mTimerQueue.push(ut);
  }

  // TODO bkd: Check if enableTimeRegulation/enableTimeConstrained both deliver a correct start time

  // mInitialOffsetTime is now valid: Shift already known UserTimer 
  std::vector<std::shared_ptr<UserTimer>> tempUserTimerList;
  while (!mTimerQueue.empty())
  {
    auto topTimer = mTimerQueue.top();
    tempUserTimerList.push_back(topTimer);
    mTimerQueue.pop();
  }
  for (auto ut : tempUserTimerList)
  {
    ut->Shift(mInitialOffsetTime);
    mTimerQueue.push(ut);
  }

  // Now we got a valid time, process callbacks for the simulation start
  //mRemoteAmbassador.ProcessCallbacksOnSimulationStart();
}

void TimingService::Reset()
{
  //DebugPrintf(SDebug, "%s\n", __FUNCTION__);
  mLogicalTime = 0;
  mGrantedTime = 0;
  mUserRequestedSimTime = 0;
  mInitialOffsetTime = 0;
  mProcessUserTimers = false;
  mIsRegulating = false;
  mIsConstrained = false;
  mRequestPending = false;
  // Clear timer queue
  while (!mTimerQueue.empty())
  {
    mTimerQueue.pop();
  }
}

bool TimingService::ProcessBuffers()
{
  if (mProcessUserTimers)
  {
    mProcessUserTimers = false;
    ProcessUserTimers();
  }
  return false;
}


bool TimingService::IsTimeConstrained() const 
{
  return this->mIsConstrained;
}

bool TimingService::IsTimeRegulating() const
{
  return this->mIsRegulating;
}

std::chrono::nanoseconds TimingService::GetNextTimerDueTime() const
{ 
  std::chrono::nanoseconds result = std::chrono::nanoseconds(mLogicalTime + 1000 * 1000 * 1000);
  if (!mTimerQueue.empty())
  {
    result = std::chrono::nanoseconds(mTimerQueue.top()->GetNextAbsoluteDueTime());
  }
  return result;
}


double TimingService::GetAnimationFactor() const
{
  return this->mAnimationFactor;
}

VTimeNS TimingService::GetEarliestSendTime() const
{
  return mGrantedTime + mLookahead;
}

bool TimingService::IsFederateTooFast() const 
{
  return this->mFederateTooFast;
}

//=========================================
// Private methods

// Set the inital time for late joining federates
void TimingService::SetInitialTime(VTimeNS initialTime)
{
  //DebugPrintf(SDebug, "%s: %lld\n", __FUNCTION__, initialTime);
  mLogicalTime = initialTime;
  mGrantedTime = initialTime;
  mInitialOffsetTime = initialTime;
}

void TimingService::SetNewTime(VTimeNS newTime)
{
  mGrantedTime = newTime;

  if (newTime > mLogicalTime)
  {
    if (!mIsConstrained && !mIsRegulating && mRemoteAmbassador.mConfig.TimingInterpolationEnabled)
    {
      // Mkb std::cout << "SetNewTimeFromEvent(" << newTime * 1e-9 << " ) last=" << mLastLogicalTimeValue * 1e-9 << std::endl;

      // Estimate animation factor
      mFederateTooFast = newTime <= mLastLogicalTimeValue;
      if (newTime > mLastLogicalTimeValue)
      {
        const double kTimeConstant = 1.0; // in seconds
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::chrono::nanoseconds diff = now - mLastLogicalTimeUpdate;
        // FIXME Mkb: Initialize mLastLogicalTimeUpdate = std::chrono::system_clock::now(); in MEasurement Start (oder Late-Joining)
        if (mLastLogicalTimeValue > 0)
        {
          double timeDiffSim = (newTime - mLastLogicalTimeValue) * 1e-9;
          double timeDiffReal = diff.count() * 1e-9;
          double p = std::min(timeDiffSim, std::min(timeDiffReal, 2.0 / 3.1415926 * atan(timeDiffReal / kTimeConstant)));
          mAnimationFactor = (1 - p) * mAnimationFactor + p * timeDiffReal / timeDiffSim;
          /* MKb std::cout << "mLastLogicalTimeValue=" << mLastLogicalTimeValue * 1e-9 << " newTime=" << newTime * 1e-9
                    << " timeDiffSim=" << timeDiffSim << " timeDiffReal=" << timeDiffReal
                    << " p=" << p << " mAnimationFactor=" << mAnimationFactor << std::endl;*/
        }
        mLastLogicalTimeUpdate = now;
        mLastLogicalTimeValue = newTime;
      }
      // End Estimate animation factor
    }

    mLogicalTime = newTime;
    mProcessUserTimers = true;
  }  
}

void TimingService::SetNewTimeFromEvent(VTimeNS newTime)
{
  if (!mIsConstrained && !mIsRegulating)
  {
    //DebugPrintf(SDebug, "%s: -> %lld\n", __FUNCTION__, newTime);
    SetNewTime(newTime);
  }
}

void TimingService::ProcessUserTimers()
{
  ProcessUserTimers(mLogicalTime);
}

void TimingService::ProcessUserTimers(VTimeNS processUntil)
{
  while (!mTimerQueue.empty())
  {
    if (mTimerQueue.top()->IsCanceled())
    {
      mTimerQueue.pop();
      continue;
    }

    VTimeNS nextTimerDueTime = mTimerQueue.top()->GetNextAbsoluteDueTime();
    if (processUntil >= nextTimerDueTime)
    {
     // DebugPrintf(SDebug, "%s nextTimerDueTime = %lld\n", __FUNCTION__, nextTimerDueTime);

      // Timer is due: get the shared_ptr and pop()
      auto topTimer = mTimerQueue.top();
      mTimerQueue.pop();

      topTimer->Execute((std::chrono::nanoseconds)processUntil);
      if (topTimer->IsCyclic())
      {
        // TODO: Diskussion: wann nextTimerDueTime, wann now+Intervall
        topTimer->Set(nextTimerDueTime);
        mTimerQueue.push(topTimer);
      }
    }
    else
    {
      break;
    }
  }
}

//=========================================
// OpenRTI callbacks
void TimingService::timeRegulationEnabled(rti::LogicalTime const& theTime)
{
  DebugPrintf(SDebug,"%s\n", __FUNCTION__);
  mIsRegulating = true;
  SetInitialTime(convertTime(theTime));
}

void TimingService::timeConstrainedEnabled(rti::LogicalTime const& theTime)
{
  DebugPrintf(SDebug, "%s\n", __FUNCTION__);
  mIsConstrained = true;
  SetInitialTime(convertTime(theTime));
}

void TimingService::timeAdvanceGrant(rti::LogicalTime const& theTime)
{
  //DebugPrintf(SDebug, "%s\n", __FUNCTION__);
  VTimeNS convertedTime = convertTime(theTime);
  SetNewTime(convertedTime);
  mRequestPending = false;
}

void TimingService::TimeRequest()
{
  if (mRemoteAmbassador.GetState() == RemoteAmbassador::AmbassadorState::Online &&
      !mRequestPending &&
      (mRemoteAmbassador.mConfig.TimeConstrainedEnabled || mRemoteAmbassador.mConfig.TimeRegulationEnabled))
  {
    // Default advance
    VTimeNS nextRequestTime = 0;

    // TODO bkd: Only kEndOfTime causes a crash because the lookahead is added to the requested time.
    // Using this workaround until this is fixed in OpenRTI
    if (mIsConstrained)
    {
      nextRequestTime = kEndOfTime - mLookahead - 1;
    }

    // If there is a timer, target the next timer due time in the NMR
    if (!mTimerQueue.empty())
    {
      nextRequestTime = mTimerQueue.top()->GetNextAbsoluteDueTime();
    }
    
    // If the user explicitly requested a valid time, prioritize that
    if (mUserRequestedSimTime > mLogicalTime)
      nextRequestTime = mUserRequestedSimTime;

    if (nextRequestTime > mLogicalTime)
    {
      // There is a UserTimer or a RequestSimulationTime 
      try
      {
        if (mIsConstrained)
        {
          // Constrained federates use nextMessageRequest and may get (constrained by) a grant for a lower time
          DebugPrintf(SDebug, "nextMessageRequest to %lld\n", nextRequestTime);
          mRemoteAmbassador.mRTIAmb->nextMessageRequest(rti::HLAinteger64Time(nextRequestTime));
        }
        else if (mIsRegulating)
        {
          // Purely regulating federates use timeAdvanceRequest
          DebugPrintf(SDebug, "timeAdvanceRequest to %lld\n", nextRequestTime);
          mRemoteAmbassador.mRTIAmb->timeAdvanceRequest(rti::HLAinteger64Time(nextRequestTime));
        }
        mRequestPending = true;
      }
      catch (const rti::Exception & e)
      {
        Error("%s: %ls\n", __FUNCTION__, e.what().c_str());
        (void)e;
        throw;
      }
    }
    //else
    //{
    //  // This federate has nothing to do, send a TAR to 1s in the future
    //  // TODO bkd: A NMR would make more sense here but causes the federation to stall for NTC/TR
    //  try
    //  {
    //    DebugPrintf(SDebug, "timeAdvanceRequest to %lld\n", mLogicalTime + 1000*1000*1000);
    //    mRemoteAmbassador.mRTIAmb->timeAdvanceRequest(rti::HLAinteger64Time(mLogicalTime + 1000 * 1000 * 1000));
    //    mRequestPending = true;
    //  }
    //  catch (const rti::Exception & e)
    //  {
    //    Error("%s: %ls\n", __FUNCTION__, e.what().c_str());
    //    (void)e;
    //    throw;
    //  }
    //}

  }
}

} // namespace NDistSimIB