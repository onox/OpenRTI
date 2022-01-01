/* -*-c++-*- OpenRTI - Copyright (C) 2009-2022 Mathias Froehlich
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

ParameterHandleValuePairSetCallback::ParameterHandleValuePairSetCallback(RTI::TransportType transportType, RTI::OrderType orderType,
                                                                         RTI::Region* region) :
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
  return RTI::ULong(_parameterValues.size());
}

RTI::Handle
ParameterHandleValuePairSetCallback::getHandle(RTI::ULong index) const
  RTI_THROW ((RTI::ArrayIndexOutOfBounds))
{
  if (_parameterValues.size() <= index)
    throw RTI::ArrayIndexOutOfBounds("Array Index out of bounds in getHandle()");
  return _parameterValues[index].getParameterHandle();
}

RTI::ULong
ParameterHandleValuePairSetCallback::getValueLength(RTI::ULong index) const
  RTI_THROW ((RTI::ArrayIndexOutOfBounds))
{
  if (_parameterValues.size() <= index)
    throw RTI::ArrayIndexOutOfBounds("Array Index out of bounds in getHandle()");
  size_t size = _parameterValues[index].getValue().size();
  if (std::numeric_limits<RTI::ULong>::max() < size)
    throw RTI::ArrayIndexOutOfBounds("Data size bigger than length data size");
  return static_cast<RTI::ULong>(size);
}

void
ParameterHandleValuePairSetCallback::getValue(RTI::ULong index, char* data, RTI::ULong& length) const
  RTI_THROW ((RTI::ArrayIndexOutOfBounds))
{
  if (_parameterValues.size() <= index)
    throw RTI::ArrayIndexOutOfBounds("Array Index out of bounds in getHandle()");
  size_t size = _parameterValues[index].getValue().size();
  if (std::numeric_limits<RTI::ULong>::max() < size)
    throw RTI::ArrayIndexOutOfBounds("Data size bigger than length data size");
  length = static_cast<RTI::ULong>(size);
  memcpy(data, _parameterValues[index].getValue().constData(), size);
}

char*
ParameterHandleValuePairSetCallback::getValuePointer(RTI::ULong index, RTI::ULong& length) const
  RTI_THROW ((RTI::ArrayIndexOutOfBounds))
{
  if (_parameterValues.size() <= index)
    throw RTI::ArrayIndexOutOfBounds("Array Index out of bounds in getHandle()");
  size_t size = _parameterValues[index].getValue().size();
  if (std::numeric_limits<RTI::ULong>::max() < size)
    throw RTI::ArrayIndexOutOfBounds("Data size bigger than length data size");
  length = static_cast<RTI::ULong>(size);
  _parameterValues[index].getValue().ensurePrivate();
  return _parameterValues[index].getValue().charData();
}

RTI::TransportType
ParameterHandleValuePairSetCallback::getTransportType() const
  RTI_THROW ((RTI::InvalidHandleValuePairSetContext))
{
  return _transportType;
}

RTI::OrderType
ParameterHandleValuePairSetCallback::getOrderType() const
  RTI_THROW ((RTI::InvalidHandleValuePairSetContext))
{
  return _orderType;
}

RTI::Region*
ParameterHandleValuePairSetCallback::getRegion() const
  RTI_THROW ((RTI::InvalidHandleValuePairSetContext))
{
  return _region;
}

void
ParameterHandleValuePairSetCallback::add(RTI::Handle handle, const char* data, RTI::ULong length)
  RTI_THROW ((RTI::ValueLengthExceeded,
         RTI::ValueCountExceeded))
{
  throw RTI::RTIinternalError("Unimplemented function: This class is only intendet for constant use.");
}

void
ParameterHandleValuePairSetCallback::remove(RTI::Handle handle)
  RTI_THROW ((RTI::ArrayIndexOutOfBounds))
{
  throw RTI::RTIinternalError("Unimplemented function: This class is only intendet for constant use.");
}

void
ParameterHandleValuePairSetCallback::moveFrom(const RTI::ParameterHandleValuePairSet& parameterHandleValuePairSet, RTI::ULong& index)
  RTI_THROW ((RTI::ValueCountExceeded,
         RTI::ArrayIndexOutOfBounds))
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
