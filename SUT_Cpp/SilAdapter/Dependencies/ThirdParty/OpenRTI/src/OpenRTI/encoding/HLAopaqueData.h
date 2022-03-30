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
#include "EncodingConfig.h"

namespace OpenRTI
{
   // Forward Declarations
   class VariableLengthData;
   class HLAopaqueDataImplementation;

   // Interface for the HLAopaqueData basic data element
   class OPENRTI_LOCAL HLAopaqueData : public OpenRTI::DataElement
   {
   public:

      // Constructor: Default
      // Uses internal memory
      HLAopaqueData ();

      // Constructor: Initial Value
      // Uses internal memory
      HLAopaqueData (
         const Octet* inData,
         size_t dataSize);

      // Constructor: Use external memory with buffer and data of given lengths.
      // This instance changes or reflects changes to contents of external memory.
      // Changes to external memory are reflected in subsequent encodings.
      // Changes to encoder (i.e., set or decode) are reflected in external memory.
      // Caller is responsible for ensuring that the external memory is
      // valid for the lifetime of this object or until this object acquires
      // new memory through setDataPointer.
      // Buffer length indicates size of memory; data length indicates size of
      // data stored in memory.
      // Exception is thrown for null memory or zero buffer size.
      HLAopaqueData (
         Octet** inData,
         size_t bufferSize,
         size_t dataSize);

      // Constructor: Copy
      // Uses internal memory
      HLAopaqueData (
         HLAopaqueData const & rhs);

      // Caller is free to delete rhs.
      virtual ~HLAopaqueData() noexcept;

      // Return a new copy of the DataElement
      std::unique_ptr<DataElement> clone () const override;

      // Return the encoding of this element in a VariableLengthData
      VariableLengthData encode () const override;

      // Encode this element into an existing VariableLengthData
      void encode (
         VariableLengthData& inData) const override;

      // Encode this element and append it to a buffer.
      void encodeInto (
         std::vector<Octet>& buffer) const override;

      // Encode this element into a preallocated buffer
      size_t encodeInto (Octet* buffer, size_t bufferSize, size_t offset) const override;

      // Decode this element from the RTI's VariableLengthData.
      void decode (
         VariableLengthData const & inData) override;

      // Decode this element starting at the index in the provided buffer
      size_t decodeFrom (
         std::vector<Octet> const & buffer,
         size_t index) override;

      size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index) override;

      // Return the size in bytes of this element's encoding.
      size_t getEncodedLength () const override;

      // Return the octet boundary of this element.
      unsigned int getOctetBoundary () const override;

      // Return the length of the contained buffer
      virtual size_t bufferLength () const;

      // Return the length of the data stored in the buffer
      virtual size_t dataLength () const;

      // Change memory to use given external memory
      // Changes to this instance will be reflected in external memory
      // Caller is responsible for ensuring that the data that is
      // pointed to is valid for the lifetime of this object, or past
      // the next time this object is given new data.
      // Buffer length indicates size of memory; data length indicates size of
      // data stored in memory.
      // Exception is thrown for null memory or zero buffer size.
      virtual void setDataPointer (
         Octet** inData,
         size_t bufferSize,
         size_t dataSize);

      // Set the data to be encoded.
      virtual void set(
         const Octet* inData,
         size_t dataSize);

      // Return a reference to the contained array
      virtual const Octet* get () const;

      // Conversion operator to std::vector<Octet>
      // Value returned is from encoded data.
      operator const Octet*() const;

   private:

      // Assignment Operator not allowed
      HLAopaqueData& operator=(
         HLAopaqueData const & rhs);

   protected:

      HLAopaqueDataImplementation* _impl;
   };
}


