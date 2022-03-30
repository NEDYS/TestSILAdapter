// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "Deserializer.hpp"
#include "DoField.hpp"
#include "ThreadContext.hpp"

#include "cla/IDoProvidedField.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

/*! \brief Class representing the getter method for a provided field.
 *  \tparam T The type of the value getter method fetches. */
template <class T>
class ProvidedGetter
{
public:
  // Public Methods
  // -------------------

  /*! \brief Sets the handler for the getter. The handler allows to customize the field getter.
   *  \param handler The handler for the getter. `nullptr` if the default handler shall be used,
   *                 which returns the current field value. */
  void SetCallHandler(std::function<T()> handler);

private:
  // Private Methods
  // -------------------

  friend GetterField<T, MemberDirection::Provided>;
  void _Init(std::shared_ptr<Cla::IDoProvidedMethod> getter);
  void _Reset();

  // Private Members
  // -------------------

  std::shared_ptr<Cla::IDoProvidedMethod> mGetter{nullptr};
};

/*! \brief Class which adds a getter to the inheriting class.
 *  \tparam T The type of the value the getter method fetches.
 */
template <class T>
class GetterField<T, MemberDirection::Provided>
{
public:
  // Public Members
  // -------------------

  ProvidedGetter<T> Get; //!< The getter method of the provided field

protected:
  // Protected Methods
  // -------------------

  void _Init(Cla::IDoProvidedField* field);
  void _Reset();
};

/*! \brief Base class for setters in a provided field member. */
class ProvidedSetterBase
{
protected:
  // Protected Methods
  // -------------------

  template <class, MemberDirection, bool, bool>
  friend class SetterField;
  void _Init(std::shared_ptr<Cla::IDoProvidedMethod> setter);
  void _Reset();

  // Protected Members
  // -------------------

  std::shared_ptr<Cla::IDoProvidedMethod> mSetter{nullptr};
};

/*! \brief Class representing a setter for a provided field.
 *  \tparam T The type of the field value.
 *  \tparam Return `true` if the setter shall have a return value, `false` otherwise.
 *  \tparam OneWay `true` if the setter shall be a one way call, `false` otherwise. */
template <class T, bool Return, bool OneWay>
class ProvidedSetter : public ProvidedSetterBase
{
public:
  // Public Methods
  // -------------------

  /*! \brief Sets the handler for the setter.
   *  \param handler The call handler for the setter. It provides the new value,
   *                 with which the setter was called, and should return the current
   *                 value of the field after it was set. `nullptr` if the default handler
   *                 shall be set, which unconditionally assigns the new value to the field
   *                 and returns it as current value. */
  void SetCallHandler(std::function<T(const T& value)> handler);
};

/*! \brief Class template specialization for a one way setter for a provided field.
 *  \tparam T The type of the field value. */
template <class T>
class ProvidedSetter<T, false, true> : public ProvidedSetterBase
{
public:
  // Public Methods
  // -------------------

  /*! \brief Sets the call handler for the setter.
   *  \param handler The call handler for the setter. It provides the new value,
   *                 with which the setter was called. `nullptr` if the default handler
   *                 shall be set, which unconditionally assigns the new value to the field. */
  void SetCallHandler(std::function<void(const T& value)> handler);
};

/*! \brief Class template specialization for a setter without a return value for a provided field.
 *  \tparam T The type of the field value. */
template <class T>
class ProvidedSetter<T, false, false> : public ProvidedSetterBase
{
public:
  // Public Methods
  // -------------------

  /*! \brief Sets the call handler for the setter.
   *  \param handler The call handler for the setter. It provides the new value,
   *                 with which the setter was called. `nullptr` if the default
   *                 handler shall be set, which unconditionally assigns the value
   *                 to the field. */
  void SetCallHandler(std::function<void(const T& value)> handler);
};

/*! \brief Class which adds a setter to the inheriting class.
 *  \tparam T The type of the value the setter method sets.
 *  \tparam Return `true` if the setter has a return value, otherwise `false`.
 *  \tparam OneWay `true` if the setter is a one way method, otherwise `false`. */
template <class T, bool Return, bool OneWay>
class SetterField<T, MemberDirection::Provided, Return, OneWay>
{
public:
  // Public Members
  // -------------------

  ProvidedSetter<T, Return, OneWay> Set; //!< The setter method of the provided field.

protected:
  // Protected Methods
  // -------------------

  void _Init(Cla::IDoProvidedField* field);
  void _Reset();
};

/*! \brief Class which adds a notify event to the inheriting class.
 *  \tparam T The type of the field value.
 */
template <class T>
class NotifyField<T, MemberDirection::Provided>
{
protected:
  // Protected Methods
  // -------------------

  void _Init(Cla::IDoProvidedField* field);
  void _Reset();

private:
  // Private Methods
  // -------------------

  std::shared_ptr<Cla::IDoProvidedEvent> mEvent{nullptr};
};

/*! \brief Class representing a provided field member of a distributed object.
 *  \tparam T The type of the field value.
 *  \tparam Features The feature configuration of the field, see Get, Set, Notify classes. */
template <class T, class... Features>
class DoProvidedField : public DoField<T>, public Features::template type<T, MemberDirection::Provided>...
{
public:
  // Public Methods
  // -------------------

  /*! \brief Sets the local value of the provided field.
   *  \param rhs The new value of the provided field. */
  DoProvidedField& SetValue(const T& rhs);

protected:
  // Protected Methods
  // -------------------

  friend Accessor;
  void _Init(Cla::IDo* distributedObject, const std::string& path, TxTrigger txTrigger);
  void _Reset() override;
  Cla::IValueEntity* _GetValueEntity() const override;

private:
  // Private Members
  // -------------------

  std::shared_ptr<Cla::IDoProvidedField> mField{nullptr};
};

// ===== INLINE IMPLEMENTATION =====
// ===== Getter<T, MemberDirection::Provided> =====

template <class T>
void ProvidedGetter<T>::SetCallHandler(std::function<T()> handler)
{
  if (!handler)
  {
    mGetter->SetCallback(nullptr);
    return;
  }

  auto wrapper = [handler](Cla::IProviderCallContext& callContext) {
    StartCallback();
    const auto value = handler();
    EndCallback();

    Serializer(*callContext.GetOutputParametersSerializer()).Serialize(value);
  };

  mGetter->SetCallback(std::move(wrapper));
}
template <class T>
void ProvidedGetter<T>::_Init(std::shared_ptr<Cla::IDoProvidedMethod> getter)
{
  mGetter = std::move(getter);
}
template <class T>
void ProvidedGetter<T>::_Reset()
{
  mGetter.reset();
}

// ===== GetterField<T, MemberDirection::Provided> =====

template <class T>
void GetterField<T, MemberDirection::Provided>::_Init(Cla::IDoProvidedField* field)
{
  Get._Init(field->GetProvidedGetter());
}
template <class T>
void GetterField<T, MemberDirection::Provided>::_Reset()
{
  Get._Reset();
}

// ===== ProvidedSetterBase =====

inline void ProvidedSetterBase::_Init(std::shared_ptr<Cla::IDoProvidedMethod> setter) { mSetter = std::move(setter); }
inline void ProvidedSetterBase::_Reset() { mSetter.reset(); }

// ===== ProvidedSetter<T, Return, OneWay> =====

template <class T, bool Return, bool OneWay>
void ProvidedSetter<T, Return, OneWay>::SetCallHandler(std::function<T(const T& value)> handler)
{
  if (!handler)
  {
    mSetter->SetCallback(nullptr);
    return;
  }

  auto wrapper = [handler](Cla::IProviderCallContext& callContext) {
    T value;
    Deserializer(*callContext.GetInputParametersDeserializer()).Deserialize(value);

    StartCallback();
    const auto returnValue = handler(value);
    EndCallback();

    Serializer(*callContext.GetOutputParametersSerializer()).Serialize(value);
  };

  mSetter->SetCallback(std::move(wrapper));
}

// ===== ProvidedSetter<T, Return = Any, OneWay = true> =====

template <class T>
void ProvidedSetter<T, false, true>::SetCallHandler(std::function<void(const T& value)> handler)
{
  if (!handler)
  {
    mSetter->SetCallback(nullptr);
    return;
  }

  auto wrapper = [handler](Cla::IProviderCallContext& callContext) {
    T value;
    Deserializer(*callContext.GetInputParametersDeserializer()).Deserialize(value);

    StartCallback();
    handler(value);
    EndCallback();
  };

  mSetter->SetCallback(std::move(wrapper));
}

// ===== ProvidedSetter<T, Return = false, OneWay = false> =====

template <class T>
void ProvidedSetter<T, false, false>::SetCallHandler(std::function<void(const T& value)> handler)
{
  if (!handler)
  {
    mSetter->SetCallback(nullptr);
    return;
  }

  auto wrapper = [handler](Cla::IProviderCallContext& callContext) {
    T value;
    Deserializer(*callContext.GetInputParametersDeserializer()).Deserialize(value);

    StartCallback();
    handler(value);
    EndCallback();

    Serializer(*callContext.GetOutputParametersSerializer()).Serialize();
  };

  mSetter->SetCallback(std::move(wrapper));
}

// ===== SetterField<T, MemberDirection::Provided, Return, OneWay> =====

template <class T, bool Return, bool OneWay>
void SetterField<T, MemberDirection::Provided, Return, OneWay>::_Init(Cla::IDoProvidedField* field)
{
  Set._Init(field->GetProvidedSetter());
}
template <class T, bool Return, bool OneWay>
void SetterField<T, MemberDirection::Provided, Return, OneWay>::_Reset()
{
  Set._Reset();
}

// ===== NotifyField<T, MemberDirection::Provided> =====

template <class T>
void NotifyField<T, MemberDirection::Provided>::_Init(Cla::IDoProvidedField* field)
{
  mEvent = field->GetProvidedEvent();
}
template <class T>
void NotifyField<T, MemberDirection::Provided>::_Reset()
{
  mEvent.reset();
}

// ===== DoProvidedField<T, Features...> =====

template <class T, class... Features>
DoProvidedField<T, Features...>& DoProvidedField<T, Features...>::SetValue(const T& rhs)
{
  DoField<T>::_Set(rhs);
  return *this;
}
template <class T, class... Features>
void DoProvidedField<T, Features...>::_Init(Cla::IDo* distributedObject, const std::string& path, TxTrigger txTrigger)
{
  DoField<T>::_Init(path);
  mField = distributedObject->GetProvidedField(path, txTrigger);
  DoField<T>::_Set(T{}); // Set initial value

  // Initialize all field features (get / set / notify)
  auto _ = {1, (this->Features::template type<T, MemberDirection::Provided>::_Init(mField.get()), 1)...};
}
template <class T, class... Features>
void DoProvidedField<T, Features...>::_Reset()
{
  // Reset all field features (get / set / notify)
  auto _ = {1, (this->Features::template type<T, MemberDirection::Provided>::_Reset(), 1)...};

  mField.reset();
  DoField<T>::_Reset();
}
template <class T, class... Features>
Cla::IValueEntity* DoProvidedField<T, Features...>::_GetValueEntity() const
{
  return mField->GetValue().get();
}

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
