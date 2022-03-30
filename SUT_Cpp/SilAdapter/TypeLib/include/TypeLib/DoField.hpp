// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "DataTypes.hpp"
#include "Serializer.hpp"

#include "cla/IDo.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

/*! \brief Abstract base class representing field members of a distributed objects.
 *  \tparam T The type of the value which is stored in the field member. */
template <class T>
class DoField
{
public:
  // Public Methods
  // -------------------

  /*! \brief Returns the local value of the field member. Does not call getter of the field.
   *  \returns The value of the field member. */
  T GetValue() const;

  /*! \brief Registers a handler, which is called when the field value was updated.
   *  \param handler The handler which shall be called when the field value was updated.
   *  \returns The handle for the handler to unregister it. */
  CallbackHandle RegisterOnUpdateHandler(std::function<void()> handler);

  /*! \brief Registers a handler, which is called when the value of the field was changed.
   *  \param handler The handler which shall be called when the field value was changed.
   *  \returns The handle for the handler to unregister it. */
  CallbackHandle RegisterOnChangeHandler(std::function<void()> handler);

  /*! \brief Unregisters an already registered handler.
   *  \param handle The handle of handler which shall be unregistered. */
  void UnregisterHandler(CallbackHandle handle);

protected:
  // Protected Methods
  // -------------------

  DoField() = default;
  virtual ~DoField() = default;

  /*! \brief Initializes the field member.
   *  \param path The path of this field member (not the full path). */
  void _Init(const std::string& path);

  /*! \brief Sets the local value of the field member.
   *  \param value The new local value of the field member. */
  void _Set(T value);

  /*! \brief Sets the initial value of the field member.
  *  \param value The initial value. */
  void _SetInitialValue(T value);

  //! \brief Resets the field member. The field member will no longer be valid.
  virtual void _Reset();

  /*! \brief Returns the value entity of this field member.
   *  \returns The CLA value entity of this field member. */
  virtual Cla::IValueEntity* _GetValueEntity() const = 0;

  /*! \brief Registers a handler for local value updates / changes of the field.
   *  \param handler The handler which shall be called on an update / change.
   *  \param mode The update mode to specify on update or on change.
   *  \returns The handle to the handler to unregister it. */
  CallbackHandle _RegisterHandler(std::function<void()> handler, Cla::ValueUpdateMode mode);

  // Protected Members
  // -------------------

  std::string mPath; ///< The path of the member without the parent path
  bool mSettingInitialValue = false;
};

// Forward declarations for the configuration of the field features

template <class T, MemberDirection D>
class GetterField;

template <class T, MemberDirection D, bool Return, bool OneWay>
class SetterField;

template <class T, MemberDirection D>
class NotifyField;

struct Get
{
  template <class T, MemberDirection D>
  using type = GetterField<T, D>;
};

template <bool Return, bool OneWay>
struct Set
{
  template <class T, MemberDirection D>
  using type = SetterField<T, D, Return, OneWay>;
};

struct Notify
{
  template <class T, MemberDirection D>
  using type = NotifyField<T, D>;
};

// ===== INLINE IMPLEMENTATION =====

// Public Methods
// -------------------

template <class T>
T DoField<T>::GetValue() const
{
  return Deserializer(*_GetValueEntity()->GetDeserializer()).Deserialize<T>();
}
template <class T>
CallbackHandle DoField<T>::RegisterOnUpdateHandler(std::function<void()> handler)
{
  return _RegisterHandler(std::move(handler), Cla::ValueUpdateMode::OnUpdate);
}
template <class T>
CallbackHandle DoField<T>::RegisterOnChangeHandler(std::function<void()> handler)
{
  return _RegisterHandler(std::move(handler), Cla::ValueUpdateMode::OnChange);
}
template <class T>
void DoField<T>::UnregisterHandler(CallbackHandle handle)
{
  _GetValueEntity()->UnregisterCallback(handle.Get());
}

// Protected Methods
// -------------------

template <class T>
void DoField<T>::_Init(const std::string& path)
{
  mPath = path;
}
template <class T>
void DoField<T>::_Set(T value)
{
  Serializer(*_GetValueEntity()->GetSerializer()).Serialize(std::move(value));
}
template <class T>
void DoField<T>::_SetInitialValue(T value)
{
  mSettingInitialValue = true;
  _Set(std::move(value));
  mSettingInitialValue = false;
}
template <class T>
void DoField<T>::_Reset()
{
  mPath.clear();
}
template <class T>
CallbackHandle DoField<T>::_RegisterHandler(std::function<void()> handler, Cla::ValueUpdateMode mode)
{
  auto wrapper = [handler](Cla::IValueEntity&) {
    StartCallback();
    handler();
    EndCallback();
  };
  return CallbackHandle(_GetValueEntity()->RegisterCallback(wrapper, mode));
}

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
