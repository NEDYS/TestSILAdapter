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

#ifndef ParameterHandleValuePairSetCallback_h
#define ParameterHandleValuePairSetCallback_h

/// The original RTI13 include
#include <RTI.hh>

#include "Message.h"

class OPENRTI_LOCAL ParameterHandleValuePairSetCallback : public RTI::ParameterHandleValuePairSet {
public:
  ParameterHandleValuePairSetCallback(RTI::TransportType transportType, RTI::OrderType orderType, RTI::Region* region);
  virtual ~ParameterHandleValuePairSetCallback();

  virtual RTI::ULong size() const;
  virtual RTI::Handle getHandle(RTI::ULong index) const;
  virtual RTI::ULong getValueLength(RTI::ULong index) const;
  virtual void getValue(RTI::ULong index, char* data, RTI::ULong& length) const;
  virtual char* getValuePointer(RTI::ULong index, RTI::ULong& length) const;
  virtual RTI::TransportType getTransportType() const;
  virtual RTI::OrderType getOrderType() const;
  virtual RTI::Region* getRegion() const;
  virtual void add(RTI::Handle handle, const char* data, RTI::ULong length);
  virtual void remove(RTI::Handle handle);
  virtual void moveFrom(const RTI::ParameterHandleValuePairSet& , RTI::ULong& );
  virtual void empty();
  virtual RTI::ULong start() const;
  virtual RTI::ULong valid(RTI::ULong i) const;
  virtual RTI::ULong next(RTI::ULong i) const;

  // Accessor used to build up the value
  std::vector<OpenRTI::ParameterValue>& getParameterValues()
  { return _parameterValues; }

private:
  mutable std::vector<OpenRTI::ParameterValue> _parameterValues;
  RTI::TransportType _transportType;
  RTI::OrderType _orderType;
  RTI::Region* _region;
};

#endif
