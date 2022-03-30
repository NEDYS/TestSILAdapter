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
   File: HLAopaqueData.h
***********************************************************************/
#ifndef RTI_HLAopaqueData_h_
#define RTI_HLAopaqueData_h_

#include <RTI/encoding/DataElement.h>
#include <RTI/encoding/EncodingConfig.h>
#include <RTI/SpecificConfig.h>

namespace rti1516e
{
   // Forward Declarations
   class VariableLengthData;
   class HLAopaqueDataImplementation;

   // Interface for the HLAopaqueData basic data element
   class RTI_EXPORT HLAopaqueData : public rti1516e::DataElement
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

#endif // RTI_HLAopaqueData_h_

