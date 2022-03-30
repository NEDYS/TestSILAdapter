// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "spdlog/logger.h"

#include "DoRegistry.hpp"
#include "distSim/IDistSimIB.h"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Class providing the different services of the CLA. */
class ServiceLocator final
{
public:
  // Public Methods
  // ------------------

  /*! \brief Returns the singleton instance of the service locator.
   *  \returns the singleton instance of the service locator. */
  static ServiceLocator& Get();

  /*! \brief Returns the logger of the service locator.
   *  \returns The logger of the service locator. */
  std::shared_ptr<spdlog::logger> GetLogger() const;

  /*! \brief Enables the experimental mode while the DistSim-Adapter is not running. */
  void EnableExperimentalMode();

  /*! \brief Registers for the DO service. The DistSim-Adapter will be started if first service.
   *  \returns The DO service. */
  NDistSimIB::IDOService* RegisterForDoService();

  /*! \brief Unregisters for the DO service. The DistSim-Adapter will be stopped if it was the last active service. */
  void UnregisterForDoService();

  /*! \brief Registers for the bus message service. The DistSim-Adapter
   *         will be started if first service. Experimental mode only.
   *  \returns The bus message service. */
  NDistSimIB::IBusMessageService* RegisterForBusMessageService();

  /*! \brief Unregisters for the bus message service. The DistSim-Adapter will be stopped if it was the last active
   *         service. */
  void UnregisterForBusMessageService();

  /*! \brief Registers for the timing service. The DistSim-Adapter
   *         will be started if first service. Experimental mode only.
   *  \returns The timing service. */
  NDistSimIB::ITimingService* RegisterForTimingService();

  /*! \brief Unregisters for the timing service. The DistSim-Adapter will be stopped if it was the last active service.
   */
  void UnregisterForTimingService();

private:
  // Private Members
  // ------------------

  bool mExperimentalModeEnabled = false;
  std::shared_ptr<spdlog::logger> mLogger;

  std::unique_ptr<NDistSimIB::IDistSimIB> mDistSim = nullptr;
  NDistSimIB::IDOService* mDoService = nullptr;
  NDistSimIB::IBusMessageService* mBusMessageService = nullptr;
  NDistSimIB::ITimingService* mTimingService = nullptr;

  // Private Methods
  // ------------------

  /*! \brief Instantiates a new service locator and initializes / starts
   *         the DistSim-Adapter. */
  ServiceLocator();

  /*! \brief Tries to initialize the service locator and its DistSim-Adapter
   *         if it is not already started. */
  void TryInit();

  /*! \brief Tries to shut down the service locator and its DistSim-Adapter
   *         if no service is registered anymore. */
  void TryShutDown();
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
