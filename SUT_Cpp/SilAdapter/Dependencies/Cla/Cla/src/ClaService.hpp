// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "spdlog/logger.h"

#include "cla/IClaServiceSil.hpp"

#include "DoRegistry.hpp"
#include "distSim/IDistSimIB.h"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Class representing the entry point for all CLA functionality. */
class ClaService : public IClaServiceSil
{
public:
  // Public Methods
  // ------------------

  /*! \brief Instantiates a new CLA service.
   *  \param logger The logger used for logging by the CLA service and all subsystems. */
  ClaService(std::shared_ptr<spdlog::logger> logger);

  // -------------------
  // IClaService
  // -------------------

  void Connect() override;
  void Disconnect() override;
  std::shared_ptr<IDo> GetDo(const std::string& path) override;

protected:
  // Protected Members
  // ------------------

  std::shared_ptr<spdlog::logger> mLogger;
  std::unique_ptr<NDistSimIB::IDistSimIB> mDistSim = nullptr;
  DoRegistry mDoRegistry;

  // Protected methods
  // ------------------

  /*! \brief Connects to CANoe with the DistSim adapter using the given configuration.
   *  \param config The configuration used to connect to CANoe. */
  void Connect(NDistSimIB::IBConfig config);

  /*! \brief Hook method to register the services of the DistSim adapter before the connection is established. */
  virtual void RegisterDistSimServices();

  /*! \brief Hook method to unregister services of the DistSim adapter after the connection was closed. */
  virtual void UnregisterDistSimServices();
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
