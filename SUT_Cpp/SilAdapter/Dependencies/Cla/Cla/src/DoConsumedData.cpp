// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DoConsumedData.hpp"

#include "ValueEntity.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

// -------------------
// DoConsumedData
// -------------------

DoConsumedData::DoConsumedData(std::string fullPath, std::shared_ptr<spdlog::logger> logger) :
  DoData(std::move(fullPath), MemberDirection::Consumed, std::move(logger))
{
}

// -------------------
// DoMember
// -------------------

void DoConsumedData::Connect(NDistSimIB::IDOService* service)
{
  auto callback = [this](const std::vector<uint8_t>& data, const int64_t&) -> uint32_t {
    Sab::Deserializer packageDeserializer(data);
    packageDeserializer.Deserialize<Sab::DataMemberUpdateHeader>();
    auto payload = packageDeserializer.GetRemainingBuffer();
    mValue->SetData(std::move(payload));
    return 0;
  };

  mDistSimService = service;
  mDistSimHandle = mDistSimService->RegisterDOMemberTarget(GetConnectionId(), callback);
  mLogger->debug("Member connected: {}", GetFullPath());
}
void DoConsumedData::Disconnect()
{
  mDistSimService->UnregisterDOMemberTarget(mDistSimHandle);
  mDistSimService = nullptr;
  mLogger->debug("Member disconnected: {}", GetFullPath());
}

// -------------------
// IDoMember
// -------------------

MemberType DoConsumedData::GetType() const { return DoMember::GetType(); }
MemberDirection DoConsumedData::GetDirection() const { return DoMember::GetDirection(); }

// -------------------
// IDoData
// -------------------

std::shared_ptr<IValueEntity> DoConsumedData::GetValue() const { return DoData::GetValue(); }

} // namespace Cla
} // namespace CANoe
} // namespace Vector
