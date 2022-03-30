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
   File: BasicDataElements.h
***********************************************************************/

#pragma once

#include <RTI/encoding/DataElement.h>
#include <RTI/encoding/EncodingConfig.h>
#include <RTI/SpecificConfig.h>
#include <string>

namespace rti1516e
{
// The following macro is used to define each of the encoding helper
// classes for basic C++ datatypes, e.g. HLAinteger16LE, HLAunicodeString,
// etc. Each kind of encoding helper contains the same set of operators
// and functions, but each is a separate class for type safety.
// Encoder uses either internal memory for value of native type or
// uses a supplied references to external memory of the native type.
// When used with a reference, the encoding and decoding is performed using
// the referenced instance of the native type.

#define DEFINE_ENCODING_HELPER_CLASS(EncodableDataType, SimpleDataType)       \
                                                                              \
/* Forward declaration for the RTI-internal class used to implement        */ \
/* a specific kind of encoding helper                                      */ \
class EncodableDataType##Implementation;                                      \
                                                                              \
class RTI_EXPORT EncodableDataType : public rti1516e::DataElement             \
{                                                                             \
public:                                                                       \
                                                                              \
   /* Constructor: Default                                                 */ \
   /* Uses internal memory.                                                */ \
   EncodableDataType();                                                       \
                                                                              \
   /* Constructor: Initial Value                                           */ \
   /* Uses internal memory.                                                */ \
   explicit EncodableDataType (                                               \
      SimpleDataType const & inData);                                         \
                                                                              \
   /* Constructor: External memory                                         */ \
   /* This instance changes or is changed by contents of external memory.  */ \
   /* Caller is responsible for ensuring that the external memory is       */ \
   /* valid for the lifetime of this object or until this object acquires  */ \
   /* new memory through setDataPointer.                                   */ \
   /* A null value will construct instance to use internal memory.         */ \
   explicit EncodableDataType (                                               \
      SimpleDataType* inData);                                                \
                                                                              \
   /* Constructor: Copy                                                    */ \
   /* Uses internal memory.                                                */ \
   EncodableDataType (                                                        \
      EncodableDataType const & rhs);                                         \
                                                                              \
   /* Destructor                                                           */ \
   virtual ~EncodableDataType() noexcept;                                     \
                                                                              \
   /* Assignment Operator                                                  */ \
   /* Uses existing memory of this instance.                               */ \
   EncodableDataType& operator=(                                              \
      EncodableDataType const & rhs);                                         \
                                                                              \
   /* Return a new copy of the DataElement                                 */ \
   /* Copy uses internal memory.                                           */ \
   std::unique_ptr<DataElement> clone () const override;                      \
                                                                              \
   /* Encode this element into a new VariableLengthData                    */ \
   VariableLengthData encode () const override;                               \
                                                                              \
   /* Encode this element into an existing VariableLengthData              */ \
   void encode (                                                              \
      VariableLengthData& inData) const override;                             \
                                                                              \
   /* Encode this element and append it to a buffer                        */ \
   void encodeInto (                                                          \
      std::vector<Octet>& buffer) const override;                             \
                                                                              \
   size_t encodeInto(                                                         \
      Octet* buffer, size_t bufferSize, size_t offset) const override;        \
                                                                              \
   /* Decode this element from the RTI's VariableLengthData.               */ \
   void decode (                                                              \
      VariableLengthData const & inData) override;                            \
                                                                              \
   /* Decode this element starting at the index in the provided buffer     */ \
   /* Return the index immediately after the decoded data.                 */ \
   size_t decodeFrom (                                                        \
      std::vector<Octet> const & buffer,                                      \
      size_t index) override;                                                 \
                                                                              \
   size_t decodeFrom(                                                         \
      const Octet* buffer, size_t bufferSize, size_t index) override;         \
                                                                              \
   /* Return the size in bytes of this element's encoding.                 */ \
   size_t getEncodedLength () const override;                                 \
                                                                              \
   /* Return the octet boundary of this element.                           */ \
   unsigned int getOctetBoundary () const override;                           \
                                                                              \
   /* Return a hash of the encoded data                                    */ \
   /* Provides mechanism to map DataElement discriminants to variants      */ \
   /* in VariantRecord.                                                    */ \
   Integer64 hash() const override;                                           \
                                                                              \
   /* Change this instance to use supplied external memory.                */ \
   /* Caller is responsible for ensuring that the external memory is       */ \
   /* valid for the lifetime of this object or until this object acquires  */ \
   /* new memory through this call.                                        */ \
   /* Null pointer results in an exception.                                */ \
   virtual void setDataPointer (                                              \
      SimpleDataType* inData);                                               \
                                                                              \
   /* Set the value to be encoded.                                         */ \
   /* If this element uses external memory, the memory will be modified.   */ \
   virtual void set (                                                         \
      SimpleDataType inData);                                                 \
                                                                              \
   /* Get the value from encoded data.                                     */ \
   virtual SimpleDataType get () const;                                       \
                                                                              \
   /* Assignment of the value to be encoded data.                          */ \
   /* If this element uses external memory, the memory will be modified.   */ \
   EncodableDataType & operator= (                                            \
      SimpleDataType rhs);                                                    \
                                                                              \
   /* Conversion operator to SimpleDataType                                */ \
   /* Return value from encoded data.                                      */ \
   operator SimpleDataType () const;                                          \
                                                                              \
protected:                                                                    \
                                                                              \
   EncodableDataType##Implementation* _impl;                                  \
};


   // Forward Declarations
   class VariableLengthData;

   // All of the RTI API's Basic DataType Encoding Helper classes are
   // defined by invoking the macro above.
   DEFINE_ENCODING_HELPER_CLASS( HLAASCIIchar, char )
   DEFINE_ENCODING_HELPER_CLASS( HLAASCIIstring, std::string )
   DEFINE_ENCODING_HELPER_CLASS( HLAboolean, bool )
   DEFINE_ENCODING_HELPER_CLASS( HLAbyte, Octet )
   DEFINE_ENCODING_HELPER_CLASS( HLAfloat32BE, float )
   DEFINE_ENCODING_HELPER_CLASS( HLAfloat32LE, float )
   DEFINE_ENCODING_HELPER_CLASS( HLAfloat64BE, double )
   DEFINE_ENCODING_HELPER_CLASS( HLAfloat64LE, double )
   DEFINE_ENCODING_HELPER_CLASS( HLAinteger16LE, Integer16 )
   DEFINE_ENCODING_HELPER_CLASS( HLAinteger16BE, Integer16 )
   DEFINE_ENCODING_HELPER_CLASS( HLAinteger32BE, Integer32 )
   DEFINE_ENCODING_HELPER_CLASS( HLAinteger32LE, Integer32 )
   DEFINE_ENCODING_HELPER_CLASS( HLAinteger64BE, Integer64 )
   DEFINE_ENCODING_HELPER_CLASS( HLAinteger64LE, Integer64 )
   DEFINE_ENCODING_HELPER_CLASS( HLAoctet, Octet )
   DEFINE_ENCODING_HELPER_CLASS( HLAoctetPairBE, OctetPair )
   DEFINE_ENCODING_HELPER_CLASS( HLAoctetPairLE, OctetPair )
   DEFINE_ENCODING_HELPER_CLASS( HLAunicodeChar, wchar_t )
   DEFINE_ENCODING_HELPER_CLASS( HLAunicodeString, std::wstring )

}

#undef DEFINE_ENCODING_HELPER_CLASS

