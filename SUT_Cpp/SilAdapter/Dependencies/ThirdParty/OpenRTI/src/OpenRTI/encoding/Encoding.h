/* -*-c++-*- OpenRTI - Copyright (C) 2011-2012 Mathias Froehlich
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

#pragma once
namespace OpenRTI {

static inline size_t encodeIntoLE64(Octet* buffer, size_t bufferSize, size_t offset, uint64_t value)
{
  if (bufferSize < offset + 8)
  {
    throw EncoderException("provided buffer too small");
  }
  Octet* p = buffer + offset;
  *p++ = Octet(0xff & (value));
  *p++ = Octet(0xff & (value >> 8));
  *p++ = Octet(0xff & (value >> 16));
  *p++ = Octet(0xff & (value >> 24));
  *p++ = Octet(0xff & (value >> 32));
  *p++ = Octet(0xff & (value >> 40));
  *p++ = Octet(0xff & (value >> 48));
  *p++ = Octet(0xff & (value >> 56));
  return offset + 8;
}

static inline size_t encodeIntoBE64(Octet* buffer, size_t bufferSize, size_t offset, uint64_t value)
{
  if (bufferSize < offset + 8)
  {
    throw EncoderException("provided buffer too small");
  }
  Octet* p = buffer + offset;
  *p++ = Octet(0xff & (value >> 56));
  *p++ = Octet(0xff & (value >> 48));
  *p++ = Octet(0xff & (value >> 40));
  *p++ = Octet(0xff & (value >> 32));
  *p++ = Octet(0xff & (value >> 24));
  *p++ = Octet(0xff & (value >> 16));
  *p++ = Octet(0xff & (value >> 8));
  *p++ = Octet(0xff & (value));
  return offset + 8;
}

static inline size_t decodeFromBE64(const Octet* buffer, size_t bufferSize, size_t offset, uint64_t& value)
{
  if (bufferSize < offset + 8)
  {
    throw EncoderException("provided buffer too small");
  }
  const Octet* p = buffer + offset;
  value  = uint64_t(*p++) << 56;
  value |= uint64_t(*p++) << 48;
  value |= uint64_t(*p++) << 40;
  value |= uint64_t(*p++) << 32;
  value |= uint64_t(*p++) << 24;
  value |= uint64_t(*p++) << 16;
  value |= uint64_t(*p++) << 8;
  value |= uint64_t(*p++);
  return offset + 8;
}

static inline size_t decodeFromLE64(const Octet* buffer, size_t bufferSize, size_t offset, uint64_t& value)
{
  if (bufferSize < offset + 8)
  {
    throw EncoderException("provided buffer too small");
  }
  const Octet* p = buffer + offset;
  value  = uint64_t(*p++);
  value |= uint64_t(*p++) << 8;
  value |= uint64_t(*p++) << 16;
  value |= uint64_t(*p++) << 24;
  value |= uint64_t(*p++) << 32;
  value |= uint64_t(*p++) << 40;
  value |= uint64_t(*p++) << 48;
  value |= uint64_t(*p++) << 56;
  return offset + 8;
}

static inline size_t encodeIntoLE32(Octet* buffer, size_t bufferSize, size_t offset, uint32_t value)
{
  if (bufferSize < offset + 4)
  {
    throw EncoderException("provided buffer too small");
  }
  Octet* p = buffer + offset;
  *p++ = Octet(0xff & (value));
  *p++ = Octet(0xff & (value >> 8));
  *p++ = Octet(0xff & (value >> 16));
  *p++ = Octet(0xff & (value >> 24));
  return offset + 4;
}

static inline size_t encodeIntoBE32(Octet* buffer, size_t bufferSize, size_t offset, uint32_t value)
{
  if (bufferSize < offset + 4)
  {
    throw EncoderException("provided buffer too small");
  }
  Octet* p = buffer + offset;
  *p++ = Octet(0xff & (value >> 24));
  *p++ = Octet(0xff & (value >> 16));
  *p++ = Octet(0xff & (value >> 8));
  *p++ = Octet(0xff & (value));
  return offset + 4;
}

static inline size_t decodeFromBE32(const Octet* buffer, size_t bufferSize, size_t offset, uint32_t& value)
{
  if (bufferSize < offset + 4)
  {
    throw EncoderException("provided buffer too small");
  }
  const Octet* p = buffer + offset;
  value  = uint32_t(*p++) << 24;
  value |= uint32_t(*p++) << 16;
  value |= uint32_t(*p++) << 8;
  value |= uint32_t(*p++);
  return offset + 4;
}

static inline size_t decodeFromLE32(const Octet* buffer, size_t bufferSize, size_t offset, uint32_t& value)
{
  if (bufferSize < offset + 4)
  {
    throw EncoderException("provided buffer too small");
  }
  const Octet* p = buffer + offset;
  value  = uint32_t(*p++);
  value |= uint32_t(*p++) << 8;
  value |= uint32_t(*p++) << 16;
  value |= uint32_t(*p++) << 24;
  return offset + 4;
}

static inline size_t encodeIntoLE16(Octet* buffer, size_t bufferSize, size_t offset, uint16_t value)
{
  if (bufferSize < offset + 2)
  {
    throw EncoderException("provided buffer too small");
  }
  Octet* p = buffer + offset;
  *p++ = Octet(0xff & (value));
  *p++ = Octet(0xff & (value >> 8));
  return offset + 2;
}

static inline size_t encodeIntoBE16(Octet* buffer, size_t bufferSize, size_t offset, uint16_t value)
{
  if (bufferSize < offset + 2)
  {
    throw EncoderException("provided buffer too small");
  }
  Octet* p = buffer + offset;
  *p++ = Octet(0xff & (value >> 8));
  *p++ = Octet(0xff & (value));
  return offset + 2;
}

static inline size_t decodeFromBE16(const Octet* buffer, size_t bufferSize, size_t offset, uint16_t& value)
{
  if (bufferSize < offset + 2)
  {
    throw EncoderException("provided buffer too small");
  }
  const Octet* p = buffer + offset;
  value  = uint16_t(*p++) << 8;
  value |= uint16_t(*p++);
  return offset + 2;
}

static inline size_t decodeFromLE16(const Octet* buffer, size_t bufferSize, size_t offset, uint16_t& value)
{
  if (bufferSize < offset + 2)
  {
    throw EncoderException("provided buffer too small");
  }
  const Octet* p = buffer + offset;
  value  = uint16_t(*p++);
  value |= uint16_t(*p++) << 8;
  return offset + 2;
}

template<typename T>
static inline size_t decodeCompressed(const Octet* buffer, size_t bufferSize, size_t offset, T& value)
{
  unsigned shift = 7;
  if (bufferSize <= offset)
  {
    throw EncoderException("provided buffer too small");
  }
  const Octet* p = buffer + offset;
  uint8_t byte = *p++; offset++;
  value = (byte & 0x7f);
  while (byte & 0x80)
  {
    if (bufferSize <= offset)
    {
      throw EncoderException("provided buffer too small");
    }
    byte = *p++; offset++;
    value |= T(byte & 0x7f) << shift;
    shift += 7;
  }
  return offset;
}

template<typename T>
static inline size_t encodeCompressed(Octet* buffer, size_t bufferSize, size_t offset, T value)
{
  if (bufferSize <= offset)
  {
    throw EncoderException("provided buffer too small");
  }
  Octet* p = buffer + offset;
  while (value & (~T(0x7f)))
  {
    if (bufferSize <= offset)
    {
      throw EncoderException("provided buffer too small");
    }
    uint8_t byte = (0x80 | (0x7f & static_cast<uint8_t>(value)));
    *p++ = byte; offset++;
    value >>= 7;
  }
  *p++ = static_cast<uint8_t>(value); offset++;
  return offset;
}

static inline size_t decodeFromBE32Compressed(const Octet* buffer, size_t bufferSize, size_t offset, uint32_t& value)
{
  return decodeCompressed<uint32_t>(buffer, bufferSize, offset, value);
}

static inline size_t encodeIntoBE32Compressed(Octet* buffer, size_t bufferSize, size_t offset, uint32_t value)
{
  return encodeCompressed<uint32_t>(buffer, bufferSize, offset, value);
}

}