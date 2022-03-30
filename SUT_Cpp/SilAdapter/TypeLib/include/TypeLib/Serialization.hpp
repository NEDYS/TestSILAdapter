// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

namespace Vector {
namespace CANoe {
namespace TypeLib {

struct BeginStruct final { };
struct EndStruct final { };
struct BeginUnion final
{
  int typeIndex;
};
struct EndUnion final { };

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
