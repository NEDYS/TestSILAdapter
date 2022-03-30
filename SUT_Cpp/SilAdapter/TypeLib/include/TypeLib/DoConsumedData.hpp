// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stdexcept>

#include "DoData.hpp"

#include "cla/IDoConsumedData.hpp"
#include "cla/IValueEntity.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

/*! \brief Class representing a consumed data member of a distributed object.
 *  \tparam T The type of the data managed by the data member. */
template <class T>
class DoConsumedData : public DoData<T>
{
public:
  // Public Members
  // -------------------

  /*! \brief Instantiates a new uninitialized consumed data member. */
  DoConsumedData() = default;

  /*!
   * \brief The data member blocks on SIL Adapter initialization
   *        until the first value from CANoe is received.
   * \param timeout When a timeout for the waiting shall appear.
   */
  void BlockAtInit(std::chrono::nanoseconds timeout);

protected:
  // Protected Methods
  // -------------------

  friend class Accessor; // To initialize / reset
  void _Init(Cla::IDo* distributedObject, const std::string& path);
  void _Reset() override;

private:
  // Private Methods
  // -------------------

  Cla::IValueEntity* _GetValueEntity() const override;
  void _WaitForFirstTransmission();

  // Private Members
  // -------------------

  std::shared_ptr<Cla::IDoConsumedData> mData = nullptr;

  // For blocking if "BlockAtInit" is enabled
  std::chrono::nanoseconds mWaitForFirstTransmissionTimeout;
  bool mWaitForFirstTransmission = false;
  std::condition_variable mConditionVariable;
  std::mutex mMutex;
  CallbackHandle mCallbackHandle{0};
};

// ===== Inline Implementation =====

// Public methods

template <class T>
void DoConsumedData<T>::BlockAtInit(std::chrono::nanoseconds timeout)
{
  mWaitForFirstTransmissionTimeout = timeout;
  mWaitForFirstTransmission = true;
}

// Protected Members
// -------------------

template <class T>
void DoConsumedData<T>::_Init(Cla::IDo* distributedObject, const std::string& path)
{
  DoData<T>::_Init(path);
  mData = distributedObject->GetConsumedData(path);

  DoData<T>::_SetInitialValue(T{});

  if (mWaitForFirstTransmission)
  {
    mCallbackHandle = DoData<T>::RegisterOnUpdateHandler([this]() {
      if (DoData<T>::mSettingInitialValue)
      {
        // Exclude initial values
        return;
      }

      std::unique_lock<std::mutex> lock(mMutex);
      mWaitForFirstTransmission = false;
      lock.unlock();
      mConditionVariable.notify_all();
      DoData<T>::UnregisterHandler(mCallbackHandle);
    });
  }
}

template <class T>
void DoConsumedData<T>::_Reset()
{
  mData.reset();
  DoData<T>::_Reset();
}

// Private Methods
// -------------------

template <class T>
Cla::IValueEntity* DoConsumedData<T>::_GetValueEntity() const
{
  return mData->GetValue().get();
}

template <class T>
void DoConsumedData<T>::_WaitForFirstTransmission()
{
  std::unique_lock<std::mutex> lock(mMutex);
  if (!mConditionVariable.wait_for(lock, mWaitForFirstTransmissionTimeout, [this]() { return !mWaitForFirstTransmission; }))
  {
    throw std::runtime_error("Waiting for first value from CANoe: Timeout");
  }
}

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
