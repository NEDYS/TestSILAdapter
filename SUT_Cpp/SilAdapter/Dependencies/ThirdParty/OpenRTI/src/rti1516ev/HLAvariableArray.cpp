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
#include <RTI/encoding/HLAvariableArray.h>

#include <RTI/VariableLengthData.h>
#include <RTI/encoding/DataElement.h>

#include <algorithm>
#include <limits>
#include <cstring>
#include <vector>
#include <cassert>

#include "Encoding.h"
#include "Export.h"

namespace rti1516ev {

class OPENRTI_LOCAL HLAvariableArrayImplementation {
public:
  HLAvariableArrayImplementation(const DataElement& protoType)
  {
    _protoType = protoType.clone().release();
  }
  HLAvariableArrayImplementation(const HLAvariableArrayImplementation& rhs)
  {
    _protoType = rhs._protoType->clone().release();
    if (rhs._dataElementVector.size() > 0)
    {
      _dataElementVector.resize(rhs._dataElementVector.size(), std::make_pair(nullptr, true));
      for (size_t i = 0; i < rhs._dataElementVector.size(); ++i)
      {
        _dataElementVector[i].first = rhs._dataElementVector[i].first->clone().release();
      }
    }
  }
  ~HLAvariableArrayImplementation() noexcept
  {
    for (DataElementVector::iterator i = _dataElementVector.begin(); i != _dataElementVector.end(); ++i) {
      if (i->second) delete i->first;
      i->first = nullptr;
    }
    delete _protoType;
    _protoType = nullptr;
  }

  size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
  {
#ifdef _DEBUG
    if (bufferSize < offset + getEncodedLength())
      throw EncoderException(L"buffer to small: bufferSize=" + std::to_wstring(bufferSize) + L" offset=" + std::to_wstring(offset) + L" encodedLength=" + std::to_wstring(getEncodedLength()));
#endif
    size_t length = _dataElementVector.size();
    if (0xffffffffu < length)
      throw EncoderException(L"HLAvariableArray::encodeInto(): array size is too big to encode!");
    offset = encodeIntoBE32(buffer, bufferSize, offset, static_cast<uint32_t>(length));

    for (DataElementVector::const_iterator i = _dataElementVector.begin(); i != _dataElementVector.end(); ++i) {
      if (i->first == nullptr)
        throw EncoderException(L"HLAvariableArray::encodeInto(): dataElement is zero!");
      offset = i->first->encodeInto(buffer, bufferSize, offset);
    }
    return offset;
  }

  size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
  {
    if (bufferSize < index + 4)
      throw EncoderException(L"Insufficient buffer size for decoding!");
    uint32_t length;
    index = decodeFromBE32(buffer, bufferSize, index, length);

    // Shrink _dataElementVector if necessary, deleting owned elements.
    // Note that _dataElementVector contains DataElements.
    while (length < _dataElementVector.size()) {
      if (_dataElementVector.back().first) delete _dataElementVector.back().first;
      _dataElementVector.pop_back();
    }

    // Grow _dataElementVector if necessary, adding owned elements.
    _dataElementVector.reserve(length);
    while (_dataElementVector.size() < length)
      _dataElementVector.push_back(std::make_pair(_protoType->clone().release(), true));

    for (DataElementVector::const_iterator i = _dataElementVector.begin(); i != _dataElementVector.end(); ++i) {
      if (i->first == nullptr) // must not happen (should actually assert)
        throw EncoderException(L"HLAvariableArray::decodeFrom(): dataElement is zero!");
      index = i->first->decodeFrom(buffer, bufferSize, index);
    }

    return index;
  }

  size_t getEncodedLength() const
  {
    size_t length = 4u;
    for (auto& element : _dataElementVector) {
      length += element.first->getEncodedLength();
    }
    return length;
  }

  unsigned int getOctetBoundary() const {
    return std::max(_protoType->getOctetBoundary(), 4u);
  }

  size_t size () const
  { return _dataElementVector.size(); }

  size_t decodedSize(const Octet* buffer, size_t bufferSize, size_t index)
  {
    if (bufferSize < index + 4)
      throw EncoderException(L"Insufficient buffer size for decoding!");
    uint32_t length;
    decodeFromBE32(buffer, bufferSize, index, length);
    return length;
  }

  void addElement(const DataElement& dataElement)
  {
    if (!dataElement.isSameTypeAs(*_protoType))
      throw EncoderException(L"HLAvariableArray::addElement(): Data type is not compatible!");
    _dataElementVector.push_back({dataElement.clone().release(), true});
  }

  void addElementPointer(DataElement* dataElement)
  {
    if (!dataElement)
      throw EncoderException(L"HLAvariableArray::addElementPointer(): dataElement is zero!");
    if (!dataElement->isSameTypeAs(*_protoType))
      throw EncoderException(L"HLAvariableArray::addElementPointer(): Data type is not compatible!");
    _dataElementVector.push_back({dataElement, false});
  }

  void set(size_t index, const DataElement& dataElement)
  {
    if (_dataElementVector.size() <= index)
      throw EncoderException(L"HLAvariableArray::set(): Index out of range!");
    if (!dataElement.isSameTypeAs(*_protoType))
      throw EncoderException(L"HLAvariableArray::set(): Data type is not compatible!");
    if (_dataElementVector[index].first) delete _dataElementVector[index].first;
    _dataElementVector[index] = { dataElement.clone().release(), true };
  }

  void setElementPointer(size_t index, DataElement* dataElement)
  {
    if (_dataElementVector.size() <= index)
      throw EncoderException(L"HLAvariableArray::setElementPointer(): Index out of range!");
    if (!dataElement)
      throw EncoderException(L"HLAvariableArray::setElementPointer(): dataElement is zero!");
    if (!dataElement->isSameTypeAs(*_protoType))
      throw EncoderException(L"HLAvariableArray::setElementPointer(): Data type is not compatible!");
    if (_dataElementVector[index].first) delete _dataElementVector[index].first;
    _dataElementVector[index] = { dataElement, false };
  }

  const DataElement& get(size_t index) const
  {
    if (_dataElementVector.size() <= index)
      throw EncoderException(L"HLAvariableArray::get(): Index out of range!");
    if (_dataElementVector[index].first == nullptr)
      throw EncoderException(L"HLAvariableArray::get(): dataElement is zero!");
    return *(_dataElementVector[index].first);
  }

  bool isSameTypeAs(const HLAvariableArrayImplementation& rhs) const
  {
    return _protoType->isSameTypeAs(*rhs._protoType);
  }
  void resize(size_t length)
  {
    _dataElementVector.resize(length);
  }
  void clear()
  {
    _dataElementVector.clear();
  }

  DataElement* _protoType;

  typedef std::vector<std::pair<DataElement*, bool>> DataElementVector;
  DataElementVector _dataElementVector;
};

HLAvariableArray::HLAvariableArray(const DataElement& protoType) :
  _impl(new HLAvariableArrayImplementation(protoType))
{
}

HLAvariableArray::HLAvariableArray(const HLAvariableArray & rhs) :
  _impl(new HLAvariableArrayImplementation(*rhs._impl))
{
}

HLAvariableArray::HLAvariableArray(HLAvariableArray&& rhs) :
  _impl(std::move(rhs._impl))
{
}

HLAvariableArray::~HLAvariableArray() noexcept
{
  delete _impl;
  _impl = 0;
}

HLAvariableArray& HLAvariableArray::operator=(const HLAvariableArray& rhs)
{
  delete _impl;
  _impl = new HLAvariableArrayImplementation(*rhs._impl);
  return *this;
}

HLAvariableArray& HLAvariableArray::operator=(HLAvariableArray&& rhs)
{
  delete _impl;
  _impl = std::move(rhs._impl);
  return *this;
}

std::unique_ptr<DataElement>
HLAvariableArray::clone () const
{
  return std::unique_ptr<DataElement>(new HLAvariableArray(*this));
}

VariableLengthData
HLAvariableArray::encode () const
{
  VariableLengthData variableLengthData;
  encode(variableLengthData);
  return variableLengthData;
}

void
HLAvariableArray::encode(VariableLengthData& outData) const
{
  outData.resize(getEncodedLength());
  _impl->encodeInto(static_cast<Octet*>(outData.data()), outData.size(), 0);
}

void
HLAvariableArray::encodeInto(std::vector<Octet>& buffer) const
{
  size_t offset = buffer.size();
  size_t encodedLength = getEncodedLength();
  buffer.resize(offset + encodedLength);
  _impl->encodeInto(static_cast<Octet*>(buffer.data()) + offset, encodedLength, offset);
}

size_t HLAvariableArray::encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  return _impl->encodeInto(buffer, bufferSize, offset);
}

void HLAvariableArray::decode(VariableLengthData const & inData)
{
  _impl->decodeFrom(static_cast<const Octet*>(inData.data()), inData.size(), 0);
}

size_t
HLAvariableArray::decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  return _impl->decodeFrom(buffer.data(), buffer.size(), index);
}

size_t
HLAvariableArray::decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  return _impl->decodeFrom(buffer, bufferSize, index);
}

size_t
HLAvariableArray::getEncodedLength() const
{
  return _impl->getEncodedLength();
}

unsigned int
HLAvariableArray::getOctetBoundary() const
{
  return _impl->getOctetBoundary();
}

size_t
HLAvariableArray::size() const
{
  return _impl->size();
}


size_t HLAvariableArray::decodedSize(const Octet* buffer, size_t bufferSize, size_t index) const
{
  return _impl->decodedSize(buffer, bufferSize, index);
}

bool
HLAvariableArray::isSameTypeAs(DataElement const& inData ) const
{
  if (!DataElement::isSameTypeAs(inData))
    return false;
  const HLAvariableArray* variableArray = dynamic_cast<const HLAvariableArray*>(&inData);
  if (!variableArray)
    return false;
  return _impl->isSameTypeAs(*variableArray->_impl);
}

bool
HLAvariableArray::hasPrototypeSameTypeAs(DataElement const& dataElement) const
{
  const HLAvariableArray* variableArray = dynamic_cast<const HLAvariableArray*>(&dataElement);
  if (!variableArray)
    return false;
  return _impl->isSameTypeAs(*variableArray->_impl);
}

void
HLAvariableArray::addElement(const DataElement& dataElement)
{
  _impl->addElement(dataElement);
}

void
HLAvariableArray::addElementPointer(DataElement* dataElement)
{
  _impl->addElementPointer(dataElement);
}

void
HLAvariableArray::set(size_t index, const DataElement& dataElement)
{
  _impl->set(index, dataElement);
}

void
HLAvariableArray::setElementPointer(size_t index, DataElement* dataElement)
{
  _impl->setElementPointer(index, dataElement);
}

const DataElement&
HLAvariableArray::get(size_t index) const
{
  return _impl->get(index);
}

DataElement const&
HLAvariableArray::operator [](size_t index) const
{
  return _impl->get(index);
}


void HLAvariableArray::resize(size_t length)
{
  _impl->resize(length);
}


void HLAvariableArray::clear()
{
  _impl->clear();
}

}
