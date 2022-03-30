// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DoConsumedEvent.hpp"

#include "ValueEntity.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

// -------------------
// DoConsumedEvent
// -------------------

DoConsumedEvent::DoConsumedEvent(std::string fullPath, std::shared_ptr<spdlog::logger> logger) :
  DoEvent(std::move(fullPath), MemberDirection::Consumed, std::move(logger))
{
}

DoConsumedEvent::DoConsumedEvent(std::string fullPath,
  std::shared_ptr<ValueEntity> valueEntity,
  std::shared_ptr<spdlog::logger> logger) :
  DoEvent(std::move(fullPath), MemberDirection::Consumed, std::move(valueEntity), std::move(logger))
{
}

// -------------------
// DoMember
// -------------------

void DoConsumedEvent::Connect(NDistSimIB::IDOService* service)
{
  auto callback = [this](const std::vector<uint8_t>& data, const int64_t&) -> uint32_t {
    Sab::Deserializer packageDeserializer(data);
    packageDeserializer.Deserialize<Sab::EventMemberUpdateHeader>();
    auto payload = packageDeserializer.GetRemainingBuffer();
    mValue->SetData(std::move(payload));
    return 0;
  };

  mDistSimService = service;
  mDistSimHandle = mDistSimService->RegisterDOMemberTarget(GetConnectionId(), std::move(callback));
  mLogger->debug("Member connected: {}", GetFullPath());
}
void DoConsumedEvent::Disconnect()
{

  mDistSimService->UnregisterDOMemberTarget(mDistSimHandle);
  mDistSimService = nullptr;
  mLogger->debug("Member disconnected: {}", GetFullPath());
}

// -------------------
// IDoMember
// -------------------

MemberType DoConsumedEvent::GetType() const { return DoMember::GetType(); }
MemberDirection DoConsumedEvent::GetDirection() const { return DoMember::GetDirection(); }

// -------------------
// IDoData
// -------------------

std::shared_ptr<IValueEntity> DoConsumedEvent::GetValue() const { return DoEvent::GetValue(); }

} // namespace Cla
} // namespace CANoe
} // namespace Vector
