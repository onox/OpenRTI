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

#include "RTI1516ELogicalTimeFactory.h"

#include <RTI/LogicalTime.h>
#include <RTI/LogicalTimeInterval.h>
#include <RTI/LogicalTimeFactory.h>

#include <memory>

#include "VariableLengthDataFriend.h"

namespace OpenRTI {

class OPENRTI_LOCAL RTI1516ELogicalTimeFactory::LogicalTimeFactoryImplementation : public Referenced {
public:
  LogicalTimeFactoryImplementation(RTI_UNIQUE_PTR<rti1516e::LogicalTimeFactory> logicalTimeFactory);
  ~LogicalTimeFactoryImplementation();

  std::string getName() const;
  LogicalTimeImplementation* initialLogicalTime() const;
  LogicalTimeImplementation* finalLogicalTime() const;
  LogicalTimeImplementation* decodeLogicalTime(const VariableLengthData& variableLengthData) const;
  LogicalTimeIntervalImplementation* zeroLogicalTimeInterval() const;

  RTI_UNIQUE_PTR<rti1516e::LogicalTimeFactory> _logicalTimeFactory;
  RTI_UNIQUE_PTR<rti1516e::LogicalTimeInterval> _zeroTimeInterval;
  RTI_UNIQUE_PTR<rti1516e::LogicalTimeInterval> _epsilonTimeInterval;
};

class OPENRTI_LOCAL RTI1516ELogicalTimeFactory::LogicalTimeIntervalImplementation : public Referenced {
public:
  LogicalTimeIntervalImplementation(RTI_UNIQUE_PTR<rti1516e::LogicalTimeInterval> logicalTimeInterval,
                                    const LogicalTimeFactoryImplementation* logicalTimeFactory) :
    _logicalTimeInterval(OpenRTI_MOVE(logicalTimeInterval)),
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

  RTI_UNIQUE_PTR<rti1516e::LogicalTimeInterval> _logicalTimeInterval;
  SharedPtr<const LogicalTimeFactoryImplementation> _logicalTimeFactory;
};

class OPENRTI_LOCAL RTI1516ELogicalTimeFactory::LogicalTimeImplementation : public Referenced {
public:
  LogicalTimeImplementation(RTI_UNIQUE_PTR<rti1516e::LogicalTime> logicalTime,
                            const LogicalTimeFactoryImplementation* logicalTimeFactory) :
    _logicalTime(OpenRTI_MOVE(logicalTime)),
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

  RTI_UNIQUE_PTR<rti1516e::LogicalTime> _logicalTime;
  SharedPtr<const LogicalTimeFactoryImplementation> _logicalTimeFactory;
};








RTI1516ELogicalTimeFactory::LogicalTimeFactoryImplementation::LogicalTimeFactoryImplementation(RTI_UNIQUE_PTR<rti1516e::LogicalTimeFactory> factory) :
  _logicalTimeFactory(OpenRTI_MOVE(factory))
{
  _zeroTimeInterval = _logicalTimeFactory->makeZero();
  _epsilonTimeInterval = _logicalTimeFactory->makeEpsilon();
}

RTI1516ELogicalTimeFactory::LogicalTimeFactoryImplementation::~LogicalTimeFactoryImplementation()
{
}

std::string
RTI1516ELogicalTimeFactory::LogicalTimeFactoryImplementation::getName() const
{
  return ucsToUtf8(_logicalTimeFactory->getName());
}

RTI1516ELogicalTimeFactory::LogicalTimeImplementation*
RTI1516ELogicalTimeFactory::LogicalTimeFactoryImplementation::initialLogicalTime() const
{
  if (!_logicalTimeFactory.get())
    return 0;
  RTI_UNIQUE_PTR<rti1516e::LogicalTime> logicalTime = _logicalTimeFactory->makeInitial();
  return new LogicalTimeImplementation(OpenRTI_MOVE(logicalTime), this);
}

RTI1516ELogicalTimeFactory::LogicalTimeImplementation*
RTI1516ELogicalTimeFactory::LogicalTimeFactoryImplementation::finalLogicalTime() const
{
  if (!_logicalTimeFactory.get())
    return 0;
  RTI_UNIQUE_PTR<rti1516e::LogicalTime> logicalTime = _logicalTimeFactory->makeFinal();
  return new LogicalTimeImplementation(OpenRTI_MOVE(logicalTime), this);
}

RTI1516ELogicalTimeFactory::LogicalTimeImplementation*
RTI1516ELogicalTimeFactory::LogicalTimeFactoryImplementation::decodeLogicalTime(const VariableLengthData& variableLengthData) const
{
  if (!_logicalTimeFactory.get())
    return 0;
  RTI_UNIQUE_PTR<rti1516e::LogicalTime> logicalTime;
  logicalTime = _logicalTimeFactory->decodeLogicalTime(rti1516e::VariableLengthDataFriend::create(variableLengthData));
  if (!logicalTime.get())
    return 0;
  return new LogicalTimeImplementation(OpenRTI_MOVE(logicalTime), this);
}

RTI1516ELogicalTimeFactory::LogicalTimeIntervalImplementation*
RTI1516ELogicalTimeFactory::LogicalTimeFactoryImplementation::zeroLogicalTimeInterval() const
{
  if (!_logicalTimeFactory.get())
    return 0;
  RTI_UNIQUE_PTR<rti1516e::LogicalTimeInterval> logicalTimeInterval = _logicalTimeFactory->makeZero();
  if (!logicalTimeInterval.get())
    return 0;
  return new LogicalTimeIntervalImplementation(OpenRTI_MOVE(logicalTimeInterval), this);
}





RTI1516ELogicalTimeFactory::LogicalTimeInterval::LogicalTimeInterval(RTI1516ELogicalTimeFactory::LogicalTimeIntervalImplementation* implementation) :
  _implementation(implementation)
{
}

RTI1516ELogicalTimeFactory::LogicalTimeInterval::LogicalTimeInterval(const RTI1516ELogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval) :
  _implementation(logicalTimeInterval._implementation)
{
}

RTI1516ELogicalTimeFactory::LogicalTimeInterval::~LogicalTimeInterval()
{
}

RTI1516ELogicalTimeFactory::LogicalTimeInterval&
RTI1516ELogicalTimeFactory::LogicalTimeInterval::operator=(const RTI1516ELogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval)
{
  _implementation = logicalTimeInterval._implementation;
  return *this;
}

bool
RTI1516ELogicalTimeFactory::LogicalTimeInterval::operator>(const RTI1516ELogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTimeInterval._implementation.valid())
    return false;
  return (*_implementation) > (*logicalTimeInterval._implementation);
}

bool
RTI1516ELogicalTimeFactory::LogicalTimeInterval::operator<(const RTI1516ELogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTimeInterval._implementation.valid())
    return false;
  return (*_implementation) < (*logicalTimeInterval._implementation);
}

bool
RTI1516ELogicalTimeFactory::LogicalTimeInterval::operator==(const RTI1516ELogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTimeInterval._implementation.valid())
    return false;
  return (*_implementation) == (*logicalTimeInterval._implementation);
}

bool
RTI1516ELogicalTimeFactory::LogicalTimeInterval::operator!=(const RTI1516ELogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTimeInterval._implementation.valid())
    return false;
  return (*_implementation) != (*logicalTimeInterval._implementation);
}

bool
RTI1516ELogicalTimeFactory::LogicalTimeInterval::operator>=(const RTI1516ELogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTimeInterval._implementation.valid())
    return false;
  return (*_implementation) >= (*logicalTimeInterval._implementation);
}

bool
RTI1516ELogicalTimeFactory::LogicalTimeInterval::operator<=(const RTI1516ELogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTimeInterval._implementation.valid())
    return false;
  return (*_implementation) <= (*logicalTimeInterval._implementation);
}



RTI1516ELogicalTimeFactory::LogicalTime::LogicalTime(RTI1516ELogicalTimeFactory::LogicalTimeImplementation* implementation) :
  _implementation(implementation)
{
}

RTI1516ELogicalTimeFactory::LogicalTime::LogicalTime(const RTI1516ELogicalTimeFactory::LogicalTime& logicalTime) :
  _implementation(logicalTime._implementation)
{
}

RTI1516ELogicalTimeFactory::LogicalTime::~LogicalTime()
{
}

RTI1516ELogicalTimeFactory::LogicalTime&
RTI1516ELogicalTimeFactory::LogicalTime::operator=(const RTI1516ELogicalTimeFactory::LogicalTime& logicalTime)
{
  _implementation = logicalTime._implementation;
  return *this;
}


RTI1516ELogicalTimeFactory::LogicalTime&
RTI1516ELogicalTimeFactory::LogicalTime::operator+=(const RTI1516ELogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval)
{
  if (!_implementation.valid())
    return *this;
  if (!logicalTimeInterval._implementation.valid())
    return *this;
  _implementation = (*_implementation) + (*logicalTimeInterval._implementation);
  return *this;
}

RTI1516ELogicalTimeFactory::LogicalTime&
RTI1516ELogicalTimeFactory::LogicalTime::operator-=(const RTI1516ELogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval)
{
  if (!_implementation.valid())
    return *this;
  if (!logicalTimeInterval._implementation.valid())
    return *this;
  _implementation = (*_implementation) - (*logicalTimeInterval._implementation);
  return *this;
}

RTI1516ELogicalTimeFactory::LogicalTimeInterval
RTI1516ELogicalTimeFactory::LogicalTime::operator-(const RTI1516ELogicalTimeFactory::LogicalTime& logicalTime) const
{
  if (!_implementation.valid())
    return LogicalTimeInterval();
  if (!logicalTime._implementation.valid())
    return LogicalTimeInterval();
  return LogicalTimeInterval((*_implementation) - (*logicalTime._implementation));
}

bool
RTI1516ELogicalTimeFactory::LogicalTime::operator>(const RTI1516ELogicalTimeFactory::LogicalTime& logicalTime) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTime._implementation.valid())
    return false;
  return (*_implementation) > (*logicalTime._implementation);
}

bool
RTI1516ELogicalTimeFactory::LogicalTime::operator<(const RTI1516ELogicalTimeFactory::LogicalTime& logicalTime) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTime._implementation.valid())
    return false;
  return (*_implementation) < (*logicalTime._implementation);
}

bool
RTI1516ELogicalTimeFactory::LogicalTime::operator==(const RTI1516ELogicalTimeFactory::LogicalTime& logicalTime) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTime._implementation.valid())
    return false;
  return (*_implementation) == (*logicalTime._implementation);
}

bool
RTI1516ELogicalTimeFactory::LogicalTime::operator!=(const RTI1516ELogicalTimeFactory::LogicalTime& logicalTime) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTime._implementation.valid())
    return false;
  return (*_implementation) != (*logicalTime._implementation);
}

bool
RTI1516ELogicalTimeFactory::LogicalTime::operator>=(const RTI1516ELogicalTimeFactory::LogicalTime& logicalTime) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTime._implementation.valid())
    return false;
  return (*_implementation) >= (*logicalTime._implementation);
}

bool
RTI1516ELogicalTimeFactory::LogicalTime::operator<=(const RTI1516ELogicalTimeFactory::LogicalTime& logicalTime) const
{
  if (!_implementation.valid())
    return false;
  if (!logicalTime._implementation.valid())
    return false;
  return (*_implementation) <= (*logicalTime._implementation);
}



RTI1516ELogicalTimeFactory::RTI1516ELogicalTimeFactory(RTI_UNIQUE_PTR<rti1516e::LogicalTimeFactory> logicalTimeFactory) :
  _implementation(new LogicalTimeFactoryImplementation(OpenRTI_MOVE(logicalTimeFactory)))
{
}

RTI1516ELogicalTimeFactory::RTI1516ELogicalTimeFactory(const RTI1516ELogicalTimeFactory& logicalTimeFactory) :
  _implementation(logicalTimeFactory._implementation)
{
}

RTI1516ELogicalTimeFactory::~RTI1516ELogicalTimeFactory()
{
}

RTI1516ELogicalTimeFactory&
RTI1516ELogicalTimeFactory::operator=(const RTI1516ELogicalTimeFactory& logicalTimeFactory)
{
  _implementation = logicalTimeFactory._implementation;
  return *this;
}

std::string
RTI1516ELogicalTimeFactory::getName() const
{
  return _implementation->getName();
}

RTI1516ELogicalTimeFactory::LogicalTime
RTI1516ELogicalTimeFactory::initialLogicalTime() const
{
  return LogicalTime(_implementation->initialLogicalTime());
}

RTI1516ELogicalTimeFactory::LogicalTime
RTI1516ELogicalTimeFactory::finalLogicalTime() const
{
  return LogicalTime(_implementation->finalLogicalTime());
}

RTI1516ELogicalTimeFactory::LogicalTimeInterval
RTI1516ELogicalTimeFactory::zeroLogicalTimeInterval() const
{
  return LogicalTimeInterval(_implementation->zeroLogicalTimeInterval());
}

RTI1516ELogicalTimeFactory::LogicalTime
RTI1516ELogicalTimeFactory::nextAfter(const RTI1516ELogicalTimeFactory::LogicalTime& logicalTime)
{
  if (!logicalTime._implementation.valid())
    return LogicalTime();
  if (!logicalTime._implementation->_logicalTime.get())
    return LogicalTime();
  if (!logicalTime._implementation->_logicalTimeFactory.valid())
    return LogicalTime();
  if (!logicalTime._implementation->_logicalTimeFactory->_epsilonTimeInterval.get())
    return LogicalTime();
  LogicalTimeImplementation* implementation;
  implementation = logicalTime._implementation->_logicalTimeFactory->initialLogicalTime();
  (*implementation->_logicalTime) = *logicalTime._implementation->_logicalTime;
  (*implementation->_logicalTime) += *logicalTime._implementation->_logicalTimeFactory->_epsilonTimeInterval;
  return LogicalTime(implementation);
}

RTI1516ELogicalTimeFactory::LogicalTime
RTI1516ELogicalTimeFactory::getLogicalTime(const rti1516e::LogicalTime& rti1516LogicalTime)
{
  if (!_implementation.valid())
    return LogicalTime();
  if (!_implementation->_logicalTimeFactory.get())
    return LogicalTime();
  RTI_UNIQUE_PTR<rti1516e::LogicalTime> logicalTime = _implementation->_logicalTimeFactory->makeInitial();
  if (!logicalTime.get())
    return LogicalTime();
  (*logicalTime) = rti1516LogicalTime;
  return LogicalTime(new LogicalTimeImplementation(OpenRTI_MOVE(logicalTime), _implementation.get()));
}

RTI1516ELogicalTimeFactory::LogicalTimeInterval
RTI1516ELogicalTimeFactory::getLogicalTimeInterval(const rti1516e::LogicalTimeInterval& rti1516LogicalTimeInterval)
{
  if (!_implementation.valid())
    return LogicalTimeInterval();
  if (!_implementation->_logicalTimeFactory.get())
    return LogicalTimeInterval();
  RTI_UNIQUE_PTR<rti1516e::LogicalTimeInterval> logicalTimeInterval = _implementation->_logicalTimeFactory->makeZero();
  if (!logicalTimeInterval.get())
    return 0;
  (*logicalTimeInterval) = rti1516LogicalTimeInterval;
  return LogicalTimeInterval(new LogicalTimeIntervalImplementation(OpenRTI_MOVE(logicalTimeInterval), _implementation.get()));
}

bool
RTI1516ELogicalTimeFactory::isPositiveTimeInterval(const rti1516e::LogicalTimeInterval& rti1516LogicalTimeInterval)
{
  if (!_implementation.valid())
    return false;
  if (!_implementation->_zeroTimeInterval.get())
    return false;
  return (*_implementation->_zeroTimeInterval) <= rti1516LogicalTimeInterval;
}

const rti1516e::LogicalTime&
RTI1516ELogicalTimeFactory::getLogicalTime(const RTI1516ELogicalTimeFactory::LogicalTime& logicalTime)
{
  if (!logicalTime._implementation.valid())
    throw rti1516e::FederateInternalError(L"Empty logical time!");

  const rti1516e::LogicalTime* rti1516LogicalTime = logicalTime._implementation->_logicalTime.get();
  if (!rti1516LogicalTime)
    throw rti1516e::FederateInternalError(L"invalid logical time!");

  return *rti1516LogicalTime;
}

const rti1516e::LogicalTimeInterval&
RTI1516ELogicalTimeFactory::getLogicalTimeInterval(const RTI1516ELogicalTimeFactory::LogicalTimeInterval& logicalTimeInterval)
{
  if (!logicalTimeInterval._implementation.valid())
    throw rti1516e::FederateInternalError(L"Empty logical time!");

  const rti1516e::LogicalTimeInterval* rti1516LogicalTime = logicalTimeInterval._implementation->_logicalTimeInterval.get();
  if (!rti1516LogicalTime)
    throw rti1516e::FederateInternalError(L"invalid logical time!");

  return *rti1516LogicalTime;
}

RTI1516ELogicalTimeFactory::LogicalTime
RTI1516ELogicalTimeFactory::decodeLogicalTime(const VariableLengthData& variableLengthData)
{
  return LogicalTime(_implementation->decodeLogicalTime(variableLengthData));
}

}
