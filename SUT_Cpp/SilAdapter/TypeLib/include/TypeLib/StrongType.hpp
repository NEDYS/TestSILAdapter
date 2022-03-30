// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

namespace Vector {
namespace CANoe {
namespace TypeLib {

/*! \brief Class which wraps a type, e.g. an alias, to be a strong type.
 *  \tparam T The type which shall be wrapped.
 *  \tparam Tag A unique Tag to differ between multiple strong types with the same `T`. */
template <class T, class Tag>
class StrongType
{
public:
  /*! \brief Instantiates a new object of the strong type.
   *  \param value The value which shall be wrapped by this strong type. */
  explicit StrongType(const T& value) : mValue(value) {}

  /*! \brief Returns the wrapped value of this strong type.
   *  \returns the wrapped value of this strong type. */
  const T& Get() const { return mValue; }

private:
  T mValue;
};

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
