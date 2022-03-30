// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <functional>

#include "DataTypes.hpp"
#include "Deserializer.hpp"
#include "Serializer.hpp"
#include "ThreadContext.hpp"

#include "cla/IDo.hpp"
#include "cla/IDoData.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

// Forward Declarations
class Accessor;

/*! \brief Class representing a data member of a distributed object. */
template <class T>
class DoData
{
public:
  // Public Methods
  // -------------------

  /*! \brief Returns the value of the data member.
   *  \returns The value of the data member. */
  operator T() const;

  /*! \brief Returns the value of the data member.
   *  \returns The value of the data member. */
  T GetValue() const;

  /*! \brief Registers a handler for a value update. The new value doesn't
   *         need to be different. \see {RegisterOnChangeHandler} for value
   *         changes.
   *  \param handler The handler which shall be called on a value update.
   *  \returns the handle to unregister the handler. */
  CallbackHandle RegisterOnUpdateHandler(std::function<void()> handler);

  /*! \brief Registers a handler for a value change. The new value of the data
   *         member is different than the one before. \see {RegisterOnUpdateHandler}
   *         for all value updates.
   *  \param handler The handler which shall be called on a value change.
   *  \returns the handle to unregister the handler. */
  CallbackHandle RegisterOnChangeHandler(std::function<void()> handler);

  /*! \brief Unregisters a previously registered OnChange or OnUpdate handler.
   *  \param handle The handle to the registered handler. */
  void UnregisterHandler(CallbackHandle handle);

protected:
  // Protected Methods
  // -------------------

  DoData() = default;
  virtual ~DoData() = default;

  /*! \brief Initializes the data member.
   *  \param path The path of this data member (not the full path). */
  void _Init(const std::string& path);

  /*! \brief Sets the data member.
  *  \param value of the data member to be set. */
  void _Set(T value);

  /*! \brief Sets the initial value of the data member.
   *  \param value The initial value. */
  void _SetInitialValue(T value);

  //! \brief Resets the data member. The data member will no longer be valid.
  virtual void _Reset();

  // Protected Members
  // -------------------

  std::string mPath; ///< The path of the member without the parent path
  bool mSettingInitialValue = false;

private:
  // Private Methods
  // -------------------

  /*! \brief Returns the value entity of this data member.
   *  \returns The CLA value entity of this data member. */
  virtual Cla::IValueEntity* _GetValueEntity() const = 0;
};

// ===== Inline Implementation =====

// Public Methods
// -------------------

template <class T>
DoData<T>::operator T() const
{
  return GetValue();
}

template <class T>
T DoData<T>::GetValue() const
{
  Deserializer deserializer(*_GetValueEntity()->GetDeserializer());
  return deserializer.Deserialize<T>();
}

template <class T>
CallbackHandle DoData<T>::RegisterOnUpdateHandler(std::function<void()> handler)
{
  auto wrapper = [handler](Cla::IValueEntity&)
  {
    StartCallback();
    handler();
    EndCallback();
  };
  return CallbackHandle(_GetValueEntity()->RegisterCallback(wrapper, Cla::ValueUpdateMode::OnUpdate));
}

template <class T>
CallbackHandle DoData<T>::RegisterOnChangeHandler(std::function<void()> handler)
{
  auto wrapper = [handler](Cla::IValueEntity&)
  {
    StartCallback();
    handler();
    EndCallback();
  };
  return CallbackHandle(_GetValueEntity()->RegisterCallback(wrapper, Cla::ValueUpdateMode::OnChange));
}

template <class T>
void DoData<T>::UnregisterHandler(CallbackHandle handle)
{
  _GetValueEntity()->UnregisterCallback(handle.Get());
}

// Protected Methods
// -------------------

template <class T>
void DoData<T>::_Init(const std::string& path)
{
  mPath = path;
}

template <class T>
void DoData<T>::_Set(T value)
{
  Serializer serializer(*_GetValueEntity()->GetSerializer());
  serializer.Serialize(std::move(value));
}

template <class T>
void DoData<T>::_SetInitialValue(T value)
{
  mSettingInitialValue = true;
  _Set(std::move(value));
  mSettingInitialValue = false;
}

template <class T>
void DoData<T>::_Reset()
{
  mPath.clear();
}

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
