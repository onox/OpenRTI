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

#include "RTI/time/HLAinteger64Interval.h"

#include <iomanip>
#include <limits>
#include <sstream>
#include "VariableLengthDataFriend.h"
#include "ValueImplementation.h"
#include "RTI/time/HLAinteger64Time.h"

namespace rti1516e {

DECLARE_VALUE_IMPLEMENTATION(HLAinteger64IntervalImpl, int64_t)

static const HLAinteger64Time& toHLAinteger64Time(const LogicalTime& logicalTime)
{
  const HLAinteger64Time* integer64Time = dynamic_cast<const HLAinteger64Time*>(&logicalTime);
  if (!integer64Time)
    throw InvalidLogicalTime(logicalTime.implementationName());
  return *integer64Time;
}

static const HLAinteger64Interval& toHLAinteger64Interval(const LogicalTimeInterval& logicalTimeInterval)
{
  const HLAinteger64Interval* integer64Interval = dynamic_cast<const HLAinteger64Interval*>(&logicalTimeInterval);
  if (!integer64Interval)
    throw InvalidLogicalTimeInterval(logicalTimeInterval.implementationName());
  return *integer64Interval;
}

HLAinteger64Interval::HLAinteger64Interval() :
  _impl(0)
{
}

HLAinteger64Interval::HLAinteger64Interval(Integer64 time) :
  _impl(0)
{
  HLAinteger64IntervalImpl::setValue(_impl, time);
}

HLAinteger64Interval::HLAinteger64Interval(const LogicalTimeInterval& logicalTimeInterval) :
  _impl(0)
{
  HLAinteger64IntervalImpl::assign(_impl, toHLAinteger64Interval(logicalTimeInterval)._impl);
}

HLAinteger64Interval::HLAinteger64Interval(const HLAinteger64Interval& integer64Interval) :
  _impl(0)
{
  HLAinteger64IntervalImpl::assign(_impl, integer64Interval._impl);
}

HLAinteger64Interval::~HLAinteger64Interval() RTI_NOEXCEPT
{
  HLAinteger64IntervalImpl::putAndDelete(_impl);
}

void
HLAinteger64Interval::setZero()
{
  HLAinteger64IntervalImpl::setValue(_impl, 0);
}

bool
HLAinteger64Interval::isZero() const
{
  return 0 == HLAinteger64IntervalImpl::getValue(_impl);
}

void
HLAinteger64Interval::setEpsilon()
{
  HLAinteger64IntervalImpl::setValue(_impl, 1);
}

bool
HLAinteger64Interval::isEpsilon() const
{
  return 1 == HLAinteger64IntervalImpl::getValue(_impl);
}

LogicalTimeInterval&
HLAinteger64Interval::operator=(const LogicalTimeInterval& logicalTimeInterval)
  RTI_THROW ((InvalidLogicalTimeInterval))
{
  HLAinteger64IntervalImpl::assign(_impl, toHLAinteger64Interval(logicalTimeInterval)._impl);
  return *this;
}

void
HLAinteger64Interval::setToDifference(const LogicalTime& minuend, const LogicalTime& subtrahend)
  RTI_THROW ((IllegalTimeArithmetic, InvalidLogicalTime))
{
  HLAinteger64IntervalImpl::setValue(_impl, toHLAinteger64Time(minuend).getTime() - toHLAinteger64Time(subtrahend).getTime());
}

LogicalTimeInterval&
HLAinteger64Interval::operator+=(const LogicalTimeInterval& logicalTimeInterval)
  RTI_THROW ((IllegalTimeArithmetic, InvalidLogicalTimeInterval))
{
  int64_t interval = HLAinteger64IntervalImpl::getValue(toHLAinteger64Interval(logicalTimeInterval)._impl);
  int64_t value = HLAinteger64IntervalImpl::getValue(_impl);
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
  HLAinteger64IntervalImpl::setValue(_impl, value);
  return *this;
}

LogicalTimeInterval&
HLAinteger64Interval::operator-=(const LogicalTimeInterval& logicalTimeInterval)
  RTI_THROW ((IllegalTimeArithmetic, InvalidLogicalTimeInterval))
{
  int64_t interval = HLAinteger64IntervalImpl::getValue(toHLAinteger64Interval(logicalTimeInterval)._impl);
  int64_t value = HLAinteger64IntervalImpl::getValue(_impl);
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
  HLAinteger64IntervalImpl::setValue(_impl, value);
  return *this;
}

bool
HLAinteger64Interval::operator>(const LogicalTimeInterval& logicalTimeInterval) const
  RTI_THROW ((InvalidLogicalTimeInterval))
{
  return HLAinteger64IntervalImpl::getValue(_impl) > HLAinteger64IntervalImpl::getValue(toHLAinteger64Interval(logicalTimeInterval)._impl);
}

bool
HLAinteger64Interval::operator<(const LogicalTimeInterval& logicalTimeInterval) const
  RTI_THROW ((InvalidLogicalTimeInterval))
{
  return HLAinteger64IntervalImpl::getValue(_impl) < HLAinteger64IntervalImpl::getValue(toHLAinteger64Interval(logicalTimeInterval)._impl);
}

bool
HLAinteger64Interval::operator==(const LogicalTimeInterval& logicalTimeInterval) const
  RTI_THROW ((InvalidLogicalTimeInterval))
{
  return HLAinteger64IntervalImpl::getValue(_impl) == HLAinteger64IntervalImpl::getValue(toHLAinteger64Interval(logicalTimeInterval)._impl);
}

bool
HLAinteger64Interval::operator>=(const LogicalTimeInterval& logicalTimeInterval) const
  RTI_THROW ((InvalidLogicalTimeInterval))
{
  return HLAinteger64IntervalImpl::getValue(_impl) >= HLAinteger64IntervalImpl::getValue(toHLAinteger64Interval(logicalTimeInterval)._impl);
}

bool
HLAinteger64Interval::operator<=(const LogicalTimeInterval& logicalTimeInterval) const
  RTI_THROW ((InvalidLogicalTimeInterval))
{
  return HLAinteger64IntervalImpl::getValue(_impl) <= HLAinteger64IntervalImpl::getValue(toHLAinteger64Interval(logicalTimeInterval)._impl);
}

VariableLengthData
HLAinteger64Interval::encode() const
{
  OpenRTI::VariableLengthData variableLengthData(8);
  variableLengthData.setInt64BE(HLAinteger64IntervalImpl::getValue(_impl), 0);
  return VariableLengthDataFriend::create(variableLengthData);
}

size_t
HLAinteger64Interval::encodedLength() const
{
  return 8;
}

size_t
HLAinteger64Interval::encode(void* buffer, size_t bufferSize) const
  RTI_THROW ((CouldNotEncode))
{
  if (bufferSize < 8)
    throw CouldNotEncode(L"Buffer size too short!");

  uint64_t u = HLAinteger64IntervalImpl::getValue(_impl);
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
HLAinteger64Interval::decode(const VariableLengthData& variableLengthData)
  RTI_THROW ((InternalError, CouldNotDecode))
{
  const OpenRTI::VariableLengthData& data = VariableLengthDataFriend::readPointer(variableLengthData);
  if (data.size() < 8)
    throw CouldNotDecode(L"Buffer size too short!");
  HLAinteger64IntervalImpl::setValue(_impl, data.getInt64BE(0));
}

void
HLAinteger64Interval::decode(void* buffer, size_t bufferSize)
  RTI_THROW ((InternalError, CouldNotDecode))
{
  if (bufferSize < 8)
    throw CouldNotDecode(L"Buffer size too short!");

  const uint8_t* data = static_cast<const uint8_t*>(buffer);
  uint64_t u = uint64_t(data[0]) << 56;
  u |= uint64_t(data[1]) << 48;
  u |= uint64_t(data[2]) << 40;
  u |= uint64_t(data[3]) << 32;
  u |= uint64_t(data[4]) << 24;
  u |= uint64_t(data[5]) << 16;
  u |= uint64_t(data[6]) << 8;
  u |= uint64_t(data[7]);
  HLAinteger64IntervalImpl::setValue(_impl, u);
}

std::wstring
HLAinteger64Interval::toString() const
{
  std::wstringstream stream;
  stream << getInterval();
  return stream.str();
}

std::wstring
HLAinteger64Interval::implementationName() const
{
  return L"HLAinteger64Time";
}

Integer64
HLAinteger64Interval::getInterval() const
{
  return HLAinteger64IntervalImpl::getValue(_impl);
}

void
HLAinteger64Interval::setInterval(Integer64 value)
{
  HLAinteger64IntervalImpl::setValue(_impl, value);
}

HLAinteger64Interval&
HLAinteger64Interval::operator=(const HLAinteger64Interval& value)
  RTI_THROW ((InvalidLogicalTimeInterval))
{
  HLAinteger64IntervalImpl::assign(_impl, value._impl);
  return *this;
}

HLAinteger64Interval::operator Integer64() const
{
  return getInterval();
}

}
