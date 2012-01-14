/* -*-c++-*- OpenRTI - Copyright (C) 2009-2012 Mathias Froehlich
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

#include "fedtime.hh"

#include <cmath>
#include <cstring>
#include <limits>
#include <sstream>

#include "Types.h"

static inline const RTIfedTime& toRTIfedTime(const RTI::FedTime& fedTime)
{
  const RTIfedTime* rtiFedTime = dynamic_cast<const RTIfedTime*>(&fedTime);
  if (!rtiFedTime)
    throw RTI::InvalidFederationTime("RTI::FedTime is not a RTIfedTime!");
  return *rtiFedTime;
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

static inline double nextAfter(const double& fedTime, const double& direction)
{
#ifdef _WIN32
  return _nextafter(fedTime, direction);
#else
  return nextafter(fedTime, direction);
#endif
}

static inline bool signbitSet(const double& fedTime)
{
#ifdef HAVE_SIGNBIT
  return signbit(fedTime);
#else
  union {
    uint64_t i;
    double d;
  } u;
  u.d = fedTime;
  return 0 != (u.i & (uint64_t(1) << 63));
#endif
}

// We treat -0 as epsilon, so if we set from a double value we need to ensure that the sign bit is not set
// Note that this comparison also preserves nan's
static inline double clearSign(const double& fedTime)
{
  if (fedTime == 0 && signbitSet(fedTime))
    return 0;
  return fedTime;
}

static inline RTI::Boolean toBoolean(bool b)
{
  return b ? RTI::RTI_TRUE : RTI::RTI_FALSE;
}

RTIfedTime::RTIfedTime() :
  _fedTime(0)
{
}

RTIfedTime::RTIfedTime(const double& fedTime) :
  _fedTime(clearSign(fedTime))
{
}

RTIfedTime::RTIfedTime(const RTI::FedTime& fedTime) :
  _fedTime(toRTIfedTime(fedTime)._fedTime)
{
}

RTIfedTime::RTIfedTime(const RTIfedTime& fedTime) :
  _fedTime(fedTime._fedTime)
{
}

RTIfedTime::~RTIfedTime()
{
}

void
RTIfedTime::setZero()
{
  _fedTime = 0;
}

RTI::Boolean
RTIfedTime::isZero()
{
  if (isNaN(_fedTime))
    throw RTI::InvalidFederationTime("Can not comare with NaN!");
  return toBoolean(_fedTime == 0 && !signbitSet(_fedTime));
}

void
RTIfedTime::setEpsilon()
{
  _fedTime = -0;
}

void
RTIfedTime::setPositiveInfinity()
{
  _fedTime = std::numeric_limits<double>::infinity();
}

RTI::Boolean
RTIfedTime::isPositiveInfinity()
{
  if (isNaN(_fedTime))
    throw RTI::InvalidFederationTime("Can not comare with NaN!");
  return toBoolean(_fedTime == std::numeric_limits<double>::infinity());
}

RTI::FedTime&
RTIfedTime::operator+=(const RTI::FedTime& fedTime)
  throw (RTI::InvalidFederationTime)
{
  if (isEpsilon())
    throw RTI::InvalidFederationTime("Can not change epsilon value!");
  const RTIfedTime& rtiFedTime = toRTIfedTime(fedTime);
  if (rtiFedTime.isEpsilon()) {
    _fedTime = clearSign(nextAfter(_fedTime, std::numeric_limits<double>::infinity()));
  } else {
    _fedTime = clearSign(_fedTime + toRTIfedTime(fedTime)._fedTime);
  }
  return *this;
}

RTI::FedTime&
RTIfedTime::operator-=(const RTI::FedTime& fedTime)
  throw (RTI::InvalidFederationTime)
{
  if (isEpsilon())
    throw RTI::InvalidFederationTime("Can not change epsilon value!");
  const RTIfedTime& rtiFedTime = toRTIfedTime(fedTime);
  if (rtiFedTime.isEpsilon()) {
    _fedTime = clearSign(nextAfter(_fedTime, -std::numeric_limits<double>::infinity()));
  } else {
    _fedTime = clearSign(_fedTime - toRTIfedTime(fedTime)._fedTime);
  }
  return *this;
}

RTI::Boolean
RTIfedTime::operator<=(const RTI::FedTime& fedTime) const
  throw (RTI::InvalidFederationTime)
{
  return operator<=(toRTIfedTime(fedTime)._fedTime);
}

RTI::Boolean
RTIfedTime::operator<(const RTI::FedTime& fedTime) const
  throw (RTI::InvalidFederationTime)
{
  return operator<(toRTIfedTime(fedTime)._fedTime);
}

RTI::Boolean
RTIfedTime::operator>=(const RTI::FedTime& fedTime) const
  throw (RTI::InvalidFederationTime)
{
  return operator>=(toRTIfedTime(fedTime)._fedTime);
}

RTI::Boolean
RTIfedTime::operator>(const RTI::FedTime& fedTime) const
  throw (RTI::InvalidFederationTime)
{
  return operator>(toRTIfedTime(fedTime)._fedTime);
}

RTI::Boolean
RTIfedTime::operator==(const RTI::FedTime& fedTime) const
  throw (RTI::InvalidFederationTime)
{
  return operator==(toRTIfedTime(fedTime)._fedTime);
}

RTI::FedTime&
RTIfedTime::operator=(const RTI::FedTime& fedTime)
  throw (RTI::InvalidFederationTime)
{
  _fedTime = toRTIfedTime(fedTime)._fedTime;
  return *this;
}

int
RTIfedTime::encodedLength() const
{
  return 8;
}

void
RTIfedTime::encode(char* buffer) const
{
  union {
    double d;
    uint64_t u;
  } u;
  u.d = _fedTime;
  uint8_t* data = reinterpret_cast<uint8_t*>(buffer);
  data[0] = uint8_t(u.u >> 56);
  data[1] = uint8_t(u.u >> 48);
  data[2] = uint8_t(u.u >> 40);
  data[3] = uint8_t(u.u >> 32);
  data[4] = uint8_t(u.u >> 24);
  data[5] = uint8_t(u.u >> 16);
  data[6] = uint8_t(u.u >> 8);
  data[7] = uint8_t(u.u);
}

int
RTIfedTime::getPrintableLength() const
{
  std::stringstream stream;
  stream << _fedTime;
  return stream.str().size() + 1;
}

void
RTIfedTime::getPrintableString(char* data)
{
  std::stringstream stream;
  stream << _fedTime;
  std::string s = stream.str();
  memcpy(data, s.c_str(), s.size() + 1);
}

bool
RTIfedTime::isEpsilon() const
{
  if (isNaN(_fedTime))
    throw RTI::InvalidFederationTime("Can not comare with NaN!");
  return _fedTime == 0 && signbitSet(_fedTime);
}

double
RTIfedTime::getTime() const
{
  return _fedTime;
}

RTI::Boolean
RTIfedTime::operator==(const double& fedTime) const
  throw (RTI::InvalidFederationTime)
{
  if (isNaN(_fedTime))
    throw RTI::InvalidFederationTime("Can not compare with NaN!");
  if (isNaN(fedTime))
    throw RTI::InvalidFederationTime("Can not compare with NaN!");
  return toBoolean(_fedTime == fedTime);
}

RTI::Boolean
RTIfedTime::operator!=(const RTI::FedTime& fedTime) const
  throw (RTI::InvalidFederationTime)
{
  return operator!=(toRTIfedTime(fedTime)._fedTime);
}

RTI::Boolean
RTIfedTime::operator!=(const double& fedTime) const
  throw (RTI::InvalidFederationTime)
{
  if (isNaN(_fedTime))
    throw RTI::InvalidFederationTime("Can not compare with NaN!");
  if (isNaN(fedTime))
    throw RTI::InvalidFederationTime("Can not compare with NaN!");
  return toBoolean(_fedTime != fedTime);
}

RTI::Boolean
RTIfedTime::operator<=(const double& fedTime) const
  throw (RTI::InvalidFederationTime)
{
  if (isNaN(_fedTime))
    throw RTI::InvalidFederationTime("Can not compare with NaN!");
  if (isNaN(fedTime))
    throw RTI::InvalidFederationTime("Can not compare with NaN!");
  return toBoolean(_fedTime <= fedTime);
}

RTI::Boolean
RTIfedTime::operator<(const double& fedTime) const
  throw (RTI::InvalidFederationTime)
{
  if (isNaN(_fedTime))
    throw RTI::InvalidFederationTime("Can not compare with NaN!");
  if (isNaN(fedTime))
    throw RTI::InvalidFederationTime("Can not compare with NaN!");
  return toBoolean(_fedTime < fedTime);
}

RTI::Boolean
RTIfedTime::operator>=(const double& fedTime) const
  throw (RTI::InvalidFederationTime)
{
  if (isNaN(_fedTime))
    throw RTI::InvalidFederationTime("Can not compare with NaN!");
  if (isNaN(fedTime))
    throw RTI::InvalidFederationTime("Can not compare with NaN!");
  return toBoolean(_fedTime >= fedTime);
}

RTI::Boolean
RTIfedTime::operator>(const double& fedTime) const
  throw (RTI::InvalidFederationTime)
{
  if (isNaN(_fedTime))
    throw RTI::InvalidFederationTime("Can not compare with NaN!");
  if (isNaN(fedTime))
    throw RTI::InvalidFederationTime("Can not compare with NaN!");
  return toBoolean(_fedTime > fedTime);
}

RTI::FedTime&
RTIfedTime::operator=(const RTIfedTime& fedTime)
  throw (RTI::InvalidFederationTime)
{
  _fedTime = toRTIfedTime(fedTime)._fedTime;
  return *this;
}

RTI::FedTime&
RTIfedTime::operator=(const double& fedTime)
  throw (RTI::InvalidFederationTime)
{
  _fedTime = clearSign(fedTime);
  return *this;
}

RTI::FedTime&
RTIfedTime::operator*=(const RTI::FedTime& fedTime)
  throw (RTI::InvalidFederationTime)
{
  _fedTime = clearSign(_fedTime * toRTIfedTime(fedTime)._fedTime);
  return *this;
}

RTI::FedTime&
RTIfedTime::operator/=(const RTI::FedTime& fedTime)
  throw (RTI::InvalidFederationTime)
{
  _fedTime = clearSign(_fedTime / toRTIfedTime(fedTime)._fedTime);
  return *this;
}

RTI::FedTime&
RTIfedTime::operator+=(const double& fedTime)
  throw (RTI::InvalidFederationTime)
{
  _fedTime = clearSign(_fedTime + fedTime);
  return *this;
}

RTI::FedTime&
RTIfedTime::operator-=(const double& fedTime)
  throw (RTI::InvalidFederationTime)
{
  _fedTime = clearSign(_fedTime - fedTime);
  return *this;
}

RTI::FedTime&
RTIfedTime::operator*=(const double& fedTime)
  throw (RTI::InvalidFederationTime)
{
  _fedTime = clearSign(_fedTime * fedTime);
  return *this;
}

RTI::FedTime&
RTIfedTime::operator/=(const double& fedTime)
  throw (RTI::InvalidFederationTime)
{
  _fedTime = clearSign(_fedTime / fedTime);
  return *this;
}

RTIfedTime
RTIfedTime::operator+(const RTI::FedTime& fedTime) const
  throw (RTI::InvalidFederationTime)
{
  return RTIfedTime(*this) += fedTime;
}

RTIfedTime
RTIfedTime::operator+(const double& fedTime) const
  throw (RTI::InvalidFederationTime)
{
  return RTIfedTime(_fedTime + fedTime);
}

RTIfedTime
RTIfedTime::operator-(const RTI::FedTime& fedTime) const
  throw (RTI::InvalidFederationTime)
{
  return RTIfedTime(*this) -= fedTime;
}

RTIfedTime
RTIfedTime::operator-(const double& fedTime) const
  throw (RTI::InvalidFederationTime)
{
  return RTIfedTime(_fedTime - fedTime);
}

RTIfedTime
RTIfedTime::operator*(const RTI::FedTime& fedTime) const
  throw (RTI::InvalidFederationTime)
{
  return RTIfedTime(*this) *= fedTime;
}

RTIfedTime
RTIfedTime::operator*(const double& fedTime) const
  throw (RTI::InvalidFederationTime)
{
  return RTIfedTime(_fedTime * fedTime);
}

RTIfedTime
RTIfedTime::operator/(const RTI::FedTime& fedTime) const
  throw (RTI::InvalidFederationTime)
{
  return RTIfedTime(*this) /= fedTime;
}

RTIfedTime
RTIfedTime::operator/(const double& fedTime) const
  throw (RTI::InvalidFederationTime)
{
  return RTIfedTime(_fedTime / fedTime);
}

RTIfedTime
operator+(const double& left, const RTI::FedTime& right)
{
  return RTIfedTime(left) += right;
}

RTIfedTime
operator-(const double& left, const RTI::FedTime& right)
{
  return RTIfedTime(left) -= right;
}

RTIfedTime
operator*(const double& left, const RTI::FedTime& right)
{
  return RTIfedTime(left) *= right;
}

RTIfedTime
operator/(const double& left, const RTI::FedTime& right)
{
  return RTIfedTime(left) /= right;
}

std::ostream&
operator<<(std::ostream& stream, const RTI::FedTime& fedTime)
{
  return stream << toRTIfedTime(fedTime).getTime();
}

RTI::FedTime*
RTI::FedTimeFactory::makeZero()
  throw (RTI::MemoryExhausted)
{
  return new RTIfedTime(0);
}

RTI::FedTime*
RTI::FedTimeFactory::decode(const char* buffer)
    throw (RTI::MemoryExhausted)
{
  union {
    double d;
    uint64_t u;
  } u;
  const uint8_t* data = reinterpret_cast<const uint8_t*>(buffer);
  u.u = uint64_t(data[0]) << 56;
  u.u |= uint64_t(data[1]) << 48;
  u.u |= uint64_t(data[2]) << 40;
  u.u |= uint64_t(data[3]) << 32;
  u.u |= uint64_t(data[4]) << 24;
  u.u |= uint64_t(data[5]) << 16;
  u.u |= uint64_t(data[6]) << 8;
  u.u |= uint64_t(data[7]);
  return new RTIfedTime(u.d);
}
