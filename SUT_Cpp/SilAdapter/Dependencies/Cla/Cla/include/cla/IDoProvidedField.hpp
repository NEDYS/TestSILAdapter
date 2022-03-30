// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IDoField.hpp"
#include "IDoProvidedEvent.hpp"
#include "IDoProvidedMethod.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

class IDoProvidedField : public IDoField
{
public:
  virtual std::shared_ptr<IDoProvidedEvent> GetProvidedEvent() const = 0;
  virtual std::shared_ptr<IDoProvidedMethod> GetProvidedGetter() const = 0;
  virtual std::shared_ptr<IDoProvidedMethod> GetProvidedSetter() const = 0;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
