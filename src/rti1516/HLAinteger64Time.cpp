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

#include "RTI/HLAinteger64Time.h"

#include <iomanip>
#include <limits>
#include <sstream>
#include "VariableLengthDataFriend.h"
#include "ValueImplementation.h"
#include "RTI/HLAinteger64Interval.h"

DECLARE_VALUE_IMPLEMENTATION(HLAinteger64TimeImpl, int64_t)

static const HLAinteger64Time& toHLAinteger64Time(const rti1516::LogicalTime& logicalTime)
{
  const HLAinteger64Time* integer64Time = dynamic_cast<const HLAinteger64Time*>(&logicalTime);
  if (!integer64Time)
    throw rti1516::InvalidLogicalTime(logicalTime.implementationName());
  return *integer64Time;
}

static const HLAinteger64Interval& toHLAinteger64Interval(const rti1516::LogicalTimeInterval& logicalTimeInterval)
{
  const HLAinteger64Interval* integer64Interval = dynamic_cast<const HLAinteger64Interval*>(&logicalTimeInterval);
  if (!integer64Interval)
    throw rti1516::InvalidLogicalTimeInterval(logicalTimeInterval.implementationName());
  return *integer64Interval;
}

HLAinteger64Time::HLAinteger64Time() :
  _impl(0)
{
}

HLAinteger64Time::HLAinteger64Time(Integer64 value) :
  _impl(0)
{
  HLAinteger64TimeImpl::setValue(_impl, value);
}

HLAinteger64Time::HLAinteger64Time(const rti1516::LogicalTime& logicalTime) :
  _impl(0)
{
  HLAinteger64TimeImpl::assign(_impl, toHLAinteger64Time(logicalTime)._impl);
}

HLAinteger64Time::HLAinteger64Time(const HLAinteger64Time& integer64Time) :
  _impl(0)
{
  HLAinteger64TimeImpl::assign(_impl, integer64Time._impl);
}

HLAinteger64Time::~HLAinteger64Time() RTI_NOEXCEPT
{
  HLAinteger64TimeImpl::putAndDelete(_impl);
}

void
HLAinteger64Time::setInitial()
{
  HLAinteger64TimeImpl::setValue(_impl, 0);
}

bool
HLAinteger64Time::isInitial() const
{
  return 0 == HLAinteger64TimeImpl::getValue(_impl);
}

void
HLAinteger64Time::setFinal()
{
  HLAinteger64TimeImpl::setValue(_impl, std::numeric_limits<int64_t>::max());
}

bool
HLAinteger64Time::isFinal() const
{
  return std::numeric_limits<int64_t>::max() == HLAinteger64TimeImpl::getValue(_impl);
}

HLAinteger64Time&
HLAinteger64Time::operator=(const rti1516::LogicalTime& logicalTime)
  RTI_THROW ((rti1516::InvalidLogicalTime))
{
  HLAinteger64TimeImpl::assign(_impl, toHLAinteger64Time(logicalTime)._impl);
  return *this;
}

HLAinteger64Time&
HLAinteger64Time::operator+=(const rti1516::LogicalTimeInterval& logicalTimeInterval)
  RTI_THROW ((rti1516::IllegalTimeArithmetic, rti1516::InvalidLogicalTimeInterval))
{
  int64_t interval = toHLAinteger64Interval(logicalTimeInterval).getInterval();
  int64_t value = HLAinteger64TimeImpl::getValue(_impl);
  if (0 < interval) {
    if (std::numeric_limits<int64_t>::max() - interval < value) {
      value = std::numeric_limits<int64_t>::max();
    } else {
      value += interval;
    }
  } else if (interval < 0) {
    if (value < std::numeric_limits<int64_t>::min() - interval) {
      value = std::numeric_limits<int64_t>::min();
    } else {
      value += interval;
    }
  } else /* if (interval == 0) */ {
  }
  HLAinteger64TimeImpl::setValue(_impl, value);
  return *this;
}

HLAinteger64Time&
HLAinteger64Time::operator-=(const rti1516::LogicalTimeInterval& logicalTimeInterval)
  RTI_THROW ((rti1516::IllegalTimeArithmetic, rti1516::InvalidLogicalTimeInterval))
{
  int64_t interval = toHLAinteger64Interval(logicalTimeInterval).getInterval();
  int64_t value = HLAinteger64TimeImpl::getValue(_impl);
  if (0 < interval) {
    if (value < std::numeric_limits<int64_t>::min() + interval) {
      value = std::numeric_limits<int64_t>::min();
    } else {
      value -= interval;
    }
  } else if (interval < 0) {
    if (std::numeric_limits<int64_t>::max() + interval < value) {
      value = std::numeric_limits<int64_t>::max();
    } else {
      value -= interval;
    }
  } else /* if (interval == 0) */ {
  }
  HLAinteger64TimeImpl::setValue(_impl, value);
  return *this;
}

bool
HLAinteger64Time::operator>(const rti1516::LogicalTime& logicalTime) const
  RTI_THROW ((rti1516::InvalidLogicalTime))
{
  return HLAinteger64TimeImpl::getValue(_impl) > toHLAinteger64Time(logicalTime).getTime();
}

bool
HLAinteger64Time::operator<(const rti1516::LogicalTime& logicalTime) const
    RTI_THROW ((rti1516::InvalidLogicalTime))
{
  return HLAinteger64TimeImpl::getValue(_impl) < toHLAinteger64Time(logicalTime).getTime();
}

bool
HLAinteger64Time::operator==(const rti1516::LogicalTime& logicalTime) const
    RTI_THROW ((rti1516::InvalidLogicalTime))
{
  return HLAinteger64TimeImpl::getValue(_impl) == toHLAinteger64Time(logicalTime).getTime();
}

bool
HLAinteger64Time::operator>=(const rti1516::LogicalTime& logicalTime) const
    RTI_THROW ((rti1516::InvalidLogicalTime))
{
  return HLAinteger64TimeImpl::getValue(_impl) >= toHLAinteger64Time(logicalTime).getTime();
}

bool
HLAinteger64Time::operator<=(const rti1516::LogicalTime& logicalTime) const
    RTI_THROW ((rti1516::InvalidLogicalTime))
{
  return HLAinteger64TimeImpl::getValue(_impl) <= toHLAinteger64Time(logicalTime).getTime();
}

rti1516::VariableLengthData
HLAinteger64Time::encode() const
{
  OpenRTI::VariableLengthData variableLengthData(8);
  variableLengthData.setInt64BE(HLAinteger64TimeImpl::getValue(_impl), 0);
  return rti1516::VariableLengthDataFriend::create(variableLengthData);
}

unsigned long
HLAinteger64Time::encodedLength() const
{
  return 8;
}

unsigned long
HLAinteger64Time::encode(void* buffer, unsigned long bufferSize) const
    RTI_THROW ((rti1516::CouldNotEncode))
{
  if (bufferSize < 8)
    throw rti1516::CouldNotEncode(L"Buffer size too short!");

  uint64_t u = HLAinteger64TimeImpl::getValue(_impl);
  uint8_t* data = static_cast<uint8_t*>(buffer);
  data[0] = uint8_t(u >> 56);
  data[1] = uint8_t(u >> 48);
  data[2] = uint8_t(u >> 40);
  data[3] = uint8_t(u >> 32);
  data[4] = uint8_t(u >> 24);
  data[5] = uint8_t(u >> 16);
  data[6] = uint8_t(u >> 8);
  data[7] = uint8_t(u);
  return 8;
}

void
HLAinteger64Time::decode(const rti1516::VariableLengthData& variableLengthData)
    RTI_THROW ((rti1516::InternalError, rti1516::CouldNotDecode))
{
  const OpenRTI::VariableLengthData& data = rti1516::VariableLengthDataFriend::readPointer(variableLengthData);
  if (data.size() < 8)
    throw rti1516::CouldNotDecode(L"Buffer size too short!");
  HLAinteger64TimeImpl::setValue(_impl, data.getInt64BE(0));
}

void
HLAinteger64Time::decode(void* buffer, unsigned long bufferSize)
    RTI_THROW ((rti1516::InternalError, rti1516::CouldNotDecode))
{
  if (bufferSize < 8)
    throw rti1516::CouldNotDecode(L"Buffer size too short!");

  const uint8_t* data = static_cast<const uint8_t*>(buffer);
  uint64_t u = uint64_t(data[0]) << 56;
  u |= uint64_t(data[1]) << 48;
  u |= uint64_t(data[2]) << 40;
  u |= uint64_t(data[3]) << 32;
  u |= uint64_t(data[4]) << 24;
  u |= uint64_t(data[5]) << 16;
  u |= uint64_t(data[6]) << 8;
  u |= uint64_t(data[7]);
  HLAinteger64TimeImpl::setValue(_impl, u);
}

std::wstring
HLAinteger64Time::toString() const
{
  std::wstringstream stream;
  stream << HLAinteger64TimeImpl::getValue(_impl);
  return stream.str();
}

std::wstring
HLAinteger64Time::implementationName() const
{
  return L"HLAinteger64Time";
}

Integer64
HLAinteger64Time::getTime() const
{
  return HLAinteger64TimeImpl::getValue(_impl);
}

void
HLAinteger64Time::setTime(Integer64 value)
{
  HLAinteger64TimeImpl::setValue(_impl, value);
}

HLAinteger64Time&
HLAinteger64Time::operator=(const HLAinteger64Time& integer64Time)
{
  HLAinteger64TimeImpl::assign(_impl, integer64Time._impl);
  return *this;
}

HLAinteger64Time::operator Integer64() const
{
  return getTime();
}
