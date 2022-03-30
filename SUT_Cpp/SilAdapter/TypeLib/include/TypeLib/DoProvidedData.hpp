// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "DoData.hpp"

#include "cla/IDoProvidedData.hpp"
#include "cla/IValueEntity.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

/*!
 * \brief Class representing a provided data member of a distributed object.
 * \tparam T The tye of the data managed by the data member.
 */
template <class T>
class DoProvidedData : public DoData<T>
{
public:
  // Public Methods
  // -------------------

  /*! \brief Instantiates a new uninitialized provided data member. */
  DoProvidedData() = default;

  /*!
   * \brief Assigns a new value to the provided data member.
   * \param value The new value of the provided data member.
   */
  DoProvidedData& operator=(T value);

  /*!
   * \brief Assigns a new value to the provided data member.
   * \param value The new value of the provided data member.
   */
  DoProvidedData& SetValue(T value);

protected:
  // Protected Methods
  // -------------------

  friend class Accessor; // To initialize / reset
  void _Init(Cla::IDo* distributedObject, const std::string& path, TxTrigger txTrigger);
  void _Reset() override;

  // Protected Members
  // -------------------

  std::shared_ptr<Cla::IDoProvidedData> mData = nullptr;

private:
  // Private Methods
  // -------------------

  Cla::IValueEntity* _GetValueEntity() const override;
};

// ===== Inline Implementation =====

// Public Methods
// -------------------

template <class T>
DoProvidedData<T>& DoProvidedData<T>::operator=(T value)
{
  return SetValue(value);
}

template <class T>
DoProvidedData<T>& DoProvidedData<T>::SetValue(T value)
{
  DoData<T>::_Set(std::move(value));
  return *this;
}

// Protected Methods
// -------------------

template <class T>
void DoProvidedData<T>::_Init(Cla::IDo* distributedObject, const std::string& path, TxTrigger txTrigger)
{
  DoData<T>::_Init(path);
  mData = distributedObject->GetProvidedData(path, txTrigger);
  SetValue(T{});
}

template <class T>
void DoProvidedData<T>::_Reset()
{
  mData.reset();
  DoData<T>::_Reset();
}

// Private Methods
// -------------------

template <class T>
Cla::IValueEntity* DoProvidedData<T>::_GetValueEntity() const
{
  return mData->GetValue().get();
}

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
