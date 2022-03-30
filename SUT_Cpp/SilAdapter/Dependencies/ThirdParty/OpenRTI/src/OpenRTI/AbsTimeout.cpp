

#include "DebugNew.h"
#include "AbsTimeout.h"
#include <iostream>

namespace OpenRTI {

Clock AbsTimeout::_watchDogPeriod    = Clock::fromMilliSeconds(100);
Clock AbsTimeout::_watchDogMinPeriod = Clock::fromMilliSeconds(200);

AbsTimeout::AbsTimeout(const Clock& abstime)
{
  _absTime = abstime;
  _delay = 0;
  _start = Clock::now();
  _done = false;
  if (_absTime > Clock::zero() && _absTime < Clock::max() && _absTime > Clock::now() + _watchDogMinPeriod)
  {
    _watchDogThreadResult = std::async(std::launch::async, &AbsTimeout::watchDogFunc, this);
  }
}


AbsTimeout::~AbsTimeout()
{
  if (_watchDogThreadResult.valid())
  {
    {
      std::unique_lock<std::mutex> lock(_doneMutex);
      _done = true;
      _cvDone.notify_one();
    }
    _watchDogThreadResult.wait();
  }
}

// Return true iff the specified absolute time, plus additional delays introduced by spending time
// in debug break, expired.
bool AbsTimeout::isExpired() const
{
  if (_absTime == Clock::max())
  {
    return false;
  }
  if (_absTime == Clock::zero())
  {
    return true;
  }
  Clock now = Clock::now();
  if (_watchDogThreadResult.valid())
  {
    Clock delay = getDelay();
    return now > _absTime + delay;
  }
  else
  {
    return now > _absTime;
  }
}

bool AbsTimeout::isExpired(Clock& remaining) const
{
  if (_absTime == Clock::max())
  {
    remaining = Clock::max();
    return false;
  }
  Clock now = Clock::now();
  if (_absTime == Clock::zero())
  {
    remaining = Clock::zero();
    return true;
  }
  Clock delay = Clock::zero();
  if (_watchDogThreadResult.valid())
  {
    delay = getDelay();
  }
  if (now < _absTime + delay)
  {
    remaining = (_absTime + delay) - now;
    return false;
  }
  else
  {
    remaining = Clock::zero();
    return true;
  }
}

// The awful double lock is required to circumvent a race condition between cv.wait_for
// and accessing the _delay provided by watchDogFunc.
// In essence, the following could happen:
// 1. watchDogFunc sits in _cvDone.wait_for, while the program is in break mode
// 2. upon return from debug break, _cvDone.wait_for returns (with timeout)
// 3. isExpired gets called, before watchDogFunc happened to update the break delay.
// The actual time spent in _cvDone.wait_for during break won't be taken into account.
OpenRTI::Clock AbsTimeout::getDelay() const
{
  // Wait until _cvDone.wait_for has been entered (this will release _doneMutex).
  _doneMutex.lock();
  _delayProvided = false;
  // Now trigger _cvDone.wait_for, in watchDogFunc.
  // Note that _cvDone.wait_for in watchDogFunc will wait for _doneMutex to be released.
  _cvDone.notify_all();
  // Get the 2nd lock before watchDogFunc can acquire it
  std::unique_lock<std::mutex> delayLock(_delayMutex);
  // Now release _cvDone.wait_for (in watchDogFunc)
  _doneMutex.unlock();
  // Wait for watchDogFunc to update it's delay
  _cvDelay.wait(delayLock, [this] { return _delayProvided; });
  // We got the _delayMutex again, so it's safe to access _delay.
  Clock delay = Clock::fromNanoSeconds(_delay);
  // Reset _delayProvided for next round
  _delayProvided = false;

  return delay;
}

void AbsTimeout::watchDogFunc()
{
  std::unique_lock<std::mutex> lock(_doneMutex);
  while (true)
  {
    if (_done) break;
    Clock start = Clock::now();
    // _doneMutex is released by this thread while waiting. If another thread takes the lock during wait, wait_for will wait additionally
    // until the lock is released by the other thread.
    _cvDone.wait_for(lock, std::chrono::steady_clock::duration(_watchDogPeriod.getNanoSeconds()), [this] { return _done || !_delayProvided; });
    // _doneMutex is now locked again by this thread.
    std::unique_lock<std::mutex> delayLock(_delayMutex);
    // now we can safely update _delay.
    Clock now = Clock::now();
    if ((now - start) > _watchDogPeriod)
    {
      _delay += ((now - start) - _watchDogPeriod).getNanoSeconds();
    }
    // notify a probably waiting getDelay callee about having updated _delay.
    _delayProvided = true;
    _cvDelay.notify_one();
  }
}

}  // namespace OpenRTI
