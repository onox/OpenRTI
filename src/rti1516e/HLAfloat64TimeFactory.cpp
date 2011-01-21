/* -*-c++-*- OpenRTI - Copyright (C) 2009-2011 Mathias Froehlich
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
  throw ()
{
}

std::auto_ptr<HLAfloat64Time>
HLAfloat64TimeFactory::makeLogicalTime(double value)
  throw (InternalError)
{
  return std::auto_ptr<HLAfloat64Time>(new HLAfloat64Time(value));
}

std::auto_ptr<LogicalTime>
HLAfloat64TimeFactory::makeInitial()
  throw (InternalError)
{
  return std::auto_ptr<LogicalTime>(new HLAfloat64Time);
}

std::auto_ptr<LogicalTime> HLAfloat64TimeFactory::makeFinal()
  throw (InternalError)
{
  std::auto_ptr<HLAfloat64Time> integerTime(new HLAfloat64Time);
  integerTime->setInitial();
  return std::auto_ptr<LogicalTime>(integerTime.release());
}

std::auto_ptr<HLAfloat64Interval>
HLAfloat64TimeFactory::makeLogicalTimeInterval(double value)
  throw (InternalError)
{
  return std::auto_ptr<HLAfloat64Interval>(new HLAfloat64Interval(value));
}

std::auto_ptr<LogicalTimeInterval>
HLAfloat64TimeFactory::makeZero()
  throw (InternalError)
{
  return std::auto_ptr<LogicalTimeInterval>(new HLAfloat64Interval);
}

std::auto_ptr<LogicalTimeInterval>
HLAfloat64TimeFactory::makeEpsilon()
  throw (InternalError)
{
  std::auto_ptr<HLAfloat64Interval> integerInterval(new HLAfloat64Interval);
  integerInterval->setEpsilon();
  return std::auto_ptr<LogicalTimeInterval>(integerInterval.release());
}

std::auto_ptr<LogicalTime>
HLAfloat64TimeFactory::decodeLogicalTime(VariableLengthData const & encodedLogicalTime)
  throw (InternalError, CouldNotDecode)
{
  std::auto_ptr<HLAfloat64Time> integerTime(new HLAfloat64Time);
  integerTime->decode(encodedLogicalTime);
  return std::auto_ptr<LogicalTime>(integerTime.release());
}

std::auto_ptr<LogicalTime>
HLAfloat64TimeFactory::decodeLogicalTime(void* buffer, size_t bufferSize)
  throw (InternalError, CouldNotDecode)
{
  std::auto_ptr<HLAfloat64Time> integerTime(new HLAfloat64Time);
  integerTime->decode(buffer, bufferSize);
  return std::auto_ptr<LogicalTime>(integerTime.release());
}

std::auto_ptr<LogicalTimeInterval>
HLAfloat64TimeFactory::decodeLogicalTimeInterval(VariableLengthData const & encodedValue)
  throw (InternalError, CouldNotDecode)
{
  std::auto_ptr<HLAfloat64Interval> integerInterval(new HLAfloat64Interval);
  integerInterval->decode(encodedValue);
  return std::auto_ptr<LogicalTimeInterval>(integerInterval.release());
}

std::auto_ptr<LogicalTimeInterval>
HLAfloat64TimeFactory::decodeLogicalTimeInterval(void* buffer, size_t bufferSize)
  throw (InternalError, CouldNotDecode)
{
  std::auto_ptr<HLAfloat64Interval> integerInterval(new HLAfloat64Interval);
  integerInterval->decode(buffer, bufferSize);
  return std::auto_ptr<LogicalTimeInterval>(integerInterval.release());
}

std::wstring
HLAfloat64TimeFactory::getName () const
{
  return L"HLAfloat64Time";
}

}
