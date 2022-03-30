// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Serializer.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

// -------------------
// Serializer
// -------------------

Serializer::Serializer(std::function<void(std::vector<uint8_t>&)> onEnd) : mOnEndSerialization(std::move(onEnd)) {}

// -------------------
// ISerializer
// -------------------

void Serializer::BeginSerialization()
{
  mMutex.lock();
  mSerializer.Reset();
}

void Serializer::EndSerialization()
{
  if (mOnEndSerialization)
  {
    auto buffer = mSerializer.ReleaseBuffer();
    mOnEndSerialization(buffer);
  }
  mMutex.unlock();
}

void Serializer::SerializeInt8(int8_t data, std::size_t bitSize) { mSerializer.Serialize(data, bitSize); }

void Serializer::SerializeInt16(int16_t data, std::size_t bitSize) { mSerializer.Serialize(data, bitSize); }

void Serializer::SerializeInt32(int32_t data, std::size_t bitSize) { mSerializer.Serialize(data, bitSize); }

void Serializer::SerializeInt64(int64_t data, std::size_t bitSize) { mSerializer.Serialize(data, bitSize); }

void Serializer::SerializeUint8(uint8_t data, std::size_t bitSize) { mSerializer.Serialize(data, bitSize); }

void Serializer::SerializeUint16(uint16_t data, std::size_t bitSize) { mSerializer.Serialize(data, bitSize); }

void Serializer::SerializeUint32(uint32_t data, std::size_t bitSize) { mSerializer.Serialize(data, bitSize); }

void Serializer::SerializeUint64(uint64_t data, std::size_t bitSize) { mSerializer.Serialize(data, bitSize); }

void Serializer::SerializeBool(bool data) { mSerializer.Serialize(data); }

void Serializer::SerializeFloat(float data) { mSerializer.Serialize(data); }

void Serializer::SerializeDouble(double data) { mSerializer.Serialize(data); }

void Serializer::SerializeString(const std::string& string) { mSerializer.Serialize(string); }

void Serializer::SerializeBytes(const std::vector<std::uint8_t>& bytes) { mSerializer.Serialize(bytes); }

void Serializer::BeginStruct() { mSerializer.BeginStruct(); }

void Serializer::EndStruct() { mSerializer.EndStruct(); }

void Serializer::BeginArray(std::size_t numElements) { mSerializer.BeginArray(numElements); }

void Serializer::EndArray() { mSerializer.EndArray(); }

void Serializer::BeginOptional(bool isAvailable) { mSerializer.BeginOptional(isAvailable); }

void Serializer::EndOptional() { mSerializer.EndOptional(); }

void Serializer::BeginUnion(int typeIndex) { mSerializer.BeginUnion(typeIndex); }

void Serializer::EndUnion() { mSerializer.EndUnion(); }

} // namespace Cla
} // namespace CANoe
} // namespace Vector
