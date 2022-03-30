// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DoProvidedField.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

// -------------------
// DoProvidedField
// -------------------

DoProvidedField::DoProvidedField(std::string fullPath, TxTrigger txTrigger, std::shared_ptr<spdlog::logger> logger) :
  DoField(std::move(fullPath), MemberDirection::Provided, std::move(logger)),
  mTxTrigger(txTrigger)
{
}

// -------------------
// DoMember
// -------------------

void DoProvidedField::Connect(NDistSimIB::IDOService* service)
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
void DoProvidedField::Disconnect()
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

MemberType DoProvidedField::GetType() const { return DoMember::GetType(); }
MemberDirection DoProvidedField::GetDirection() const { return DoMember::GetDirection(); }

// -------------------
// IDoField
// -------------------

std::shared_ptr<IValueEntity> DoProvidedField::GetValue() const { return DoField::GetValue(); }
std::shared_ptr<IDoMethod> DoProvidedField::GetGetter() { return GetProvidedGetter(); }
std::shared_ptr<IDoMethod> DoProvidedField::GetSetter() { return GetProvidedSetter(); }
std::shared_ptr<IDoEvent> DoProvidedField::GetEvent() { return GetProvidedEvent(); }

// -------------------
// IDoConsumedField
// -------------------

std::shared_ptr<IDoProvidedMethod> DoProvidedField::GetProvidedGetter() const
{
  if (mDistSimService)
  {
    throw std::runtime_error("Cannot get new getter while the provided field member is connected.");
  }

  if (!mGetter)
  {
    mGetter = std::make_shared<Getter>(GetGetterPath(), mValue, mLogger);
  }

  return mGetter;
}
std::shared_ptr<IDoProvidedMethod> DoProvidedField::GetProvidedSetter() const
{
  if (mDistSimService)
  {
    throw std::runtime_error("Cannot get new setter while the provided field member is connected");
  }

  if (!mSetter)
  {
    mSetter = std::make_shared<Setter>(GetSetterPath(), mValue, mLogger);
  }

  return mSetter;
}
std::shared_ptr<IDoProvidedEvent> DoProvidedField::GetProvidedEvent() const
{
  if (mDistSimService)
  {
    throw std::runtime_error("Cannot get new event while the provided field member is connected.");
  }

  if (!mEvent)
  {
    mEvent = std::make_shared<DoProvidedEvent>(GetEventPath(), mTxTrigger, mValue, mLogger);
  }

  return mEvent;
}

// ===== DoProvidedField::Getter =====

DoProvidedField::Getter::Getter(std::string fullPath,
  std::shared_ptr<ValueEntity> valueEntity,
  std::shared_ptr<spdlog::logger> logger) :
  DoProvidedMethod(std::move(fullPath), std::move(logger)),
  mValue(std::move(valueEntity))
{
  SetDefaultCallback();
}

void DoProvidedField::Getter::SetCallback(std::function<void(IProviderCallContext&)> callback)
{
  if (callback)
  {
    DoProvidedMethod::SetCallback(callback);
  }
  else
  {
    SetDefaultCallback();
  }
}

void DoProvidedField::Getter::SetDefaultCallback()
{
  std::lock_guard<std::mutex> lock(mCallbackMutex);
  mCallback = [this](ProviderCallContext& cc) { cc.SetOutputParametersData(mValue->CopyData()); };
}

// ===== DoProvidedField::Setter =====

DoProvidedField::Setter::Setter(std::string fullPath,
  std::shared_ptr<ValueEntity> valueEntity,
  std::shared_ptr<spdlog::logger> logger) :
  DoProvidedMethod(std::move(fullPath), std::move(logger)),
  mValue(std::move(valueEntity))
{
  SetDefaultCallback();
}

void DoProvidedField::Setter::SetCallback(std::function<void(IProviderCallContext&)> callback)
{
  if (callback)
  {
    DoProvidedMethod::SetCallback(callback);
  }
  else
  {
    SetDefaultCallback();
  }
}

void DoProvidedField::Setter::SetDefaultCallback()
{
  std::lock_guard<std::mutex> lock(mCallbackMutex);
  mCallback = [this](ProviderCallContext& cc) {
    auto data = cc.GetInputParametersData();
    mValue->SetData(data);
    cc.SetOutputParametersData(std::move(data));
  };
}

} // namespace Cla
} // namespace CANoe
} // namespace Vector
