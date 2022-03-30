/* -*-c++-*- OpenRTI - Copyright (C) 2011-2016 Mathias Froehlich
 *
 * This file is part of OpenRTI.
 *
 * OpenRTI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * OpenRTI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenRTI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "DebugNew.h"
#include <RTI/encoding/BasicDataElements.h>

#include <RTI/VariableLengthData.h>
#include <RTI/encoding/DataElement.h>

#include <cstring>
#include <limits>
#include <vector>

#include "Encoding.h"
#include "Export.h"
#include "Types.h"
//#include "VariableLengthDataImplementation.h"

#pragma warning(disable: 4018)

namespace rti1516ev
{

#define IMPLEMENT_ENCODING_HELPER_CLASS(EncodableDataType, SimpleDataType, encoder) \
                                                                        \
class OPENRTI_LOCAL EncodableDataType##Implementation {                 \
public:                                                                 \
  EncodableDataType##Implementation() :                                 \
    _value()                                                            \
  {                                                                     \
    _valuePointer = & _value;                                           \
  }                                                                     \
  EncodableDataType##Implementation(const SimpleDataType& value) :      \
    _value(value)                                                       \
  {                                                                     \
    _valuePointer = & _value;                                           \
  }                                                                     \
  EncodableDataType##Implementation(SimpleDataType* valuePointer) :     \
    _value()                                                            \
  {                                                                     \
    _valuePointer = valuePointer;                                       \
  }                                                                     \
  EncodableDataType##Implementation(const EncodableDataType##Implementation& rhs) : \
    _value(rhs._value)                                                  \
  {                                                                     \
    if (rhs._valuePointer != &rhs._value) {                             \
      _valuePointer = rhs._valuePointer;                                \
    } else {                                                            \
      _valuePointer = &_value;                                          \
    }                                                                   \
  }                                                                     \
  virtual ~EncodableDataType##Implementation() noexcept                 \
  { }                                                                   \
                                                                        \
  EncodableDataType##Implementation&                                    \
  operator=(const EncodableDataType##Implementation& rhs)               \
  { set(rhs.get()); return *this; }                                     \
                                                                        \
  encoder                                                               \
                                                                        \
  void set(const SimpleDataType& inData)                                \
  {                                                                     \
    *_valuePointer = inData;                                            \
  }                                                                     \
                                                                        \
  void setDataPointer(SimpleDataType* inData)                           \
  {                                                                     \
    _valuePointer = inData;                                             \
  }                                                                     \
                                                                        \
  const SimpleDataType& get() const                                     \
  {                                                                     \
    return *_valuePointer;                                              \
  }                                                                     \
                                                                        \
  SimpleDataType _value;                                                \
  SimpleDataType* _valuePointer;                                        \
};                                                                      \
                                                                        \
EncodableDataType::EncodableDataType() :                                \
  _impl(new EncodableDataType##Implementation)                          \
{                                                                       \
}                                                                       \
                                                                        \
EncodableDataType::EncodableDataType(SimpleDataType const& inData) :    \
  _impl(new EncodableDataType##Implementation(inData))                  \
{                                                                       \
}                                                                       \
                                                                        \
EncodableDataType::EncodableDataType(SimpleDataType* inData) :          \
  _impl(new EncodableDataType##Implementation(inData))                  \
{                                                                       \
}                                                                       \
                                                                        \
EncodableDataType::EncodableDataType(EncodableDataType const& rhs) :    \
  _impl(new EncodableDataType##Implementation(*rhs._impl))              \
{                                                                       \
}                                                                       \
                                                                        \
EncodableDataType::~EncodableDataType() noexcept                        \
{                                                                       \
  delete _impl;                                                         \
}                                                                       \
                                                                        \
EncodableDataType&                                                      \
EncodableDataType::operator=(EncodableDataType const& rhs)              \
{                                                                       \
  *_impl = *rhs._impl;                                                  \
  return *this;                                                         \
}                                                                       \
                                                                        \
std::unique_ptr<DataElement>                                            \
EncodableDataType::clone() const                                        \
{                                                                       \
  return std::unique_ptr<DataElement>(new EncodableDataType(*this));    \
}                                                                       \
                                                                        \
VariableLengthData                                                      \
EncodableDataType::encode() const                                       \
{                                                                       \
  VariableLengthData variableLengthData;                                \
  encode(variableLengthData);                                           \
  return variableLengthData;                                            \
}                                                                       \
                                                                        \
void                                                                    \
EncodableDataType::encode(VariableLengthData& outData) const            \
{                                                                       \
  outData.resize(getEncodedLength());                                   \
  _impl->encodeInto(static_cast<Octet*>(outData.data()),                \
                    outData.size(), 0);                                 \
}                                                                       \
                                                                        \
void                                                                    \
EncodableDataType::encodeInto(std::vector<Octet>& buffer) const         \
{                                                                       \
  size_t offset = buffer.size();                                        \
  size_t encodedLength = getEncodedLength();                            \
  buffer.resize(offset + encodedLength);                                \
  _impl->encodeInto(static_cast<Octet*>(buffer.data()) + offset,        \
                    encodedLength, offset);                             \
}                                                                       \
                                                                        \
size_t                                                                  \
EncodableDataType::encodeInto(                                          \
  Octet* buffer, size_t bufferSize, size_t offset) const                \
{                                                                       \
  return _impl->encodeInto(buffer, bufferSize, offset);                 \
}                                                                       \
                                                                        \
void                                                                    \
EncodableDataType::decode(VariableLengthData const & inData)            \
{                                                                       \
  _impl->decodeFrom(static_cast<const Octet*>(inData.data()),           \
                   inData.size(), 0);                                   \
}                                                                       \
                                                                        \
size_t                                                                  \
EncodableDataType::decodeFrom(std::vector<Octet> const & buffer, size_t index) \
{                                                                       \
  return _impl->decodeFrom(buffer.data(), buffer.size(), index);        \
}                                                                       \
                                                                        \
size_t                                                                  \
EncodableDataType::decodeFrom(const Octet* buffer, size_t bufferSize, size_t index) \
{                                                                       \
  return _impl->decodeFrom(buffer, bufferSize, index);                   \
}                                                                       \
                                                                        \
size_t                                                                  \
EncodableDataType::getEncodedLength() const                             \
{                                                                       \
  return _impl->getEncodedLength();                                     \
}                                                                       \
                                                                        \
unsigned int                                                            \
EncodableDataType::getOctetBoundary () const                            \
{                                                                       \
  return _impl->getOctetBoundary();                                     \
}                                                                       \
                                                                        \
Integer64                                                               \
EncodableDataType::hash() const                                         \
{                                                                       \
  return _impl->hash();                                                 \
}                                                                       \
                                                                        \
void                                                                    \
EncodableDataType::setDataPointer(SimpleDataType* inData)               \
{                                                                       \
  _impl->setDataPointer(inData);                                        \
}                                                                       \
                                                                        \
void                                                                    \
EncodableDataType::set(SimpleDataType inData)                           \
{                                                                       \
  _impl->set(inData);                                                   \
}                                                                       \
                                                                        \
SimpleDataType                                                          \
EncodableDataType::get() const                                          \
{                                                                       \
  return _impl->get();                                                  \
}                                                                       \
                                                                        \
EncodableDataType&                                                      \
EncodableDataType::operator=(SimpleDataType rhs)                        \
{                                                                       \
  _impl->set(rhs);                                                      \
  return *this;                                                         \
}                                                                       \
                                                                        \
EncodableDataType::operator SimpleDataType() const                      \
{                                                                       \
  return _impl->get();                                                  \
}

// 8 bit values
IMPLEMENT_ENCODING_HELPER_CLASS(HLAASCIIchar, char,
size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  if (index >= bufferSize)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  *_valuePointer = buffer[index];
  return index + 1;
}

size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  *(buffer + offset) = *_valuePointer;
  return offset + 1;
}

size_t
getEncodedLength() const
{
  return 1;
}

unsigned int
getOctetBoundary() const
{
  return 1;
}

Integer64 hash() const
{
  return Integer64(*_valuePointer);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAboolean, bool,
size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  if (index >= bufferSize)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  *_valuePointer = buffer[index] != 0;
  return index + 1;
}

size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  *(buffer + offset) = *_valuePointer;
  return offset + 1;
}

size_t
getEncodedLength() const
{
  return 1;
}

unsigned int
getOctetBoundary() const
{
  return 1;
}

Integer64 hash() const
{
  return Integer64(*_valuePointer);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAbyte, Octet,

size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  if (index >= bufferSize)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  *_valuePointer = buffer[index];
  return index + 1;
}

size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  *(buffer + offset) = *_valuePointer;
  return offset + 1;
}

size_t
getEncodedLength() const
{
  return 1;
}

unsigned int
getOctetBoundary() const
{
  return 1;
}

Integer64 hash() const
{
  return Integer64(*_valuePointer);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAoctet, Octet,
size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  if (index >= bufferSize)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  *_valuePointer = buffer[index];
  return index + 1;
}

size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  *(buffer + offset) = *_valuePointer;
  return offset + 1;
}

size_t
getEncodedLength() const
{
  return 1;
}

unsigned int
getOctetBoundary() const
{
  return 1;
}

Integer64 hash() const
{
  return Integer64(*_valuePointer);
}
)

// > 8 bit values, fixed endianess
IMPLEMENT_ENCODING_HELPER_CLASS(HLAunicodeChar, wchar_t,
size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  if (bufferSize < index + 2)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  uint16_t u;
  index = decodeFromBE16(buffer, bufferSize, index, u);
  *_valuePointer = wchar_t(u);
  return index;
}

size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  return encodeIntoBE16(buffer, bufferSize, offset, uint16_t(*_valuePointer));
}

size_t
getEncodedLength() const
{
  return 2;
}

unsigned int
getOctetBoundary() const
{
  return 2;
}

Integer64 hash() const
{
  return Integer64(*_valuePointer);
}
)

// > 8 bit values, both endianess
IMPLEMENT_ENCODING_HELPER_CLASS(HLAinteger16BE, Integer16,
size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  if (bufferSize < index + 2)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  uint16_t u;
  index = decodeFromBE16(buffer, bufferSize, index, u);
  *_valuePointer = Integer16(u);
  return index;
}

size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  return encodeIntoBE16(buffer, bufferSize, offset, uint16_t(*_valuePointer));
}

size_t
getEncodedLength() const
{
  return 2;
}

unsigned int
getOctetBoundary() const
{
  return 2;
}

Integer64 hash() const
{
  return Integer64(*_valuePointer);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAinteger16LE, Integer16,
size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  if (bufferSize < index + 2)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  uint16_t u;
  index = decodeFromLE16(buffer, bufferSize, index, u);
  *_valuePointer = Integer16(u);
  return index;
}

size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  return encodeIntoLE16(buffer, bufferSize, offset, uint16_t(*_valuePointer));
}

size_t
getEncodedLength() const
{
  return 2;
}

unsigned int
getOctetBoundary() const
{
  return 2;
}

Integer64 hash() const
{
  return Integer64(*_valuePointer);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAinteger32BE, Integer32,
size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  if (bufferSize < index + 4)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  uint32_t u;
  index = decodeFromBE32(buffer, bufferSize, index, u);
  *_valuePointer = Integer32(u);
  return index;
}

size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  return encodeIntoBE32(buffer, bufferSize, offset, uint32_t(*_valuePointer));
}


size_t
getEncodedLength() const
{
  return 4;
}

unsigned int
getOctetBoundary() const
{
  return 4;
}

Integer64 hash() const
{
  return Integer64(*_valuePointer);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAinteger32LE, Integer32,
size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  if (bufferSize < index + 4)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  uint32_t u;
  index = decodeFromLE32(buffer, bufferSize, index, u);
  *_valuePointer = Integer32(u);
  return index;
}

size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  return encodeIntoLE32(buffer, bufferSize, offset, uint32_t(*_valuePointer));
}

size_t
getEncodedLength() const
{
  return 4;
}

unsigned int
getOctetBoundary() const
{
  return 4;
}

Integer64 hash() const
{
  return Integer64(*_valuePointer);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAinteger64BE, Integer64,
size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  if (bufferSize < index + 8)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  uint64_t u;
  index = decodeFromBE64(buffer, bufferSize, index, u);
  *_valuePointer = Integer64(u);
  return index;
}

size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  return encodeIntoBE64(buffer, bufferSize, offset, uint64_t(*_valuePointer));
}

size_t
getEncodedLength() const
{
  return 8;
}

unsigned int
getOctetBoundary() const
{
  return 8;
}

Integer64 hash() const
{
  return Integer64(*_valuePointer);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAinteger64LE, Integer64,
size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  if (bufferSize < index + 8)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  uint64_t u;
  index = decodeFromLE64(buffer, bufferSize, index, u);
  *_valuePointer = Integer64(u);
  return index;
}

size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  return encodeIntoLE64(buffer, bufferSize, offset, uint64_t(*_valuePointer));
}

size_t
getEncodedLength() const
{
  return 8;
}

unsigned int
getOctetBoundary() const
{
  return 8;
}

Integer64 hash() const
{
  return Integer64(*_valuePointer);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAoctetPairBE, OctetPair,

size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  if (bufferSize < index + 2)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _valuePointer->first = buffer[index];
  _valuePointer->second = buffer[index + 1];
  return index + 2;
}

size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  auto* p = buffer + offset;
  *p++ = _valuePointer->first;
  *p++ = _valuePointer->second;
  return offset + 2;
}


size_t
getEncodedLength() const
{
  return 2;
}

unsigned int
getOctetBoundary() const
{
  return 2;
}

Integer64 hash() const
{
  return (Integer64(_valuePointer->first) << 8) | Integer64(_valuePointer->second);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAoctetPairLE, OctetPair,
size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  if (bufferSize < index + 2)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _valuePointer->second = buffer[index];
  _valuePointer->first = buffer[index + 1];
  return index + 2;
}

size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  auto* p = buffer + offset;
  *p++ = _valuePointer->second;
  *p++ = _valuePointer->first;
  return offset + 2;
}

size_t
getEncodedLength() const
{
  return 2;
}

unsigned int
getOctetBoundary() const
{
  return 2;
}

Integer64 hash() const
{
  return (Integer64(_valuePointer->first) << 8) | Integer64(_valuePointer->second);
}
)

// float, > 8 bit values, both endianess
IMPLEMENT_ENCODING_HELPER_CLASS(HLAfloat32BE, float,
size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  if (bufferSize < index + 4)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  union {
    uint32_t u;
    float s;
  } u;
  index = decodeFromBE32(buffer, bufferSize, index, u.u);
  *_valuePointer = u.s;
  return index;
}

size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  union {
    uint32_t u;
    float s;
  } u;
  u.s = *_valuePointer;
  return encodeIntoBE32(buffer, bufferSize, offset, u.u);
}


size_t
getEncodedLength() const
{
  return 4;
}

unsigned int
getOctetBoundary() const
{
  return 4;
}

Integer64 hash() const
{
  union {
    uint32_t u;
    float s;
  } u;
  u.s = *_valuePointer;
  return Integer64(u.u);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAfloat32LE, float,
size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  if (bufferSize < index + 4)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  union {
    uint32_t u;
    float s;
  } u;
  index = decodeFromLE32(buffer, bufferSize, index, u.u);
  *_valuePointer = u.s;
  return index;
}

size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  union {
    uint32_t u;
    float s;
  } u;
  u.s = *_valuePointer;
  return encodeIntoLE32(buffer, bufferSize, offset, u.u);
}

size_t
getEncodedLength() const
{
  return 4;
}

unsigned int
getOctetBoundary() const
{
  return 4;
}

Integer64 hash() const
{
  union {
    uint32_t u;
    float s;
  } u;
  u.s = *_valuePointer;
  return Integer64(u.u);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAfloat64BE, double,
size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  if (bufferSize < index + 8)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  union {
    uint64_t u;
    double s;
  } u;
  index = decodeFromBE64(buffer, bufferSize, index, u.u);
  *_valuePointer = u.s;
  return index;
}

size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  union {
    uint64_t u;
    double s;
  } u;
  u.s = *_valuePointer;
  return encodeIntoBE64(buffer, bufferSize, offset, u.u);
}


size_t
getEncodedLength() const
{
  return 8;
}

unsigned int
getOctetBoundary() const
{
  return 8;
}

Integer64 hash() const
{
  union {
    uint64_t u;
    double s;
  } u;
  u.s = *_valuePointer;
  return Integer64(u.u);
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAfloat64LE, double,
size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  if (bufferSize < index + 8)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  union {
    uint64_t u;
    double s;
  } u;
  index = decodeFromLE64(buffer, bufferSize, index, u.u);
  *_valuePointer = u.s;
  return index;
}

size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  union {
    uint64_t u;
    double s;
  } u;
  u.s = *_valuePointer;
  return encodeIntoLE64(buffer, bufferSize, offset, u.u);
}

size_t
getEncodedLength() const
{
  return 8;
}

unsigned int
getOctetBoundary() const
{
  return 8;
}

Integer64 hash() const
{
  union {
    uint64_t u;
    double s;
  } u;
  u.s = *_valuePointer;
  return Integer64(u.u);
}
)

// Array values
IMPLEMENT_ENCODING_HELPER_CLASS(HLAASCIIstring, std::string,
size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  if (bufferSize < index + 4)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  uint32_t u;
  index = decodeFromBE32(buffer, bufferSize, index, u);
  Integer32 length = Integer32(u);
  if (length < 0)
    length = 0;

  if (bufferSize < index + length)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _valuePointer->clear();
  _valuePointer->reserve(length);
  while (0 < length--)
    _valuePointer->push_back(buffer[index++]);

  return index;
}

size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  size_t length = _valuePointer->size();
  if (std::numeric_limits<Integer32>::max() < length)
    length = std::numeric_limits<Integer32>::max();
  offset = encodeIntoBE32(buffer, bufferSize, offset, static_cast<int32_t>(length));
  memcpy(buffer + offset, _valuePointer->data(), static_cast<int32_t>(length));
  return offset + length;
}

size_t
getEncodedLength() const
{
  return 4 + _valuePointer->size();
}

unsigned int
getOctetBoundary() const
{
  return 4;
}

Integer64 hash() const
{
  Integer64 hash = 8191;
  for (std::string::const_iterator i = _valuePointer->begin(); i != _valuePointer->end(); ++i)
    hash = hash*Integer64(127) + Integer64(*i);
  return hash;
}
)
IMPLEMENT_ENCODING_HELPER_CLASS(HLAunicodeString, std::wstring,
size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  if (bufferSize < index + 4)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  uint32_t u;
  index = decodeFromBE32(buffer, bufferSize, index, u);
  Integer32 length = Integer32(u);
  if (length < 0)
    length = 0;

  if (bufferSize < index + 2*length)
    throw EncoderException(L"Insufficient buffer size for decoding!");
  _valuePointer->clear();
  _valuePointer->reserve(length);
  while (0 < length--) {
    uint16_t encodedChar;
    encodedChar = uint16_t(uint8_t(buffer[index])) << 8;
    encodedChar |= uint16_t(uint8_t(buffer[index + 1]));
    _valuePointer->push_back(wchar_t(encodedChar));
    index += 2;
  }

  return index;
}

size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  size_t length = _valuePointer->size();
  if (std::numeric_limits<Integer32>::max() < length)
    length = std::numeric_limits<Integer32>::max();
  offset = encodeIntoBE32(buffer, bufferSize, offset, static_cast<uint32_t>(length));
  for (size_t i = 0; i < length; ++i) {
    uint16_t unicodeChar = uint16_t((*_valuePointer)[i]);
    offset = encodeIntoBE16(buffer, bufferSize, offset, unicodeChar);
  }
  return offset;
}

size_t
getEncodedLength() const
{
  return 4 + 2*_valuePointer->size();
}

unsigned int
getOctetBoundary() const
{
  return 4;
}

Integer64 hash() const
{
  Integer64 hash = 8191;
  for (std::wstring::const_iterator i = _valuePointer->begin(); i != _valuePointer->end(); ++i)
    hash = hash*Integer64(127) + Integer64(*i);
  return hash;
}
)

}
