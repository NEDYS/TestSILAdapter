// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ClaServiceExperimental.hpp"

#include "spdlog/sinks/stdout_sinks.h"

namespace Vector {
namespace CANoe {
namespace Cla {
namespace Experimental {

// Public methods
// ------------------

// ------------------------
// ClaServiceExperimental
// ------------------------

ClaServiceExperimental::ClaServiceExperimental(std::shared_ptr<spdlog::logger> logger) : ClaService(std::move(logger))
{
}

// ------------------------
// IClaService
// ------------------------

void ClaServiceExperimental::Connect()
{
  ClaService::Connect();
  mBusMessageService = mDistSim->GetBusMessageService();
  mTimingService = mDistSim->GetTimingService();
}
void ClaServiceExperimental::Disconnect()
{
  ClaService::Disconnect();
  mTimingService = nullptr;
  mBusMessageService = nullptr;
}
std::shared_ptr<IDo> ClaServiceExperimental::GetDo(const std::string& path) { return ClaService::GetDo(path); }

// ------------------------
// IClaServiceExperimental
// ------------------------

bool ClaServiceExperimental::IsConnectionEstablished() { return mDistSim && mDistSim->IsConnected(); }
NDistSimIB::IBusMessageService* ClaServiceExperimental::GetBusMessageService() { return mBusMessageService; }
NDistSimIB::ITimingService* ClaServiceExperimental::GetTimingService() { return mTimingService; }

// Protected methods
// ------------------

void ClaServiceExperimental::RegisterDistSimServices()
{
  ClaService::RegisterDistSimServices();
  mDistSim->RegisterService(NDistSimIB::Service::BusMessages);
  mDistSim->RegisterService(NDistSimIB::Service::Timing);
}
void ClaServiceExperimental::UnregisterDistSimServices()
{
  mDistSim->UnregisterService(NDistSimIB::Service::Timing);
  mDistSim->UnregisterService(NDistSimIB::Service::BusMessages);
  ClaService::UnregisterDistSimServices();
}

} // namespace Experimental
} // namespace Cla
} // namespace CANoe
} // namespace Vector
