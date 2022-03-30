// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>

#include "ICallContext.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

class IConsumerCallContext : public ICallContext
{
public:
  virtual ~IConsumerCallContext() = default;

  //Call method asynchronously. The callback will be called after the call has been returned or timed out (see ICallContext::GetCallState()).
  virtual void CallAsync(std::function<void()> callback) = 0; 
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
