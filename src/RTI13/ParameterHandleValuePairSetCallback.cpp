/* -*-c++-*- OpenRTI - Copyright (C) 2009-2011 Mathias Froehlich 
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

#include "ParameterHandleValuePairSetCallback.h"

ParameterHandleValuePairSetCallback::ParameterHandleValuePairSetCallback(const std::vector<OpenRTI::ParameterValue>& parameterValues,
                                                                         RTI::TransportType transportType, RTI::OrderType orderType,
                                                                         RTI::Region* region) :
  _parameterValues(parameterValues),
  _transportType(transportType),
  _orderType(orderType),
  _region(region)
{
}

ParameterHandleValuePairSetCallback::~ParameterHandleValuePairSetCallback()
{
}

RTI::ULong
ParameterHandleValuePairSetCallback::size() const
{
  return _parameterValues.size();
}

RTI::Handle
ParameterHandleValuePairSetCallback::getHandle(RTI::ULong index) const
  throw (RTI::ArrayIndexOutOfBounds)
{
  if (_parameterValues.size() <= index)
    throw RTI::ArrayIndexOutOfBounds("Array Index out of bounds in getHandle()");
  return _parameterValues[index].getParameterHandle();
}

RTI::ULong
ParameterHandleValuePairSetCallback::getValueLength(RTI::ULong index) const
  throw (RTI::ArrayIndexOutOfBounds)
{
  if (_parameterValues.size() <= index)
    throw RTI::ArrayIndexOutOfBounds("Array Index out of bounds in getHandle()");
  return _parameterValues[index].getValue().size();
}

void
ParameterHandleValuePairSetCallback::getValue(RTI::ULong index, char* data, RTI::ULong& length) const
  throw (RTI::ArrayIndexOutOfBounds)
{
  if (_parameterValues.size() <= index)
    throw RTI::ArrayIndexOutOfBounds("Array Index out of bounds in getHandle()");
  size_t size = _parameterValues[index].getValue().size();
  if (size < length)
    length = size;
  memcpy(data, _parameterValues[index].getValue().data(), length);
}

char*
ParameterHandleValuePairSetCallback::getValuePointer(RTI::ULong index, RTI::ULong& length) const
  throw (RTI::ArrayIndexOutOfBounds)
{
  if (_parameterValues.size() <= index)
    throw RTI::ArrayIndexOutOfBounds("Array Index out of bounds in getHandle()");
  length = _parameterValues[index].getValue().size();
  _parameterValues[index].getValue().ensurePrivate();
  return _parameterValues[index].getValue().charData();
}

RTI::TransportType
ParameterHandleValuePairSetCallback::getTransportType() const
  throw (RTI::InvalidHandleValuePairSetContext)
{
  return _transportType;
}

RTI::OrderType
ParameterHandleValuePairSetCallback::getOrderType() const
  throw (RTI::InvalidHandleValuePairSetContext)
{
  return _orderType;
}

RTI::Region*
ParameterHandleValuePairSetCallback::getRegion() const
  throw (RTI::InvalidHandleValuePairSetContext)
{
  return _region;
}

void
ParameterHandleValuePairSetCallback::add(RTI::Handle handle, const char* data, RTI::ULong length)
  throw (RTI::ValueLengthExceeded,
         RTI::ValueCountExceeded)
{
  throw RTI::RTIinternalError("Unimplemented function: This class is only intendet for constant use.");
}

void
ParameterHandleValuePairSetCallback::remove(RTI::Handle handle)
  throw (RTI::ArrayIndexOutOfBounds)
{
  throw RTI::RTIinternalError("Unimplemented function: This class is only intendet for constant use.");
}

void
ParameterHandleValuePairSetCallback::moveFrom(const RTI::ParameterHandleValuePairSet& parameterHandleValuePairSet, RTI::ULong& index)
  throw (RTI::ValueCountExceeded,
         RTI::ArrayIndexOutOfBounds)
{
  throw RTI::RTIinternalError("Unimplemented function: This class is only intendet for constant use.");
}

void
ParameterHandleValuePairSetCallback::empty()
{
  throw RTI::RTIinternalError("Unimplemented function: This class is only intendet for constant use.");
}

/// Do not understand that, but guessing somewhat ...
RTI::ULong
ParameterHandleValuePairSetCallback::start() const
{
  return 0;
}

RTI::ULong
ParameterHandleValuePairSetCallback::valid(RTI::ULong i) const
{
  return i < _parameterValues.size();
}

RTI::ULong
ParameterHandleValuePairSetCallback::next(RTI::ULong i) const
{
  return i+1;
}
