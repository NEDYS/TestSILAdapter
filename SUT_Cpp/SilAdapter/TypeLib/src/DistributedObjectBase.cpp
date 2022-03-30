// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "TypeLib/DistributedObjectBase.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

// Protected Methods
// -------------------

void DistributedObjectBase::_Init(const std::string& path) { mDo = Cla::GetDo(path); }

void DistributedObjectBase::_WaitForFirstTransmission() {}

void DistributedObjectBase::_Reset() { mDo.reset(); }

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
