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
   File: RTI/time/HLAfloat64TimeFactory.h
***********************************************************************/

#ifndef RTI_HLAfloat64TimeFactory_H_
#define RTI_HLAfloat64TimeFactory_H_

#include <RTI/LogicalTimeFactory.h>

namespace rti1516e {
class HLAfloat64Time;
class HLAfloat64Interval;

// Defines interface for HLAfloat64TimeFactory which presents a
// floating point-based time/interval representation in the range 0 - 2^63-1.

const std::wstring HLAfloat64TimeName(L"HLAfloat64Time");

class RTI_EXPORT HLAfloat64TimeFactory : public rti1516e::LogicalTimeFactory
{
  public:
    HLAfloat64TimeFactory();

    virtual ~HLAfloat64TimeFactory() noexcept;

    // Return a LogicalTime with the given value
    /*virtual */std::unique_ptr< HLAfloat64Time > makeLogicalTime(double value);

    // Return a LogicalTime with a value of "initial"
    virtual std::unique_ptr< LogicalTime > makeInitial() override;

    // Return a LogicalTime with a value of "final"
    virtual std::unique_ptr< LogicalTime > makeFinal() override;

    // Return a LogicalTimeInterval with the given value
    /*virtual*/ std::unique_ptr< HLAfloat64Interval > makeLogicalTimeInterval(
      double value);

    // Return a LogicalTimeInterval with a value of "zero"
    virtual std::unique_ptr< LogicalTimeInterval > makeZero() override;

    // Return a LogicalTimeInterval with a value of "epsilon"
    virtual std::unique_ptr< LogicalTimeInterval > makeEpsilon() override;

    // LogicalTime decode from an encoded LogicalTime
    virtual std::unique_ptr< LogicalTime > decodeLogicalTime(
      VariableLengthData const& encodedLogicalTime) override;

    // Alternate LogicalTime decode that reads directly from a buffer
    std::unique_ptr< LogicalTime > decodeLogicalTime(
      void* buffer,
      size_t bufferSize) override;

    // LogicalTimeInterval decode from an encoded LogicalTimeInterval
    std::unique_ptr< LogicalTimeInterval > decodeLogicalTimeInterval(
      VariableLengthData const& encodedValue) override;

    // Alternate LogicalTimeInterval decode that reads directly from a buffer
    virtual std::unique_ptr< LogicalTimeInterval > decodeLogicalTimeInterval(
      void* buffer,
      size_t bufferSize) override;

    virtual std::wstring getName() const override;
};
}

#endif // RTI_HLAfloat64TimeFactory_H_

