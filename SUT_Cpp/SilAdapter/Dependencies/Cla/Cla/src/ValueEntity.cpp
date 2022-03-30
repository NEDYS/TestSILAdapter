// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <stdexcept>

#include "Deserializer.hpp"
#include "Serializer.hpp"
#include "ValueEntity.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

// -------------------
// ValueEntity
// -------------------

ValueEntity::ValueEntity(std::shared_ptr<spdlog::logger> logger) :
  mLogger(std::move(logger)),
  mSerializer(new Serializer([this](std::vector<uint8_t>& buffer) { SetData(std::move(buffer)); })),
  mDeserializer(new Deserializer([this]() { return CopyData(); })),
  mNextCallbackHandle(1)
{
}

void ValueEntity::SetData(std::vector<uint8_t> newData)
{
  std::unique_lock<std::mutex> lock(mDataMutex);

  auto changed = false;
  if (newData != mData)
  {
    mData = std::move(newData);
    changed = true;
  }

  lock.unlock();

  if (changed)
  {
    OnChange();
  }
  OnUpdate();
}

std::vector<uint8_t> ValueEntity::CopyData() const
{
  std::lock_guard<std::mutex> lock(mDataMutex);
  return mData;
}

// -------------------
// IValueEntity
// -------------------

ISerializer* ValueEntity::GetSerializer() const { return mSerializer.get(); }
IDeserializer* ValueEntity::GetDeserializer() const { return mDeserializer.get(); }

bool ValueEntity::IsValid() const { return true; }

ValueState ValueEntity::GetState() const { return ValueState::Measured; }

Time ValueEntity::GetLastChangeTime() const { return mLastChangeTime; }

Time ValueEntity::GetLastUpdateTime() const { return mLastUpdateTime; }

CallbackHandle ValueEntity::RegisterCallback(std::function<void(IValueEntity&)> callback, ValueUpdateMode mode)
{
  if (!callback)
  {
    mLogger->debug("Try to register and empty callback on the value entity.");
    return 0;
  }

  const auto handle = CreateUniqueCallbackHandle();
  std::lock_guard<std::mutex> lock(mCallbackMutex);
  switch (mode)
  {
  case ValueUpdateMode::OnChange:
    mOnChangeCallbacks.emplace_back(std::make_shared<Callback>(handle, std::move(callback)));
    return handle;
  case ValueUpdateMode::OnUpdate:
    mOnUpdateCallbacks.emplace_back(std::make_shared<Callback>(handle, std::move(callback)));
    return handle;
  }
  throw std::runtime_error("Invalid ValueUpdateMode.");
}

void ValueEntity::UnregisterCallback(CallbackHandle handle)
{
  std::lock_guard<std::mutex> lock(mCallbackMutex);
  const auto hasHandle = [=](const std::shared_ptr<Callback>& callback) { return callback->handle == handle; };
  const auto onUpdateIt = std::remove_if(mOnUpdateCallbacks.begin(), mOnUpdateCallbacks.end(), hasHandle);
  mOnUpdateCallbacks.erase(onUpdateIt, mOnUpdateCallbacks.end());
  const auto onChangeIt = std::remove_if(mOnChangeCallbacks.begin(), mOnChangeCallbacks.end(), hasHandle);
  mOnChangeCallbacks.erase(onChangeIt, mOnChangeCallbacks.end());
}

CallbackHandle ValueEntity::CreateUniqueCallbackHandle() { return mNextCallbackHandle++; }

void ValueEntity::OnChange()
{
  std::unique_lock<std::mutex> lock(mCallbackMutex);
  const auto onChangeCallbacks = mOnChangeCallbacks;
  lock.unlock();

  for (auto&& callback : onChangeCallbacks)
  {
    callback->function(*this);
  }
}

void ValueEntity::OnUpdate()
{
  std::unique_lock<std::mutex> lock(mCallbackMutex);
  const auto onUpdateCallbacks = mOnUpdateCallbacks;
  lock.unlock();

  for (auto&& callback : onUpdateCallbacks)
  {
    callback->function(*this);
  }
}

} // namespace Cla
} // namespace CANoe
} // namespace Vector
