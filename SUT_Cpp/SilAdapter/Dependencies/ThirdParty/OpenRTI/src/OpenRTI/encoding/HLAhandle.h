/* -*-c++-*- OpenRTI - Copyright (C) 2009-2012 Mathias Froehlich
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

#include "OpenRTIConfig.h"
#include "DataElement.h"
#include "BasicDataElements.h"
#include "EncodingConfig.h"
#include "Handle.h"
#include "VariableLengthData.h"

namespace OpenRTI
{


// This is fake as it currently only supports federate handles.
// It should be reworked to support all kinds of handles
// 1
class OPENRTI_API HLAhandle : public DataElement
{
  enum HandleKind {
    eFederateHandle,
    eObjectClassHandle,
    eObjectInstanceHandle,
    eInteractionClassHandle,
    eAttributeHandle,
    eParameterHandle,
    eInvalid
  };
public:
  HLAhandle();
  HLAhandle(const HLAhandle& ref);
  HLAhandle(const FederateHandle& handle);
  HLAhandle(const ObjectClassHandle& handle);
  HLAhandle(const ObjectInstanceHandle& handle);
  HLAhandle(const AttributeHandle& handle);
  HLAhandle(const InteractionClassHandle& handle);
  HLAhandle(const ParameterHandle& handle);
  ~HLAhandle() noexcept;
  FederateHandle getFederateHandle() const;
  ObjectClassHandle getObjectClassHandle() const;
  ObjectInstanceHandle getObjectInstanceHandle() const;
  AttributeHandle getAttributeHandle() const;
  InteractionClassHandle getInteractionClassHandle() const;
  ParameterHandle getParameterHandle() const;

  void set(FederateHandle handle);
  void set(ObjectClassHandle handle);
  void set(ObjectInstanceHandle handle);
  void set(AttributeHandle handle);
  void set(InteractionClassHandle handle);
  void set(ParameterHandle handle);

  std::unique_ptr<DataElement> clone() const override;

  VariableLengthData encode() const override;
  void encode(VariableLengthData& inData) const override;
  void encodeInto(std::vector<Octet>& buffer) const override;

  // Encode this element into a preallocated buffer
  virtual size_t encodeInto(Octet* buffer, size_t bufferSize, size_t offset) const override;

  void decode(VariableLengthData const & inData) override;
  size_t decodeFrom(std::vector<Octet> const & buffer, size_t index) override;
  size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index) override;

  size_t getEncodedLength() const override;
  unsigned int getOctetBoundary() const override;

private:
  HLAinteger32BE mHandleKindEncoding;
  HLAinteger64BE mHandleValueEncoding;
};

}