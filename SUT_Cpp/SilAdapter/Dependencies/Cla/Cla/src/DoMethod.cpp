// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DoMethod.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

// -------------------
// DoMethod
// -------------------

DoMethod::DoMethod(std::string fullPath, MemberDirection direction, std::shared_ptr<spdlog::logger> logger) :
  DoMember(std::move(fullPath), MemberType::Method, direction, std::move(logger))
{
}

// -------------------
// DoMember
// -------------------

void DoMethod::Connect(NDistSimIB::IDOService* service)
{
  std::string connectionIdSender;
  std::string connectionIdReceiver;

  if (DoMember::GetDirection() == MemberDirection::Consumed)
  {
    connectionIdSender = GetConnectionIdRequest();
    connectionIdReceiver = GetConnectionIdResponse();
  }
  else
  {
    connectionIdSender = GetConnectionIdResponse();
    connectionIdReceiver = GetConnectionIdRequest();
  }

  auto cb = [this](const std::vector<uint8_t>& data, const int64_t& timestamp) {
    return OnReceiveData(data, timestamp);
  };

  mDistSimService = service;
  mDistSimHandleReceiver = mDistSimService->RegisterDOMemberTarget(connectionIdReceiver, std::move(cb));
  mDistSimHandleSender = mDistSimService->RegisterDOMemberSource(connectionIdSender);
  mLogger->debug("Member connected: {}", GetFullPath());
}
void DoMethod::Disconnect()
{
  mDistSimService->UnregisterDOMemberTarget(mDistSimHandleReceiver);
  mDistSimService->UnregisterDOMemberSource(mDistSimHandleSender);
  mDistSimService = nullptr;
  mLogger->debug("Member disconnected: {}", GetFullPath());
}

// Protected methods
// -------------------

NDistSimIB::IDOService::TransmitResult DoMethod::SendData(const std::vector<uint8_t>& data) const
{
  if (!mDistSimService)
  {
    return NDistSimIB::IDOService::TransmitResult::NotConnected;
  }

  return mDistSimService->TransmitDOMember(mDistSimHandleSender, data);
}

// Private methods
// -------------------

std::string DoMethod::GetConnectionIdRequest() const { return GetVirtualNetworkName() + "#" + GetFullPath() + "_Call"; }
std::string DoMethod::GetConnectionIdResponse() const
{
  return GetVirtualNetworkName() + "#" + GetFullPath() + "_CallReturn";
}

} // namespace Cla
} // namespace CANoe
} // namespace Vector
