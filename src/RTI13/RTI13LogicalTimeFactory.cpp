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

#include "RTI13LogicalTimeFactory.h"

#include <memory>
#include <vector>

#include "StringUtils.h"

namespace OpenRTI {

class OPENRTI_LOCAL RTI13LogicalTimeFactory::FedTimeImplementation : public Referenced {
public:
  FedTimeImplementation(RTI::FedTime* fedTime) :
    _fedTime(fedTime)
  { }
  FedTimeImplementation() :
    _fedTime(RTI::FedTimeFactory::makeZero())
  { }

  FedTimeImplementation* operator+(const FedTimeImplementation& implementation) const
  {
    if (!_fedTime.get())
      return 0;
    if (!implementation._fedTime.get())
      return 0;
    FedTimeImplementation* fedTimeImplementation = new FedTimeImplementation;
    (*fedTimeImplementation->_fedTime) = *_fedTime;
    (*fedTimeImplementation->_fedTime) += *implementation._fedTime;
    return fedTimeImplementation;
  }
  FedTimeImplementation* operator-(const FedTimeImplementation& implementation) const
  {
    if (!_fedTime.get())
      return 0;
    if (!implementation._fedTime.get())
      return 0;
    FedTimeImplementation* fedTimeImplementation = new FedTimeImplementation;
    *fedTimeImplementation->_fedTime = *_fedTime;
    *fedTimeImplementation->_fedTime -= *implementation._fedTime;
    return fedTimeImplementation;
  }

  bool operator>(const FedTimeImplementation& implementation) const
  {
    if (!_fedTime.get())
      return false;
    if (!implementation._fedTime.get())
      return false;
    return (*_fedTime > *implementation._fedTime) == RTI::RTI_TRUE;
  }
  bool operator<(const FedTimeImplementation& implementation) const
  {
    if (!_fedTime.get())
      return false;
    if (!implementation._fedTime.get())
      return false;
    return (*_fedTime < *implementation._fedTime) == RTI::RTI_TRUE;
  }
  bool operator==(const FedTimeImplementation& implementation) const
  {
    if (!_fedTime.get())
      return false;
    if (!implementation._fedTime.get())
      return false;
    return (*_fedTime == *implementation._fedTime) == RTI::RTI_TRUE;
  }
  bool operator!=(const FedTimeImplementation& implementation) const
  {
    if (!_fedTime.get())
      return false;
    if (!implementation._fedTime.get())
      return false;
    return (*_fedTime == *implementation._fedTime) != RTI::RTI_TRUE;
  }
  bool operator>=(const FedTimeImplementation& implementation) const
  {
    if (!_fedTime.get())
      return false;
    if (!implementation._fedTime.get())
      return false;
    return (*_fedTime >= *implementation._fedTime) == RTI::RTI_TRUE;
  }
  bool operator<=(const FedTimeImplementation& implementation) const
  {
    if (!_fedTime.get())
      return false;
    if (!implementation._fedTime.get())
      return false;
    return (*_fedTime <= *implementation._fedTime) == RTI::RTI_TRUE;
  }

  RTI_UNIQUE_PTR<RTI::FedTime> _fedTime;
};



RTI13LogicalTimeFactory::LogicalTimeInterval::LogicalTimeInterval(RTI13LogicalTimeFactory::FedTimeImplementation* implementation) :
  _implementation(implementation)
{
}

RTI13LogicalTimeFactory::LogicalTimeInterval::LogicalTimeInterval(const RTI13LogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval) :
  _implementation(logicalTimeInterval._implementation)
{
}

RTI13LogicalTimeFactory::LogicalTimeInterval::~LogicalTimeInterval()
{
}

RTI13LogicalTimeFactory::LogicalTimeInterval&
RTI13LogicalTimeFactory::LogicalTimeInterval::operator=(const RTI13LogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval)
{
  _implementation = logicalTimeInterval._implementation;
  return *this;
}

bool
RTI13LogicalTimeFactory::LogicalTimeInterval::operator>(const RTI13LogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTimeInterval._implementation.valid())
    return false;
  return (*_implementation) > (*logicalTimeInterval._implementation);
}

bool
RTI13LogicalTimeFactory::LogicalTimeInterval::operator<(const RTI13LogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTimeInterval._implementation.valid())
    return false;
  return (*_implementation) < (*logicalTimeInterval._implementation);
}

bool
RTI13LogicalTimeFactory::LogicalTimeInterval::operator==(const RTI13LogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTimeInterval._implementation.valid())
    return false;
  return (*_implementation) == (*logicalTimeInterval._implementation);
}

bool
RTI13LogicalTimeFactory::LogicalTimeInterval::operator!=(const RTI13LogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTimeInterval._implementation.valid())
    return false;
  return (*_implementation) != (*logicalTimeInterval._implementation);
}

bool
RTI13LogicalTimeFactory::LogicalTimeInterval::operator>=(const RTI13LogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTimeInterval._implementation.valid())
    return false;
  return (*_implementation) >= (*logicalTimeInterval._implementation);
}

bool
RTI13LogicalTimeFactory::LogicalTimeInterval::operator<=(const RTI13LogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTimeInterval._implementation.valid())
    return false;
  return (*_implementation) <= (*logicalTimeInterval._implementation);
}



RTI13LogicalTimeFactory::LogicalTime::LogicalTime(RTI13LogicalTimeFactory::FedTimeImplementation* implementation) :
  _implementation(implementation)
{
}

RTI13LogicalTimeFactory::LogicalTime::LogicalTime(const RTI13LogicalTimeFactory::LogicalTime& logicalTime) :
  _implementation(logicalTime._implementation)
{
}

RTI13LogicalTimeFactory::LogicalTime::~LogicalTime()
{
}

RTI13LogicalTimeFactory::LogicalTime&
RTI13LogicalTimeFactory::LogicalTime::operator=(const RTI13LogicalTimeFactory::LogicalTime& logicalTime)
{
  _implementation = logicalTime._implementation;
  return *this;
}


RTI13LogicalTimeFactory::LogicalTime&
RTI13LogicalTimeFactory::LogicalTime::operator+=(const RTI13LogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval)
{
  if (!_implementation.valid())
    return *this;
  if (!logicalTimeInterval._implementation.valid())
    return *this;
  _implementation = (*_implementation) + (*logicalTimeInterval._implementation);
  return *this;
}

RTI13LogicalTimeFactory::LogicalTime&
RTI13LogicalTimeFactory::LogicalTime::operator-=(const RTI13LogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval)
{
  if (!_implementation.valid())
    return *this;
  if (!logicalTimeInterval._implementation.valid())
    return *this;
  _implementation = (*_implementation) - (*logicalTimeInterval._implementation);
  return *this;
}

RTI13LogicalTimeFactory::LogicalTimeInterval
RTI13LogicalTimeFactory::LogicalTime::operator-(const RTI13LogicalTimeFactory::LogicalTime& logicalTime) const
{
  if (!_implementation.valid())
    return LogicalTimeInterval();
  if (!logicalTime._implementation.valid())
    return LogicalTimeInterval();
  return LogicalTimeInterval((*_implementation) - (*logicalTime._implementation));
}

bool
RTI13LogicalTimeFactory::LogicalTime::operator>(const RTI13LogicalTimeFactory::LogicalTime& logicalTime) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTime._implementation.valid())
    return false;
  return (*_implementation) > (*logicalTime._implementation);
}

bool
RTI13LogicalTimeFactory::LogicalTime::operator<(const RTI13LogicalTimeFactory::LogicalTime& logicalTime) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTime._implementation.valid())
    return false;
  return (*_implementation) < (*logicalTime._implementation);
}

bool
RTI13LogicalTimeFactory::LogicalTime::operator==(const RTI13LogicalTimeFactory::LogicalTime& logicalTime) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTime._implementation.valid())
    return false;
  return (*_implementation) == (*logicalTime._implementation);
}

bool
RTI13LogicalTimeFactory::LogicalTime::operator!=(const RTI13LogicalTimeFactory::LogicalTime& logicalTime) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTime._implementation.valid())
    return false;
  return (*_implementation) != (*logicalTime._implementation);
}

bool
RTI13LogicalTimeFactory::LogicalTime::operator>=(const RTI13LogicalTimeFactory::LogicalTime& logicalTime) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTime._implementation.valid())
    return false;
  return (*_implementation) >= (*logicalTime._implementation);
}

bool
RTI13LogicalTimeFactory::LogicalTime::operator<=(const RTI13LogicalTimeFactory::LogicalTime& logicalTime) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTime._implementation.valid())
    return false;
  return (*_implementation) <= (*logicalTime._implementation);
}



RTI13LogicalTimeFactory::RTI13LogicalTimeFactory(const std::string& name) :
  _name(name)
{
}

RTI13LogicalTimeFactory::RTI13LogicalTimeFactory(const RTI13LogicalTimeFactory&)
{
}

RTI13LogicalTimeFactory::~RTI13LogicalTimeFactory()
{
}

RTI13LogicalTimeFactory&
RTI13LogicalTimeFactory::operator=(const RTI13LogicalTimeFactory& logicalTimeFactory)
{
  return *this;
}

std::string
RTI13LogicalTimeFactory::getName() const
{
  return _name;
}

RTI13LogicalTimeFactory::LogicalTime
RTI13LogicalTimeFactory::initialLogicalTime() const
{
  return LogicalTime(new FedTimeImplementation);
}

RTI13LogicalTimeFactory::LogicalTime
RTI13LogicalTimeFactory::finalLogicalTime() const
{
  FedTimeImplementation* implementation = new FedTimeImplementation;
  implementation->_fedTime->setPositiveInfinity();
  return LogicalTime(implementation);
}

RTI13LogicalTimeFactory::LogicalTimeInterval
RTI13LogicalTimeFactory::zeroLogicalTimeInterval() const
{
  return LogicalTimeInterval(new FedTimeImplementation);
}

RTI13LogicalTimeFactory::LogicalTime
RTI13LogicalTimeFactory::nextAfter(const RTI13LogicalTimeFactory::LogicalTime& logicalTime)
{
  FedTimeImplementation* implementation = new FedTimeImplementation;
  (*implementation->_fedTime) = *logicalTime._implementation->_fedTime;
  RTI_UNIQUE_PTR<RTI::FedTime> epsilonTime(RTI::FedTimeFactory::makeZero());
  epsilonTime->setEpsilon();
  (*implementation->_fedTime) += *epsilonTime;
  return LogicalTime(implementation);
}

RTI13LogicalTimeFactory::LogicalTime
RTI13LogicalTimeFactory::getLogicalTime(const RTI::FedTime& fedTime)
{
  FedTimeImplementation* implementation = new FedTimeImplementation;
  (*implementation->_fedTime) = fedTime;
  return LogicalTime(implementation);
}

RTI13LogicalTimeFactory::LogicalTimeInterval
RTI13LogicalTimeFactory::getLogicalTimeInterval(const RTI::FedTime& fedTime)
{
  FedTimeImplementation* implementation = new FedTimeImplementation;
  (*implementation->_fedTime) = fedTime;
  return LogicalTimeInterval(implementation);
}

bool
RTI13LogicalTimeFactory::isZeroTimeInterval(const RTI::FedTime& fedTime)
{
  RTI_UNIQUE_PTR<RTI::FedTime> logicalTime(RTI::FedTimeFactory::makeZero());
  *logicalTime = fedTime;
  return logicalTime->isZero() == RTI::RTI_TRUE;
}

bool
RTI13LogicalTimeFactory::isPositiveTimeInterval(const RTI::FedTime& fedTime)
{
  SharedPtr<FedTimeImplementation> implementation = new FedTimeImplementation;
  return ((*implementation->_fedTime) <= fedTime) == RTI::RTI_TRUE;
}

const RTI::FedTime&
RTI13LogicalTimeFactory::getLogicalTime(const RTI13LogicalTimeFactory::LogicalTime& logicalTime)
{
  return *logicalTime._implementation->_fedTime;
}

const RTI::FedTime&
RTI13LogicalTimeFactory::getLogicalTimeInterval(const RTI13LogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval)
{
  return *logicalTimeInterval._implementation->_fedTime;
}

RTI13LogicalTimeFactory::LogicalTime
RTI13LogicalTimeFactory::decodeLogicalTime(const VariableLengthData& variableLengthData)
{
  RTI::FedTime* fedTime = RTI::FedTimeFactory::decode(static_cast<const char*>(variableLengthData.constData()));
  return LogicalTime(new FedTimeImplementation(fedTime));
}

VariableLengthData
RTI13LogicalTimeFactory::encodeLogicalTime(const RTI::FedTime& fedTime)
{
  VariableLengthData variableLengthData;
  variableLengthData.resize(fedTime.encodedLength());
  fedTime.encode(static_cast<char*>(variableLengthData.data()));
  return variableLengthData;
}

std::string
RTI13LogicalTimeFactory::toString(const RTI::FedTime& fedTime)
{
  RTI_UNIQUE_PTR<RTI::FedTime> logicalTime(RTI::FedTimeFactory::makeZero());
  *logicalTime = fedTime;
  std::vector<char> data(logicalTime->getPrintableLength(), 0);
  logicalTime->getPrintableString(&data.front());
  return localeToUtf8(std::string(&data.front(), data.size()));
}

}
