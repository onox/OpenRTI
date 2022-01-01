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

// This time, the first include is above the api include.
// the rti1516/Exception header misses that.
#include <iosfwd>

#include "RTI/RTI1516fedTime.h"

#include "RTI/HLAfloat64Time.h"
#include "RTI/HLAfloat64Interval.h"

RTI1516fedTime::RTI1516fedTime()
{
}

RTI1516fedTime::RTI1516fedTime(double value) :
  HLAfloat64Time(value)
{
}

RTI1516fedTime::RTI1516fedTime(const rti1516::LogicalTime& logicalTime) :
  HLAfloat64Time(logicalTime)
{
}

RTI1516fedTime::RTI1516fedTime(const RTI1516fedTime& fedTime) :
  HLAfloat64Time(fedTime)
{
}

RTI1516fedTime::~RTI1516fedTime() RTI_NOEXCEPT
{
}

std::wstring
RTI1516fedTime::implementationName() const
{
  return L"certiFedTime1516";
}


RTI1516fedTimeInterval::RTI1516fedTimeInterval()
{
}

RTI1516fedTimeInterval::RTI1516fedTimeInterval(double value) :
  HLAfloat64Interval(value)
{
}

RTI1516fedTimeInterval::RTI1516fedTimeInterval(const rti1516::LogicalTimeInterval& logicalTimeInterval) :
  HLAfloat64Interval(logicalTimeInterval)
{
}

RTI1516fedTimeInterval::RTI1516fedTimeInterval(const RTI1516fedTimeInterval& logicalTimeInterval) :
  HLAfloat64Interval(logicalTimeInterval)
{
}

RTI1516fedTimeInterval::~RTI1516fedTimeInterval() RTI_NOEXCEPT
{
}

std::wstring
RTI1516fedTimeInterval::implementationName() const
{
  return L"certiFedTime1516";
}

RTI1516fedTimeFactory::RTI1516fedTimeFactory()
{
}

RTI1516fedTimeFactory::~RTI1516fedTimeFactory() RTI_NOEXCEPT
{
}

RTI_UNIQUE_PTR<rti1516::LogicalTime>
RTI1516fedTimeFactory::makeLogicalTime()
  RTI_THROW ((rti1516::InternalError))
{
  return RTI_UNIQUE_PTR<rti1516::LogicalTime>(new RTI1516fedTime);
}

RTI_UNIQUE_PTR<rti1516::LogicalTimeInterval>
RTI1516fedTimeFactory::makeLogicalTimeInterval()
  RTI_THROW ((rti1516::InternalError))
{
  return RTI_UNIQUE_PTR<rti1516::LogicalTimeInterval>(new RTI1516fedTimeInterval);
}
