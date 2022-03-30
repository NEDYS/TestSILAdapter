// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "DoEvent.hpp"

#include "cla/IDoProvidedEvent.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

/*! \brief Class describing a consumed DO event member. */
template <class T>
class DoConsumedEvent final : public DoEvent<T>
{
public:
  // Public Methods
  // -------------------

  DoConsumedEvent() = default;

   /*!
   * \brief The event member blocks on SIL Adapter initialization
   *        until the first transmission from CANoe is received.
   * \param timeout When a timeout for the waiting shall appear.
   */
  void BlockAtInit(std::chrono::nanoseconds timeout);

protected:
  // Protected Methods
  // -------------------

  friend class Accessor; // To initialize / reset
  void _Init(Cla::IDo* distributedObject, const std::string& path);
  void _Reset() override;
  Cla::IValueEntity* _GetValueEntity() const override;
  void _WaitForFirstTransmission();

private:
  // Private members
  // -------------------

  std::shared_ptr<Cla::IDoConsumedEvent> mEvent = nullptr;

  // For blocking if "BlockAtInit" is enabled
  std::chrono::nanoseconds mWaitForFirstTransmissionTimeout;
  bool mWaitForFirstTransmission = false;
  std::condition_variable mConditionVariable;
  std::mutex mMutex;
  CallbackHandle mCallbackHandle{0};
};

/*! \brief Class describing a consumed DO event member. */
template <>
class DoConsumedEvent<void> final : public DoEvent<void>
{
public:
  // Public Methods
  // -------------------

  DoConsumedEvent() = default;

   /*!
   * \brief The event member blocks on SIL Adapter initialization
   *        until the first transmission from CANoe is received.
   * \param timeout When a timeout for the waiting shall appear.
   */
  void BlockAtInit(std::chrono::nanoseconds timeout);

protected:
  // Protected Methods
  // -------------------

  friend class Accessor; // To initialize / reset
  void _Init(Cla::IDo* distributedObject, const std::string& path);
  void _Reset() override;
  Cla::IValueEntity* _GetValueEntity() const override;
  void _WaitForFirstTransmission();

private:
  // Private members
  // -------------------

  std::shared_ptr<Cla::IDoConsumedEvent> mEvent = nullptr;

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
void DoConsumedEvent<T>::BlockAtInit(std::chrono::nanoseconds timeout)
{
  mWaitForFirstTransmissionTimeout = timeout;
  mWaitForFirstTransmission = true;
}

inline void DoConsumedEvent<void>::BlockAtInit(std::chrono::nanoseconds timeout)
{
  mWaitForFirstTransmissionTimeout = timeout;
  mWaitForFirstTransmission = true;
}

// Protected Methods
// -------------------

template <class T>
void DoConsumedEvent<T>::_Init(Cla::IDo* distributedObject, const std::string& path)
{
  DoEvent<T>::_Init(path);
  mEvent = distributedObject->GetConsumedEvent(path);

  DoEvent<T>::_SetInitialValue(T{});

  if (mWaitForFirstTransmission)
  {
    mCallbackHandle = DoEvent<T>::RegisterOnUpdateHandler([this]() {
      if (DoEvent<T>::mSettingInitialValue)
      {
        // Exclude initial values
        return;
      }

      std::unique_lock<std::mutex> lock(mMutex);
      mWaitForFirstTransmission = false;
      lock.unlock();
      mConditionVariable.notify_all();
      DoEvent<T>::UnregisterHandler(mCallbackHandle);
    });
  }
}

template <class T>
void DoConsumedEvent<T>::_Reset()
{
  mEvent.reset();
  DoEvent<T>::_Reset();
}

template <class T>
Cla::IValueEntity* DoConsumedEvent<T>::_GetValueEntity() const
{
  return mEvent->GetValue().get();
}

template <class T>
void DoConsumedEvent<T>::_WaitForFirstTransmission()
{
  std::unique_lock<std::mutex> lock(mMutex);
  if (!mConditionVariable.wait_for(lock, mWaitForFirstTransmissionTimeout, [this]() { return !mWaitForFirstTransmission; }))
  {
    throw std::runtime_error("Waiting for first transmission from CANoe: Timeout");
  }
}

inline void DoConsumedEvent<void>::_Init(Cla::IDo* distributedObject, const std::string& path)
{
  DoEvent<void>::_Init(path);
  mEvent = distributedObject->GetConsumedEvent(path);

  if (mWaitForFirstTransmission)
  {
    mCallbackHandle = DoEvent<void>::RegisterOnUpdateHandler([this]() {
      std::unique_lock<std::mutex> lock(mMutex);
      mWaitForFirstTransmission = false;
      lock.unlock();
      mConditionVariable.notify_all();
      DoEvent<void>::UnregisterHandler(mCallbackHandle);
    });
  }
}

inline void DoConsumedEvent<void>::_Reset()
{
  mEvent.reset();
  DoEvent<void>::_Reset();
}

inline Cla::IValueEntity* DoConsumedEvent<void>::_GetValueEntity() const { return mEvent->GetValue().get(); }

inline void DoConsumedEvent<void>::_WaitForFirstTransmission()
{
  std::unique_lock<std::mutex> lock(mMutex);
  if (!mConditionVariable.wait_for(lock, mWaitForFirstTransmissionTimeout, [this]() { return !mWaitForFirstTransmission; }))
  {
    throw std::runtime_error("Waiting for first event trigger from CANoe: Timeout");
  }
}

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
