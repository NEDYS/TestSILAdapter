// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "TypeLib/Serializer.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

Serializer::Serializer(Cla::ISerializer& serializer) : mSerializer(serializer) {}

Serializer& Serializer::operator<<(float rhs)
{
  mSerializer.SerializeFloat(rhs);
  return *this;
}

Serializer& Serializer::operator<<(double rhs)
{
  mSerializer.SerializeDouble(rhs);
  return *this;
}

Serializer& Serializer::operator<<(bool rhs)
{
  mSerializer.SerializeBool(rhs);
  return *this;
}

Serializer& Serializer::operator<<(int8_t rhs)
{
  mSerializer.SerializeInt8(rhs, 8);
  return *this;
}

Serializer& Serializer::operator<<(int16_t rhs)
{
  mSerializer.SerializeInt16(rhs, 16);
  return *this;
}

Serializer& Serializer::operator<<(int32_t rhs)
{
  mSerializer.SerializeInt32(rhs, 32);
  return *this;
}

Serializer& Serializer::operator<<(int64_t rhs)
{
  mSerializer.SerializeInt64(rhs, 64);
  return *this;
}

Serializer& Serializer::operator<<(uint8_t rhs)
{
  mSerializer.SerializeUint8(rhs, 8);
  return *this;
}

Serializer& Serializer::operator<<(uint16_t rhs)
{
  mSerializer.SerializeUint16(rhs, 16);
  return *this;
}

Serializer& Serializer::operator<<(uint32_t rhs)
{
  mSerializer.SerializeUint32(rhs, 32);
  return *this;
}

Serializer& Serializer::operator<<(uint64_t rhs)
{
  mSerializer.SerializeUint64(rhs, 64);
  return *this;
}

Serializer& Serializer::operator<<(const std::string& rhs)
{
  mSerializer.SerializeString(rhs);
  return *this;
}

Serializer& Serializer::operator<<(const std::vector<uint8_t>& rhs)
{
  mSerializer.SerializeBytes(rhs);
  return *this;
}

Serializer& Serializer::operator<<(const BeginStruct&)
{
  mSerializer.BeginStruct();
  return *this;
}

Serializer& Serializer::operator<<(const EndStruct&)
{
  mSerializer.EndStruct();
  return *this;
}

Serializer& Serializer::operator<<(const BeginUnion& token)
{
  mSerializer.BeginUnion(token.typeIndex);
  return *this;
}

Serializer& Serializer::operator<<(const EndUnion&)
{
  mSerializer.EndUnion();
  return *this;
}

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
