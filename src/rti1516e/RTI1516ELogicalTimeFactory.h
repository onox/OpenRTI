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

#ifndef RTI1516ELogicalTimeFactory_h
#define RTI1516ELogicalTimeFactory_h

#include <RTI/LogicalTime.h>
#include <RTI/LogicalTimeInterval.h>
#include <RTI/LogicalTimeFactory.h>

#include <memory>

#include "StringUtils.h"

namespace OpenRTI {

// Generic interface to build up an OpenRTI::Federate for an unknown opaque logical time implementation

class OPENRTI_LOCAL RTI1516ELogicalTimeFactory {
public:
  class LogicalTime;
  class LogicalTimeInterval;
  class LogicalTimeIntervalImplementation;
  class LogicalTimeImplementation;
  class LogicalTimeFactoryImplementation;

  class OPENRTI_LOCAL LogicalTimeInterval {
  public:
    LogicalTimeInterval(LogicalTimeIntervalImplementation* implementation = 0);
    LogicalTimeInterval(const LogicalTimeInterval&);
    ~LogicalTimeInterval();

    LogicalTimeInterval& operator=(const LogicalTimeInterval&);

    bool operator>(const LogicalTimeInterval& logicalTimeInterval) const;
    bool operator<(const LogicalTimeInterval& logicalTimeInterval) const;
    bool operator==(const LogicalTimeInterval& logicalTimeInterval) const;
    bool operator!=(const LogicalTimeInterval& logicalTimeInterval) const;
    bool operator>=(const LogicalTimeInterval& logicalTimeInterval) const;
    bool operator<=(const LogicalTimeInterval& logicalTimeInterval) const;

  private:
    friend class LogicalTime;
    friend class RTI1516ELogicalTimeFactory;
    SharedPtr<LogicalTimeIntervalImplementation> _implementation;
  };

  class OPENRTI_LOCAL LogicalTime {
  public:
    LogicalTime(LogicalTimeImplementation* implementation = 0);
    LogicalTime(const LogicalTime&);
    ~LogicalTime();

    LogicalTime& operator=(const LogicalTime&);

    LogicalTime& operator+=(const LogicalTimeInterval& logicalTimeInterval);
    LogicalTime& operator-=(const LogicalTimeInterval& logicalTimeInterval);
    LogicalTimeInterval operator-(const LogicalTime& logicalTime) const;

    bool operator>(const LogicalTime& logicalTime) const;
    bool operator<(const LogicalTime& logicalTime) const;
    bool operator==(const LogicalTime& logicalTime) const;
    bool operator!=(const LogicalTime& logicalTime) const;
    bool operator>=(const LogicalTime& logicalTime) const;
    bool operator<=(const LogicalTime& logicalTime) const;

  private:
    friend class LogicalTimeInterval;
    friend class RTI1516ELogicalTimeFactory;
    SharedPtr<LogicalTimeImplementation> _implementation;
  };

  RTI1516ELogicalTimeFactory(RTI_UNIQUE_PTR<rti1516e::LogicalTimeFactory> logicalTimeFactory);
  RTI1516ELogicalTimeFactory(const RTI1516ELogicalTimeFactory&);
  ~RTI1516ELogicalTimeFactory();

  RTI1516ELogicalTimeFactory& operator=(const RTI1516ELogicalTimeFactory&);

  std::string getName() const;

  LogicalTime initialLogicalTime() const;
  LogicalTime finalLogicalTime() const;
  LogicalTimeInterval zeroLogicalTimeInterval() const;

  static LogicalTime nextAfter(const LogicalTime& logicalTime);

  LogicalTime getLogicalTime(const rti1516e::LogicalTime& rti1516LogicalTime);
  LogicalTimeInterval getLogicalTimeInterval(const rti1516e::LogicalTimeInterval& rti1516LogicalTimeInterval);

  bool isZeroTimeInterval(const LogicalTimeInterval& logicalTimeInterval)
  { return isZeroTimeInterval(getLogicalTimeInterval(logicalTimeInterval)); }
  bool isZeroTimeInterval(const rti1516e::LogicalTimeInterval& rti1516LogicalTimeInterval)
  { return rti1516LogicalTimeInterval.isZero(); }
  bool isPositiveTimeInterval(const rti1516e::LogicalTimeInterval& rti1516LogicalTimeInterval);

  const rti1516e::LogicalTime& getLogicalTime(const LogicalTime& logicalTime);
  const rti1516e::LogicalTimeInterval& getLogicalTimeInterval(const LogicalTimeInterval& logicalTimeInterval);

  LogicalTime decodeLogicalTime(const VariableLengthData& variableLengthData);
  VariableLengthData encodeLogicalTime(const rti1516e::LogicalTime& logicalTime)
  {
    size_t encodedLength = logicalTime.encodedLength();
    VariableLengthData variableLengthData;
    variableLengthData.resize(encodedLength);
    logicalTime.encode(variableLengthData.data(), encodedLength);
    return variableLengthData;
  }
  VariableLengthData encodeLogicalTime(const LogicalTime& logicalTime)
  { return encodeLogicalTime(getLogicalTime(logicalTime)); }

  std::string toString(const rti1516e::LogicalTime& logicalTime)
  { return ucsToUtf8(logicalTime.toString()); }
  std::string toString(const rti1516e::LogicalTimeInterval& logicalTimeInterval)
  { return ucsToUtf8(logicalTimeInterval.toString()); }

private:
  SharedPtr<LogicalTimeFactoryImplementation> _implementation;
};

}

#endif
