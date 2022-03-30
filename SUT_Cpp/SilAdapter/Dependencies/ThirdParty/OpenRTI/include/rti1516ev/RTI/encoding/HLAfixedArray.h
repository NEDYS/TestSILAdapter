/***********************************************************************
   The IEEE hereby grants a general, royalty-free license to copy, distribute,
   display and make derivative works from this material, for all purposes,
   provided that any use of the material contains the following
   attribution: "Reprinted with permission from IEEE 1516.1(TM)-2010".
   Should you require additional information, contact the Manager, Standards
   Intellectual Property, IEEE Standards Association (stds-ipr@ieee.org).
***********************************************************************/
/***********************************************************************
   IEEE 1516.1 High Level Architecture Interface Specification C++ API
   File: HLAfixedArray.h
***********************************************************************/
#ifndef RTI_HLAfixedArray_h_
#define RTI_HLAfixedArray_h_

#include <RTI/encoding/DataElement.h>
#include <RTI/encoding/EncodingConfig.h>
#include <RTI/SpecificConfig.h>

namespace rti1516ev
{
   // Forward Declarations
   class VariableLengthData;
   class HLAfixedArrayImplementation;

   // Interface for the HLAfixedArray complex data element
   class RTI_EXPORT HLAfixedArray : public rti1516ev::DataElement
   {
   public:
      // Constructor which accepts a prototype element and size
      // that specifies the type and number of elements to be stored in the array.
      // A clone of the given element works as a prototype.
      explicit HLAfixedArray (
         const DataElement& protoType,
         size_t length );

      // Copy Constructor
      // Copied elements use internal memory
      HLAfixedArray (
         HLAfixedArray const & rhs);

      // Destructor
      virtual ~HLAfixedArray() noexcept;

      // Return a new copy of the array
      std::unique_ptr<DataElement> clone () const override;

      // Encode this element into a new VariableLengthData
      VariableLengthData encode () const override;

      // Encode this element into an existing VariableLengthData
      void encode (
         VariableLengthData& inData) const override;

      // Encode this element and append it to a buffer
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

      // Return true if given element is same type as this; otherwise, false.
      bool isSameTypeAs(
         DataElement const& inData ) const override;

      // Return true if given element matches prototype of this array.
      virtual bool hasPrototypeSameTypeAs(DataElement const& dataElement ) const;

      // Return the number of elements in this fixed array.
      virtual size_t size () const;

      // Sets the element at the given index to a copy of the given element instance
      // Element must match prototype.
      // If indexed element uses external memory, the memory will be modified.
      virtual void set (size_t index,
         const DataElement& dataElement);

      // Sets the element at the given index to the given element instance
      // Element must match prototype.
      // Null pointer results in an exception.
      // Caller is responsible for ensuring that the external memory is
      // valid for the lifetime of this object or until the indexed element
      // acquires new memory through this call.
      virtual void setElementPointer (size_t index,
         DataElement* dataElement);

      // Return a reference to the element instance at the specified index.
      // Access of indexed element that has not been set will set that index
      // with a clone of prototype and return it.
      // Must use set to change element.
      virtual const DataElement& get (
         size_t index) const;

      // Return a const reference to the element instance at the specified index.
      // Access of indexed element that has not been set will set that index
      // with a clone of prototype and return it.
      // Must use set to change element.
      DataElement const& operator [](size_t index) const;

      void resize(size_t length);
      void clear();
   private:

      // Default Constructor not allowed
      HLAfixedArray ();

      // Assignment Operator not allowed
      HLAfixedArray& operator=(
         HLAfixedArray const & rhs);

   protected:

      HLAfixedArrayImplementation* _impl;
   };
}

#endif // RTI_HLAfixedArray_h_

