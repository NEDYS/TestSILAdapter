// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <array>
#include <string>
#include <type_traits>
#include <vector>

#include "Optional.hpp"
#include "Serialization.hpp"

#include "cla/ISerializer.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

/*! \brief Wrapper for the `Cla::ISerializer` interface which
 *         supports additional operator overloads, templates and tokens. */
class Serializer final
{
public:
  /*! \brief Instantiates a new `ISerializer` wrapper.
   *  \param serializer The serializer which shall be wrapped. */
  Serializer(Cla::ISerializer& serializer);

  template <class... Args>
  void Serialize(const Args&... args);

  // Floating point
  Serializer& operator<<(float rhs);
  Serializer& operator<<(double rhs);

  // Bool
  Serializer& operator<<(bool rhs);

  // Integer
  Serializer& operator<<(int8_t rhs);
  Serializer& operator<<(int16_t rhs);
  Serializer& operator<<(int32_t rhs);
  Serializer& operator<<(int64_t rhs);

  // Unsigned integer
  Serializer& operator<<(uint8_t rhs);
  Serializer& operator<<(uint16_t rhs);
  Serializer& operator<<(uint32_t rhs);
  Serializer& operator<<(uint64_t rhs);

  // Strings, encoded in UTF-8
  Serializer& operator<<(const std::string& rhs);

  // Bytes
  Serializer& operator<<(const std::vector<uint8_t>& rhs);

  // Structs
  Serializer& operator<<(const BeginStruct& token);
  Serializer& operator<<(const EndStruct& token);

  // Arrays
  template <class T, std::size_t N>
  Serializer& operator<<(const std::array<T, N>& rhs);

  // Lists
  template <class T>
  Serializer& operator<<(const std::vector<T>& rhs);

  // Optionals
  template <class T>
  Serializer& operator<<(const optional<T>& rhs);

  // Unions
  Serializer& operator<<(const BeginUnion& token);
  Serializer& operator<<(const EndUnion& token);

  // Enums
  template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
  Serializer& operator<<(T rhs);

private:
  Cla::ISerializer& mSerializer;
};

// ===== Inline Implementation =====

template <class... Args>
void Serializer::Serialize(const Args&... args)
{
  mSerializer.BeginSerialization();
  auto _ = {1, (*this << args, 1)...};
  mSerializer.EndSerialization();
}

template <class T>
Serializer& Serializer::operator<<(const std::vector<T>& rhs)
{
  mSerializer.BeginArray(rhs.size());
  for (const auto& element : rhs)
  {
    *this << element;
  }
  mSerializer.EndArray();
  return *this;
}

template <class T, std::size_t N>
Serializer& Serializer::operator<<(const std::array<T, N>& rhs)
{
  mSerializer.BeginArray(rhs.size());
  for (const auto& element : rhs)
  {
    *this << element;
  }
  mSerializer.EndArray();
  return *this;
}

template <class T>
Serializer& Serializer::operator<<(const optional<T>& rhs)
{

  mSerializer.BeginOptional(rhs.has_value());
  if (rhs.has_value())
  {
    *this << rhs.value();
  }
  mSerializer.EndOptional();
  return *this;
}

template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type>
Serializer& Serializer::operator<<(T rhs)
{
  return *this << static_cast<typename std::underlying_type<T>::type>(rhs);
}

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
