/* -*-c++-*- OpenRTI - Copyright (C) 2009-2015 Mathias Froehlich
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

#include "DebugNew.h"
#include <RTI/time/HLAfloat64TimeFactory.h>

#include <memory>
#include <RTI/LogicalTimeFactory.h>
#include <RTI/time/HLAfloat64Interval.h>
#include <RTI/time/HLAfloat64Time.h>

namespace rti1516ev {

HLAfloat64TimeFactory::HLAfloat64TimeFactory()
{
}

HLAfloat64TimeFactory::~HLAfloat64TimeFactory() noexcept
{
}

std::unique_ptr<HLAfloat64Time>
HLAfloat64TimeFactory::makeLogicalTime(double value)
{
  return std::unique_ptr<HLAfloat64Time>(new HLAfloat64Time(value));
}

std::unique_ptr<LogicalTime>
HLAfloat64TimeFactory::makeInitial()
{
  return std::unique_ptr<LogicalTime>(new HLAfloat64Time);
}

std::unique_ptr<LogicalTime> HLAfloat64TimeFactory::makeFinal()
{
  std::unique_ptr<HLAfloat64Time> integerTime(new HLAfloat64Time);
  integerTime->setFinal();
  return std::unique_ptr<LogicalTime>(integerTime.release());
}

std::unique_ptr<HLAfloat64Interval>
HLAfloat64TimeFactory::makeLogicalTimeInterval(double value)
{
  return std::unique_ptr<HLAfloat64Interval>(new HLAfloat64Interval(value));
}

std::unique_ptr<LogicalTimeInterval>
HLAfloat64TimeFactory::makeZero()
{
  return std::unique_ptr<LogicalTimeInterval>(new HLAfloat64Interval);
}

std::unique_ptr<LogicalTimeInterval>
HLAfloat64TimeFactory::makeEpsilon()
{
  std::unique_ptr<HLAfloat64Interval> integerInterval(new HLAfloat64Interval);
  integerInterval->setEpsilon();
  return std::unique_ptr<LogicalTimeInterval>(integerInterval.release());
}

std::unique_ptr<LogicalTime>
HLAfloat64TimeFactory::decodeLogicalTime(VariableLengthData const & encodedLogicalTime)
{
  std::unique_ptr<HLAfloat64Time> integerTime(new HLAfloat64Time);
  integerTime->decode(encodedLogicalTime);
  return std::unique_ptr<LogicalTime>(integerTime.release());
}

std::unique_ptr<LogicalTime>
HLAfloat64TimeFactory::decodeLogicalTime(void* buffer, size_t bufferSize)
{
  std::unique_ptr<HLAfloat64Time> integerTime(new HLAfloat64Time);
  integerTime->decode(buffer, bufferSize);
  return std::unique_ptr<LogicalTime>(integerTime.release());
}

std::unique_ptr<LogicalTimeInterval>
HLAfloat64TimeFactory::decodeLogicalTimeInterval(VariableLengthData const & encodedValue)
{
  std::unique_ptr<HLAfloat64Interval> integerInterval(new HLAfloat64Interval);
  integerInterval->decode(encodedValue);
  return std::unique_ptr<LogicalTimeInterval>(integerInterval.release());
}

std::unique_ptr<LogicalTimeInterval>
HLAfloat64TimeFactory::decodeLogicalTimeInterval(void* buffer, size_t bufferSize)
{
  std::unique_ptr<HLAfloat64Interval> integerInterval(new HLAfloat64Interval);
  integerInterval->decode(buffer, bufferSize);
  return std::unique_ptr<LogicalTimeInterval>(integerInterval.release());
}

std::wstring
HLAfloat64TimeFactory::getName () const
{
  return L"HLAfloat64Time";
}

}
