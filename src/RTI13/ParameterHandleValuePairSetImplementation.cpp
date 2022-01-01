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

#include "ParameterHandleValuePairSetImplementation.h"

ParameterHandleValuePairSetImplementation::ParameterHandleValuePairSetImplementation(RTI::ULong size)
{
  _parameterValues.reserve(size);
}

ParameterHandleValuePairSetImplementation::~ParameterHandleValuePairSetImplementation()
{
}

RTI::ULong
ParameterHandleValuePairSetImplementation::size() const
{
  return RTI::ULong(_parameterValues.size());
}

RTI::Handle
ParameterHandleValuePairSetImplementation::getHandle(RTI::ULong index) const
  RTI_THROW ((RTI::ArrayIndexOutOfBounds))
{
  if (_parameterValues.size() <= index)
    throw RTI::ArrayIndexOutOfBounds("Array Index out of bounds in getHandle()");
  return _parameterValues[index].getParameterHandle();
}

RTI::ULong
ParameterHandleValuePairSetImplementation::getValueLength(RTI::ULong index) const
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
ParameterHandleValuePairSetImplementation::getValue(RTI::ULong index, char* data, RTI::ULong& length) const
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
ParameterHandleValuePairSetImplementation::getValuePointer(RTI::ULong index, RTI::ULong& length) const
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
ParameterHandleValuePairSetImplementation::getTransportType() const
  RTI_THROW ((RTI::InvalidHandleValuePairSetContext))
{
  throw RTI::InvalidHandleValuePairSetContext("Don't ask for the transportation type here!");
}

RTI::OrderType
ParameterHandleValuePairSetImplementation::getOrderType() const
  RTI_THROW ((RTI::InvalidHandleValuePairSetContext))
{
  throw RTI::InvalidHandleValuePairSetContext("Don't ask for the order type here!");
}

RTI::Region*
ParameterHandleValuePairSetImplementation::getRegion() const
  RTI_THROW ((RTI::InvalidHandleValuePairSetContext))
{
  throw RTI::InvalidHandleValuePairSetContext("Don't ask for the region here!");
}

void
ParameterHandleValuePairSetImplementation::add(RTI::Handle handle, const char* data, RTI::ULong length)
  RTI_THROW ((RTI::ValueLengthExceeded,
         RTI::ValueCountExceeded))
{
  OpenRTI::ParameterHandle parameterHandle = handle;
  _parameterValues.push_back(OpenRTI::ParameterValue());
  _parameterValues.back().setParameterHandle(parameterHandle);
  _parameterValues.back().getValue().setData(data, length);
}

void
ParameterHandleValuePairSetImplementation::remove(RTI::Handle handle)
  RTI_THROW ((RTI::ArrayIndexOutOfBounds))
{
  bool found = false;
  OpenRTI::ParameterHandle parameterHandle = handle;
  for (std::vector<OpenRTI::ParameterValue>::iterator i = _parameterValues.begin(); i != _parameterValues.end();) {
    if (i->getParameterHandle() != parameterHandle) {
      ++i;
    } else {
      found = true;
      i = _parameterValues.erase(i);
    }
  }
  if (!found)
    throw RTI::ArrayIndexOutOfBounds("Handle not found in remove(Handle)");
}

void
ParameterHandleValuePairSetImplementation::moveFrom(const RTI::ParameterHandleValuePairSet& parameterHandleValuePairSet, RTI::ULong& index)
  RTI_THROW ((RTI::ValueCountExceeded,
         RTI::ArrayIndexOutOfBounds))
{
  throw RTI::RTIinternalError("Unimplemented function: To be honest I did not find any reference about the desired semantics. Provide me, any I implement ...");
}

/// Sigh, should have been called clear???
void
ParameterHandleValuePairSetImplementation::empty()
{
  _parameterValues.clear();
}

/// Do not understand that, but guessing somewhat ...
RTI::ULong
ParameterHandleValuePairSetImplementation::start() const
{
  return 0;
}

RTI::ULong
ParameterHandleValuePairSetImplementation::valid(RTI::ULong i) const
{
  return i < _parameterValues.size();
}

RTI::ULong
ParameterHandleValuePairSetImplementation::next(RTI::ULong i) const
{
  return i+1;
}

RTI::ParameterHandleValuePairSet*
RTI::ParameterSetFactory::create(ULong size)
  RTI_THROW ((RTI::MemoryExhausted, RTI::ValueCountExceeded, RTI::HandleValuePairMaximumExceeded))
{
  return new ParameterHandleValuePairSetImplementation(size);
}
