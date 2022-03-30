// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>

#include "IDoMethod.hpp"
#include "IProviderCallContext.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

class IDoProvidedMethod : public IDoMethod {
public:
  virtual void SetCallback(std::function<void(IProviderCallContext&)> callback) = 0;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
