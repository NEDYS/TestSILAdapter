// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>
#include <string>

#include "DataTypes.hpp"
#include "Deserializer.hpp"
#include "Serializer.hpp"
#include "ThreadContext.hpp"

#include "cla/IDo.hpp"
#include "cla/IDoEvent.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

// Forward Declarations
class Accessor;

/*! \brief Abstract base class for all DO event members. */
class DoEventBase
{
public:
  // Public Methods
  // -------------------

  /*! \brief Registers a handler, which is called when the event was triggered and
   *         its value updated (in case of a non-void event).
   *  \param handler The handler which shall be called when the event is triggered.
   *  \returns The handle for the handler to unregister it. */
  CallbackHandle RegisterOnUpdateHandler(std::function<void()> handler);

  /*! \brief Unregisters an already registered handler.
   *  \param handle The handle of handler which shall be unregistered. */
  void UnregisterHandler(CallbackHandle handle);

protected:
  // Protected Methods
  // -------------------

  DoEventBase() = default;
  virtual ~DoEventBase() = default;

  /*! \brief Initializes the event member.
   *  \param path The path of this event member (not the full path).
   */
  void _Init(const std::string& path);

  //! \brief Resets the event member. The event member will no longer be valid.
  virtual void _Reset();

  /*! \brief Returns the value entity of this event member.
   *  \returns The CLA value entity of this event member. */
  virtual Cla::IValueEntity* _GetValueEntity() const = 0;

  // Protected Members
  // -------------------

  std::string mPath; ///< The path of the member without the parent path
};

/*! \brief Base class for DO event members.
 *  \tparam T The value type of the event. */
template <class T>
class DoEvent : public DoEventBase
{
public:
  // Public Methods
  // -------------------

  /*! \brief Returns the value of the event member.
   *  \returns The value of the event member. */
  operator T() const;

  /*! \brief Returns the value of the event member.
   *  \returns The value of the event member. */
  T GetValue() const;

  /*! \brief Registers a handler, which is called when the event was triggered and
   *         its value changed.
   *  \param handler The handler which shall be called when the event is triggered.
   *  \returns The handle for the handler to unregister it. */
  CallbackHandle RegisterOnChangeHandler(std::function<void()> handler);

protected:
  // Protected Methods

  /*! \brief Sets the event member.
  *  \param value of the event member to be set. */
  void _Set(T value);

  /*! \brief Sets the initial value of the event member.
  *  \param value The initial value. */
  void _SetInitialValue(T value);

  // Protected Members
  // -------------------
  bool mSettingInitialValue = false;
};

/*! \brief Template specialization for void event members, which do not have a value. */
template <>
class DoEvent<void> : public DoEventBase
{
};

// ===== INLINE IMPLEMENTATION =====

// ===== DoEvent<T> =====

// Public Methods
// -------------------

template <class T>
DoEvent<T>::operator T() const
{
  return GetValue();
}

template <class T>
T DoEvent<T>::GetValue() const
{
  Deserializer deserializer(*_GetValueEntity()->GetDeserializer());
  return deserializer.Deserialize<T>();
}

template <class T>
CallbackHandle DoEvent<T>::RegisterOnChangeHandler(std::function<void()> handler)
{
  auto wrapper = [handler](Cla::IValueEntity&) {
    StartCallback();
    handler();
    EndCallback();
  };
  return CallbackHandle(_GetValueEntity()->RegisterCallback(wrapper, Cla::ValueUpdateMode::OnChange));
}

// Protected Methods
// -------------------

template <class T>
void DoEvent<T>::_Set(T value)
{
  Serializer serializer(*_GetValueEntity()->GetSerializer());
  serializer.Serialize(std::move(value));
}

template <class T>
void DoEvent<T>::_SetInitialValue(T value)
{
  mSettingInitialValue = true;
  _Set(std::move(value));
  mSettingInitialValue = false;
}

// ===== DoEventBase =====

inline CallbackHandle DoEventBase::RegisterOnUpdateHandler(std::function<void()> handler)
{
  auto wrapper = [handler](Cla::IValueEntity&) {
    StartCallback();
    handler();
    EndCallback();
  };
  return CallbackHandle(_GetValueEntity()->RegisterCallback(wrapper, Cla::ValueUpdateMode::OnUpdate));
}

inline void DoEventBase::UnregisterHandler(CallbackHandle handle)
{
  _GetValueEntity()->UnregisterCallback(handle.Get());
}

// Protected Methods
// -------------------

inline void DoEventBase::_Init(const std::string& path) { mPath = path; }

inline void DoEventBase::_Reset() { mPath.clear(); }

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
