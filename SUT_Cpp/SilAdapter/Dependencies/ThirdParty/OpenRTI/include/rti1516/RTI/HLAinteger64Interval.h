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

#ifndef RTI_HLAinteger64Interval_h
#define RTI_HLAinteger64Interval_h

#include "LogicalTimeInterval.h"

// FIXME
typedef long long Integer64;

class HLAinteger64Time;
class HLAinteger64IntervalImpl;

class RTI_EXPORT_FEDTIME HLAinteger64Interval : public rti1516::LogicalTimeInterval {
public:
  HLAinteger64Interval();
  HLAinteger64Interval(Integer64);
  HLAinteger64Interval(const rti1516::LogicalTimeInterval&);
  HLAinteger64Interval(const HLAinteger64Interval&);
  virtual ~HLAinteger64Interval() noexcept;

  virtual void setZero();
  virtual bool isZero() const;

  virtual void setEpsilon();
  virtual bool isEpsilon() const;

  virtual HLAinteger64Interval& operator=(const rti1516::LogicalTimeInterval& logicalTimeInterval);

  virtual void setToDifference(const rti1516::LogicalTime& minuend, const rti1516::LogicalTime& subtrahend);

  virtual HLAinteger64Interval& operator+=(const rti1516::LogicalTimeInterval& logicalTimeInterval);
  virtual HLAinteger64Interval& operator-=(const rti1516::LogicalTimeInterval& logicalTimeInterval);

  virtual bool operator>(const rti1516::LogicalTimeInterval& logicalTimeInterval) const;
  virtual bool operator<(const rti1516::LogicalTimeInterval& logicalTimeInterval) const;
  virtual bool operator==(const rti1516::LogicalTimeInterval& logicalTimeInterval) const;
  virtual bool operator>=(const rti1516::LogicalTimeInterval& logicalTimeInterval) const;
  virtual bool operator<=(const rti1516::LogicalTimeInterval& logicalTimeInterval) const;

  virtual rti1516::VariableLengthData encode() const;
  virtual unsigned long encodedLength() const;
  virtual unsigned long encode(void* buffer, unsigned long bufferSize) const;

  virtual void decode(const rti1516::VariableLengthData& variableLengthData);
  virtual void decode(void* buffer, unsigned long bufferSize);

  virtual std::wstring toString() const;
  virtual std::wstring implementationName() const;

  Integer64 getInterval() const;
  void setInterval(Integer64 value);

  HLAinteger64Interval& operator=(const HLAinteger64Interval& value);
  operator Integer64 () const;

private:
  HLAinteger64IntervalImpl* _impl;

  friend class HLAinteger64Time;
};

#endif
