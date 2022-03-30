// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "cla/IClaServiceSil.hpp"

#include "distSim/IServices.h"

namespace Vector {
namespace CANoe {
namespace Cla {
namespace Experimental {

/*! \brief Interface representing a CLA service with additional experimental features. */
class IClaServiceExperimental : public IClaServiceSil
{
public:
  /*! \brief Gets the bus message service interface.
   *  \returns The bus message service. */
  virtual NDistSimIB::IBusMessageService* GetBusMessageService() = 0;

  /*! \brief Gets the timing service interface.
   *  \returns The bus message service. */
  virtual NDistSimIB::ITimingService* GetTimingService() = 0;

  /*! \brief Checks if the connection is established.
   *  \returns Whether or not the connection is established. */
  virtual bool IsConnectionEstablished() = 0;
};

} // namespace Experimental
} // namespace Cla
} // namespace CANoe
} // namespace Vector
