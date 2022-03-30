// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace Vector {
namespace CANoe {
namespace Cla {

class IDeserializer
{
public:
  virtual ~IDeserializer() = default;

  //! \brief Begin a new deserialization process
  virtual void BeginDeserialization() = 0;
  //! \brief End the current deserialization process
  virtual void EndDeserialization() = 0;

  // ----------------------------------------
  // Scalar data types
  // ----------------------------------------
  // integer
  virtual int8_t DeserializeInt8(std::size_t bitSize) = 0;
  virtual int16_t DeserializeInt16(std::size_t bitSize) = 0;
  virtual int32_t DeserializeInt32(std::size_t bitSize) = 0;
  virtual int64_t DeserializeInt64(std::size_t bitSize) = 0;
  // Unsigned integer
  virtual uint8_t DeserializeUint8(std::size_t bitSize) = 0;
  virtual uint16_t DeserializeUint16(std::size_t bitSize) = 0;
  virtual uint32_t DeserializeUint32(std::size_t bitSize) = 0;
  virtual uint64_t DeserializeUint64(std::size_t bitSize) = 0;
  // Bool
  virtual bool DeserializeBool() = 0;
  // Floating point
  virtual float DeserializeFloat() = 0;
  virtual double DeserializeDouble() = 0;

  // ----------------------------------------
  // Complex data types
  // ----------------------------------------
  // Strings, encoded in UTF-8
  virtual std::string DeserializeString() = 0;

  // Bytes
  virtual std::vector<std::uint8_t> DeserializeBytes() = 0;
  
  // Structs
  virtual void BeginStruct() = 0;
  virtual void EndStruct() = 0;
  // Arrays
  //! \returns the number of elements in the array
  virtual std::size_t BeginArray() = 0;
  virtual void EndArray() = 0;
  // Optionals
  //! \returns if the optional type has a value
  virtual bool BeginOptional() = 0;
  virtual void EndOptional() = 0;
  // Unions
  //! \returns the 1-based index into the corresponding union. I.e., a return value of 0 indicates an invalid type.
  virtual int BeginUnion() = 0;
  virtual void EndUnion() = 0;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
