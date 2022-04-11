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

#include "RTI/HLAfloat64Time.h"

#include <cmath>
#include <limits>
#include <sstream>
#include "VariableLengthDataFriend.h"
#include "ValueImplementation.h"
#include "RTI/HLAfloat64Interval.h"

DECLARE_VALUE_IMPLEMENTATION(HLAfloat64TimeImpl, double)

static const HLAfloat64Time& toHLAfloat64Time(const rti1516::LogicalTime& logicalTime)
{
  const HLAfloat64Time* float64Time = dynamic_cast<const HLAfloat64Time*>(&logicalTime);
  if (!float64Time)
    throw rti1516::InvalidLogicalTime(logicalTime.implementationName());
  return *float64Time;
}

static const HLAfloat64Interval& toHLAfloat64Interval(const rti1516::LogicalTimeInterval& logicalTimeInterval)
{
  const HLAfloat64Interval* float64Interval = dynamic_cast<const HLAfloat64Interval*>(&logicalTimeInterval);
  if (!float64Interval)
    throw rti1516::InvalidLogicalTimeInterval(logicalTimeInterval.implementationName());
  return *float64Interval;
}

static inline bool isNaN(const double& fedTime)
{
#if 1
  // Code that here ourselfs, compilers might decide to optimize isnan away
  // because of some assumtions based on optimization flags

  // Neat trick:
  // A nan has all bits in its exponent set and any bit in the mantissa.
  // So a nan's bitpattern int representation can be written as
  //  0x7f800000 + x
  // where x > 0.
  // That means compute:
  //   t = 0x7f800000 - (0x7f800000 + x) = -x
  // and look for the sign of t.
  // If the exponent is not 0x7f800000 (that is less than 0x7f800000), the
  // resulting t value will be >= 0.

  union {
    double d;
    uint64_t u;
  } u;
  u.d = fedTime;

  // remove the sign bit since in the above the sign bit is not set
  int64_t i = (u.u & 0x7fffffffffffffffll);
  // compute the above t
  int64_t t = 0x7ff0000000000000ll - i;
  return t < 0;

#elif defined _WIN32
  return 0 != _isnan(fedTime);
#elif defined(__sun) || defined(__hpux)
  return isnan(fedTime);
#else
  return std::isnan(fedTime);
#endif
}

static inline double nextAfter(const double& logicalTime, const double& direction)
{
#if 201103L <= __cplusplus
  return std::nextafter(logicalTime, direction);
#elif defined _WIN32
  return _nextafter(logicalTime, direction);
#else
  return nextafter(logicalTime, direction);
#endif
}

HLAfloat64Time::HLAfloat64Time() :
  _impl(0)
{
}

HLAfloat64Time::HLAfloat64Time(double value) :
  _impl(0)
{
  HLAfloat64TimeImpl::setValue(_impl, value);
}

HLAfloat64Time::HLAfloat64Time(const rti1516::LogicalTime& logicalTime) :
  _impl(0)
{
  HLAfloat64TimeImpl::assign(_impl, toHLAfloat64Time(logicalTime)._impl);
}

HLAfloat64Time::HLAfloat64Time(const HLAfloat64Time& float64Time) :
  _impl(0)
{
  HLAfloat64TimeImpl::assign(_impl, float64Time._impl);
}

HLAfloat64Time::~HLAfloat64Time() RTI_NOEXCEPT
{
  HLAfloat64TimeImpl::putAndDelete(_impl);
}

void
HLAfloat64Time::setInitial()
{
  HLAfloat64TimeImpl::setValue(_impl, 0);
}

bool
HLAfloat64Time::isInitial() const
{
  return 0 == HLAfloat64TimeImpl::getValue(_impl);
}

void
HLAfloat64Time::setFinal()
{
  HLAfloat64TimeImpl::setValue(_impl, std::numeric_limits<double>::infinity());
}

bool
HLAfloat64Time::isFinal() const
{
  return std::numeric_limits<double>::infinity() == HLAfloat64TimeImpl::getValue(_impl);
}

HLAfloat64Time&
HLAfloat64Time::operator=(const rti1516::LogicalTime& logicalTime)
  RTI_THROW ((rti1516::InvalidLogicalTime))
{
  HLAfloat64TimeImpl::assign(_impl, toHLAfloat64Time(logicalTime)._impl);
  return *this;
}

HLAfloat64Time&
HLAfloat64Time::operator+=(const rti1516::LogicalTimeInterval& logicalTimeInterval)
  RTI_THROW ((rti1516::IllegalTimeArithmetic, rti1516::InvalidLogicalTimeInterval))
{
  double interval = toHLAfloat64Interval(logicalTimeInterval).getInterval();
  if (isNaN(interval))
    throw rti1516::InvalidLogicalTimeInterval(L"Logical time interval is NaN!");
  double value = HLAfloat64TimeImpl::getValue(_impl);
  if (isNaN(value))
    throw rti1516::IllegalTimeArithmetic(L"Logical time is NaN!");
  if (0 < interval) {
    double next = nextAfter(value, std::numeric_limits<double>::infinity());
    double sum = value + interval;
    value = std::max(sum, next);
    if (isNaN(value))
      throw rti1516::IllegalTimeArithmetic(L"Result of logical time operation is NaN!");
    HLAfloat64TimeImpl::setValue(_impl, value);
  } else if (interval < 0) {
    double next = nextAfter(value, -std::numeric_limits<double>::infinity());
    double sum = value + interval;
    value = std::min(sum, next);
    if (isNaN(value))
      throw rti1516::IllegalTimeArithmetic(L"Result of logical time operation is NaN!");
    HLAfloat64TimeImpl::setValue(_impl, value);
  } else /* if (interval == 0) */ {
    // Nothing on zero
  }
  return *this;
}

HLAfloat64Time&
HLAfloat64Time::operator-=(const rti1516::LogicalTimeInterval& logicalTimeInterval)
  RTI_THROW ((rti1516::IllegalTimeArithmetic, rti1516::InvalidLogicalTimeInterval))
{
  double interval = toHLAfloat64Interval(logicalTimeInterval).getInterval();
  if (isNaN(interval))
    throw rti1516::InvalidLogicalTimeInterval(L"Logical time interval is NaN!");
  double value = HLAfloat64TimeImpl::getValue(_impl);
  if (isNaN(value))
    throw rti1516::IllegalTimeArithmetic(L"Logical time is NaN!");
  if (0 < interval) {
    double next = nextAfter(value, -std::numeric_limits<double>::infinity());
    double sum = value - interval;
    value = std::min(sum, next);
    if (isNaN(value))
      throw rti1516::IllegalTimeArithmetic(L"Result of logical time operation is NaN!");
    HLAfloat64TimeImpl::setValue(_impl, value);
  } else if (interval < 0) {
    double next = nextAfter(value, std::numeric_limits<double>::infinity());
    double sum = value - interval;
    value = std::max(sum, next);
    if (isNaN(value))
      throw rti1516::IllegalTimeArithmetic(L"Result of logical time operation is NaN!");
    HLAfloat64TimeImpl::setValue(_impl, value);
  } else /* if (interval == 0) */ {
    // Nothing on zero
  }
  return *this;
}

bool
HLAfloat64Time::operator>(const rti1516::LogicalTime& logicalTime) const
  RTI_THROW ((rti1516::InvalidLogicalTime))
{
  double left = HLAfloat64TimeImpl::getValue(_impl);
  if (isNaN(left))
    throw rti1516::InvalidLogicalTime(L"Can not compare with NaN!");
  double right = HLAfloat64TimeImpl::getValue(toHLAfloat64Time(logicalTime)._impl);
  if (isNaN(right))
    throw rti1516::InvalidLogicalTime(L"Can not compare with NaN!");
  return left > right;
}

bool
HLAfloat64Time::operator<(const rti1516::LogicalTime& logicalTime) const
    RTI_THROW ((rti1516::InvalidLogicalTime))
{
  double left = HLAfloat64TimeImpl::getValue(_impl);
  if (isNaN(left))
    throw rti1516::InvalidLogicalTime(L"Can not compare with NaN!");
  double right = HLAfloat64TimeImpl::getValue(toHLAfloat64Time(logicalTime)._impl);
  if (isNaN(right))
    throw rti1516::InvalidLogicalTime(L"Can not compare with NaN!");
  return left < right;
}

bool
HLAfloat64Time::operator==(const rti1516::LogicalTime& logicalTime) const
    RTI_THROW ((rti1516::InvalidLogicalTime))
{
  double left = HLAfloat64TimeImpl::getValue(_impl);
  if (isNaN(left))
    throw rti1516::InvalidLogicalTime(L"Can not compare with NaN!");
  double right = HLAfloat64TimeImpl::getValue(toHLAfloat64Time(logicalTime)._impl);
  if (isNaN(right))
    throw rti1516::InvalidLogicalTime(L"Can not compare with NaN!");
  return left == right;
}

bool
HLAfloat64Time::operator>=(const rti1516::LogicalTime& logicalTime) const
    RTI_THROW ((rti1516::InvalidLogicalTime))
{
  double left = HLAfloat64TimeImpl::getValue(_impl);
  if (isNaN(left))
    throw rti1516::InvalidLogicalTime(L"Can not compare with NaN!");
  double right = HLAfloat64TimeImpl::getValue(toHLAfloat64Time(logicalTime)._impl);
  if (isNaN(right))
    throw rti1516::InvalidLogicalTime(L"Can not compare with NaN!");
  return left >= right;
}

bool
HLAfloat64Time::operator<=(const rti1516::LogicalTime& logicalTime) const
    RTI_THROW ((rti1516::InvalidLogicalTime))
{
  double left = HLAfloat64TimeImpl::getValue(_impl);
  if (isNaN(left))
    throw rti1516::InvalidLogicalTime(L"Can not compare with NaN!");
  double right = HLAfloat64TimeImpl::getValue(toHLAfloat64Time(logicalTime)._impl);
  if (isNaN(right))
    throw rti1516::InvalidLogicalTime(L"Can not compare with NaN!");
  return left <= right;
}

rti1516::VariableLengthData
HLAfloat64Time::encode() const
{
  OpenRTI::VariableLengthData variableLengthData(8);
  variableLengthData.setFloat64BE(HLAfloat64TimeImpl::getValue(_impl), 0);
  return rti1516::VariableLengthDataFriend::create(variableLengthData);
}

unsigned long
HLAfloat64Time::encodedLength() const
{
  return 8;
}

unsigned long
HLAfloat64Time::encode(void* buffer, unsigned long bufferSize) const
    RTI_THROW ((rti1516::CouldNotEncode))
{
  if (bufferSize < 8)
    throw rti1516::CouldNotEncode(L"Buffer size too short!");
  union {
    double d;
    uint64_t u;
  } u;
  u.d = HLAfloat64TimeImpl::getValue(_impl);
  uint8_t* data = static_cast<uint8_t*>(buffer);
  data[0] = uint8_t(u.u >> 56);
  data[1] = uint8_t(u.u >> 48);
  data[2] = uint8_t(u.u >> 40);
  data[3] = uint8_t(u.u >> 32);
  data[4] = uint8_t(u.u >> 24);
  data[5] = uint8_t(u.u >> 16);
  data[6] = uint8_t(u.u >> 8);
  data[7] = uint8_t(u.u);
  return 8;
}

void
HLAfloat64Time::decode(const rti1516::VariableLengthData& variableLengthData)
    RTI_THROW ((rti1516::InternalError, rti1516::CouldNotDecode))
{
  const OpenRTI::VariableLengthData& data = rti1516::VariableLengthDataFriend::readPointer(variableLengthData);
  if (data.size() < 8)
    throw rti1516::CouldNotDecode(L"Buffer size too short!");
  HLAfloat64TimeImpl::setValue(_impl, data.getFloat64BE(0));
}

void
HLAfloat64Time::decode(void* buffer, unsigned long bufferSize)
    RTI_THROW ((rti1516::InternalError, rti1516::CouldNotDecode))
{
  if (bufferSize < 8)
    throw rti1516::CouldNotDecode(L"Buffer size too short!");
  union {
    double d;
    uint64_t u;
  } u;
  const uint8_t* data = static_cast<const uint8_t*>(buffer);
  u.u = uint64_t(data[0]) << 56;
  u.u |= uint64_t(data[1]) << 48;
  u.u |= uint64_t(data[2]) << 40;
  u.u |= uint64_t(data[3]) << 32;
  u.u |= uint64_t(data[4]) << 24;
  u.u |= uint64_t(data[5]) << 16;
  u.u |= uint64_t(data[6]) << 8;
  u.u |= uint64_t(data[7]);
  HLAfloat64TimeImpl::setValue(_impl, u.d);
}

std::wstring
HLAfloat64Time::toString() const
{
  std::wstringstream stream;
  stream << HLAfloat64TimeImpl::getValue(_impl);
  return stream.str();
}

std::wstring
HLAfloat64Time::implementationName() const
{
  return L"HLAfloat64Time";
}

double
HLAfloat64Time::getTime() const
{
  return HLAfloat64TimeImpl::getValue(_impl);
}

void
HLAfloat64Time::setTime(double value)
{
  HLAfloat64TimeImpl::setValue(_impl, value);
}

HLAfloat64Time&
HLAfloat64Time::operator=(const HLAfloat64Time& float64Time)
{
  HLAfloat64TimeImpl::assign(_impl, float64Time._impl);
  return *this;
}

HLAfloat64Time::operator double() const
{
  return getTime();
}
