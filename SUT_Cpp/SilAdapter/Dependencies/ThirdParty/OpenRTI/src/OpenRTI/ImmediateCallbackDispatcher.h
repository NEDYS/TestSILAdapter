#pragma once

#include "Thread.h"
#include "Condition.h"
#include "InternalAmbassador.h"
#include "Mutex.h"

namespace OpenRTI {


class ImmediateCallbackDispatcher : public Thread
{
private:
  class InternalNotificationHandle;
public:
  ImmediateCallbackDispatcher(InternalAmbassador& ambassador);
  virtual ~ImmediateCallbackDispatcher();
  void stop();
protected:
  void run() override;
private:
  InternalAmbassador& _ambassador;
  std::shared_ptr<InternalNotificationHandle> _notificationHandle;
  std::atomic_bool _done;
};

}