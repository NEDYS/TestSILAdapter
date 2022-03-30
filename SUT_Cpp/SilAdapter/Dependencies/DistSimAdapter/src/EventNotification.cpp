
/*-----------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/

#include "EventNotification.h"

namespace NDistSimIB {

EventNotification::EventNotification()
{
  mHandle = nullptr;
  mSignalCounter = 0;
  // Create Event
#if defined(_WIN32)
  static SECURITY_ATTRIBUTES sSec = { sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
  mHandle = ::CreateEvent(&sSec, false, false, nullptr);
#elif defined (__linux__)

  int32_t fd = eventfd(false, EFD_CLOEXEC | EFD_NONBLOCK);
  if (fd != -1)
  {
    mHandle = new epoll_event();
    mHandle->events = EPOLLIN;
    mHandle->data.fd = fd;
  }
#endif
}

void EventNotification::CloseEvent()
{
#if defined(_WIN32)
  ::CloseHandle(mHandle);
  mHandle = nullptr;
#elif defined (__linux__)
  if (mHandle != nullptr)
  {
    close(mHandle->data.fd);
    delete mHandle;
  }
#endif
}

void EventNotification::Signal() 
{
#if defined(_WIN32)
  ::SetEvent(mHandle);
  mSignalCounter++;
#elif defined (__linux__)
  if (mHandle != nullptr)
  {
    uint64_t u = 41;
    ssize_t ret = write(mHandle->data.fd, &u, sizeof(uint64_t));
    (void)ret;
  }
#endif
}

EventNotification::WaitResult EventNotification::WaitForSingleEvent(int32_t timeoutMs)
{
#if defined(_WIN32)
  DWORD result = ::WaitForSingleObject(mHandle, timeoutMs);
  if (result < WAIT_OBJECT_0 + MAXIMUM_WAIT_OBJECTS) { return WaitResult::Success; }
  if (result == WAIT_TIMEOUT) { return WaitResult::Timeout; }
  return WaitResult::Failure;

#elif defined (__linux__)

  if (mHandle == nullptr) { return WaitResult::Failure; }

  constexpr uint32_t nfds = 1; // we are waiting only for one file descriptor
  pollfd fds[nfds];
  fds[0].fd = mHandle->data.fd;
  fds[0].events = EpollEventsToPollEvents(mHandle->events); // requested events :  There is data to read
  fds[0].revents = 0; //  returned events (output parameter, filled by the kernel)

  const auto pollResult = poll(fds, nfds, timeoutMs);
  if (pollResult == 1)
  {
    // Reset Event
    uint64_t u = 0;
    if (mHandle != nullptr)
    {
      auto sr = read(mHandle->data.fd, &u, sizeof(uint64_t));
      if (sr == -1)
      {
        assert(errno == EWOULDBLOCK);  // EWOULDBLOCK is the case when we try to reset a currently not set event
      }
      else
      {
        assert(sr == sizeof(uint64_t));
      }
    }

    return WaitResult::Success;
  }
  else if (pollResult == 0)
  {
    return WaitResult::Timeout;
  }
  else
  {
    int pollErr = errno;
    if (pollErr == EINTR)
    {
      // System call was interrupted by a signal handler
      return WaitResult::Interrupted;
    }
  }

  return WaitResult::Failure;
#endif

}

} // namespace NDistSimIB
