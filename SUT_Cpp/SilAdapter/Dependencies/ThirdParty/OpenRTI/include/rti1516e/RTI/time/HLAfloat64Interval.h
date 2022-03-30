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
   File: RTI/time/HLAfloat64Interval.h
***********************************************************************/

#ifndef RTI_HLAfloat64Interval_H_
#define RTI_HLAfloat64Interval_H_

#include <RTI/LogicalTimeInterval.h>


namespace rti1516e
{
   class HLAfloat64IntervalImpl;

   class RTI_EXPORT HLAfloat64Interval : public rti1516e::LogicalTimeInterval
   {
   public:

      // Constructors
      HLAfloat64Interval ();

      HLAfloat64Interval (
         double);

      HLAfloat64Interval (
         rti1516e::LogicalTimeInterval const &);

      HLAfloat64Interval (
         const HLAfloat64Interval&);

      // Destructor
      virtual ~HLAfloat64Interval () noexcept;

      // Basic accessors/mutators

      virtual void setZero () noexcept override;

      virtual bool isZero () const override;

      virtual void setEpsilon () noexcept override;

      virtual bool isEpsilon () const override;

      // Operators

      virtual rti1516e::LogicalTimeInterval& operator= (
         rti1516e::LogicalTimeInterval const & value) override;

      virtual rti1516e::LogicalTimeInterval& operator+= (
         rti1516e::LogicalTimeInterval const & addend) override;

      virtual rti1516e::LogicalTimeInterval& operator-= (
         rti1516e::LogicalTimeInterval const & subtrahend) override;

      virtual bool operator> (
         rti1516e::LogicalTimeInterval const & value) const override;

      virtual bool operator< (
         rti1516e::LogicalTimeInterval const & value) const override;

      virtual bool operator== (
         rti1516e::LogicalTimeInterval const & value) const override;

      virtual bool operator>= (
         rti1516e::LogicalTimeInterval const & value) const override;

      virtual bool operator<= (
         rti1516e::LogicalTimeInterval const & value) const override;

      // Set self to the difference between two LogicalTimes
      virtual void setToDifference (
         rti1516e::LogicalTime const & minuend,
         rti1516e::LogicalTime const& subtrahend) override;

      // Generates an encoded value that can be used to send
      // LogicalTimeIntervals to other federates in updates or interactions
      virtual rti1516e::VariableLengthData encode () const override;

      // Alternate encode for directly filling a buffer
      virtual size_t encode (
         void* buffer,
         size_t bufferSize) const override;

      // The length of the encoded data
      virtual size_t encodedLength () const override;

      // Decode encodedValue into self
      virtual void decode (
         rti1516e::VariableLengthData const & encodedValue) override;

      // Alternate decode that reads directly from a buffer
      virtual void decode (
         void* buffer,
         size_t bufferSize) override;

      // Diagnostic string representation of time
      virtual std::wstring toString () const override;

      // Return the name of the Implementation, as needed by
      // createFederationExecution.
      virtual std::wstring implementationName() const override;

   public:
      //-----------------------------------------------------------------
      // Implementation functions
      //-----------------------------------------------------------------

      virtual double getInterval () const;

      virtual void setInterval (
         double value);

      //-----------------------------------------------------------------
      // Implementation operators
      //-----------------------------------------------------------------
      virtual HLAfloat64Interval& operator= (
         const HLAfloat64Interval& value);

      operator double () const;


   private:

      //-----------------------------------------------------------------
      // Pointer to internal implementation
      //-----------------------------------------------------------------
      HLAfloat64IntervalImpl* _impl;
   };
}

#endif // RTI_HLAfloat64Interval_H_

