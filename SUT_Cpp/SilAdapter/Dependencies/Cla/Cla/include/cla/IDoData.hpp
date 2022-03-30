// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IDoMember.hpp"
#include "IValueEntity.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

class IDoData : public IDoMember {
public:
  virtual std::shared_ptr<IValueEntity> GetValue() const = 0;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
