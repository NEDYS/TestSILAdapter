// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ICallContext.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

class IProviderCallContext : public ICallContext
{
public:
  virtual ~IProviderCallContext() = default;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
