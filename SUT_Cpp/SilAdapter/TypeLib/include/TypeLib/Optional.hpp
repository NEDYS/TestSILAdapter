// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

namespace Vector {
namespace CANoe {
namespace TypeLib {

/*!
 * \brief Class template for an data structure which contains an optional value.
 *        Supports a sub-set of the C++17 std::optional functionality. In comparison
 *        to the std::optional, the contained value is always default constructed
 *        and destroyed, even if no value is / was assigned.
 * \tparam T The type of the value contained in the optional.
 */
template <class T>
class optional
{
public:
  // ----------------------------------------
  // Type aliases

  /*! \brief the type of the value contained in the optional. */
  using value_type = T;

  // ----------------------------------------
  // Constructors and destructor

  /*! \brief Instantiates a new optional whose value is not set. */
  optional() = default;

  optional(const optional&) = default;
  optional(optional&&) = default;

  /*! \brief Instantiates a new optional with an initial value.
   *  \param value The initial value. */
  optional(const T& value) : mHasValue(true), mValue(value) {}

  /*! \brief Instantiates a new optional with an initial value.
   *  \param value The initial value. */
  optional(T&& value) : mHasValue(true), mValue(std::move(value)) {}

  ~optional() = default;

  // ----------------------------------------
  // Operators

  optional& operator=(const optional&) = default;
  optional& operator=(optional&&) = default;

  /*! \brief Assigns explicitly a value to the optional.
   *  \param value The value which shall be assigned. */
  optional& operator=(const T& value)
  {
    mValue = value;
    mHasValue = true;
    return *this;
  }

  /*! \brief Move-assigns explicitly a value to the optional.
   *  \param value The value which shall be assigned. */
  optional& operator=(T&& value)
  {
    mValue = std::move(value);
    mHasValue = true;
    return *this;
  }

  const T* operator->() const { return &mValue; };
  T* operator->() { return &mValue; }
  const T& operator*() const { return mValue; }
  T& operator*() { return mValue; }

  /*! \brief Returns whether the optional contains a value which was explicitly set.
   *  \returns `true` if the value was set, otherwise `false`. */
  operator bool() const { return mHasValue; }

  // ----------------------------------------
  // Public methods

  /*! \brief Returns whether the optional contains a value which was explicitly set.
   *  \returns `true` if the value was set, otherwise `false`. */
  bool has_value() const { return mHasValue; }

  /*! \brief Returns the value which is contained in the optional.
   *  \returns the value which is contained. */
  const T& value() const { return mValue; }

private:
  // ----------------------------------------
  // private members

  bool mHasValue = false;
  T mValue{};
};

/*! \brief Compares two optionals for equality. If both contain an explicitly set value,
 *         their values are compared accordingly.
 *  \tparam T The type of the value contained in the optionals.
 *  \returns `true` if both optionals are equal, otherwise `false`. */
template <class T>
bool operator==(const optional<T>& lhs, const optional<T>& rhs)
{
  if (lhs.has_value() != rhs.has_value())
  {
    return false;
  }

  return lhs.has_value() && lhs.value() == rhs.value();
}

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
