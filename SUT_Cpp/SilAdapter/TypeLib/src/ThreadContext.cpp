// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <cstdint>

#include "TypeLib/ThreadContext.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

static thread_local uint32_t callbackCounter{0};

bool IsWithinCallback() { return callbackCounter > 0; }

void StartCallback() { callbackCounter++; }

void EndCallback() { callbackCounter--; }

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
