// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>
#include <mutex>
#include <vector>

#include "cla/ISerializer.hpp"

#include "SabSerDes/Serializer.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Implementation of the 'ISerializer' using the 'Sab::Serializer'. */
class Serializer : public ISerializer
{
public:
  // -------------------
  // Serializer
  // -------------------

  /*! \brief Instantiates a new serializer.
   *  \param onEnd The callback which shall be called when the serialization ends.
   *               The serializer buffer is passed as reference to the callback and can
   *               be moved or copied. */
  Serializer(std::function<void(std::vector<uint8_t>&)> onEnd);

  // -------------------
  // ISerializer
  // -------------------

  void BeginSerialization() override;
  void EndSerialization() override;

  void SerializeInt8(int8_t data, std::size_t bitSize) override;
  void SerializeInt16(int16_t data, std::size_t bitSize) override;
  void SerializeInt32(int32_t data, std::size_t bitSize) override;
  void SerializeInt64(int64_t data, std::size_t bitSize) override;

  void SerializeUint8(uint8_t data, std::size_t bitSize) override;
  void SerializeUint16(uint16_t data, std::size_t bitSize) override;
  void SerializeUint32(uint32_t data, std::size_t bitSize) override;
  void SerializeUint64(uint64_t data, std::size_t bitSize) override;

  void SerializeBool(bool data) override;

  void SerializeFloat(float data) override;
  void SerializeDouble(double data) override;

  void SerializeString(const std::string& string) override;

  void SerializeBytes(const std::vector<std::uint8_t>& bytes) override;

  void BeginStruct() override;
  void EndStruct() override;

  void BeginArray(std::size_t numElements) override;
  void EndArray() override;

  void BeginOptional(bool isAvailable) override;
  void EndOptional() override;

  void BeginUnion(int typeIndex) override;
  void EndUnion() override;

private:
  /*! \brief Type used to serialize the size of lists / arrays or indices of unions. */
  using SizeType = uint32_t; // CANoe expects always 32-bit!

  // Private members
  // -------------------

  Sab::Serializer mSerializer;
  std::mutex mMutex;
  std::function<void(std::vector<uint8_t>&)> mOnEndSerialization = nullptr;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
