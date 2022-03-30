// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "StrongType.hpp"

#include "cla/DataTypes.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

using CallbackHandle = StrongType<Cla::CallbackHandle, struct CallbackHandleTag>;
using TxTrigger = Cla::TxTrigger;
using MemberDirection = Cla::MemberDirection;

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
