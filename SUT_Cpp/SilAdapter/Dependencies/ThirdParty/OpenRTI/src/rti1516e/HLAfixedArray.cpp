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
#include <RTI/encoding/HLAfixedArray.h>

#include <RTI/VariableLengthData.h>
#include <RTI/encoding/DataElement.h>

#include <cstring>
#include <vector>

#include "Encoding.h"
#include "Export.h"

namespace rti1516e
{

class OPENRTI_LOCAL HLAfixedArrayImplementation {
public:
  HLAfixedArrayImplementation(const DataElement& protoType, size_t length)
  {
    _protoType = protoType.clone().release();
    _dataElementVector.resize(length);
    for (DataElementVector::iterator i = _dataElementVector.begin(); i != _dataElementVector.end(); ++i)
    {
      i->first = _protoType->clone().release();
      i->second = true;
    }
  }
  HLAfixedArrayImplementation(const HLAfixedArrayImplementation& rhs)
  {
    _protoType = rhs._protoType->clone().release();
    _dataElementVector.resize(rhs._dataElementVector.size(), std::make_pair(nullptr, true));
    for (size_t i = 0; i < rhs._dataElementVector.size(); ++i) {
      _dataElementVector[i].first = rhs._dataElementVector[i].first->clone().release();
    }
  }
  virtual ~HLAfixedArrayImplementation() noexcept
  {
    for (DataElementVector::iterator i = _dataElementVector.begin(); i != _dataElementVector.end(); ++i) {
      if (i->second) delete i->first;
      i->first = nullptr;
    }
    delete _protoType;
    _protoType = 0;
  }

  size_t getEncodedLength() const
  {
    size_t length = 0;
    for (auto& element : _dataElementVector) {
      length += element.first->getEncodedLength();
    }
    return length;
  }

  size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset)
  {
#ifdef _DEBUG
    if (bufferSize < offset + getEncodedLength())
      throw EncoderException(L"buffer to small: bufferSize=" + std::to_wstring(bufferSize) + L" offset=" + std::to_wstring(offset) + L" encodedLength=" + std::to_wstring(getEncodedLength()));
#endif
    for (DataElementVector::const_iterator i = _dataElementVector.begin(); i != _dataElementVector.end(); ++i) {
      if (i->first == nullptr)
        throw EncoderException(L"HLAfixedArray::encodeInto(): dataElement is zero!");
      offset = i->first->encodeInto(buffer, bufferSize, offset);
    }
    return offset;
  }

  size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
  {
    for (DataElementVector::const_iterator i = _dataElementVector.begin(); i != _dataElementVector.end(); ++i) {
      if (i->first == nullptr)
        throw EncoderException(L"HLAfixedArray::decodeFrom(): dataElement is zero!");
      index = i->first->decodeFrom(buffer, bufferSize, index);
    }
    return index;
  }

  unsigned int getOctetBoundary() const
  {
    if (_dataElementVector.empty())
      return 1;
    return _dataElementVector.front().first->getOctetBoundary();
  }

  size_t size () const
  { return _dataElementVector.size(); }

  void set(size_t index, const DataElement& dataElement)
  {
    if (_dataElementVector.size() <= index)
      throw EncoderException(L"HLAfixedArray::set(): Index out of range!");
    auto& element = _dataElementVector[index];
    if (!dataElement.isSameTypeAs(*_protoType))
      throw EncoderException(L"HLAfixedArray::set(): Data type is not compatible!");
    if (element.second) delete element.first;
    element.first = dataElement.clone().release();
    element.second = true;
  }

  void setElementPointer(size_t index, DataElement* dataElement)
  {
    if (_dataElementVector.size() <= index)
      throw EncoderException(L"HLAfixedArray::setElementPointer(): Index out of range!");
    if (!dataElement)
      throw EncoderException(L"HLAfixedArray::setElementPointer(): dataElement is zero!");
    auto& element = _dataElementVector[index];
    if (!dataElement->isSameTypeAs(*_protoType))
      throw EncoderException(L"HLAfixedArray::setElementPointer(): Data type is not compatible!");
    if (element.second) delete element.first;
    element.first = dataElement;
    element.second = false;
  }

  const DataElement& get(size_t index) const
  {
    if (_dataElementVector.size() <= index)
      throw EncoderException(L"HLAfixedArray::get(): Index out of range!");
    auto& element = _dataElementVector[index];
    if (element.first == nullptr)
      throw EncoderException(L"HLAfixedArray::get(): dataElement is zero!");
    return *(_dataElementVector[index].first);
  }

  bool isSameTypeAs(const HLAfixedArrayImplementation& rhs) const
  {
    if (_dataElementVector.empty()) {
      return rhs._dataElementVector.empty();
    } else {
      if (rhs._dataElementVector.empty())
        return false;
      return _protoType->isSameTypeAs(*(rhs._dataElementVector.front().first));
    }
  }

  void resize(size_t length)
  {
    _dataElementVector.resize(length);
  }
  void clear()
  {
    _dataElementVector.clear();
  }
  // bool 'second' specifies whether the DataElement pointed is owned by this instance.
  typedef std::vector<std::pair<DataElement*, bool>> DataElementVector;
  DataElementVector _dataElementVector;
  DataElement* _protoType;
};

HLAfixedArray::HLAfixedArray(const DataElement& protoType, size_t length) :
  _impl(new HLAfixedArrayImplementation(protoType, length))
{
}

HLAfixedArray::HLAfixedArray(HLAfixedArray const & rhs) :
  _impl(new HLAfixedArrayImplementation(*rhs._impl))
{
}

HLAfixedArray::~HLAfixedArray() noexcept
{
  delete _impl;
  _impl = 0;
}

std::unique_ptr<DataElement>
HLAfixedArray::clone () const
{
  return std::unique_ptr<DataElement>(new HLAfixedArray(*this));
}

VariableLengthData
HLAfixedArray::encode () const
{
  VariableLengthData variableLengthData;
  encode(variableLengthData);
  return variableLengthData;
}

void
HLAfixedArray::encode(VariableLengthData& outData) const
{
  outData.resize(getEncodedLength());
  _impl->encodeInto(static_cast<Octet*>(outData.data()), outData.size(), 0);
}

void
HLAfixedArray::encodeInto(std::vector<Octet>& buffer) const
{
  size_t offset = buffer.size();
  size_t encodedLength = getEncodedLength();
  buffer.resize(offset + encodedLength);
  _impl->encodeInto(static_cast<Octet*>(buffer.data()) + offset, encodedLength, offset);
}


size_t HLAfixedArray::encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  return _impl->encodeInto(buffer, bufferSize, offset);
}

void HLAfixedArray::decode(VariableLengthData const & inData)
{
  _impl->decodeFrom(static_cast<const Octet*>(inData.data()), inData.size(), 0);
}

size_t
HLAfixedArray::decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  return _impl->decodeFrom(buffer.data(), buffer.size(), index);
}


size_t HLAfixedArray::decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  return _impl->decodeFrom(buffer, bufferSize, index);
}

size_t
HLAfixedArray::getEncodedLength() const
{
  return _impl->getEncodedLength();
}

unsigned int
HLAfixedArray::getOctetBoundary() const
{
  return _impl->getOctetBoundary();
}

bool
HLAfixedArray::isSameTypeAs(DataElement const& inData ) const
{
  if (!DataElement::isSameTypeAs(inData))
    return false;
  const HLAfixedArray* fixedArray = dynamic_cast<const HLAfixedArray*>(&inData);
  if (!fixedArray)
    return false;
  return _impl->isSameTypeAs(*fixedArray->_impl);
}

bool
HLAfixedArray::hasPrototypeSameTypeAs(DataElement const& dataElement) const
{
  const HLAfixedArray* fixedArray = dynamic_cast<const HLAfixedArray*>(&dataElement);
  if (!fixedArray)
    return false;
  return _impl->isSameTypeAs(*fixedArray->_impl);
}

size_t
HLAfixedArray::size () const
{
  return _impl->size();
}

void
HLAfixedArray::set(size_t index, const DataElement& dataElement)
{
  _impl->set(index, dataElement);
}

void
HLAfixedArray::setElementPointer(size_t index, DataElement* dataElement)
{
  _impl->setElementPointer(index, dataElement);
}

const DataElement&
HLAfixedArray::get(size_t index) const
{
  return _impl->get(index);
}

DataElement const&
HLAfixedArray::operator [](size_t index) const
{
  return _impl->get(index);
}


void HLAfixedArray::resize(size_t length)
{
  _impl->resize(length);
}

void HLAfixedArray::clear()
{
  _impl->clear();
}

}
