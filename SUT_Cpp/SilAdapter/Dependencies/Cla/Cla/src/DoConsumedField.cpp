// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DoConsumedField.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

// -------------------
// DoConsumedField
// -------------------

DoConsumedField::DoConsumedField(std::string fullPath, std::shared_ptr<spdlog::logger> logger) :
  DoField(std::move(fullPath), MemberDirection::Consumed, std::move(logger))
{
}

// -------------------
// DoMember
// -------------------

void DoConsumedField::Connect(NDistSimIB::IDOService* service)
{
  mDistSimService = service;
  if (mGetter)
  {
    mGetter->Connect(mDistSimService);
  }
  if (mSetter)
  {
    mSetter->Connect(mDistSimService);
  }
  if (mEvent)
  {
    mEvent->Connect(mDistSimService);
  }
}
void DoConsumedField::Disconnect()
{
  if (mEvent)
  {
    mEvent->Disconnect();
  }
  if (mSetter)
  {
    mSetter->Disconnect();
  }
  if (mGetter)
  {
    mGetter->Disconnect();
  }
  mDistSimService = nullptr;
}

// -------------------
// IDoMember
// -------------------

MemberType DoConsumedField::GetType() const { return DoMember::GetType(); }
MemberDirection DoConsumedField::GetDirection() const { return DoMember::GetDirection(); }

// -------------------
// IDoField
// -------------------

std::shared_ptr<IValueEntity> DoConsumedField::GetValue() const { return DoField::GetValue(); }
std::shared_ptr<IDoMethod> DoConsumedField::GetGetter() { return GetConsumedGetter(); }
std::shared_ptr<IDoMethod> DoConsumedField::GetSetter() { return GetConsumedSetter(); }
std::shared_ptr<IDoEvent> DoConsumedField::GetEvent() { return GetConsumedEvent(); }

// -------------------
// IDoConsumedField
// -------------------

std::shared_ptr<IDoConsumedMethod> DoConsumedField::GetConsumedGetter() const
{
  if (mDistSimService)
  {
    throw std::runtime_error("Cannot get new getter while the consumed field member is connected.");
  }

  if (!mGetter)
  {
    mGetter = std::make_shared<Getter>(GetGetterPath(), mValue, mLogger);
  }

  return mGetter;
}
std::shared_ptr<IDoConsumedMethod> DoConsumedField::GetConsumedSetter() const
{
  if (mDistSimService)
  {
    throw std::runtime_error("Cannot get new setter while the consumed field member is connected.");
  }

  if (!mSetter)
  {
    mSetter = std::make_shared<Setter>(GetSetterPath(), mValue, mLogger);
  }

  return mSetter;
}
std::shared_ptr<IDoConsumedEvent> DoConsumedField::GetConsumedEvent() const
{
  if (mDistSimService)
  {
    throw std::runtime_error("Cannot get new event while the consumed field member is connected.");
  }

  if (!mEvent)
  {
    mEvent = std::make_shared<DoConsumedEvent>(GetEventPath(), mValue, mLogger);
  }

  return mEvent;
}

// ===== DoConsumedField::Getter =====

DoConsumedField::Getter::Getter(std::string fullPath,
  std::shared_ptr<ValueEntity> valueEntity,
  std::shared_ptr<spdlog::logger> logger) :
  DoConsumedMethod(std::move(fullPath), std::move(logger)),
  mValue(std::move(valueEntity))
{
}

bool DoConsumedField::Getter::OnReceiveResponse(const Sab::MethodCallHeader& header,
  const std::vector<uint8_t>& payload)
{
  if (DoConsumedMethod::OnReceiveResponse(header, payload))
  {
    // If the field attribute 'setterHasReturn' is set to false, the return is empty
    if (!payload.empty())
    {
      mValue->SetData(payload);
    }

    return true;
  }

  return false;
}

} // namespace Cla
} // namespace CANoe
} // namespace Vector
