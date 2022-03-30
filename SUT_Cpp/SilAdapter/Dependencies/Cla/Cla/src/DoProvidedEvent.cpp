// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DoProvidedEvent.hpp"

#include "DistSimIBHelper.hpp"
#include "ValueEntity.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

DoProvidedEvent::DoProvidedEvent(std::string fullPath, TxTrigger txTrigger, std::shared_ptr<spdlog::logger> logger) :
  DoEvent(std::move(fullPath), MemberDirection::Provided, std::move(logger)),
  mTxTrigger(txTrigger)
{
}

DoProvidedEvent::DoProvidedEvent(std::string fullPath,
  TxTrigger txTrigger,
  std::shared_ptr<ValueEntity> valueEntity,
  std::shared_ptr<spdlog::logger> logger) :
  DoEvent(std::move(fullPath), MemberDirection::Provided, std::move(valueEntity), std::move(logger)),
  mTxTrigger(txTrigger)
{
}

// -------------------
// DoMember
// -------------------

void DoProvidedEvent::Connect(NDistSimIB::IDOService* service)
{
  mDistSimService = service;
  mDistSimHandle = mDistSimService->RegisterDOMemberSource(GetConnectionId());

  const auto updateMode = mTxTrigger == TxTrigger::OnChange ? ValueUpdateMode::OnChange : ValueUpdateMode::OnUpdate;
  mOnTransmitHandle = mValue->RegisterCallback(
    [this](IValueEntity&) {
      Sab::Serializer packageSerializer;
      packageSerializer.Serialize(Sab::EventMemberUpdateHeader());
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
void DoProvidedEvent::Disconnect()
{
  mValue->UnregisterCallback(mOnTransmitHandle);
  mDistSimService->UnregisterDOMemberSource(mDistSimHandle);
  mDistSimService = nullptr;
  mLogger->debug("Member disconnected: {}", GetFullPath());
}

// -------------------
// IDoMember
// -------------------

MemberType DoProvidedEvent::GetType() const { return DoMember::GetType(); }
MemberDirection DoProvidedEvent::GetDirection() const { return DoMember::GetDirection(); }

// -------------------
// IDoEvent
// -------------------

std::shared_ptr<IValueEntity> DoProvidedEvent::GetValue() const { return DoEvent::GetValue(); }

} // namespace Cla
} // namespace CANoe
} // namespace Vector
