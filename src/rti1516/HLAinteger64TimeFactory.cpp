/* -*-c++-*- OpenRTI - Copyright (C) 2009-2023 Mathias Froehlich
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

// This time, the first include is above the api include.
// the rti1516/Exception header misses that.
#include <iosfwd>

#include <RTI/HLAinteger64TimeFactory.h>

#include <memory>
#include <RTI/LogicalTimeFactory.h>
#include <RTI/HLAinteger64Interval.h>
#include <RTI/HLAinteger64Time.h>

HLAinteger64TimeFactory::HLAinteger64TimeFactory()
{
}

HLAinteger64TimeFactory::~HLAinteger64TimeFactory()
  RTI_NOEXCEPT
{
}

RTI_UNIQUE_PTR<rti1516::LogicalTime>
HLAinteger64TimeFactory::makeLogicalTime()
  RTI_THROW ((rti1516::InternalError))
{
  return RTI_UNIQUE_PTR<rti1516::LogicalTime>(new HLAinteger64Time);
}

RTI_UNIQUE_PTR<rti1516::LogicalTimeInterval>
HLAinteger64TimeFactory::makeLogicalTimeInterval()
  RTI_THROW ((rti1516::InternalError))
{
  return RTI_UNIQUE_PTR<rti1516::LogicalTimeInterval>(new HLAinteger64Interval);
}
