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

#include <RTI/time/HLAfloat64TimeFactory.h>

#include <memory>
#include <RTI/LogicalTimeFactory.h>
#include <RTI/time/HLAfloat64Interval.h>
#include <RTI/time/HLAfloat64Time.h>

namespace rti1516e {

HLAfloat64TimeFactory::HLAfloat64TimeFactory()
{
}

HLAfloat64TimeFactory::~HLAfloat64TimeFactory()
  RTI_NOEXCEPT
{
}

RTI_UNIQUE_PTR<HLAfloat64Time>
HLAfloat64TimeFactory::makeLogicalTime(double value)
  RTI_THROW ((InternalError))
{
  return RTI_UNIQUE_PTR<HLAfloat64Time>(new HLAfloat64Time(value));
}

RTI_UNIQUE_PTR<LogicalTime>
HLAfloat64TimeFactory::makeInitial()
  RTI_THROW ((InternalError))
{
  return RTI_UNIQUE_PTR<LogicalTime>(new HLAfloat64Time);
}

RTI_UNIQUE_PTR<LogicalTime> HLAfloat64TimeFactory::makeFinal()
  RTI_THROW ((InternalError))
{
  RTI_UNIQUE_PTR<HLAfloat64Time> integerTime(new HLAfloat64Time);
  integerTime->setFinal();
  return RTI_UNIQUE_PTR<LogicalTime>(integerTime.release());
}

RTI_UNIQUE_PTR<HLAfloat64Interval>
HLAfloat64TimeFactory::makeLogicalTimeInterval(double value)
  RTI_THROW ((InternalError))
{
  return RTI_UNIQUE_PTR<HLAfloat64Interval>(new HLAfloat64Interval(value));
}

RTI_UNIQUE_PTR<LogicalTimeInterval>
HLAfloat64TimeFactory::makeZero()
  RTI_THROW ((InternalError))
{
  return RTI_UNIQUE_PTR<LogicalTimeInterval>(new HLAfloat64Interval);
}

RTI_UNIQUE_PTR<LogicalTimeInterval>
HLAfloat64TimeFactory::makeEpsilon()
  RTI_THROW ((InternalError))
{
  RTI_UNIQUE_PTR<HLAfloat64Interval> integerInterval(new HLAfloat64Interval);
  integerInterval->setEpsilon();
  return RTI_UNIQUE_PTR<LogicalTimeInterval>(integerInterval.release());
}

RTI_UNIQUE_PTR<LogicalTime>
HLAfloat64TimeFactory::decodeLogicalTime(VariableLengthData const & encodedLogicalTime)
  RTI_THROW ((InternalError, CouldNotDecode))
{
  RTI_UNIQUE_PTR<HLAfloat64Time> integerTime(new HLAfloat64Time);
  integerTime->decode(encodedLogicalTime);
  return RTI_UNIQUE_PTR<LogicalTime>(integerTime.release());
}

RTI_UNIQUE_PTR<LogicalTime>
HLAfloat64TimeFactory::decodeLogicalTime(void* buffer, size_t bufferSize)
  RTI_THROW ((InternalError, CouldNotDecode))
{
  RTI_UNIQUE_PTR<HLAfloat64Time> integerTime(new HLAfloat64Time);
  integerTime->decode(buffer, bufferSize);
  return RTI_UNIQUE_PTR<LogicalTime>(integerTime.release());
}

RTI_UNIQUE_PTR<LogicalTimeInterval>
HLAfloat64TimeFactory::decodeLogicalTimeInterval(VariableLengthData const & encodedValue)
  RTI_THROW ((InternalError, CouldNotDecode))
{
  RTI_UNIQUE_PTR<HLAfloat64Interval> integerInterval(new HLAfloat64Interval);
  integerInterval->decode(encodedValue);
  return RTI_UNIQUE_PTR<LogicalTimeInterval>(integerInterval.release());
}

RTI_UNIQUE_PTR<LogicalTimeInterval>
HLAfloat64TimeFactory::decodeLogicalTimeInterval(void* buffer, size_t bufferSize)
  RTI_THROW ((InternalError, CouldNotDecode))
{
  RTI_UNIQUE_PTR<HLAfloat64Interval> integerInterval(new HLAfloat64Interval);
  integerInterval->decode(buffer, bufferSize);
  return RTI_UNIQUE_PTR<LogicalTimeInterval>(integerInterval.release());
}

std::wstring
HLAfloat64TimeFactory::getName () const
{
  return L"HLAfloat64Time";
}

}
