// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IDoConsumedEvent.hpp"
#include "IDoConsumedMethod.hpp"
#include "IDoField.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

class IDoConsumedField : public IDoField
{
public:
  virtual std::shared_ptr<IDoConsumedEvent> GetConsumedEvent() const = 0;
  virtual std::shared_ptr<IDoConsumedMethod> GetConsumedGetter() const = 0;
  virtual std::shared_ptr<IDoConsumedMethod> GetConsumedSetter() const = 0;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
