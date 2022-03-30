// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DoField.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

// -------------------
// DoField
// -------------------

DoField::DoField(std::string fullPath, MemberDirection direction, std::shared_ptr<spdlog::logger> logger) :
  DoMember(std::move(fullPath), MemberType::Field, direction, std::move(logger)),
  mValue(std::make_shared<ValueEntity>(mLogger))
{
}

// -------------------
// IDoField
// -------------------

std::shared_ptr<IValueEntity> DoField::GetValue() const { return mValue; }

// Protected Methods
// -------------------

std::string DoField::GetGetterPath() const { return GetFullPath() + ".Get"; }
std::string DoField::GetSetterPath() const { return GetFullPath() + ".Set"; }
std::string DoField::GetEventPath() const { return GetFullPath() + ".Notification"; }

} // namespace Cla
} // namespace CANoe
} // namespace Vector
