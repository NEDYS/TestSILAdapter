/*-----------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/
#pragma once

//#include "RemoteAmbassador.h"
#include "distSim/IServices.h"

namespace NDistSimIB {

//class TimingService;

class UserTimer : public ITimingService::ITimer
{
public:
  UserTimer(ITimingService::TimerCallback cb, VTimeNS offsetTime, VTimeNS period, bool cyclic);
  ~UserTimer();

  // Public API Methods
  void Cancel() override;
  void SetPeriod(VTimeNS newPeriod) override;

  // Getters
  VTimeNS GetNextAbsoluteDueTime() const { return mNextAbsoluteDueTime; };
  VTimeNS GetPeriod() const { return mPeriod; };
  bool IsCyclic() const { return mCyclic; }
  bool IsCanceled() const { return mCanceled; }

  void Execute(const std::chrono::nanoseconds currentTime) const;
  void Set(VTimeNS absoluteTime);
  void Shift(VTimeNS offsetTime);
  void Restart();

private:

  VTimeNS mNextAbsoluteDueTime;
  VTimeNS mPeriod;
  VTimeNS mOffsetTime;
  ITimingService::TimerCallback mCallback;
  bool mCyclic;
  bool mCanceled;
};


} // namespace NDistSimIB