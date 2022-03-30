// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DoProvidedData.hpp"

#include "DistSimIBHelper.hpp"
#include "ValueEntity.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

DoProvidedData::DoProvidedData(std::string fullPath, TxTrigger txTrigger, std::shared_ptr<spdlog::logger> logger) :
  DoData(std::move(fullPath), MemberDirection::Provided, std::move(logger)),
  mTxTrigger(txTrigger)
{
}

// -------------------
// DoMember
// -------------------

void DoProvidedData::Connect(NDistSimIB::IDOService* service)
{
  mDistSimService = service;
  mDistSimHandle = mDistSimService->RegisterDOMemberSource(GetConnectionId());

  const auto updateMode = mTxTrigger == TxTrigger::OnChange ? ValueUpdateMode::OnChange : ValueUpdateMode::OnUpdate;
  mOnTransmitHandle = mValue->RegisterCallback(
    [this](IValueEntity&) {
      Sab::Serializer packageSerializer;
      packageSerializer.Serialize(Sab::DataMemberUpdateHeader());
      packageSerializer.AppendBuffer(mValue->CopyData());

      const auto result = mDistSimService->TransmitDOMember(mDistSimHandle, packageSerializer.ReleaseBuffer());
      switch (result)
      {
      case NDistSimIB::IDOService::TransmitResult::NotConnected:
      case NDistSimIB::IDOService::TransmitResult::Queued:
      case NDistSimIB::IDOService::TransmitResult::TransmitError:
      case NDistSimIB::IDOService::TransmitResult::NotRegistered:
        mLogger->debug("Couldn't transmit data for '{}': {}.", GetFullPath(), to_string(result));
        return;
      case NDistSimIB::IDOService::TransmitResult::OK:
        return;
      }
      throw std::runtime_error("Unknown transmit result.");
    },
    updateMode);

  mLogger->debug("Member connected: {}", GetFullPath());
}
void DoProvidedData::Disconnect()
{
  mValue->UnregisterCallback(mOnTransmitHandle);
  mDistSimService->UnregisterDOMemberSource(mDistSimHandle);
  mDistSimService = nullptr;
  mLogger->debug("Member disconnected: {}", GetFullPath());
}

// -------------------
// IDoMember
// -------------------

MemberType DoProvidedData::GetType() const { return DoMember::GetType(); }
MemberDirection DoProvidedData::GetDirection() const { return DoMember::GetDirection(); }

// -------------------
// IDoData
// -------------------

std::shared_ptr<IValueEntity> DoProvidedData::GetValue() const { return DoData::GetValue(); }

} // namespace Cla
} // namespace CANoe
} // namespace Vector
