// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "DoEvent.hpp"

#include "cla/IDoProvidedEvent.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

/*! \brief Base class for provided DO event member.
 *  \tparam T The value type of the event. */
template <class T>
class DoProvidedEventBase : public DoEvent<T>
{
protected:
  // Protected Methods
  // -------------------

  friend Accessor; // To initialize / reset
  void _Init(Cla::IDo* distributedObject, const std::string& path, Cla::TxTrigger);
  void _Reset() override;
  Cla::IValueEntity* _GetValueEntity() const override;

  // Protected Members
  // -------------------

  std::shared_ptr<Cla::IDoProvidedEvent> mEvent = nullptr;
};

/*! \brief Class representing a provided event. The event
 *         can be trigger by setting its value via \see Set
 *         or \see operator=(T).
 *  \tparam T The value type of the provided event. */
template <class T>
class DoProvidedEvent final : public DoProvidedEventBase<T>
{
public:
  // Public Methods
  // -------------------

  DoProvidedEvent() = default;

  /*! \brief Assigns a new value to the provided event member.
   *  \param value The new value of the provided event member. */
  DoProvidedEvent& operator=(T value);

  /*! \brief Assigns a new value to the provided event member.
   *  \param value The new value of the provided event member. */
  DoProvidedEvent& SetValue(T value);

protected:
  // Protected Methods
  // -------------------

  friend Accessor;
  void _Init(Cla::IDo* distributedObject, const std::string& path, TxTrigger);
};

/*! \brief Template specialization for provided void events.
 *         Since void events cannot have a value, the event
 *         must be triggered by calling \see Trigger. */
template <>
class DoProvidedEvent<void> final : public DoProvidedEventBase<void>
{
public:
  // Public Methods
  // -------------------

  DoProvidedEvent() = default;

  /*! \brief Triggers the event. */
  void Trigger();
};

// ===== INLINE IMPLEMENTATION =====
// ===== DoProvidedEventBase<T> =====

// Protected Methods
// -------------------

template <class T>
void DoProvidedEventBase<T>::_Init(Cla::IDo* distributedObject, const std::string& path, TxTrigger txTrigger)
{
  DoEvent<T>::_Init(path);
  mEvent = distributedObject->GetProvidedEvent(path, txTrigger);
}

template <class T>
void DoProvidedEventBase<T>::_Reset()
{
  mEvent.reset();
  DoEvent<T>::_Reset();
}

template <class T>
Cla::IValueEntity* DoProvidedEventBase<T>::_GetValueEntity() const
{
  return mEvent->GetValue().get();
}

// ===== DoProvidedEvent<T> =====

// Public Methods
// -------------------

template <class T>
DoProvidedEvent<T>& DoProvidedEvent<T>::operator=(T value)
{
  return SetValue(std::move(value));
}

template <class T>
DoProvidedEvent<T>& DoProvidedEvent<T>::SetValue(T value)
{
  DoEvent<T>::_Set(std::move(value));
  return *this;
}

// Protected Methods
// -------------------

template <class T>
void DoProvidedEvent<T>::_Init(Cla::IDo* distributedObject, const std::string& path, TxTrigger txTrigger)
{
  DoProvidedEventBase<T>::_Init(distributedObject, path, txTrigger);
  DoEvent<T>::_Set(T{});
}

// ===== DoProvidedEvent<void> =====

// Public Methods
// -------------------

inline void DoProvidedEvent<void>::Trigger() { Serializer(*mEvent->GetValue()->GetSerializer()).Serialize(); }

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
