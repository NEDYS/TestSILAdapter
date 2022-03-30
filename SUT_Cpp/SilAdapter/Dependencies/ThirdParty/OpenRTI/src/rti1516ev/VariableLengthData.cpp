/* -*-c++-*- OpenRTI - Copyright (C) 2009-2013 Mathias Froehlich
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
#include <limits>
#include <iosfwd>
#include <RTI/VariableLengthData.h>
#include <RTI/Exception.h>
#include "VariableLengthDataImplementation.h"

namespace rti1516ev
{

// Note that the VariableLengthDataFriend implementation relies on
// this method setting impl to zero.
VariableLengthData::VariableLengthData()
  : _impl(std::make_shared<VariableLengthDataImplementation>())
{
}

VariableLengthData::VariableLengthData(void const* data, size_t size)
  : _impl(std::make_shared<VariableLengthDataImplementation>(data, size))
{
}

VariableLengthData::VariableLengthData(VariableLengthData const & rhs)
  : _impl(rhs._impl)
{
}

VariableLengthData::~VariableLengthData() noexcept
{
  _impl.reset();
}

VariableLengthData &
VariableLengthData::operator=(VariableLengthData const & rhs)
{
  _impl = rhs._impl;
  return *this;
}

const void*
VariableLengthData::data() const
{
  if (!_impl)
    return 0;
  return _impl->_variableLengthData.constData();
}

void*
VariableLengthData::data()
{
  if (!_impl)
    return 0;
  return _impl->_variableLengthData.data();
}

size_t
VariableLengthData::size() const
{
  if (!_impl)
    return 0;
  return _impl->_variableLengthData.size();
}


void VariableLengthData::resize(size_t size)
{
  _impl->_variableLengthData.resize(size);
}

void
VariableLengthData::setData(void const * inData, size_t inSize)
{
  // Note that we do not copy the old content here since we
  // will write a new content in any case
  _impl = std::make_shared<VariableLengthDataImplementation>(inData, inSize);
}

void
VariableLengthData::setDataPointer(void* inData, size_t inSize)
{
  /// FIXME: for now copy the external stuff in any case ...
  /// First think about multi threading before optimizing that
  _impl = std::make_shared<VariableLengthDataImplementation>();
  _impl->_variableLengthData.setData(inData, inSize);
  // _impl->setDataPointer(inData, inSize);
}

void
VariableLengthData::takeDataPointer(void* inData, size_t inSize, VariableLengthDataDeleteFunction func)
{
  // FIXME: make use of the delete function!!!
  // Note that we do not copy the old content here since we
  // will write a new content in any case
  _impl = std::make_shared<VariableLengthDataImplementation>();
  _impl->_variableLengthData.takeDataPointer(inData, inSize);
}

}
