/***********************************************************************
  IEEE 1516.1 High Level Architecture Interface Specification C++ API
  File: RTI/LogicalTimeInterval.h
***********************************************************************/

#ifndef RTI_LogicalTimeInterval_h
#define RTI_LogicalTimeInterval_h

// The classes associated with logical time allow a federation to provide
// their own representation for logical time and logical time interval. The
// federation is responsible to inherit from the abstract classes declared
// below. The encoded time classes are used to hold the arbitrary bit
// representation of the logical time and logical time intervals.

namespace rti1516
{
  class LogicalTime;
}

#include <RTI/SpecificConfig.h>
#include <RTI/Exception.h>
#include <string>
#include <RTI/VariableLengthData.h>

namespace rti1516
{
  class RTI_EXPORT LogicalTimeInterval
  {
  public:
    virtual
    ~LogicalTimeInterval() = 0;

    virtual
    void
    setZero() = 0;

    virtual
    bool
    isZero() const = 0;
  
    virtual
    void
    setEpsilon() = 0;

    virtual
    bool
    isEpsilon() const = 0;

    virtual
    LogicalTimeInterval &
    operator=(LogicalTimeInterval const & value) = 0;

    // Set self to the difference between two LogicalTimes
    virtual
    void
    setToDifference(LogicalTime const & minuend,
      LogicalTime const& subtrahend) = 0;

    virtual
    LogicalTimeInterval &
    operator+=(LogicalTimeInterval const & addend) = 0;

    virtual
    LogicalTimeInterval &
    operator-=(LogicalTimeInterval const & subtrahend) = 0;
    
    virtual
    bool
    operator>(LogicalTimeInterval const & value) const = 0;

    virtual
    bool
    operator<(LogicalTimeInterval const & value) const = 0;

    virtual
    bool
    operator==(LogicalTimeInterval const & value) const = 0;

    virtual
    bool
    operator>=(LogicalTimeInterval const & value) const = 0;

    virtual
    bool
    operator<=(LogicalTimeInterval const & value) const = 0;
    
    // Generates an encoded value that can be used to send
    // LogicalTimeIntervals to other federates in updates or interactions
    virtual VariableLengthData encode() const = 0;

    // Alternate encode for directly filling a buffer
    virtual unsigned long encodedLength() const = 0;
    virtual unsigned long encode(void* buffer, unsigned long bufferSize) const = 0;

    // Decode encodedValue into self
    virtual void decode(VariableLengthData const & encodedValue) = 0;

    // Alternate decode that reads directly from a buffer
    virtual void decode(void* buffer, unsigned long bufferSize) = 0;

    virtual std::wstring toString() const = 0;

    // Returns the name of the implementation, as needed by
    // createFederationExecution.
    virtual std::wstring implementationName() const = 0;
  };

  // Output operator for LogicalTimeInterval
  std::wostream RTI_EXPORT &
    operator << (std::wostream &, LogicalTimeInterval const &);
}
#endif // RTI_LogicalTimeInterval_h
