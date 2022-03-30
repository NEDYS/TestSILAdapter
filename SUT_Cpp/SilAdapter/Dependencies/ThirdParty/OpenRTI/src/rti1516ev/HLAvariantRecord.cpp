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
#include <RTI/encoding/HLAvariantRecord.h>

#include <RTI/VariableLengthData.h>
#include <RTI/encoding/DataElement.h>

#include <cstring>
#include <list>

#include "Encoding.h"
#include "Export.h"

namespace rti1516ev
{

class OPENRTI_LOCAL HLAvariantRecordImplementation {
public:
  HLAvariantRecordImplementation(DataElement const& discriminantPrototype) :
    _octetBoundary(0)
  {
  }
  HLAvariantRecordImplementation(const HLAvariantRecordImplementation& rhs) :
    _discriminant(rhs._discriminant->clone()),
    _variant(rhs._variant->clone()),
    _octetBoundary(0)
  {
  }
  ~HLAvariantRecordImplementation() noexcept
  {
  }

  void encodeInto(std::vector<Octet>& buffer)
  {
    /* FIXME */
  }

  size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset)
  {
    return offset;
  }

  size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
  {
    /* FIXME */
    return index;
  }

  size_t getEncodedLength()
  {
    /* FIXME */
    return 0;
  }

  unsigned getOctetBoundary()
  {
    if (_octetBoundary)
      return _octetBoundary;
    /*FIXME*/
    return _octetBoundary;
  }

  bool isSameTypeAs(HLAvariantRecordImplementation const& inData) const
  {
    return false; /*FIXME*/
  }

  bool isSameTypeAs(DataElement const& discriminant, DataElement const& inData) const
  {
    return false; /*FIXME*/
  }

  bool hasMatchingDiscriminantTypeAs(DataElement const& dataElement) const
  {
    return false; /*FIXME*/
  }

  void addVariant(const DataElement& discriminant, const DataElement& valuePrototype)
  {
    /*FIXME*/
  }

  void addVariantPointer(const DataElement& discriminant, DataElement* valuePtr)
  {
    /*FIXME*/
  }

  void setDiscriminant(const DataElement& discriminant)
  {
    /*FIXME*/
  }

  void setVariant(const DataElement& discriminant, DataElement const& value)
  {
    /*FIXME*/
  }

  void setVariantPointer(const DataElement& discriminant, DataElement* valuePtr)
  {
    /*FIXME*/
  }

  const DataElement& getDiscriminant() const
  {
    return *_discriminant;
  }

  const DataElement& getVariant() const
  {
    return *_variant;
  }

  std::unique_ptr<DataElement> _discriminant;
  std::unique_ptr<DataElement> _variant;
  unsigned _octetBoundary;
};

HLAvariantRecord::HLAvariantRecord(DataElement const& discriminantPrototype) :
    /*FIXME*/
  // _impl(new HLAvariantRecordImplementation(discriminantPrototype))
  _impl(0)
{
}

HLAvariantRecord::HLAvariantRecord(HLAvariantRecord const & rhs) :
    /*FIXME*/
  // _impl(new HLAvariantRecordImplementation(*rhs._impl))
  _impl(0)
{
}

HLAvariantRecord::~HLAvariantRecord() noexcept
{
  delete _impl;
  _impl = 0;
}

std::unique_ptr<DataElement>
HLAvariantRecord::clone () const
{
  return std::unique_ptr<rti1516ev::DataElement>(new HLAvariantRecord(*this));
}

VariableLengthData
HLAvariantRecord::encode () const
{
  VariableLengthData variableLengthData;
  encode(variableLengthData);
  return variableLengthData;
}

void
HLAvariantRecord::encode(VariableLengthData& inData) const
{
  std::vector<Octet> buffer;
  buffer.reserve(getEncodedLength());
  encodeInto(buffer);
  inData.setData(&buffer.front(), buffer.size());
}

void
HLAvariantRecord::encodeInto(std::vector<Octet>& buffer) const
{
  _impl->encodeInto(buffer);
}


size_t HLAvariantRecord::encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const
{
  return _impl->encodeInto(buffer, bufferSize, offset);
}

void HLAvariantRecord::decode(VariableLengthData const & inData)
{
  _impl->decodeFrom(static_cast<const Octet*>(inData.data()), inData.size(), 0);
}

size_t
HLAvariantRecord::decodeFrom(std::vector<Octet> const & buffer, size_t index)
{
  return _impl->decodeFrom(buffer.data(), buffer.size(), index);
}


size_t HLAvariantRecord::decodeFrom(const Octet* buffer, size_t bufferSize, size_t index)
{
  return _impl->decodeFrom(buffer, bufferSize, index);
}

size_t
HLAvariantRecord::getEncodedLength() const
{
  return _impl->getEncodedLength();
}

unsigned int
HLAvariantRecord::getOctetBoundary() const
{
  return _impl->getOctetBoundary();
}

bool
HLAvariantRecord::isSameTypeAs(DataElement const& inData) const
{
  const HLAvariantRecord* variantRecord = dynamic_cast<const HLAvariantRecord*>(&inData);
  if (!variantRecord)
    return false;
  return _impl->isSameTypeAs(*variantRecord->_impl);
}

bool
HLAvariantRecord::isSameTypeAs(DataElement const& discriminant, DataElement const& inData) const
{
  return _impl->isSameTypeAs(discriminant, inData);
}

bool
HLAvariantRecord::hasMatchingDiscriminantTypeAs(DataElement const& dataElement) const
{
  return _impl->hasMatchingDiscriminantTypeAs(dataElement);
}

void
HLAvariantRecord::addVariant(const DataElement& discriminant, const DataElement& valuePrototype)
{
  _impl->addVariant(discriminant, valuePrototype);
}

void
HLAvariantRecord::addVariantPointer(const DataElement& discriminant, DataElement* valuePtr)
{
  _impl->addVariantPointer(discriminant, valuePtr);
}

void
HLAvariantRecord::setDiscriminant(const DataElement& discriminant)
{
  _impl->setDiscriminant(discriminant);
}

void
HLAvariantRecord::setVariant(const DataElement& discriminant, DataElement const& value)
{
  _impl->setVariant(discriminant, value);
}

void
HLAvariantRecord::setVariantPointer(const DataElement& discriminant, DataElement* valuePtr)
{
  _impl->setVariantPointer(discriminant, valuePtr);
}

const DataElement&
HLAvariantRecord::getDiscriminant() const
{
  return _impl->getDiscriminant();
}

const DataElement&
HLAvariantRecord::getVariant() const
{
  return _impl->getVariant();
}

}
