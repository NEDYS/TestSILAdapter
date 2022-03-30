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
   class HLAvariableArrayImplementation;

   // Interface for the HLAvariableArray complex data element
   class OPENRTI_LOCAL HLAvariableArray : public OpenRTI::DataElement
   {
   public:

      // Constructor which accepts a prototype element
      // that specifies the type of elements to be stored in the array.
      // A clone of the given element works as a seed.
      explicit HLAvariableArray (
         const DataElement& prototype );

      // Copy Constructor
      HLAvariableArray (const HLAvariableArray& rhs);
      HLAvariableArray (HLAvariableArray&& rhs);

      // Destructor
      virtual ~HLAvariableArray () noexcept;

      // Return a new copy of the DataElement
      virtual std::unique_ptr<DataElement> clone () const override;

      // Encode this element into a new VariableLengthData
      virtual VariableLengthData encode () const override;

      // Encode this element into an existing VariableLengthData
      virtual void encode (
         VariableLengthData& inData) const override;

      // Encode this element and append it to a buffer
      virtual void encodeInto (
         std::vector<Octet>& buffer) const override;

      virtual size_t encodeInto (Octet* buffer, size_t bufferSize, size_t offset) const override;

      // Decode this element from the RTI's VariableLengthData.
      virtual void decode (
         VariableLengthData const & inData) override;

      // Decode this element starting at the index in the provided buffer
      virtual size_t decodeFrom (
         std::vector<Octet> const & buffer,
         size_t index) override;

      size_t decodeFrom(const Octet* buffer, size_t bufferSize, size_t index) override;

      // Return the size in bytes of this element's encoding.
      virtual size_t getEncodedLength () const override;

      // Return the octet boundary of this element.
      virtual unsigned int getOctetBoundary () const override;

      // Return the number of elements in this variable array.
      virtual size_t size () const;

      size_t decodedSize(const Octet* buffer, size_t bufferSize, size_t index) const;

      // Return true if given element is same type as this; otherwise, false.
      virtual bool isSameTypeAs(
         DataElement const& inData ) const override;

      // Return true if given element matches prototype of array.
      virtual bool hasPrototypeSameTypeAs(DataElement const& dataElement ) const;

      // Adds a copy of the given element instance to this array
      // Element must match prototype.
      virtual void addElement (
         const DataElement& dataElement);

      // Adds the given element instance to this variable array
      // Element must match prototype.
      // Caller is responsible for ensuring that the external memory is
      // valid for the lifetime of this object or until the indexed element
      // acquires new memory through the set method.
      // Null pointer results in an exception.
      virtual void addElementPointer (
         DataElement* dataElement);

      // Sets the indexed element to a copy of the given element instance.
      // Element must match prototype.
      // If indexed element uses external memory, the memory will be modified.
      virtual void set (
         size_t index,
         const DataElement& dataElement);

      // Sets the indexed element to the given element instance.
      // Element must match prototype.
      // Null pointer results in an exception.
      // Caller is responsible for ensuring that the external memory is
      // valid for the lifetime of this object or until the indexed element
      // acquires new memory through this call.
      virtual void setElementPointer (
         size_t index,
         DataElement* dataElement);

      // Return a const reference to the element instance at the specified index.
      // Must use set to change element.
      virtual const DataElement& get(size_t index) const;

      // Return a const reference to the element instance at the specified index.
      // Must use set to change element.
      DataElement const& operator [](size_t index) const;
      // Assignment Operator not allowed
      HLAvariableArray& operator=(HLAvariableArray const & rhs);
      HLAvariableArray& operator=(HLAvariableArray&& rhs);

   private:

      // Default Constructor not allowed
      HLAvariableArray() = delete;

   protected:

      HLAvariableArrayImplementation* _impl;
   };
}

