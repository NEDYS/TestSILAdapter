// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <array>
#include <string>
#include <vector>

#include "Optional.hpp"
#include "Serialization.hpp"

#include "cla/IDeserializer.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

/*! \brief Wrapper for the `Cla::IDeserializer` interface which
 *         supports additional operator overloads, templates and tokens. */
class Deserializer final
{
public:
  /*! \brief Instantiates a new `IDeserializer` wrapper.
   *  \param deserializer The deserializer which shall be wrapped. */
  Deserializer(Cla::IDeserializer& deserializer);

  template <class T>
  T Deserialize();

  template <class... Args>
  void Deserialize(Args&... args);

  // Floating point
  Deserializer& operator>>(float& rhs);
  Deserializer& operator>>(double& rhs);

  // Bool
  Deserializer& operator>>(bool& rhs);

  // Integer
  Deserializer& operator>>(int8_t& rhs);
  Deserializer& operator>>(int16_t& rhs);
  Deserializer& operator>>(int32_t& rhs);
  Deserializer& operator>>(int64_t& rhs);

  // Unsigned integer
  Deserializer& operator>>(uint8_t& rhs);
  Deserializer& operator>>(uint16_t& rhs);
  Deserializer& operator>>(uint32_t& rhs);
  Deserializer& operator>>(uint64_t& rhs);

  // Strings, encoded in UTF-8
  Deserializer& operator>>(std::string& rhs);

  // Bytes
  Deserializer& operator>>(std::vector<uint8_t>& rhs);

  // Structs
  Deserializer& operator>>(const BeginStruct& token);
  Deserializer& operator>>(const EndStruct& token);

  // Arrays
  template <class T, std::size_t N>
  Deserializer& operator>>(std::array<T, N>& rhs);

  // Lists
  template <class T>
  Deserializer& operator>>(std::vector<T>& rhs);

  // Optionals
  template <class T>
  Deserializer& operator>>(optional<T>& rhs);

  // Unions
  int operator>>(const BeginUnion& token);
  Deserializer& operator>>(const EndUnion& token);

  // Enums
  template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
  Deserializer& operator>>(T& rhs);

private:
  Cla::IDeserializer& mDeserializer;
};

// ===== Inline Implementation =====

template <class T>
T Deserializer::Deserialize()
{
  T value{};
  mDeserializer.BeginDeserialization();
  *this >> value;
  mDeserializer.EndDeserialization();
  return value;
}

template <class... Args>
void Deserializer::Deserialize(Args&... args)
{
  mDeserializer.BeginDeserialization();
  auto _ = {1, (*this >> args, 1)...};
  mDeserializer.EndDeserialization();
}

template <class T>
Deserializer& Deserializer::operator>>(std::vector<T>& rhs)
{
  rhs.resize(mDeserializer.BeginArray());
  for (auto& element : rhs)
  {
    *this >> element;
  }
  mDeserializer.EndArray();
  return *this;
}

template <class T, std::size_t N>
Deserializer& Deserializer::operator>>(std::array<T, N>& rhs)
{
  mDeserializer.BeginArray();
  for (auto& element : rhs)
  {
    *this >> element;
  }
  mDeserializer.EndArray();
  return *this;
}

template <class T>
Deserializer& Deserializer::operator>>(optional<T>& rhs)
{
  if (mDeserializer.BeginOptional())
  {
    T value;
    *this >> value;
    rhs = std::move(value);
  }
  else
  {
    rhs = optional<T>();
  }

  mDeserializer.EndOptional();
  return *this;
}

template <typename T, typename std::enable_if<std::is_enum<T>::value, int>::type>
Deserializer& Deserializer::operator>>(T& rhs)
{
  typename std::underlying_type<T>::type value;
  *this >> value;
  rhs = static_cast<T>(value);
  return *this;
}

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
