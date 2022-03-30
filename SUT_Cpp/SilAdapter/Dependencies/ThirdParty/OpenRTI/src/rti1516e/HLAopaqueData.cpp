/* -*-c++-*- OpenRTI - Copyright (C) 2011-2015 Mathias Froehlich
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
#include <RTI/encoding/HLAopaqueData.h>

#include <RTI/VariableLengthData.h>
#include <RTI/encoding/DataElement.h>

#include <cstring>
#include <vector>

#include "Export.h"

namespace rti1516e
{

class OPENRTI_LOCAL HLAopaqueDataImplementation {
public:
  HLAopaqueDataImplementation()
  {
  }
  HLAopaqueDataImplementation(const Octet* inData, size_t dataSize) :
    _buffer(inData, inData + dataSize)
  {
  }
  HLAopaqueDataImplementation(Octet** inData, size_t bufferSize, size_t dataSize)
  {
    // FIXME no clue if this is right!!
    if (!dataSize) {
      _buffer.clear();
    } else {
      set(*inData, dataSize);
    }
    if (*inData)
      delete *inData;
    inData = 0;
  }
  HLAopaqueDataImplementation(const HLAopaqueDataImplementation& rhs) :
    _buffer(rhs._buffer)
  {
  }
  ~HLAopaqueDataImplementation()
  {
  }

  void encodeInto(std::vector<Octet>& buffer) const
  {
    buffer.insert(buffer.end(), _buffer.begin(), _buffer.end());
  }

  size_t decodeFrom(std::vector<Octet> const & buffer, size_t index)
  {
    std::vector<Octet>::const_iterator i = buffer.begin();
    std::advance(i, index);
    _buffer.clear();
    _buffer.insert(_buffer.end(), i, buffer.end());
    return buffer.size();
  }

  size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
  {
    memcpy(buffer + offset, _buffer.data(), _buffer.size());
    return offset + _buffer.size();
  }

  size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
  {
    const Octet* i = buffer + index;
    _buffer.clear();
    _buffer.insert(_buffer.end(), i, buffer + bufferSize);
    return bufferSize;
  }

  void setDataPointer(Octet** inData, size_t bufferSize, size_t dataSize)
  {
    // FIXME no clue if this is right!!
    if (!dataSize) {
      _buffer.clear();
    } else {
      set(*inData, dataSize);
    }
    if (*inData)
      delete *inData;
    inData = 0;
  }

  void set(const Octet* inData, size_t dataSize)
  {
    _buffer.reserve(dataSize);
    if (!dataSize)
      return;
    _buffer.assign(inData, inData + dataSize);
  }

  const Octet* get() const
  {
    if (_buffer.empty())
      return 0;
    return &_buffer.front();
  }

  std::vector<Octet> _buffer;
};

HLAopaqueData::HLAopaqueData() :
  _impl(new HLAopaqueDataImplementation())
{
}

HLAopaqueData::HLAopaqueData(const Octet* inData, size_t dataSize) :
  _impl(new HLAopaqueDataImplementation(inData, dataSize))
{
}

HLAopaqueData::HLAopaqueData(Octet** inData, size_t bufferSize, size_t dataSize) :
  _impl(new HLAopaqueDataImplementation(inData, bufferSize, dataSize))
{
}

HLAopaqueData::HLAopaqueData(HLAopaqueData const & rhs) :
  _impl(new HLAopaqueDataImplementation(*rhs._impl))
{
}

HLAopaqueData::~HLAopaqueData() noexcept
{
  delete _impl;
  _impl = 0;
}

std::unique_ptr<DataElement>
HLAopaqueData::clone () const
{
  return std::unique_ptr<DataElement>(new HLAopaqueData(*this));
}

VariableLengthData
HLAopaqueData::encode () const
{
  VariableLengthData variableLengthData;
  encode(variableLengthData);
  return variableLengthData;
}

void
HLAopaqueData::encode(VariableLengthData& inData) const
{
  std::vector<Octet> buffer;
  buffer.reserve(getEncodedLength());
  encodeInto(buffer);
  if (buffer.size() > 0) inData.setData(&buffer.front(), buffer.size());
}

void
HLAopaqueData::encodeInto(std::vector<Octet>& buffer) const
{
  _impl->encodeInto(buffer);
}


size_t HLAopaqueData::encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  return _impl->encodeInto(buffer, bufferSize, offset);
}

void HLAopaqueData::decode(VariableLengthData const & inData)
{
  _impl->decodeFrom(static_cast<const Octet*>(inData.data()), inData.size(), 0);
}

size_t
HLAopaqueData::decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  return _impl->decodeFrom(buffer.data(), buffer.size(), index);
}


size_t HLAopaqueData::decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  return _impl->decodeFrom(buffer, bufferSize, index);
}

size_t
HLAopaqueData::getEncodedLength() const
{
  return _impl->_buffer.size();
}

unsigned int
HLAopaqueData::getOctetBoundary() const
{
  return 1;
}

size_t
HLAopaqueData::bufferLength() const
{
  return _impl->_buffer.size();
}

size_t
HLAopaqueData::dataLength() const
{
  return _impl->_buffer.size();
}

void
HLAopaqueData::setDataPointer(Octet** inData, size_t bufferSize, size_t dataSize)
{
  _impl->setDataPointer(inData, bufferSize, dataSize);
}

void
HLAopaqueData::set(const Octet* inData, size_t dataSize)
{
  _impl->set(inData, dataSize);
}

const Octet*
HLAopaqueData::get() const
{
  return _impl->get();
}

HLAopaqueData::operator const Octet*() const
{
  return _impl->get();
}

}
