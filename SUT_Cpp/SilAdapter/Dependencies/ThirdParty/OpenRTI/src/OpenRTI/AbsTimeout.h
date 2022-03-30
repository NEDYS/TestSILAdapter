#pragma once

#include "Clock.h"
#include <chrono>
#include <thread>
#include <future>
#include <condition_variable>
#include <mutex>
#include <iostream>
#include <atomic>

#ifndef __CPlusPlusStd
  #error "must include OpenRTIConfig.h!"
#endif

namespace OpenRTI {

// AbsTimeout implements an interruptible timeout clock.
// Interruptible means that, if program execution gets suspended (e.g. by a debugger)
// the wall clock time the program does not get executed will be added to the (absolute)
// timeout time, thus preventing the caller from running into timeouts just because of
// being halted by the debugger.
// This is implemented by a watchdog thread waiting repeatedly for a given, short period of time.
// When the program execution halts, the watchdog thread is also halted and the actual time spent
// waiting in the watchdog thread differs from the given time period. The difference is added to the 
// absolute timeout.
class OPENRTI_API AbsTimeout
{
  public:

    explicit AbsTimeout(const Clock& abstime);

    ~AbsTimeout();
    void set(const Clock& abstime) noexcept { _absTime = abstime; }
    bool isExpired() const;
    bool isExpired(Clock& remaining) const;

    Clock getTimeout() const noexcept { return _absTime; }
  private:
    AbsTimeout(const AbsTimeout& abstime) = delete;
    AbsTimeout(AbsTimeout&&) = delete;
    AbsTimeout& operator=(const AbsTimeout& abstime) = delete;
    AbsTimeout& operator=(AbsTimeout&&) = delete;

    Clock getDelay() const;
    void watchDogFunc();

    Clock                           _start;
    Clock                           _absTime;
    // the waitable object for the watchdog thread.
    std::future<void>               _watchDogThreadResult;
    // the accumulated delay.
    uint64_t                        _delay;
    // the watch dog thread cycle time.
    static Clock                    _watchDogPeriod;
    // Minimum timeout period. The watchdog thread will only be started if
    // the timeout period is longer than _watchDogMinPeriod.
    static Clock                    _watchDogMinPeriod;
    bool                            _done = false;
    mutable std::condition_variable _cvDone;
    mutable std::mutex              _doneMutex;
    mutable bool                    _delayProvided = false;
    mutable std::condition_variable _cvDelay;
    mutable std::mutex              _delayMutex;
};

}  // namespace OpenRTI
