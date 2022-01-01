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

#include "FederateHandleSetImplementation.h"

FederateHandleSetImplementation::FederateHandleSetImplementation(RTI::ULong size)
{
  _federateHandleVector.reserve(size);
}

FederateHandleSetImplementation::~FederateHandleSetImplementation()
{
}

RTI::ULong
FederateHandleSetImplementation::size() const
{
  return RTI::ULong(_federateHandleVector.size());
}

RTI::FederateHandle
FederateHandleSetImplementation::getHandle(RTI::ULong index) const
  RTI_THROW ((RTI::ArrayIndexOutOfBounds))
{
  if (_federateHandleVector.size() <= index)
    throw RTI::ArrayIndexOutOfBounds("Array Index out of bounds in getHandle()");
  return _federateHandleVector[index].getHandle();
}

void
FederateHandleSetImplementation::add(RTI::FederateHandle handle)
  RTI_THROW ((RTI::ValueCountExceeded))
{
  OpenRTI::FederateHandle federateHandle = handle;
  _federateHandleVector.push_back(federateHandle);
}

void
FederateHandleSetImplementation::remove(RTI::FederateHandle handle)
  RTI_THROW ((RTI::ArrayIndexOutOfBounds))
{
  bool found = false;
  OpenRTI::FederateHandle federateHandle = handle;
  for (std::vector<OpenRTI::FederateHandle>::iterator i = _federateHandleVector.begin(); i != _federateHandleVector.end();) {
    if (*i != federateHandle) {
      ++i;
    } else {
      found = true;
      i = _federateHandleVector.erase(i);
    }
  }
  if (!found)
    throw RTI::ArrayIndexOutOfBounds("Handle not found in remove(Handle)");
}

void
FederateHandleSetImplementation::empty()
{
  _federateHandleVector.clear();
}

RTI::Boolean
FederateHandleSetImplementation::isMember(RTI::FederateHandle handle) const
{
  OpenRTI::FederateHandle federateHandle = handle;
  for (std::vector<OpenRTI::FederateHandle>::const_iterator i = _federateHandleVector.begin(); i != _federateHandleVector.end(); ++i) {
    if (*i == federateHandle)
      return RTI::RTI_TRUE;
  }
  return RTI::RTI_FALSE;
}

RTI::FederateHandleSet*
RTI::FederateHandleSetFactory::create(ULong size)
  RTI_THROW ((MemoryExhausted, ValueCountExceeded))
{
  return new FederateHandleSetImplementation(size);
}
