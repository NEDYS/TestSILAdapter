// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <cstdlib> //getenv

#include "ClaService.hpp"

#include "ClaDi/ServiceFactory.hpp"

namespace {

void OptionallySetFromEnvVar(std::string& target, const char* varName)
{
  auto* varValue = ::getenv(varName);
  if (varValue)
  {
    target.assign(varValue);
  }
}

} // anonymous namespace

namespace Vector {
namespace CANoe {
namespace Cla {

// Public Methods
// ------------------

ClaService::ClaService(std::shared_ptr<spdlog::logger> logger) : mLogger(std::move(logger)), mDoRegistry(mLogger)
{
#ifdef CLA_ENABLE_LOGGING
  mLogger->set_level(spdlog::level::trace);
#else
  mLogger->set_level(spdlog::level::off);
#endif
}

void ClaService::Connect() { Connect(NDistSimIB::IBConfig()); }
void ClaService::Disconnect()
{
  assert(mDistSim && "CLA service is not connected!");
  mDoRegistry.Disconnect();
  mDistSim->Stop();

  UnregisterDistSimServices();

  mDistSim.reset();
  mLogger->debug("CLA service disconnected.");
}
std::shared_ptr<IDo> ClaService::GetDo(const std::string& path) { return mDoRegistry.GetDo(path); }

// Protected methods
// ------------------

void ClaService::Connect(NDistSimIB::IBConfig config)
{
  assert(!mDistSim && "CLA service is already connected!");
  OptionallySetFromEnvVar(config.RtinodeAddress, "VSIL_CONNECTION_ADDRESS");
  OptionallySetFromEnvVar(config.FederateName, "VSIL_INSTANCE_NAME");
  OptionallySetFromEnvVar(config.FederationNamePrefix, "VSIL_SIMULATION_NAME");

  mDistSim = ClaDi::ServiceFactory::CreateService<NDistSimIB::IDistSimIB>(config);

  RegisterDistSimServices();

  mDistSim->Start();
  mDoRegistry.Connect(mDistSim->GetDOService());

  mLogger->debug("CLA service connected.");
}
void ClaService::RegisterDistSimServices() { mDistSim->RegisterService(NDistSimIB::Service::DistributedObjects); }
void ClaService::UnregisterDistSimServices() { mDistSim->UnregisterService(NDistSimIB::Service::DistributedObjects); }

} // namespace Cla
} // namespace CANoe
} // namespace Vector
