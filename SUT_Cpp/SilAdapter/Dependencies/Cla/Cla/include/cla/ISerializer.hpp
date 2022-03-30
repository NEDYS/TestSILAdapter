// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace Vector {
namespace CANoe {
namespace Cla {

class ISerializer
{
public:
  virtual ~ISerializer() = default;

  //! \brief Begin a new serialization process
  virtual void BeginSerialization() = 0;
  /*! \brief End the current serialization process
   *
   *  Commits the serialized data to the corresponding IValueEntity
   */
  virtual void EndSerialization() = 0;

  // ----------------------------------------
  // Scalar data types
  // ----------------------------------------
  // integer
  virtual void SerializeInt8(int8_t data, std::size_t bitSize) = 0;
  virtual void SerializeInt16(int16_t data, std::size_t bitSize) = 0;
  virtual void SerializeInt32(int32_t data, std::size_t bitSize) = 0;
  virtual void SerializeInt64(int64_t data, std::size_t bitSize) = 0;
  // Unsigned integer
  virtual void SerializeUint8(uint8_t data, std::size_t bitSize) = 0;
  virtual void SerializeUint16(uint16_t data, std::size_t bitSize) = 0;
  virtual void SerializeUint32(uint32_t data, std::size_t bitSize) = 0;
  virtual void SerializeUint64(uint64_t data, std::size_t bitSize) = 0;
  // Bool
  virtual void SerializeBool(bool data) = 0;
  // Floating point
  virtual void SerializeFloat(float data) = 0;
  virtual void SerializeDouble(double data) = 0;

  // ----------------------------------------
  // Complex data types
  // ----------------------------------------
  // Strings, encoded in UTF-8
  virtual void SerializeString(const std::string& string) = 0;
    
  // Bytes
  virtual void SerializeBytes(const std::vector<std::uint8_t>& bytes) = 0;

  // Structs
  virtual void BeginStruct() = 0;
  virtual void EndStruct() = 0;
  // Arrays
  virtual void BeginArray(std::size_t numElements) = 0;
  virtual void EndArray() = 0;
  // Optionals
  virtual void BeginOptional(bool isAvailable) = 0;
  virtual void EndOptional() = 0;
  // Unions
  //! \param typeIndex 1-based type index into the corresponding union. I.e., typeIndex = 0 is invalid type.
  virtual void BeginUnion(int typeIndex) = 0;
  virtual void EndUnion() = 0;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
