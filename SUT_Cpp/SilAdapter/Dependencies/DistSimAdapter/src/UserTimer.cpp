/*-----------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/
#include "UserTimer.h"
#include "TimingService.h"
#include "Utils.h"

namespace NDistSimIB {

UserTimer::UserTimer(ITimingService::TimerCallback cb, VTimeNS offsetTime, VTimeNS period, bool cyclic) : 
  mPeriod(period), mOffsetTime(offsetTime), mCallback(cb), mCyclic(cyclic)
{
  Set(offsetTime);
}

UserTimer::~UserTimer()
{
  DebugPrintf(SDebug, "~UserTimer\n");
}

void UserTimer::Restart()
{
  Set(mOffsetTime);
}

void UserTimer::Set(VTimeNS absoluteTime)
{
  mCanceled = false;
  mNextAbsoluteDueTime = absoluteTime + mPeriod;
}

void UserTimer::Shift(VTimeNS offsetTime)
{
  mNextAbsoluteDueTime += offsetTime;
}

void UserTimer::Execute(const std::chrono::nanoseconds currentTime) const
{
  mCallback(std::chrono::nanoseconds(this->mNextAbsoluteDueTime));
}

// Public API
void UserTimer::Cancel()
{
  mCanceled = true;
}

void UserTimer::SetPeriod(VTimeNS newPeriod)
{
  Shift(newPeriod - mPeriod);
  mPeriod = newPeriod;
}


} // namespace NDistSimIB