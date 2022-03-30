// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cassert>
#include <future>

#include "Deserializer.hpp"
#include "DoField.hpp"
#include "ThreadContext.hpp"

#include "cla/IDoConsumedField.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

/*! \brief Class representing the getter method for a consumed field.
 *  \tparam T The type of the value the getter method fetches.
 */
template <class T>
class ConsumedGetter
{
public:
  // Public Methods
  // -------------------

  /*! \brief Calls the getter synchronously to fetch the current field value
   *         from the provider. The getter implementation can be customized
   *         by the provider.
   *  \returns The return value of the getter implementation of the providing side.
   *           Should be the current value of the field.
   *  \remarks Updates the local value of the field on return with the returned value.
   */
  T Call();

  /*! \brief Calls the getter synchronously to fetch the current field value
   *         from the provider. See Call() */
  T operator()();

  /*! \brief Calls the getter asynchronously to fetch the current field value
   *         from the provider. The getter implementation can be customized
   *         by the provider.
   *  \param handler The handler which is called when the call returns.
   *                 The given value is the return value of the getter implementation
   *                 of the providing side. Should be the current value of the field.
   *  \remarks Updates the local value of the field on return with the returned value. */
  void CallAsync(std::function<void(const T& value)> handler);

private:
  // Private Methods
  // -------------------

  friend class GetterField<T, MemberDirection::Consumed>;
  void _Init(std::shared_ptr<Cla::IDoConsumedMethod> getter);
  void _Reset();

  // Private Members
  // -------------------

  std::shared_ptr<Cla::IDoConsumedMethod> mGetter{nullptr};
};

/*! \brief Class which adds a getter to the inheriting class.
 *  \tparam T The type of the value the getter method fetches.
 */
template <class T>
class GetterField<T, MemberDirection::Consumed>
{
public:
  // Public Members
  // -------------------

  ConsumedGetter<T> Get; //!< The getter method of the consumed field

protected:
  // Protected Methods
  // -------------------

  void _Init(Cla::IDoConsumedField* field);
  void _Reset();
};

/*! \brief Base class for setters in a consumed field member. */
class ConsumedSetterBase
{
protected:
  // Protected Methods
  // -------------------

  template <class, MemberDirection, bool, bool>
  friend class SetterField;
  void _Init(std::shared_ptr<Cla::IDoConsumedMethod> setter);
  void _Reset();

  // Protected Members
  // -------------------

  std::shared_ptr<Cla::IDoConsumedMethod> mSetter{nullptr};
};

/*! \brief Class representing a setter for a consumed field.
 *  \tparam T The type of the field value.
 *  \tparam Return `true` if the setter shall have a return value, `false` otherwise.
 *  \tparam OneWay `true` if the setter shall be a one way call, `false` otherwise.
 */
template <class T, bool Return, bool OneWay>
class ConsumedSetter : public ConsumedSetterBase
{
public:
  // Public Methods
  // -------------------

  /*! \brief Calls the setter synchronously.
   *  \param value The value to which the field value shall be set.
   *  \returns The returned value by the setter implementation of the providing side. Should be
   *           the new value of the field.
   *  \remarks Calling the setter will automatically update the local value with
   *           the returned value.
   */
  T Call(const T& value);

  /*! \brief Calls the setter synchronously. See Call(const T&). */
  T operator()(const T& value);

  /*! \brief Calls the setter asynchronously.
   *  \param value The value to which the field value shall be set.
   *  \param handler The handler which shall be called when the call returns. The handler
   *                 gets the value which is returned by the setter implementation of
   *                 the providing side. Should be the new value of the field.
   *  \remarks Calling the setter will automatically update the local value with the returned value.
   */
  void CallAsync(const T& value, std::function<void(const T& newValue)> handler);
};

/*! \brief Class template specialization for a one way setter for a consumed field.
 *         Since there is no setter response, the no return value can be provided and
 *         the local field value is not updated.
 *  \tparam T The type of the field value.
 */
template <class T>
class ConsumedSetter<T, false, true> : public ConsumedSetterBase
{
public:
  // Public Methods
  // -------------------

  /*! \brief Calls the setter. Since there is no response it's the same as CallAsync(const T&).
   *  \remarks The local field value is not updated on call. */
  void Call(const T& value);

  /*! \brief Calls the setter. Since there is no response it's the same as CallAsync(const T&).
   *  \remarks The local field value is not updated on call. */
  void operator()(const T& value);

  /*! \brief Calls the setter asynchronously.
   *  \param value The value to which the field value shall be set.
   *  \remarks The local field value is not updated on call. */
  void CallAsync(const T& value);
};

/*! \brief Class template specialization for a setter without return value for a consumed field.
 *         Since there is no return value, the local value of the field is not updated on return.
 *  \tparam T The type of the field value. */
template <class T>
class ConsumedSetter<T, false, false> : public ConsumedSetterBase
{
public:
  // Public Methods
  // -------------------

  /*! \brief Calls the setter synchronously.
   *  \param value The value to which the field value shall be set.
   *  \remarks The local value is not updated to the new value on call or return. */
  void Call(const T& value);

  /*! \brief Calls the setter synchronously. See Call(const T&) */
  void operator()(const T& value);

  /*! \brief Calls the setter asynchronously.
   *  \param value The value to which the field value shall be set.
   *  \param handler The handler which shall be called when the call returns.
   *  \remarks The local value is not updated to the new value on call or return. */
  void CallAsync(const T& value, std::function<void()> handler);
};

/*! \brief Class which adds a setter to the inheriting class.
 *  \tparam T The type of the value the setter method sets.
 *  \tparam Return `true` if the setter has a return value, otherwise `false`.
 *  \tparam OneWay `true` if the setter is a one way method, otherwise `false`. */
template <class T, bool Return, bool OneWay>
class SetterField<T, MemberDirection::Consumed, Return, OneWay>
{
public:
  // Public Members
  // -------------------

  ConsumedSetter<T, Return, OneWay> Set; //!< The setter method of the consumed field.

protected:
  // Protected Methods
  // -------------------

  void _Init(Cla::IDoConsumedField* field);
  void _Reset();
};

/*! \brief Class which adds a notify event to the inheriting class.
 *  \tparam T The type of the field value.
 */
template <class T>
class NotifyField<T, MemberDirection::Consumed>
{
protected:
  // Protected Methods
  // -------------------

  void _Init(Cla::IDoConsumedField* field);
  void _Reset();

private:
  // Private Members
  // -------------------

  std::shared_ptr<Cla::IDoConsumedEvent> mEvent;
};

/*! \brief Class representing a consumed field member of a distributed object.
 *  \tparam T The type of the field value.
 *  \tparam Features The feature configuration of the field, see Get, Set, Notify classes. */
template <class T, class... Features>
class DoConsumedField : public DoField<T>, public Features::template type<T, MemberDirection::Consumed>...
{
public:
  // Public Methods
  // -------------------

  /*!
   * \brief The field member blocks on SIL Adapter initialization
   *        until the first transmission from CANoe is received.
   * \param timeout When a timeout for the waiting shall appear.
   */
  void BlockAtInit(std::chrono::nanoseconds timeout);

protected:
  // Protected Methods
  // -------------------

  friend Accessor;
  void _Init(Cla::IDo* distributedObject, const std::string& path);
  void _Reset() override;
  Cla::IValueEntity* _GetValueEntity() const override;

private:
  // Private Methods
  // -------------------

  void _WaitForFirstTransmission();

  // Private Members
  // -------------------

  std::shared_ptr<Cla::IDoConsumedField> mField{nullptr};

  // For blocking if "BlockAtInit" is enabled
  std::chrono::nanoseconds mWaitForFirstTransmissionTimeout;
  bool mWaitForFirstTransmission = false;
  std::condition_variable mConditionVariable;
  std::mutex mMutex;
  CallbackHandle mCallbackHandle{0};
};

// ===== INLINE IMPLEMENTATION =====
// ===== ConsumedGetter<T> =====

template <class T>
T ConsumedGetter<T>::Call()
{
  assert(!IsWithinCallback() && "Field getter cannot be called synchronously while in a callback.");

  std::promise<T> promise;
  auto future = promise.get_future();
  CallAsync([&](const T& returnValue) { promise.set_value(returnValue); });
  return future.get();
}
template <class T>
T ConsumedGetter<T>::operator()()
{
  return Call();
}
template <class T>
void ConsumedGetter<T>::CallAsync(std::function<void(const T&)> handler)
{
  auto callContext = mGetter->CreateCallContext();

  auto wrapper = [callContext, handler]() mutable {
    T returnValue;
    Deserializer(*callContext->GetOutputParametersDeserializer()).Deserialize(returnValue);

    StartCallback();
    handler(returnValue);
    EndCallback();

    callContext.reset();
  };

  callContext->CallAsync(wrapper);
}
template <class T>
void ConsumedGetter<T>::_Init(std::shared_ptr<Cla::IDoConsumedMethod> getter)
{
  mGetter = std::move(getter);
}
template <class T>
void ConsumedGetter<T>::_Reset()
{
  mGetter.reset();
}

// ===== GetterField<T, MemberDirection::Consumed> =====

template <class T>
void GetterField<T, MemberDirection::Consumed>::_Init(Cla::IDoConsumedField* field)
{
  Get._Init(field->GetConsumedGetter());
}
template <class T>
void GetterField<T, MemberDirection::Consumed>::_Reset()
{
  Get._Reset();
}

// ===== ConsumedSetterBase =====

inline void ConsumedSetterBase::_Init(std::shared_ptr<Cla::IDoConsumedMethod> setter) { mSetter = std::move(setter); }
inline void ConsumedSetterBase::_Reset() { mSetter.reset(); }

// ===== ConsumedSetter<T, Return, OneWay> =====

template <class T, bool Return, bool OneWay>
T ConsumedSetter<T, Return, OneWay>::Call(const T& value)
{
  assert(!IsWithinCallback() && "Field setter cannot be called synchronously while in a callback.");

  std::promise<T> promise;
  auto future = promise.get_future();
  CallAsync(value, [&](const T& returnValue) { promise.set_value(returnValue); });
  return future.get();
}
template <class T, bool Return, bool OneWay>
T ConsumedSetter<T, Return, OneWay>::operator()(const T& value)
{
  return Call(value);
}
template <class T, bool Return, bool OneWay>
void ConsumedSetter<T, Return, OneWay>::CallAsync(const T& value, std::function<void(const T& newValue)> handler)
{
  auto callContext = mSetter->CreateCallContext();
  Serializer(*callContext->GetInputParametersSerializer()).Serialize(value);

  auto wrapper = [callContext, handler]() mutable {
    T returnValue;
    Deserializer(*callContext->GetOutputParametersDeserializer()).Deserialize(returnValue);

    StartCallback();
    handler(returnValue);
    EndCallback();

    callContext.reset();
  };

  callContext->CallAsync(std::move(wrapper));
}

// ===== ConsumedSetter<T, Return = Any, OneWay = true> =====

template <class T>
void ConsumedSetter<T, false, true>::Call(const T& value)
{
  return CallAsync(value);
}
template <class T>
void ConsumedSetter<T, false, true>::operator()(const T& value)
{
  return Call(value);
}
template <class T>
void ConsumedSetter<T, false, true>::CallAsync(const T& value)
{
  auto callContext = mSetter->CreateCallContext();
  Serializer(*callContext->GetInputParametersSerializer()).Serialize(value);
  callContext->CallAsync(nullptr);
}

// ===== ConsumedSetter<T, Return = false, OneWay = false> =====

template <class T>
void ConsumedSetter<T, false, false>::Call(const T& value)
{
  assert(!IsWithinCallback() && "Field setter cannot be called synchronously while in a callback.");

  std::promise<void> promise;
  auto future = promise.get_future();
  CallAsync(value, [&]() { promise.set_value(); });
  return future.get();
}
template <class T>
void ConsumedSetter<T, false, false>::operator()(const T& value)
{
  Call(value);
}
template <class T>
void ConsumedSetter<T, false, false>::CallAsync(const T& value, std::function<void()> handler)
{
  auto callContext = mSetter->CreateCallContext();
  Serializer(*callContext->GetInputParametersSerializer()).Serialize(value);

  auto wrapper = [callContext, handler]() mutable {
    StartCallback();
    handler();
    EndCallback();

    callContext.reset();
  };

  callContext->CallAsync(std::move(wrapper));
}

// ===== SetterField<T, MemberDirection::Consumed, Return, OneWay> =====

template <class T, bool Return, bool OneWay>
void SetterField<T, MemberDirection::Consumed, Return, OneWay>::_Init(Cla::IDoConsumedField* field)
{
  Set._Init(field->GetConsumedSetter());
}
template <class T, bool Return, bool OneWay>
void SetterField<T, MemberDirection::Consumed, Return, OneWay>::_Reset()
{
  Set._Reset();
}

// ===== NotifyField<T, MemberDirection::Consumed> =====

template <class T>
void NotifyField<T, MemberDirection::Consumed>::_Init(Cla::IDoConsumedField* field)
{
  mEvent = field->GetConsumedEvent();
}
template <class T>
void NotifyField<T, MemberDirection::Consumed>::_Reset()
{
  mEvent.reset();
}

// ===== DoConsumedField<T, Features...> =====

// Public methods

template <class T, class... Features>
void DoConsumedField<T, Features...>::BlockAtInit(std::chrono::nanoseconds timeout)
{
  mWaitForFirstTransmissionTimeout = timeout;
  mWaitForFirstTransmission = true;
}

// Protected methods

template <class T, class... Features>
void DoConsumedField<T, Features...>::_Init(Cla::IDo* distributedObject, const std::string& path)
{
  DoField<T>::_Init(path);
  mField = distributedObject->GetConsumedField(path);
  DoField<T>::_SetInitialValue(T{}); // Set initial value

  if (mWaitForFirstTransmission)
  {
    mCallbackHandle = DoField<T>::RegisterOnUpdateHandler([this]() {
      if (DoField<T>::mSettingInitialValue)
      {
        // Exclude initial values
        return;
      }

      std::unique_lock<std::mutex> lock(mMutex);
      mWaitForFirstTransmission = false;
      lock.unlock();
      mConditionVariable.notify_all();
      DoField<T>::UnregisterHandler(mCallbackHandle);
    });
  }

  // Initialize all features (get / set / notify)
  auto _ = {1, (this->Features::template type<T, MemberDirection::Consumed>::_Init(mField.get()), 1)...};
}
template <class T, class... Features>
void DoConsumedField<T, Features...>::_Reset()
{
  // Reset all features (get / set / notify)
  auto _ = {1, (this->Features::template type<T, MemberDirection::Consumed>::_Reset(), 1)...};

  mField.reset();
  DoField<T>::_Reset();
}

template <class T, class... Features>
void DoConsumedField<T, Features...>::_WaitForFirstTransmission()
{
  std::unique_lock<std::mutex> lock(mMutex);
  if (!mConditionVariable.wait_for(lock, mWaitForFirstTransmissionTimeout, [this]() { return !mWaitForFirstTransmission; }))
  {
    throw std::runtime_error("Waiting for first transmission from CANoe: Timeout");
  }
}

template <class T, class... Features>
Cla::IValueEntity* DoConsumedField<T, Features...>::_GetValueEntity() const
{
  return mField->GetValue().get();
}

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
