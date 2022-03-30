// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>

#include "DataTypes.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

class IDoMember
{
public:
  virtual ~IDoMember() = default;

  virtual MemberType GetType() const = 0;
  virtual MemberDirection GetDirection() const = 0;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
