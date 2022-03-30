// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "TypeLib/Deserializer.hpp"

namespace Vector {
namespace CANoe {
namespace TypeLib {

Deserializer::Deserializer(Cla::IDeserializer& deserializer) : mDeserializer(deserializer)
{
}

Deserializer& Deserializer::operator>>(float& rhs)
{
  rhs = mDeserializer.DeserializeFloat();
  return *this;
}

Deserializer& Deserializer::operator>>(double& rhs)
{
  rhs = mDeserializer.DeserializeDouble();
  return *this;
}

Deserializer& Deserializer::operator>>(bool& rhs)
{
  rhs = mDeserializer.DeserializeBool();
  return *this;
}

Deserializer& Deserializer::operator>>(int8_t& rhs)
{
  rhs = mDeserializer.DeserializeInt8(8);
  return *this;
}

Deserializer& Deserializer::operator>>(int16_t& rhs)
{
  rhs = mDeserializer.DeserializeInt16(16);
  return *this;
}

Deserializer& Deserializer::operator>>(int32_t& rhs)
{
  rhs = mDeserializer.DeserializeInt32(32);
  return *this;
}

Deserializer& Deserializer::operator>>(int64_t& rhs)
{
  rhs = mDeserializer.DeserializeInt64(64);
  return *this;
}

Deserializer& Deserializer::operator>>(uint8_t& rhs)
{
  rhs = mDeserializer.DeserializeUint8(8);
  return *this;
}

Deserializer& Deserializer::operator>>(uint16_t& rhs)
{
  rhs = mDeserializer.DeserializeUint16(16);
  return *this;
}

Deserializer& Deserializer::operator>>(uint32_t& rhs)
{
  rhs = mDeserializer.DeserializeUint32(32);
  return *this;
}

Deserializer& Deserializer::operator>>(uint64_t& rhs)
{
  rhs = mDeserializer.DeserializeUint64(64);
  return *this;
}

Deserializer& Deserializer::operator>>(std::string& rhs)
{
  rhs = mDeserializer.DeserializeString();
  return *this;
}

class Deserializer& Deserializer::operator>>(std::vector<uint8_t>& rhs)
{
  rhs = mDeserializer.DeserializeBytes();
  return *this;
}

Deserializer& Deserializer::operator>>(const BeginStruct&)
{
  mDeserializer.BeginStruct();
  return *this;
}

Deserializer& Deserializer::operator>>(const EndStruct&)
{
  mDeserializer.EndStruct();
  return *this;
}

int Deserializer::operator>>(const BeginUnion&) { return mDeserializer.BeginUnion(); }

Deserializer& Deserializer::operator>>(const EndUnion&)
{
  mDeserializer.EndUnion();
  return *this;
}

} // namespace TypeLib
} // namespace CANoe
} // namespace Vector
