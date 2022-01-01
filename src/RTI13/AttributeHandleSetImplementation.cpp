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

#include "AttributeHandleSetImplementation.h"

AttributeHandleSetImplementation::AttributeHandleSetImplementation(RTI::ULong size)
{
  _attributeHandleVector.reserve(size);
}

AttributeHandleSetImplementation::~AttributeHandleSetImplementation()
{
}

RTI::ULong
AttributeHandleSetImplementation::size() const
{
  return RTI::ULong(_attributeHandleVector.size());
}

RTI::AttributeHandle
AttributeHandleSetImplementation::getHandle(RTI::ULong index) const
  RTI_THROW ((RTI::ArrayIndexOutOfBounds))
{
  if (_attributeHandleVector.size() <= index)
    throw RTI::ArrayIndexOutOfBounds("Array Index out of bounds in getHandle()");
  return _attributeHandleVector[index].getHandle();
}

void
AttributeHandleSetImplementation::add(RTI::AttributeHandle handle)
  RTI_THROW ((RTI::ArrayIndexOutOfBounds, RTI::AttributeNotDefined))
{
  OpenRTI::AttributeHandle attributeHandle = handle;
  _attributeHandleVector.push_back(attributeHandle);
}

void
AttributeHandleSetImplementation::remove(RTI::AttributeHandle handle)
  RTI_THROW ((RTI::AttributeNotDefined))
{
  bool found = false;
  OpenRTI::AttributeHandle attributeHandle = handle;
  for (std::vector<OpenRTI::AttributeHandle>::iterator i = _attributeHandleVector.begin(); i != _attributeHandleVector.end();) {
    if (*i != attributeHandle) {
      ++i;
    } else {
      found = true;
      i = _attributeHandleVector.erase(i);
    }
  }
  if (!found)
    throw RTI::ArrayIndexOutOfBounds("Handle not found in remove(Handle)");
}

void
AttributeHandleSetImplementation::empty()
{
  _attributeHandleVector.clear();
}

RTI::Boolean
AttributeHandleSetImplementation::isEmpty() const
{
  if (_attributeHandleVector.empty())
    return RTI::RTI_TRUE;
  else
    return RTI::RTI_FALSE;
}

RTI::Boolean
AttributeHandleSetImplementation::isMember(RTI::AttributeHandle handle) const
{
  OpenRTI::AttributeHandle attributeHandle = handle;
  for (std::vector<OpenRTI::AttributeHandle>::const_iterator i = _attributeHandleVector.begin(); i != _attributeHandleVector.end(); ++i) {
    if (*i == attributeHandle)
      return RTI::RTI_TRUE;
  }
  return RTI::RTI_FALSE;
}

RTI::AttributeHandleSet*
RTI::AttributeHandleSetFactory::create(ULong size)
  RTI_THROW((MemoryExhausted, ValueCountExceeded))
{
  return new AttributeHandleSetImplementation(size);
}
