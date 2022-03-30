// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "TypeLib/DoMethod.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

void DoMethod::_Init(Cla::IDo*, const std::string& path) { mPath = path; }

void DoMethod::_Reset() { mPath.clear(); }

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
