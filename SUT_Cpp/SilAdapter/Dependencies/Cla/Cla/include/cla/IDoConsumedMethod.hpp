// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <functional>

#include "IDoMethod.hpp"
#include "IConsumerCallContext.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

class IDoConsumedMethod : public IDoMethod {
public:
  //Creates a new call context in order to start a new call. The caller needs to hold a reference until the call has returned or timed out.
  virtual std::shared_ptr<IConsumerCallContext> CreateCallContext() = 0;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
