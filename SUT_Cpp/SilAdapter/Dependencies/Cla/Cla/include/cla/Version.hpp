// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <string>

#include "ClaMacros.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Structure to hold the version information. */
struct Version
{
  uint32_t major; //! \brief The major version.
  uint32_t minor; //! \brief the minor version.
  uint32_t patch; //! \brief the patch version.
};

/*! \brief Gets the version of the CLA implementation.
 *  \returns the version of the CLA implementation. */
CLAFUNCTIONDEF Version GetImplementationVersion();

/*! \brief Gets the name of the CLA implementation.
 *  \returns the name of the CLA implementation. */
CLAFUNCTIONDEF std::string GetImplementationName();

}
}
}
