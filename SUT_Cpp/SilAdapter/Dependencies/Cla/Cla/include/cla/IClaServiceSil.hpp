// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>

#include "IClaService.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Interface which extends the `IClaService` for the SIL use case. */
class IClaServiceSil : public IClaService
{
public:
  /*! \brief Establishes a connection with CANoe for all distributed object members.
   *         Must be called once before data can be sent. */
  virtual void Connect() = 0;

  /*! \brief Closes the connection with CANoe for all distributed object members,
   *         which was previously established via `Connect()`. */
  virtual void Disconnect() = 0;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
