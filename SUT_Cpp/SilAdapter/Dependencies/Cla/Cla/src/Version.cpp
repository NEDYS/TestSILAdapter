// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "cla/Version.hpp"
#include "VersionMacros.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

CLAFUNCTIONDEF Version GetImplementationVersion() { return { CLA_MAJOR_VERSION, CLA_MINOR_VERSION, CLA_PATCH_VERSION }; }

CLAFUNCTIONDEF std::string GetImplementationName() { return "SabCla"; }

} // namespace Cla
} // namespace CANoe
} // namespace Vector
