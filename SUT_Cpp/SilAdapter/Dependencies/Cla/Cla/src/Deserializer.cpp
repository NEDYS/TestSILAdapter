// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Deserializer.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

// -------------------
// Deserializer
// -------------------

Deserializer::Deserializer(std::function<std::vector<uint8_t>()> onBegin) : mOnBeginDeserialization(std::move(onBegin))
{
  if (!mOnBeginDeserialization)
  {
    throw std::invalid_argument("Deserializer callback mustn't be a null.");
  }
}

// -------------------
// IDeserializer
// -------------------

void Deserializer::BeginDeserialization()
{
  mMutex.lock();
  mDeserializer.Reset(mOnBeginDeserialization());
}

void Deserializer::EndDeserialization() { mMutex.unlock(); }

int8_t Deserializer::DeserializeInt8(std::size_t bitSize) { return mDeserializer.Deserialize<int8_t>(bitSize); }

int16_t Deserializer::DeserializeInt16(std::size_t bitSize) { return mDeserializer.Deserialize<int16_t>(bitSize); }

int32_t Deserializer::DeserializeInt32(std::size_t bitSize) { return mDeserializer.Deserialize<int32_t>(bitSize); }

int64_t Deserializer::DeserializeInt64(std::size_t bitSize) { return mDeserializer.Deserialize<int64_t>(bitSize); }

uint8_t Deserializer::DeserializeUint8(std::size_t bitSize) { return mDeserializer.Deserialize<uint8_t>(bitSize); }

uint16_t Deserializer::DeserializeUint16(std::size_t bitSize) { return mDeserializer.Deserialize<uint16_t>(bitSize); }

uint32_t Deserializer::DeserializeUint32(std::size_t bitSize) { return mDeserializer.Deserialize<uint32_t>(bitSize); }

uint64_t Deserializer::DeserializeUint64(std::size_t bitSize) { return mDeserializer.Deserialize<uint64_t>(bitSize); }

bool Deserializer::DeserializeBool() { return mDeserializer.Deserialize<bool>(); }

float Deserializer::DeserializeFloat() { return mDeserializer.Deserialize<float>(); }

double Deserializer::DeserializeDouble() { return mDeserializer.Deserialize<double>(); }

std::string Deserializer::DeserializeString() { return mDeserializer.Deserialize<std::string>(); }

std::vector<std::uint8_t> Deserializer::DeserializeBytes() { return mDeserializer.Deserialize<std::vector<uint8_t>>(); }

void Deserializer::BeginStruct() { mDeserializer.BeginStruct(); }

void Deserializer::EndStruct() { mDeserializer.EndStruct(); }

std::size_t Deserializer::BeginArray() { return mDeserializer.BeginArray(); }

void Deserializer::EndArray() { mDeserializer.EndArray(); }

bool Deserializer::BeginOptional() { return mDeserializer.BeginOptional(); }

void Deserializer::EndOptional() { mDeserializer.EndOptional(); }

int Deserializer::BeginUnion() { return mDeserializer.BeginUnion(); }

void Deserializer::EndUnion() { mDeserializer.EndUnion(); }

} // namespace Cla
} // namespace CANoe
} // namespace Vector
