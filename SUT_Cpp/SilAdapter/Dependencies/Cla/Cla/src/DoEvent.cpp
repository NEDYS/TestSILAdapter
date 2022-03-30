// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DoEvent.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

DoEvent::DoEvent(std::string fullPath, MemberDirection direction, std::shared_ptr<spdlog::logger> logger) :
  DoMember(std::move(fullPath), MemberType::Event, direction, std::move(logger)),
  mValue(std::make_shared<ValueEntity>(mLogger))
{
}
DoEvent::DoEvent(std::string fullPath,
  MemberDirection direction,
  std::shared_ptr<ValueEntity> valueEntity,
  std::shared_ptr<spdlog::logger> logger) :
  DoMember(std::move(fullPath), MemberType::Event, direction, std::move(logger)),
  mValue(std::move(valueEntity))
{
}

std::string DoEvent::GetConnectionId() const { return GetVirtualNetworkName() + "#" + GetFullPath() + "_Value"; }

// -------------------
// IDoEvent
// -------------------

std::shared_ptr<IValueEntity> DoEvent::GetValue() const { return mValue; }

} // namespace Cla
} // namespace CANoe
} // namespace Vector
