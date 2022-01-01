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

#ifndef RTI13LogicalTimeFactory_h
#define RTI13LogicalTimeFactory_h

#include <RTI.hh>

#include <Export.h>
#include <Referenced.h>
#include <SharedPtr.h>
#include <VariableLengthData.h>

#include <memory>

namespace OpenRTI {

// Generic interface to build up an OpenRTI::Federate for an unknown opaque logical time implementation

class OPENRTI_LOCAL RTI13LogicalTimeFactory {
public:
  class LogicalTime;
  class LogicalTimeInterval;
  class FedTimeImplementation;

  class OPENRTI_LOCAL LogicalTimeInterval {
  public:
    LogicalTimeInterval(FedTimeImplementation* implementation = 0);
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
    friend class RTI13LogicalTimeFactory;
    SharedPtr<FedTimeImplementation> _implementation;
  };

  class OPENRTI_LOCAL LogicalTime {
  public:
    LogicalTime(FedTimeImplementation* implementation = 0);
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
    friend class RTI13LogicalTimeFactory;
    SharedPtr<FedTimeImplementation> _implementation;
  };

  RTI13LogicalTimeFactory(const std::string& name);
  RTI13LogicalTimeFactory(const RTI13LogicalTimeFactory&);
  ~RTI13LogicalTimeFactory();

  RTI13LogicalTimeFactory& operator=(const RTI13LogicalTimeFactory&);

  std::string getName() const;

  LogicalTime initialLogicalTime() const;
  LogicalTime finalLogicalTime() const;
  LogicalTimeInterval zeroLogicalTimeInterval() const;

  static LogicalTime nextAfter(const LogicalTime& logicalTime);

  LogicalTime getLogicalTime(const RTI::FedTime& fedTime);
  LogicalTimeInterval getLogicalTimeInterval(const RTI::FedTime& fedTime);

  bool isZeroTimeInterval(const LogicalTimeInterval& logicalTimeInterval)
  { return isZeroTimeInterval(getLogicalTimeInterval(logicalTimeInterval)); }
  bool isZeroTimeInterval(const RTI::FedTime& fedTime);
  bool isPositiveTimeInterval(const RTI::FedTime& fedTime);

  const RTI::FedTime& getLogicalTime(const LogicalTime& logicalTime);
  const RTI::FedTime& getLogicalTimeInterval(const LogicalTimeInterval& logicalTimeInterval);

  LogicalTime decodeLogicalTime(const VariableLengthData& variableLengthData);
  VariableLengthData encodeLogicalTime(const RTI::FedTime& fedTime);
  VariableLengthData encodeLogicalTime(const LogicalTime& logicalTime)
  { return encodeLogicalTime(getLogicalTime(logicalTime)); }

  std::string toString(const RTI::FedTime& fedTime);

private:
  std::string _name;
};

}

#endif
