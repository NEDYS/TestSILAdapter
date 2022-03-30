// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <cstdlib> //getenv

#include "spdlog/sinks/stdout_sinks.h"

#include "ServiceLocator.hpp"

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

ServiceLocator& ServiceLocator::Get()
{
  static ServiceLocator locator;
  return locator;
}

std::shared_ptr<spdlog::logger> ServiceLocator::GetLogger() const { return mLogger; }

void ServiceLocator::EnableExperimentalMode()
{
  if (mDistSim)
  {
    mLogger->warn("Experimental mode cannot be changed while the DistSim-Adapter is running.");
    return;
  }

  mExperimentalModeEnabled = true;
}

NDistSimIB::IDOService* ServiceLocator::RegisterForDoService()
{
  TryInit();

  if (!mDoService)
  {
    mDoService = mDistSim->GetDOService();
  }

  return mDoService;
}

void ServiceLocator::UnregisterForDoService()
{
  if (mDoService)
  {
    mDoService = nullptr;
    TryShutDown();
  }
}

NDistSimIB::IBusMessageService* ServiceLocator::RegisterForBusMessageService()
{
  if (!mExperimentalModeEnabled)
  {
    return nullptr;
  }

  TryInit();

  if (!mBusMessageService)
  {
    mBusMessageService = mDistSim->GetBusMessageService();
  }

  return mBusMessageService;
}

void ServiceLocator::UnregisterForBusMessageService()
{
  if (mBusMessageService)
  {
    mBusMessageService = nullptr;
    TryShutDown();
  }
}

NDistSimIB::ITimingService* ServiceLocator::RegisterForTimingService()
{
  if (!mExperimentalModeEnabled)
  {
    return nullptr;
  }

  TryInit();

  if (!mTimingService)
  {
    mTimingService = mDistSim->GetTimingService();
  }

  return mTimingService;
}

void ServiceLocator::UnregisterForTimingService()
{
  if (mTimingService)
  {
    mTimingService = nullptr;
    TryShutDown();
  }
}

// Private Methods
// ------------------

ServiceLocator::ServiceLocator()
{
  const auto sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
  mLogger = std::make_shared<spdlog::logger>("ClaLogger", sink);

#ifdef CLA_ENABLE_LOGGING
  mLogger->set_level(spdlog::level::trace);
#else
  mLogger->set_level(spdlog::level::off);
#endif
}

void ServiceLocator::TryInit()
{
  if (mDistSim)
  {
    return;
  }

  NDistSimIB::IBConfig config;
  OptionallySetFromEnvVar(config.RtinodeAddress, "VSIL_CONNECTION_ADDRESS");
  OptionallySetFromEnvVar(config.FederateName, "VSIL_INSTANCE_NAME");
  OptionallySetFromEnvVar(config.FederationNamePrefix, "VSIL_SIMULATION_NAME");

  mDistSim = ClaDi::ServiceFactory::CreateService<NDistSimIB::IDistSimIB>(config);

  mDistSim->RegisterService(NDistSimIB::Service::DistributedObjects);

  if (mExperimentalModeEnabled)
  {
    mDistSim->RegisterService(NDistSimIB::Service::BusMessages);
    mDistSim->RegisterService(NDistSimIB::Service::Timing);
  }

  mDistSim->Start();
  mLogger->debug("Initialized service locator.");
}

void ServiceLocator::TryShutDown()
{
  if (mDistSim == nullptr || mDoService != nullptr || mBusMessageService != nullptr || mTimingService != nullptr)
  {
    // As long as one service is still registered, the DistSim adapter will not be stopped.
    return;
  }

  mDistSim->Stop();

  mDistSim->UnregisterService(NDistSimIB::Service::DistributedObjects);

  if (mExperimentalModeEnabled)
  {
    mDistSim->UnregisterService(NDistSimIB::Service::BusMessages);
    mDistSim->UnregisterService(NDistSimIB::Service::Timing);
  }

  mExperimentalModeEnabled = false;
  mDistSim.reset();
  mLogger->debug("Shut down service locator.");
}

} // namespace Cla
} // namespace CANoe
} // namespace Vector
