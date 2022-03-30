// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "cla/ClaMacros.hpp"
#include "cla/IClaServiceExperimental.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {
namespace Experimental {

/*! \brief Creates a new CLA service instance for the experimental features.
 *  \returns The CLA service instance. */
CLAFUNCTIONDEF std::shared_ptr<IClaServiceExperimental> CreateClaService();

} // namespace Experimental
} // namespace Cla
} // namespace CANoe
} // namespace Vector
