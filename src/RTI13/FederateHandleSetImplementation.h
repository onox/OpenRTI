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

#ifndef FederateHandleSetImplementation_h
#define FederateHandleSetImplementation_h

/// The original RTI13 include
#include <RTI.hh>

#include <vector>
#include "Handle.h"

class OPENRTI_LOCAL FederateHandleSetImplementation : public RTI::FederateHandleSet {
public:
  FederateHandleSetImplementation(RTI::ULong size);
  virtual ~FederateHandleSetImplementation();

  virtual RTI::ULong size() const;
  virtual RTI::FederateHandle getHandle(RTI::ULong index) const
    RTI_THROW ((RTI::ArrayIndexOutOfBounds));
  virtual void add(RTI::FederateHandle handle)
    RTI_THROW ((RTI::ValueCountExceeded));
  virtual void remove(RTI::FederateHandle handle)
    RTI_THROW ((RTI::ArrayIndexOutOfBounds));
  virtual void empty();
  virtual RTI::Boolean isMember(RTI::FederateHandle handle) const;

private:
  std::vector<OpenRTI::FederateHandle> _federateHandleVector;
};

#endif
