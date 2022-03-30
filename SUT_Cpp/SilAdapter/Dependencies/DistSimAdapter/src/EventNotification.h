/*-----------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/

#pragma once

#include <cassert>

#include "RTI/RTIambassador.h"
#include "DistSimIBConfig.h"

#if defined(_WIN32)
#include <windows.h>
#elif defined (__linux__)
#include <unistd.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#endif

#if defined(_WIN32)
using TriggerHandle = HANDLE;
#elif defined (__linux__)
using TriggerHandle = epoll_event*;
#endif

namespace NDistSimIB {

class EventNotification : public rti::RTInotificationHandle
{
public:

  enum class WaitResult
  {
    Success,       //!< The wait operation succeeded
    Failure,       //!< The wait operation failed
    Timeout,       //!< The wait operation timed out
    Interrupted,   //!< The wait operation was interrupted by a signal handler (i.e. SIGINT). Usually the wait operation can be tried again.
  };

  // Creates the event handle
  EventNotification();

  virtual ~EventNotification() {};

  // Called by openRTI
  virtual void Signal() override;

  WaitResult WaitForSingleEvent(int32_t timeoutMs);
  void CloseEvent();

  uint32_t mSignalCounter;

private:
  TriggerHandle mHandle;

  // Utility
#if defined (__linux__)
  static short EpollEventsToPollEvents(uint32_t events)
  {
    short result = 0;
    if (events & EPOLLIN) { result |= POLLIN; }
    if (events & EPOLLOUT) { result |= POLLOUT; }
    if (events & EPOLLHUP) { result |= POLLHUP; }
    if (events & EPOLLERR) { result |= POLLERR; }
    return result;
  }
#endif

};

} // namespace NDistSimIB
