// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>

#include "ClaMacros.hpp"
#include "IClaServiceSil.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Establish a connection with CANoe for all distributed object members
 *         get by `GetDo()`. Must be called once before data can be sent. */
CLAFUNCTIONDEF void Connect();

/*! \brief Closes a connection with CANoe for all distributed object members
 *         get by `GetDo()`, which was previously established via `Connect()`. */
CLAFUNCTIONDEF void Disconnect();

/*! \brief Creates a new CLA service object for the SIL use case.
 *  \returns the created CLA service instance. */
CLAFUNCTIONDEF std::shared_ptr<IClaServiceSil> CreateClaService();

} // namespace Cla
} // namespace CANoe
} // namespace Vector
