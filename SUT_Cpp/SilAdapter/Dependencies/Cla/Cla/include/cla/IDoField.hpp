// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IDoEvent.hpp"
#include "IDoMember.hpp"
#include "IDoMethod.hpp"
#include "IValueEntity.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

class IDoField : public IDoMember {
public:
  virtual std::shared_ptr<IValueEntity> GetValue() const = 0;
  virtual std::shared_ptr<IDoEvent> GetEvent() = 0;
  virtual std::shared_ptr<IDoMethod> GetGetter() = 0;
  virtual std::shared_ptr<IDoMethod> GetSetter() = 0;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
