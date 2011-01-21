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

#include <RTI/time/HLAinteger64TimeFactory.h>

#include <memory>
#include <RTI/LogicalTimeFactory.h>
#include <RTI/time/HLAinteger64Interval.h>
#include <RTI/time/HLAinteger64Time.h>

namespace rti1516e {

HLAinteger64TimeFactory::HLAinteger64TimeFactory()
{
}

HLAinteger64TimeFactory::~HLAinteger64TimeFactory()
  throw ()
{
}

std::auto_ptr<HLAinteger64Time>
HLAinteger64TimeFactory::makeLogicalTime(Integer64 value)
  throw (InternalError)
{
  return std::auto_ptr<HLAinteger64Time>(new HLAinteger64Time(value));
}

std::auto_ptr<LogicalTime>
HLAinteger64TimeFactory::makeInitial()
  throw (InternalError)
{
  return std::auto_ptr<LogicalTime>(new HLAinteger64Time);
}

std::auto_ptr<LogicalTime> HLAinteger64TimeFactory::makeFinal()
  throw (InternalError)
{
  std::auto_ptr<HLAinteger64Time> integerTime(new HLAinteger64Time);
  integerTime->setInitial();
  return std::auto_ptr<LogicalTime>(integerTime.release());
}

std::auto_ptr<HLAinteger64Interval>
HLAinteger64TimeFactory::makeLogicalTimeInterval(Integer64 value)
  throw (InternalError)
{
  return std::auto_ptr<HLAinteger64Interval>(new HLAinteger64Interval(value));
}

std::auto_ptr<LogicalTimeInterval>
HLAinteger64TimeFactory::makeZero()
  throw (InternalError)
{
  return std::auto_ptr<LogicalTimeInterval>(new HLAinteger64Interval);
}

std::auto_ptr<LogicalTimeInterval>
HLAinteger64TimeFactory::makeEpsilon()
  throw (InternalError)
{
  std::auto_ptr<HLAinteger64Interval> integerInterval(new HLAinteger64Interval);
  integerInterval->setEpsilon();
  return std::auto_ptr<LogicalTimeInterval>(integerInterval.release());
}

std::auto_ptr<LogicalTime>
HLAinteger64TimeFactory::decodeLogicalTime(VariableLengthData const & encodedLogicalTime)
  throw (InternalError, CouldNotDecode)
{
  std::auto_ptr<HLAinteger64Time> integerTime(new HLAinteger64Time);
  integerTime->decode(encodedLogicalTime);
  return std::auto_ptr<LogicalTime>(integerTime.release());
}

std::auto_ptr<LogicalTime>
HLAinteger64TimeFactory::decodeLogicalTime(void* buffer, size_t bufferSize)
  throw (InternalError, CouldNotDecode)
{
  std::auto_ptr<HLAinteger64Time> integerTime(new HLAinteger64Time);
  integerTime->decode(buffer, bufferSize);
  return std::auto_ptr<LogicalTime>(integerTime.release());
}

std::auto_ptr<LogicalTimeInterval>
HLAinteger64TimeFactory::decodeLogicalTimeInterval(VariableLengthData const & encodedValue)
  throw (InternalError, CouldNotDecode)
{
  std::auto_ptr<HLAinteger64Interval> integerInterval(new HLAinteger64Interval);
  integerInterval->decode(encodedValue);
  return std::auto_ptr<LogicalTimeInterval>(integerInterval.release());
}

std::auto_ptr<LogicalTimeInterval>
HLAinteger64TimeFactory::decodeLogicalTimeInterval(void* buffer, size_t bufferSize)
  throw (InternalError, CouldNotDecode)
{
  std::auto_ptr<HLAinteger64Interval> integerInterval(new HLAinteger64Interval);
  integerInterval->decode(buffer, bufferSize);
  return std::auto_ptr<LogicalTimeInterval>(integerInterval.release());
}

std::wstring
HLAinteger64TimeFactory::getName () const
{
  return L"HLAinteger64Time";
}

}
