// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ClaService.hpp"
#include "cla/ClaExperimental.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {
namespace Experimental {

/*! \brief Implementation of the experimental CLA service which uses the DistSim
 *         bus and timing services. */
class ClaServiceExperimental : public IClaServiceExperimental, public ClaService
{
public:
  // Public methods
  // ------------------

  /*! \brief Instantiates a new CLA service for the experimental features.
   *  \param logger The logger which shall be used by the CLA service and all its subsystems. */
  ClaServiceExperimental(std::shared_ptr<spdlog::logger> logger);

  // -------------------------
  // IClaService
  // -------------------------

  void Connect() override;
  void Disconnect() override;
  std::shared_ptr<IDo> GetDo(const std::string& path) override;

  // -------------------------
  // IClaServiceExperimental
  // -------------------------

  bool IsConnectionEstablished() override;
  NDistSimIB::IBusMessageService* GetBusMessageService() override;
  NDistSimIB::ITimingService* GetTimingService() override;

protected:
  // Protected methods
  // ------------------

  void RegisterDistSimServices() override;
  void UnregisterDistSimServices() override;

private:
  // Private members
  // ------------------

  NDistSimIB::IBusMessageService* mBusMessageService{nullptr};
  NDistSimIB::ITimingService* mTimingService{nullptr};
};

} // namespace Experimental
} // namespace Cla
} // namespace CANoe
} // namespace Vector