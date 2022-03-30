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
#include "EncodingConfig.h"
#include "Export.h"
#include <memory>


namespace OpenRTI
{
   // Forward Declarations
   class VariableLengthData;

   // Interface provided by all HLA data elements.
   class OPENRTI_API DataElement
   {
   public:

      // Destructor
      virtual ~DataElement() noexcept = 0;

      // Return a new copy of the DataElement
      virtual std::unique_ptr<DataElement> clone () const = 0;

      // Encode this element into a new VariableLengthData
      virtual VariableLengthData encode () const = 0;

      // Encode this element into an existing VariableLengthData
      virtual void encode (
         VariableLengthData& inData) const = 0;

      // Encode this element and append it to a buffer
      virtual void encodeInto (
         std::vector<Octet>& buffer) const = 0;

      // Encode this element and append it to a buffer
      virtual size_t encodeInto (Octet* buffer, size_t bufferSize, size_t offset) const = 0;

      // Decode this element from the RTI's VariableLengthData.
      virtual void decode (
         VariableLengthData const & inData) = 0;

      // Decode this element starting at the index in the provided buffer
      virtual size_t decodeFrom (
         std::vector<Octet> const & buffer,
         size_t index) = 0;
      
      virtual size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index) = 0;

      // Return the size in bytes of this element's encoding.
      virtual size_t getEncodedLength () const = 0;

      // Return the octet boundary of this element.
      virtual unsigned int getOctetBoundary () const = 0;

      // Return true if given element is same type as this; otherwise, false.
      virtual bool isSameTypeAs(
         DataElement const& inData ) const;

      // Return a hash of the encoded data
      // Provides mechanism to map DataElement discriminants to variants
      // in VariantRecord.
      virtual Integer64 hash() const;

   };
}

