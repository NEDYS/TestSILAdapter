

#include "DebugNew.h"
#include "ImmediateCallbackDispatcher.h"


namespace OpenRTI {

class ImmediateCallbackDispatcher::InternalNotificationHandle : public AbstractNotificationHandle
{
public:
  void Signal() override
  {
    ScopeLock lock(_cvMutex);
    _signaled = true;
    _cv.notify_all();
  }
  void Wait()
  {
    ScopeLock lock(_cvMutex);
    while (!_signaled)
    {
      _cv.wait(lock);
    }
  }
private:
  Condition _cv;
  Mutex     _cvMutex;
  bool      _signaled = false;
};


ImmediateCallbackDispatcher::ImmediateCallbackDispatcher(InternalAmbassador& ambassador)
  : _ambassador(ambassador), _done(false)
{
  _notificationHandle = std::make_shared<InternalNotificationHandle>();
  _ambassador._setNotificationHandle(_notificationHandle);
}


ImmediateCallbackDispatcher::~ImmediateCallbackDispatcher()
{
  stop();
}

void ImmediateCallbackDispatcher::stop()
{
  if (!_done)
  {
    _done = true;
    _notificationHandle->Signal();
    wait();
  }
}

void ImmediateCallbackDispatcher::run()
{
  while (!_done)
  {
    _notificationHandle->Wait();

    if (_done) return; // check first, otherwise dispatchCallback may hang indefinitely
    try {
      if (_ambassador.getCallbacksEnabled())
      {
        while(_ambassador.dispatchCallback())
          ;
      }
    }
    catch (const OpenRTI::Exception& e)
    {
      DebugPrintf("exception in dispatchCallback: %s\n", e.what());
    }
    catch (...)
    {
    }
  }
}

}