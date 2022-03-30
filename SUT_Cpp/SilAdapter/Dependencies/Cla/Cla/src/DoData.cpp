// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DoData.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

DoData::DoData(std::string fullPath, MemberDirection direction, std::shared_ptr<spdlog::logger> logger) :
  DoMember(std::move(fullPath), MemberType::DataMember, direction, std::move(logger)),
  mValue(std::make_shared<ValueEntity>(mLogger))
{
}

std::string DoData::GetConnectionId() const { return GetVirtualNetworkName() + "#" + GetFullPath() + "_Value"; }

// -------------------
// IDoData
// -------------------

std::shared_ptr<IValueEntity> DoData::GetValue() const { return mValue; }

} // namespace Cla
} // namespace CANoe
} // namespace Vector
