// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>
#include <mutex>
#include <vector>

#include "cla/IDeserializer.hpp"

#include "SabSerDes/Deserializer.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Implementation of the 'IDeserializer' interface using the 'Sab::Deserializer'. */
class Deserializer : public IDeserializer
{
public:
  // -------------------
  // Deserializer
  // -------------------

  /*! \brief Instantiates a new deserializer.
   *  \param onBegin The callback which shall be called when the deserialization
   *                 begins. Must not be `nullptr` and it must return the data
   *                 buffer, which shall be deserialized. */
  Deserializer(std::function<std::vector<uint8_t>()> onBegin);

  // -------------------
  // IDeserializer
  // -------------------

  void BeginDeserialization() override;
  void EndDeserialization() override;

  int8_t DeserializeInt8(std::size_t bitSize) override;
  int16_t DeserializeInt16(std::size_t bitSize) override;
  int32_t DeserializeInt32(std::size_t bitSize) override;
  int64_t DeserializeInt64(std::size_t bitSize) override;

  uint8_t DeserializeUint8(std::size_t bitSize) override;
  uint16_t DeserializeUint16(std::size_t bitSize) override;
  uint32_t DeserializeUint32(std::size_t bitSize) override;
  uint64_t DeserializeUint64(std::size_t bitSize) override;

  bool DeserializeBool() override;

  float DeserializeFloat() override;
  double DeserializeDouble() override;

  std::string DeserializeString() override;

  std::vector<std::uint8_t> DeserializeBytes() override;

  void BeginStruct() override;
  void EndStruct() override;

  std::size_t BeginArray() override;
  void EndArray() override;

  bool BeginOptional() override;
  void EndOptional() override;

  int BeginUnion() override;
  void EndUnion() override;

private:
  /*! \brief Type used to serialize the size of lists / arrays or indices of unions. */
  using SizeType = uint32_t; // CANoe expects always 32-bit!

  // Private members
  // -------------------

  Sab::Deserializer mDeserializer;
  std::mutex mMutex;
  std::function<std::vector<uint8_t>()> mOnBeginDeserialization = nullptr;
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
