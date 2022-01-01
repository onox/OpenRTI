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

#include "AttributeHandleSetCallback.h"

AttributeHandleSetCallback::AttributeHandleSetCallback(const std::vector<OpenRTI::AttributeHandle>& attributeHandleVector) :
  _attributeHandleVector(attributeHandleVector)
{
}

AttributeHandleSetCallback::~AttributeHandleSetCallback()
{
}

RTI::ULong
AttributeHandleSetCallback::size() const
{
  return RTI::ULong(_attributeHandleVector.size());
}

RTI::AttributeHandle
AttributeHandleSetCallback::getHandle(RTI::ULong index) const
  RTI_THROW ((RTI::ArrayIndexOutOfBounds))
{
  if (_attributeHandleVector.size() <= index)
    throw RTI::ArrayIndexOutOfBounds("Array Index out of bounds in getHandle()");
  return _attributeHandleVector[index].getHandle();
}

void
AttributeHandleSetCallback::add(RTI::AttributeHandle handle)
  RTI_THROW ((RTI::ArrayIndexOutOfBounds, RTI::AttributeNotDefined))
{
  throw RTI::RTIinternalError("Unimplemented function: This class is only intendet for constant use.");
}

void
AttributeHandleSetCallback::remove(RTI::AttributeHandle handle)
  RTI_THROW ((RTI::AttributeNotDefined))
{
  throw RTI::RTIinternalError("Unimplemented function: This class is only intendet for constant use.");
}

void
AttributeHandleSetCallback::empty()
{
  throw RTI::RTIinternalError("Unimplemented function: This class is only intendet for constant use.");
}

RTI::Boolean
AttributeHandleSetCallback::isEmpty() const
{
  if (_attributeHandleVector.empty())
    return RTI::RTI_TRUE;
  else
    return RTI::RTI_FALSE;
}

RTI::Boolean
AttributeHandleSetCallback::isMember(RTI::AttributeHandle handle) const
{
  OpenRTI::AttributeHandle attributeHandle = handle;
  for (std::vector<OpenRTI::AttributeHandle>::const_iterator i = _attributeHandleVector.begin(); i != _attributeHandleVector.end(); ++i) {
    if (*i == attributeHandle)
      return RTI::RTI_TRUE;
  }
  return RTI::RTI_FALSE;
}
