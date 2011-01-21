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

#include "RTI1516LogicalTimeFactory.h"

#include <RTI/LogicalTime.h>
#include <RTI/LogicalTimeInterval.h>
#include <RTI/LogicalTimeFactory.h>

#include <memory>

namespace OpenRTI {

class OPENRTI_LOCAL RTI1516LogicalTimeFactory::LogicalTimeFactoryImplementation : public Referenced {
public:
  LogicalTimeFactoryImplementation(std::auto_ptr<rti1516::LogicalTimeFactory> logicalTimeFactory);
  ~LogicalTimeFactoryImplementation();

  std::wstring getName() const;
  LogicalTimeImplementation* initialLogicalTime() const;
  LogicalTimeImplementation* decodeLogicalTime(const VariableLengthData& variableLengthData) const;
  LogicalTimeIntervalImplementation* zeroLogicalTimeInterval() const;

  std::auto_ptr<rti1516::LogicalTimeFactory> _logicalTimeFactory;
  std::auto_ptr<rti1516::LogicalTimeInterval> _zeroTimeInterval;
};

class OPENRTI_LOCAL RTI1516LogicalTimeFactory::LogicalTimeIntervalImplementation : public Referenced {
public:
  LogicalTimeIntervalImplementation(std::auto_ptr<rti1516::LogicalTimeInterval> logicalTimeInterval,
                                    const LogicalTimeFactoryImplementation* logicalTimeFactory) :
    _logicalTimeInterval(logicalTimeInterval),
    _logicalTimeFactory(logicalTimeFactory)
  {
  }
  bool operator>(const LogicalTimeIntervalImplementation& implementation) const
  {
    if (!_logicalTimeInterval.get())
      return false;
    if (!implementation._logicalTimeInterval.get())
      return false;
    return *_logicalTimeInterval > *implementation._logicalTimeInterval;
  }
  bool operator<(const LogicalTimeIntervalImplementation& implementation) const
  {
    if (!_logicalTimeInterval.get())
      return false;
    if (!implementation._logicalTimeInterval.get())
      return false;
    return *_logicalTimeInterval < *implementation._logicalTimeInterval;
  }
  bool operator==(const LogicalTimeIntervalImplementation& implementation) const
  {
    if (!_logicalTimeInterval.get())
      return false;
    if (!implementation._logicalTimeInterval.get())
      return false;
    return *_logicalTimeInterval == *implementation._logicalTimeInterval;
  }
  bool operator!=(const LogicalTimeIntervalImplementation& implementation) const
  {
    if (!_logicalTimeInterval.get())
      return false;
    if (!implementation._logicalTimeInterval.get())
      return false;
    return !(*_logicalTimeInterval == *implementation._logicalTimeInterval);
  }
  bool operator>=(const LogicalTimeIntervalImplementation& implementation) const
  {
    if (!_logicalTimeInterval.get())
      return false;
    if (!implementation._logicalTimeInterval.get())
      return false;
    return *_logicalTimeInterval >= *implementation._logicalTimeInterval;
  }
  bool operator<=(const LogicalTimeIntervalImplementation& implementation) const
  {
    if (!_logicalTimeInterval.get())
      return false;
    if (!implementation._logicalTimeInterval.get())
      return false;
    return *_logicalTimeInterval <= *implementation._logicalTimeInterval;
  }

  std::auto_ptr<rti1516::LogicalTimeInterval> _logicalTimeInterval;
  SharedPtr<const LogicalTimeFactoryImplementation> _logicalTimeFactory;
};

class OPENRTI_LOCAL RTI1516LogicalTimeFactory::LogicalTimeImplementation : public Referenced {
public:
  LogicalTimeImplementation(std::auto_ptr<rti1516::LogicalTime> logicalTime,
                            const LogicalTimeFactoryImplementation* logicalTimeFactory) :
    _logicalTime(logicalTime),
    _logicalTimeFactory(logicalTimeFactory)
  {
  }
  LogicalTimeImplementation* operator+(const LogicalTimeIntervalImplementation& implementation) const
  {
    if (!_logicalTime.get())
      return 0;
    if (!implementation._logicalTimeInterval.get())
      return 0;
    LogicalTimeImplementation* logicalTime;
    logicalTime = _logicalTimeFactory->initialLogicalTime();
    (*logicalTime->_logicalTime) = *_logicalTime;
    (*logicalTime->_logicalTime) += *implementation._logicalTimeInterval;
    return logicalTime;
  }
  LogicalTimeImplementation* operator-(const LogicalTimeIntervalImplementation& implementation) const
  {
    if (!_logicalTime.get())
      return 0;
    if (!implementation._logicalTimeInterval.get())
      return 0;
    LogicalTimeImplementation* logicalTime = _logicalTimeFactory->initialLogicalTime();
    (*logicalTime->_logicalTime) = *_logicalTime;
    (*logicalTime->_logicalTime) -= *implementation._logicalTimeInterval;
    return logicalTime;
  }
  LogicalTimeIntervalImplementation* operator-(const LogicalTimeImplementation& implementation) const
  {
    if (!_logicalTime.get())
      return 0;
    if (!implementation._logicalTime.get())
      return 0;
    LogicalTimeIntervalImplementation* logicalTimeInterval = _logicalTimeFactory->zeroLogicalTimeInterval();
    logicalTimeInterval->_logicalTimeInterval->setToDifference(*_logicalTime, *implementation._logicalTime);
    return logicalTimeInterval;
  }
  bool operator>(const LogicalTimeImplementation& implementation) const
  {
    if (!_logicalTime.get())
      return false;
    if (!implementation._logicalTime.get())
      return false;
    return *_logicalTime > *implementation._logicalTime;
  }
  bool operator<(const LogicalTimeImplementation& implementation) const
  {
    if (!_logicalTime.get())
      return false;
    if (!implementation._logicalTime.get())
      return false;
    return *_logicalTime < *implementation._logicalTime;
  }
  bool operator==(const LogicalTimeImplementation& implementation) const
  {
    if (!_logicalTime.get())
      return false;
    if (!implementation._logicalTime.get())
      return false;
    return *_logicalTime == *implementation._logicalTime;
  }
  bool operator!=(const LogicalTimeImplementation& implementation) const
  {
    if (!_logicalTime.get())
      return false;
    if (!implementation._logicalTime.get())
      return false;
    return !(*_logicalTime == *implementation._logicalTime);
  }
  bool operator>=(const LogicalTimeImplementation& implementation) const
  {
    if (!_logicalTime.get())
      return false;
    if (!implementation._logicalTime.get())
      return false;
    return *_logicalTime >= *implementation._logicalTime;
  }
  bool operator<=(const LogicalTimeImplementation& implementation) const
  {
    if (!_logicalTime.get())
      return false;
    if (!implementation._logicalTime.get())
      return false;
    return *_logicalTime <= *implementation._logicalTime;
  }

  std::auto_ptr<rti1516::LogicalTime> _logicalTime;
  SharedPtr<const LogicalTimeFactoryImplementation> _logicalTimeFactory;
};








RTI1516LogicalTimeFactory::LogicalTimeFactoryImplementation::LogicalTimeFactoryImplementation(std::auto_ptr<rti1516::LogicalTimeFactory> factory) :
  _logicalTimeFactory(factory)
{
  _zeroTimeInterval = _logicalTimeFactory->makeLogicalTimeInterval();
  if (_zeroTimeInterval.get())
    _zeroTimeInterval->setZero();
}

RTI1516LogicalTimeFactory::LogicalTimeFactoryImplementation::~LogicalTimeFactoryImplementation()
{
}

std::wstring
RTI1516LogicalTimeFactory::LogicalTimeFactoryImplementation::getName() const
{
  return _logicalTimeFactory->makeLogicalTime()->implementationName();
}

RTI1516LogicalTimeFactory::LogicalTimeImplementation*
RTI1516LogicalTimeFactory::LogicalTimeFactoryImplementation::initialLogicalTime() const
{
  if (!_logicalTimeFactory.get())
    return 0;
  std::auto_ptr<rti1516::LogicalTime> logicalTime = _logicalTimeFactory->makeLogicalTime();
  if (!logicalTime.get())
    return 0;
  logicalTime->setInitial();
  return new LogicalTimeImplementation(logicalTime, this);
}

RTI1516LogicalTimeFactory::LogicalTimeImplementation*
RTI1516LogicalTimeFactory::LogicalTimeFactoryImplementation::decodeLogicalTime(const VariableLengthData& variableLengthData) const
{
  if (!_logicalTimeFactory.get())
    return 0;
  std::auto_ptr<rti1516::LogicalTime> logicalTime = _logicalTimeFactory->makeLogicalTime();
  if (!logicalTime.get())
    return 0;
  logicalTime->decode(rti1516::VariableLengthDataFriend::create(variableLengthData));
  return new LogicalTimeImplementation(logicalTime, this);
}

RTI1516LogicalTimeFactory::LogicalTimeIntervalImplementation*
RTI1516LogicalTimeFactory::LogicalTimeFactoryImplementation::zeroLogicalTimeInterval() const
{
  if (!_logicalTimeFactory.get())
    return 0;
  std::auto_ptr<rti1516::LogicalTimeInterval> logicalTimeInterval = _logicalTimeFactory->makeLogicalTimeInterval();
  if (!logicalTimeInterval.get())
    return 0;
  logicalTimeInterval->setZero();
  return new LogicalTimeIntervalImplementation(logicalTimeInterval, this);
}





RTI1516LogicalTimeFactory::LogicalTimeInterval::LogicalTimeInterval(RTI1516LogicalTimeFactory::LogicalTimeIntervalImplementation* implementation) :
  _implementation(implementation)
{
}

RTI1516LogicalTimeFactory::LogicalTimeInterval::LogicalTimeInterval(const RTI1516LogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval) :
  _implementation(logicalTimeInterval._implementation)
{
}

RTI1516LogicalTimeFactory::LogicalTimeInterval::~LogicalTimeInterval()
{
}

RTI1516LogicalTimeFactory::LogicalTimeInterval&
RTI1516LogicalTimeFactory::LogicalTimeInterval::operator=(const RTI1516LogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval)
{
  _implementation = logicalTimeInterval._implementation;
  return *this;
}

bool
RTI1516LogicalTimeFactory::LogicalTimeInterval::operator>(const RTI1516LogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTimeInterval._implementation.valid())
    return false;
  return (*_implementation) > (*logicalTimeInterval._implementation);
}

bool
RTI1516LogicalTimeFactory::LogicalTimeInterval::operator<(const RTI1516LogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTimeInterval._implementation.valid())
    return false;
  return (*_implementation) < (*logicalTimeInterval._implementation);
}

bool
RTI1516LogicalTimeFactory::LogicalTimeInterval::operator==(const RTI1516LogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTimeInterval._implementation.valid())
    return false;
  return (*_implementation) == (*logicalTimeInterval._implementation);
}

bool
RTI1516LogicalTimeFactory::LogicalTimeInterval::operator!=(const RTI1516LogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTimeInterval._implementation.valid())
    return false;
  return (*_implementation) != (*logicalTimeInterval._implementation);
}

bool
RTI1516LogicalTimeFactory::LogicalTimeInterval::operator>=(const RTI1516LogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTimeInterval._implementation.valid())
    return false;
  return (*_implementation) >= (*logicalTimeInterval._implementation);
}

bool
RTI1516LogicalTimeFactory::LogicalTimeInterval::operator<=(const RTI1516LogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTimeInterval._implementation.valid())
    return false;
  return (*_implementation) <= (*logicalTimeInterval._implementation);
}



RTI1516LogicalTimeFactory::LogicalTime::LogicalTime(RTI1516LogicalTimeFactory::LogicalTimeImplementation* implementation) :
  _implementation(implementation)
{
}

RTI1516LogicalTimeFactory::LogicalTime::LogicalTime(const RTI1516LogicalTimeFactory::LogicalTime& logicalTime) :
  _implementation(logicalTime._implementation)
{
}

RTI1516LogicalTimeFactory::LogicalTime::~LogicalTime()
{
}

RTI1516LogicalTimeFactory::LogicalTime&
RTI1516LogicalTimeFactory::LogicalTime::operator=(const RTI1516LogicalTimeFactory::LogicalTime& logicalTime)
{
  _implementation = logicalTime._implementation;
  return *this;
}


RTI1516LogicalTimeFactory::LogicalTime&
RTI1516LogicalTimeFactory::LogicalTime::operator+=(const RTI1516LogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval)
{
  if (!_implementation.valid())
    return *this;
  if (!logicalTimeInterval._implementation.valid())
    return *this;
  _implementation = (*_implementation) + (*logicalTimeInterval._implementation);
  return *this;
}

RTI1516LogicalTimeFactory::LogicalTime&
RTI1516LogicalTimeFactory::LogicalTime::operator-=(const RTI1516LogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval)
{
  if (!_implementation.valid())
    return *this;
  if (!logicalTimeInterval._implementation.valid())
    return *this;
  _implementation = (*_implementation) - (*logicalTimeInterval._implementation);
  return *this;
}

RTI1516LogicalTimeFactory::LogicalTimeInterval
RTI1516LogicalTimeFactory::LogicalTime::operator-(const RTI1516LogicalTimeFactory::LogicalTime& logicalTime) const
{
  if (!_implementation.valid())
    return LogicalTimeInterval();
  if (!logicalTime._implementation.valid())
    return LogicalTimeInterval();
  return LogicalTimeInterval((*_implementation) - (*logicalTime._implementation));
}

bool
RTI1516LogicalTimeFactory::LogicalTime::operator>(const RTI1516LogicalTimeFactory::LogicalTime& logicalTime) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTime._implementation.valid())
    return false;
  return (*_implementation) > (*logicalTime._implementation);
}

bool
RTI1516LogicalTimeFactory::LogicalTime::operator<(const RTI1516LogicalTimeFactory::LogicalTime& logicalTime) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTime._implementation.valid())
    return false;
  return (*_implementation) < (*logicalTime._implementation);
}

bool
RTI1516LogicalTimeFactory::LogicalTime::operator==(const RTI1516LogicalTimeFactory::LogicalTime& logicalTime) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTime._implementation.valid())
    return false;
  return (*_implementation) == (*logicalTime._implementation);
}

bool
RTI1516LogicalTimeFactory::LogicalTime::operator!=(const RTI1516LogicalTimeFactory::LogicalTime& logicalTime) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTime._implementation.valid())
    return false;
  return (*_implementation) != (*logicalTime._implementation);
}

bool
RTI1516LogicalTimeFactory::LogicalTime::operator>=(const RTI1516LogicalTimeFactory::LogicalTime& logicalTime) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTime._implementation.valid())
    return false;
  return (*_implementation) >= (*logicalTime._implementation);
}

bool
RTI1516LogicalTimeFactory::LogicalTime::operator<=(const RTI1516LogicalTimeFactory::LogicalTime& logicalTime) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTime._implementation.valid())
    return false;
  return (*_implementation) <= (*logicalTime._implementation);
}



RTI1516LogicalTimeFactory::RTI1516LogicalTimeFactory(std::auto_ptr<rti1516::LogicalTimeFactory> logicalTimeFactory) :
  _implementation(new LogicalTimeFactoryImplementation(logicalTimeFactory))
{
}

RTI1516LogicalTimeFactory::RTI1516LogicalTimeFactory(const RTI1516LogicalTimeFactory& logicalTimeFactory) :
  _implementation(logicalTimeFactory._implementation)
{
}

RTI1516LogicalTimeFactory::~RTI1516LogicalTimeFactory()
{
}

RTI1516LogicalTimeFactory&
RTI1516LogicalTimeFactory::operator=(const RTI1516LogicalTimeFactory& logicalTimeFactory)
{
  _implementation = logicalTimeFactory._implementation;
  return *this;
}

std::wstring
RTI1516LogicalTimeFactory::getName() const
{
  return _implementation->getName();
}

RTI1516LogicalTimeFactory::LogicalTime
RTI1516LogicalTimeFactory::initialLogicalTime() const
{
  return LogicalTime(_implementation->initialLogicalTime());
}

RTI1516LogicalTimeFactory::LogicalTimeInterval
RTI1516LogicalTimeFactory::zeroLogicalTimeInterval() const
{
  return LogicalTimeInterval(_implementation->zeroLogicalTimeInterval());
}

RTI1516LogicalTimeFactory::LogicalTime
RTI1516LogicalTimeFactory::getLogicalTime(const rti1516::LogicalTime& rti1516LogicalTime)
{
  if (!_implementation.valid())
    return LogicalTime();
  if (!_implementation->_logicalTimeFactory.get())
    return LogicalTime();
  std::auto_ptr<rti1516::LogicalTime> logicalTime = _implementation->_logicalTimeFactory->makeLogicalTime();
  if (!logicalTime.get())
    return LogicalTime();
  (*logicalTime) = rti1516LogicalTime;
  return LogicalTime(new LogicalTimeImplementation(logicalTime, _implementation.get()));
}

RTI1516LogicalTimeFactory::LogicalTimeInterval
RTI1516LogicalTimeFactory::getLogicalTimeInterval(const rti1516::LogicalTimeInterval& rti1516LogicalTimeInterval)
{
  if (!_implementation.valid())
    return LogicalTimeInterval();
  if (!_implementation->_logicalTimeFactory.get())
    return LogicalTimeInterval();
  std::auto_ptr<rti1516::LogicalTimeInterval> logicalTimeInterval = _implementation->_logicalTimeFactory->makeLogicalTimeInterval();
  if (!logicalTimeInterval.get())
    return 0;
  (*logicalTimeInterval) = rti1516LogicalTimeInterval;
  return LogicalTimeInterval(new LogicalTimeIntervalImplementation(logicalTimeInterval, _implementation.get()));
}

bool
RTI1516LogicalTimeFactory::isPositiveTimeInterval(const rti1516::LogicalTimeInterval& rti1516LogicalTimeInterval)
{
  if (!_implementation.valid())
    return false;
  if (!_implementation->_zeroTimeInterval.get())
    return false;
  return (*_implementation->_zeroTimeInterval) <= rti1516LogicalTimeInterval;
}

const rti1516::LogicalTime&
RTI1516LogicalTimeFactory::getLogicalTime(const RTI1516LogicalTimeFactory::LogicalTime& logicalTime)
{
  if (!logicalTime._implementation.valid())
    throw rti1516::FederateInternalError(L"Empty logical time!");

  const rti1516::LogicalTime* rti1516LogicalTime = logicalTime._implementation->_logicalTime.get();
  if (!rti1516LogicalTime)
    throw rti1516::FederateInternalError(L"invalid logical time!");

  return *rti1516LogicalTime;
}

const rti1516::LogicalTimeInterval&
RTI1516LogicalTimeFactory::getLogicalTimeInterval(const RTI1516LogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval)
{
  if (!logicalTimeInterval._implementation.valid())
    throw rti1516::FederateInternalError(L"Empty logical time!");

  const rti1516::LogicalTimeInterval* rti1516LogicalTime = logicalTimeInterval._implementation->_logicalTimeInterval.get();
  if (!rti1516LogicalTime)
    throw rti1516::FederateInternalError(L"invalid logical time!");

  return *rti1516LogicalTime;
}

RTI1516LogicalTimeFactory::LogicalTime
RTI1516LogicalTimeFactory::decodeLogicalTime(const VariableLengthData& variableLengthData)
{
  return LogicalTime(_implementation->decodeLogicalTime(variableLengthData));
}

}
