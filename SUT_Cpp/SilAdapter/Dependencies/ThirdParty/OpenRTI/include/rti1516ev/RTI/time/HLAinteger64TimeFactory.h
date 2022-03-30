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
   File: RTI/time/HLAinteger64TimeFactory.h
***********************************************************************/

#ifndef RTI_HLAinteger64TimeFactory_H_
#define RTI_HLAinteger64TimeFactory_H_

#include <RTI/LogicalTimeFactory.h>
#include <RTI/encoding/EncodingConfig.h>


// Defines interface for HLAinteger64TimeFactory which presents an integer-based
// time /interval representation in the range 0 - 2^63-1.

namespace rti1516ev
{
   const std::wstring HLAinteger64TimeName(L"HLAinteger64Time");

   class HLAinteger64Time;
   class HLAinteger64Interval;

   class RTI_EXPORT HLAinteger64TimeFactory : public rti1516ev::LogicalTimeFactory
   {
   public:
      HLAinteger64TimeFactory ();

      virtual ~HLAinteger64TimeFactory () noexcept;

      // Return a LogicalTime with a value of "initial"
      std::unique_ptr< LogicalTime > makeInitial() override;

      // Return a LogicalTime with a value of "final"
      std::unique_ptr< LogicalTime > makeFinal() override;

      // Return a LogicalTimeInterval with a value of "zero"
      std::unique_ptr< LogicalTimeInterval > makeZero() override;

      // Return a LogicalTimeInterval with a value of "epsilon"
      std::unique_ptr< LogicalTimeInterval > makeEpsilon() override;

      virtual std::unique_ptr< HLAinteger64Time > makeLogicalTime (
         Integer64 value);

      virtual std::unique_ptr< HLAinteger64Interval > makeLogicalTimeInterval (
         Integer64 value);

      // LogicalTime decode from an encoded LogicalTime
      std::unique_ptr< LogicalTime > decodeLogicalTime (
         VariableLengthData const & encodedLogicalTime) override;

      // Alternate LogicalTime decode that reads directly from a buffer
      std::unique_ptr< LogicalTime > decodeLogicalTime (
         void* buffer,
         size_t bufferSize) override;

      // LogicalTimeInterval decode from an encoded LogicalTimeInterval
      std::unique_ptr< LogicalTimeInterval > decodeLogicalTimeInterval (
         VariableLengthData const & encodedValue) override;

      // Alternate LogicalTimeInterval decode that reads directly from a buffer
      std::unique_ptr< LogicalTimeInterval > decodeLogicalTimeInterval (
         void* buffer,
         size_t bufferSize) override;

      virtual std::wstring getName () const override;
   };
}

#endif // RTI_HLAinteger64TimeFactory_H_

