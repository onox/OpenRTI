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

#ifndef RTI1516float64TimeFactory_h
#define RTI1516float64TimeFactory_h

// This time, the first include is above the api include.
// the rti1516/Exception header misses that.
#include <iosfwd>

#include <RTI/LogicalTime.h>
#include <RTI/LogicalTimeInterval.h>
#include <RTI/LogicalTimeFactory.h>

#include <RTI/HLAfloat64Time.h>
#include <RTI/HLAfloat64Interval.h>

#include <cmath>
#include <limits>

namespace OpenRTI {

// Interface to build up an optimized OpenRTI::Federate for the builtin
// float64 logical time

class OPENRTI_LOCAL RTI1516float64TimeFactory {
public:
  typedef double LogicalTime;
  typedef double LogicalTimeInterval;

  RTI1516float64TimeFactory(const std::string& name) :
    _name(name)
  { }

  const std::string& getName() const
  { return _name; }

  LogicalTime initialLogicalTime() const
  { return LogicalTime(0); }
  LogicalTime finalLogicalTime() const
  { return LogicalTime(std::numeric_limits<LogicalTime>::max()); }
  LogicalTimeInterval zeroLogicalTimeInterval() const
  { return LogicalTimeInterval(0); }

  static LogicalTime nextAfter(const LogicalTime& logicalTime)
  {
#if 201103L <= __cplusplus
    return std::nextafter(logicalTime, std::numeric_limits<double>::infinity());
#elif defined _WIN32
    return _nextafter(logicalTime, std::numeric_limits<double>::infinity());
#else
    return nextafter(logicalTime, std::numeric_limits<double>::infinity());
#endif
  }

  LogicalTime getLogicalTime(const rti1516::LogicalTime& rti1516LogicalTime)
  { _float64Time = rti1516LogicalTime; return LogicalTime(_float64Time.getTime()); }
  LogicalTimeInterval getLogicalTimeInterval(const rti1516::LogicalTimeInterval& rti1516LogicalTimeInterval)
  { _float64Interval = rti1516LogicalTimeInterval; return LogicalTimeInterval(_float64Interval.getInterval()); }

  const rti1516::LogicalTime& getLogicalTime(const LogicalTime& logicalTime)
  { _float64Time.setTime(logicalTime); return _float64Time; }
  const rti1516::LogicalTimeInterval& getLogicalTimeInterval(const LogicalTimeInterval& logicalTimeInterval)
  { _float64Interval.setInterval(logicalTimeInterval); return _float64Interval; }

  bool isZeroTimeInterval(const LogicalTimeInterval& logicalTimeInterval)
  { return logicalTimeInterval == 0; }
  bool isZeroTimeInterval(const rti1516::LogicalTimeInterval& rti1516LogicalTimeInterval)
  { return rti1516LogicalTimeInterval.isZero(); }
  bool isPositiveTimeInterval(const rti1516::LogicalTimeInterval& rti1516LogicalTimeInterval)
  { _float64Interval.setInterval(0); return _float64Interval <= rti1516LogicalTimeInterval; }

  LogicalTime decodeLogicalTime(const VariableLengthData& variableLengthData) const
  {
    if (variableLengthData.size() < 8)
      throw rti1516::CouldNotDecode(L"Buffer size too short!");
    return LogicalTime(variableLengthData.getFloat64BE(0));
  }
  VariableLengthData encodeLogicalTime(const rti1516::LogicalTime& logicalTime)
  {
    unsigned long encodedLength = logicalTime.encodedLength();
    VariableLengthData variableLengthData;
    variableLengthData.resize(encodedLength);
    logicalTime.encode(variableLengthData.data(), encodedLength);
    return variableLengthData;
  }
  VariableLengthData encodeLogicalTime(const LogicalTime& logicalTime)
  { return encodeLogicalTime(getLogicalTime(logicalTime)); }

  std::string toString(const rti1516::LogicalTime& logicalTime)
  { return ucsToUtf8(logicalTime.toString()); }
  std::string toString(const rti1516::LogicalTimeInterval& logicalTimeInterval)
  { return ucsToUtf8(logicalTimeInterval.toString()); }

private:
  std::string _name;
  HLAfloat64Time _float64Time;
  HLAfloat64Interval _float64Interval;
};

}

#endif
