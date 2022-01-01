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

#include "AttributeHandleValuePairSetImplementation.h"

AttributeHandleValuePairSetImplementation::AttributeHandleValuePairSetImplementation(RTI::ULong size)
{
  _attributeValues.reserve(size);
}

AttributeHandleValuePairSetImplementation::~AttributeHandleValuePairSetImplementation()
{
}

RTI::ULong
AttributeHandleValuePairSetImplementation::size() const
{
  return RTI::ULong(_attributeValues.size());
}

RTI::Handle
AttributeHandleValuePairSetImplementation::getHandle(RTI::ULong index) const
  RTI_THROW ((RTI::ArrayIndexOutOfBounds))
{
  if (_attributeValues.size() <= index)
    throw RTI::ArrayIndexOutOfBounds("Array Index out of bounds in getHandle()");
  return _attributeValues[index].getAttributeHandle();
}

RTI::ULong
AttributeHandleValuePairSetImplementation::getValueLength(RTI::ULong index) const
  RTI_THROW ((RTI::ArrayIndexOutOfBounds))
{
  if (_attributeValues.size() <= index)
    throw RTI::ArrayIndexOutOfBounds("Array Index out of bounds in getHandle()");
  size_t size = _attributeValues[index].getValue().size();
  if (std::numeric_limits<RTI::ULong>::max() < size)
    throw RTI::ArrayIndexOutOfBounds("Data size bigger than length data size");
  return static_cast<RTI::ULong>(size);
}

void
AttributeHandleValuePairSetImplementation::getValue(RTI::ULong index, char* data, RTI::ULong& length) const
  RTI_THROW ((RTI::ArrayIndexOutOfBounds))
{
  if (_attributeValues.size() <= index)
    throw RTI::ArrayIndexOutOfBounds("Array Index out of bounds in getHandle()");
  size_t size = _attributeValues[index].getValue().size();
  if (std::numeric_limits<RTI::ULong>::max() < size)
    throw RTI::ArrayIndexOutOfBounds("Data size bigger than length data size");
  length = static_cast<RTI::ULong>(size);
  memcpy(data, _attributeValues[index].getValue().constData(), size);
}

char*
AttributeHandleValuePairSetImplementation::getValuePointer(RTI::ULong index, RTI::ULong& length) const
  RTI_THROW ((RTI::ArrayIndexOutOfBounds))
{
  if (_attributeValues.size() <= index)
    throw RTI::ArrayIndexOutOfBounds("Array Index out of bounds in getHandle()");
  size_t size = _attributeValues[index].getValue().size();
  if (std::numeric_limits<RTI::ULong>::max() < size)
    throw RTI::ArrayIndexOutOfBounds("Data size bigger than length data size");
  length = static_cast<RTI::ULong>(size);
  _attributeValues[index].getValue().ensurePrivate();
  return _attributeValues[index].getValue().charData();
}

RTI::TransportType
AttributeHandleValuePairSetImplementation::getTransportType(RTI::ULong index) const
  RTI_THROW ((RTI::InvalidHandleValuePairSetContext))
{
  throw RTI::InvalidHandleValuePairSetContext("Don't ask for the transportation type here!");
}

RTI::OrderType
AttributeHandleValuePairSetImplementation::getOrderType(RTI::ULong index) const
  RTI_THROW ((RTI::ArrayIndexOutOfBounds,
         RTI::InvalidHandleValuePairSetContext))
{
  throw RTI::InvalidHandleValuePairSetContext("Don't ask for the order type here!");
}

RTI::Region*
AttributeHandleValuePairSetImplementation::getRegion(RTI::ULong index) const
  RTI_THROW ((RTI::ArrayIndexOutOfBounds,
         RTI::InvalidHandleValuePairSetContext))
{
  throw RTI::InvalidHandleValuePairSetContext("Don't ask for the region here!");
}

void
AttributeHandleValuePairSetImplementation::add(RTI::Handle handle, const char* data, RTI::ULong length)
  RTI_THROW ((RTI::ValueLengthExceeded,
         RTI::ValueCountExceeded))
{
  OpenRTI::AttributeHandle attributeHandle = handle;
  _attributeValues.push_back(OpenRTI::AttributeValue());
  _attributeValues.back().setAttributeHandle(attributeHandle);
  _attributeValues.back().getValue().setData(data, length);
}

void
AttributeHandleValuePairSetImplementation::remove(RTI::Handle handle)
  RTI_THROW ((RTI::ArrayIndexOutOfBounds))
{
  bool found = false;
  OpenRTI::AttributeHandle attributeHandle = handle;
  for (std::vector<OpenRTI::AttributeValue>::iterator i = _attributeValues.begin(); i != _attributeValues.end();) {
    if (i->getAttributeHandle() != attributeHandle) {
      ++i;
    } else {
      found = true;
      i = _attributeValues.erase(i);
    }
  }
  if (!found)
    throw RTI::ArrayIndexOutOfBounds("Handle not found in remove(Handle)");
}

void
AttributeHandleValuePairSetImplementation::moveFrom(const RTI::AttributeHandleValuePairSet& attributeHandleValuePairSet, RTI::ULong& index)
  RTI_THROW ((RTI::ValueCountExceeded,
         RTI::ArrayIndexOutOfBounds))
{
  throw RTI::RTIinternalError("Unimplemented function: To be honest I did not find any reference about the desired semantics. Provide me, any I implement ...");
}

/// Sigh, should have been called clear???
void
AttributeHandleValuePairSetImplementation::empty()
{
  _attributeValues.clear();
}

/// Do not understand that, but guessing somewhat ...
RTI::ULong
AttributeHandleValuePairSetImplementation::start() const
{
  return 0;
}

RTI::ULong
AttributeHandleValuePairSetImplementation::valid(RTI::ULong i) const
{
  return i < _attributeValues.size();
}

RTI::ULong
AttributeHandleValuePairSetImplementation::next(RTI::ULong i) const
{
  return i+1;
}

RTI::AttributeHandleValuePairSet*
RTI::AttributeSetFactory::create(ULong size)
  RTI_THROW ((RTI::MemoryExhausted, RTI::ValueCountExceeded, RTI::HandleValuePairMaximumExceeded))
{
  return new AttributeHandleValuePairSetImplementation(size);
}
