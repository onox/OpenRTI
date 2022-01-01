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

#include <cstdlib>
#include <string>
#include <memory>
#include <vector>
#include <iostream>

#include <RTI/HLAfloat64Time.h>
#include <RTI/HLAfloat64Interval.h>
#include <RTI/HLAinteger64Time.h>
#include <RTI/HLAinteger64Interval.h>

#include <Options.h>
#include <StringUtils.h>
#include <Thread.h>

#include <RTI1516TestLib.h>

namespace OpenRTI {

enum TimeAdvanceMode {
  TimeAdvanceRequest = 0,
  TimeAdvanceRequestAvailable = 1,
  NextMessageRequest = 2,
  NextMessageRequestAvailable = 3,
  FlushQueueRequest = 4,
  AllTimeAdvanceRequests = 5
};

template<typename LogicalTime, typename LogicalTimeInterval>
class OPENRTI_LOCAL TestAmbassador : public RTI1516TestAmbassador {
public:
  typedef std::pair<LogicalTime, bool> LogicalTimePair;

  TestAmbassador(const RTITest::ConstructorArgs& constructorArgs, unsigned lookahead,
                 TimeAdvanceMode timeAdvanceMode, unsigned numTimesteps, unsigned numInteractions, unsigned numUpdates) :
    RTI1516TestAmbassador(constructorArgs),
    _testTimeAdvanceMode(timeAdvanceMode),
    _numTimesteps(numTimesteps),
    _numInteractions(numInteractions),
    _numUpdates(numUpdates),
    _lookahead(lookahead),
    _timeRegulationEnabled(false),
    _timeConstrainedEnabled(false),
    _timeAdvanceMode(timeAdvanceMode),
    _timeAdvancePending(false),
    _nextMessageTimePending(false),
    _fail(false)
  {
    _lowerBoundSendTime.first.setInitial();
    _lowerBoundSendTime.second = false;
    _lastLowerBoundSendTime = _lowerBoundSendTime;
    _lowerBoundReceiveTime = _lowerBoundSendTime;
    _upperBoundReceiveTime.setFinal();
    setLogicalTimeFactoryName(_logicalTime.implementationName());
  }
  virtual ~TestAmbassador()
    RTI_NOEXCEPT
  { }

  virtual bool execJoined(rti1516::RTIambassador& ambassador)
  {
    _lowerBoundSendTime.first.setInitial();
    _lowerBoundSendTime.second = false;
    _lastLowerBoundSendTime = _lowerBoundSendTime;
    _lowerBoundReceiveTime = _lowerBoundSendTime;
    _upperBoundReceiveTime.setFinal();

    bool enableConstrained = (getRandomNumber() % 2) == 1;
    bool enableRegulation = (getRandomNumber() % 2) == 1;
    bool enableConstrainedPastRegulation = (getRandomNumber() % 2) == 1;
    bool disableRegulationPastConstrained = (getRandomNumber() % 2) == 1;

    // Get some handles
    try {
      _interactionClassHandle0 = ambassador.getInteractionClassHandle(L"InteractionClass0");
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      _class0Parameter0Handle = ambassador.getParameterHandle(_interactionClassHandle0, L"parameter0");
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      _objectClassHandle0 = ambassador.getObjectClassHandle(L"ObjectClass0");
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      _class0Attribute0Handle = ambassador.getAttributeHandle(_objectClassHandle0, L"attribute0");
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      ambassador.queryLogicalTime(_logicalTime);
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    // Enable time regulation
    if (enableConstrainedPastRegulation && enableRegulation && !enableTimeRegulation(ambassador))
      return false;

    // Enable time constrained
    if (enableConstrained && !enableTimeConstrained(ambassador))
      return false;

    // Now that we are constrained, subscribe
    if (_numInteractions) {
      try {
        ambassador.subscribeInteractionClass(_interactionClassHandle0);
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }
    }

    if (_numUpdates) {
      try {
        rti1516::AttributeHandleSet attributeHandleSet;
        attributeHandleSet.insert(_class0Attribute0Handle);
        ambassador.subscribeObjectClassAttributes(_objectClassHandle0, attributeHandleSet);
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }
    }

    // Enable time regulation
    if (!enableConstrainedPastRegulation && enableRegulation && !enableTimeRegulation(ambassador))
      return false;

    // Now that we are regulating, publish
    if (_numInteractions) {
      try {
        ambassador.publishInteractionClass(_interactionClassHandle0);
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }
    }

    if (_numUpdates) {
      try {
        rti1516::AttributeHandleSet attributeHandleSet;
        attributeHandleSet.insert(_class0Attribute0Handle);
        ambassador.publishObjectClassAttributes(_objectClassHandle0, attributeHandleSet);
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      if (!createObjectInstance(ambassador))
        return false;
    }

    // Ok, here time advance and so on
    for (unsigned i = 0; i < _numTimesteps; ++i) {

      // Send some interactions.
      for (unsigned j = 0; j < _numInteractions; ++j) {
        if (!sendTimeStampedInteraction(ambassador, _logicalTime + LogicalTimeInterval(j)))
          return false;
      }

      for (unsigned j = 0; j < _numUpdates; ++j) {
        if (!sendTimeStampedUpdate(ambassador, _logicalTime + LogicalTimeInterval(j)))
          return false;
      }

      // Do the time advance
      try {
        _advanceLogicalTime = _logicalTime;
        _advanceLogicalTime += LogicalTimeInterval(i%10);
        if (_testTimeAdvanceMode == AllTimeAdvanceRequests) {
          // pessimize the flush queue somehow
          if (getRandomNumber() % 10 == 0)
            _timeAdvanceMode = FlushQueueRequest;
          else
            _timeAdvanceMode = TimeAdvanceMode(getRandomNumber() % unsigned(FlushQueueRequest));
        } else {
          _timeAdvanceMode = _testTimeAdvanceMode;
        }

        _lastLowerBoundSendTime = _lowerBoundSendTime;
        _timeAdvancePending = true;
        _nextMessageTimePending = true;
        switch (_timeAdvanceMode) {
        case TimeAdvanceRequest:
          if (_timeRegulationEnabled) {
            _lowerBoundSendTime = std::max(_lowerBoundSendTime, LogicalTimePair(_advanceLogicalTime + _lookahead, _lookahead.isZero()));
            setLBTS(_lowerBoundSendTime);
          }
          if (_timeConstrainedEnabled)
            _upperBoundReceiveTime = _advanceLogicalTime;
          ambassador.timeAdvanceRequest(_advanceLogicalTime);
          break;
        case TimeAdvanceRequestAvailable:
          if (_timeRegulationEnabled) {
            _lowerBoundSendTime = std::max(_lowerBoundSendTime, LogicalTimePair(_advanceLogicalTime + _lookahead, false));
            setLBTS(_lowerBoundSendTime);
          }
          if (_timeConstrainedEnabled)
            _upperBoundReceiveTime = _advanceLogicalTime;
          ambassador.timeAdvanceRequestAvailable(_advanceLogicalTime);
          break;
        case NextMessageRequest:
          if (_timeRegulationEnabled) {
            _lowerBoundSendTime = std::max(_lowerBoundSendTime, LogicalTimePair(_advanceLogicalTime + _lookahead, _lookahead.isZero()));
            setLBTS(_lowerBoundSendTime);
          }
          if (_timeConstrainedEnabled)
            _upperBoundReceiveTime = _advanceLogicalTime;
          ambassador.nextMessageRequest(_advanceLogicalTime);
          break;
        case NextMessageRequestAvailable:
          if (_timeRegulationEnabled) {
            _lowerBoundSendTime = std::max(_lowerBoundSendTime, LogicalTimePair(_advanceLogicalTime + _lookahead, false));
            setLBTS(_lowerBoundSendTime);
          }
          if (_timeConstrainedEnabled)
            _upperBoundReceiveTime = _advanceLogicalTime;
          ambassador.nextMessageRequestAvailable(_advanceLogicalTime);
          break;
        case FlushQueueRequest:
          if (_timeRegulationEnabled) {
            _lowerBoundSendTime = std::max(_lowerBoundSendTime, LogicalTimePair(_advanceLogicalTime + _lookahead, false));
            setLBTS(_lowerBoundSendTime);
          }
          if (_timeConstrainedEnabled)
            _upperBoundReceiveTime.setFinal();
          ambassador.flushQueueRequest(_advanceLogicalTime);
          break;
        default:
          std::wcout << L"Internal test error: cannot time advance on all requests!" << std::endl;
          return false;
        }
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }

      // Send some interactions while in time advance

      for (unsigned j = 0; j < _numInteractions; ++j) {
        if (!sendTimeStampedInteraction(ambassador, _logicalTime + LogicalTimeInterval(j)))
          return false;
      }

      for (unsigned j = 0; j < _numUpdates; ++j) {
        if (!sendTimeStampedUpdate(ambassador, _logicalTime + LogicalTimeInterval(j)))
          return false;
      }

      try {
        Clock timeout = Clock::now() + Clock::fromSeconds(50);
        while (_timeAdvancePending) {
          if (ambassador.evokeCallback(1.0))
            continue;
          if (_fail)
            return false;
          if (timeout < Clock::now()) {
            std::wcout << L"Timeout waiting for next message" << std::endl;
            return false;
          }
        }
      } catch (const rti1516::Exception& e) {
        std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
        return false;
      } catch (...) {
        std::wcout << L"Unknown Exception!" << std::endl;
        return false;
      }
    }

    // Try different logical times for a timestamped delete and ...
    for (unsigned j = 0; j < _numUpdates; ++j) {
      if (!sendTimeStampedDelete(ambassador, _logicalTime + LogicalTimeInterval(j)))
        return false;
    }
    // ... finally delete the object instance
    if (!sendDelete(ambassador))
      return false;

    // Cleanup time management

    // Disable time regulation
    if (!disableRegulationPastConstrained && enableRegulation && !disableTimeRegulation(ambassador))
      return false;

    // Disable time constrained
    if (enableConstrained && !disableTimeConstrained(ambassador))
      return false;

    // Disable time regulation
    if (disableRegulationPastConstrained && enableRegulation && !disableTimeRegulation(ambassador))
      return false;

    return !_fail;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  bool enableTimeConstrained(rti1516::RTIambassador& ambassador)
  {
    // Enable time constrained
    try {
      ambassador.disableTimeConstrained();
      std::wcout << L"disableTimeConstrained does not fail as required!" << std::endl;
      return false;
    } catch (const rti1516::TimeConstrainedIsNotEnabled&) {
      // Ok, must do that
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      _lastLowerBoundSendTime = _lowerBoundSendTime;
      ambassador.enableTimeConstrained();
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      ambassador.enableTimeConstrained();
      std::wcout << L"enableTimeConstrained does not fail as required!" << std::endl;
      return false;
    } catch (const rti1516::RequestForTimeConstrainedPending&) {
      // Ok, must do that
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      Clock timeout = Clock::now() + Clock::fromSeconds(50);
      while (!_timeConstrainedEnabled) {
        if (ambassador.evokeCallback(1.0))
          continue;
        if (_fail)
          return false;
        if (timeout < Clock::now()) {
          std::wcout << L"Timeout waiting for next message" << std::endl;
          return false;
        }
      }

    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      LogicalTime logicalTime;
      ambassador.queryLogicalTime(logicalTime);
      if (logicalTime != _logicalTime) {
        std::wcout << L"Queried logical time does not match the one in timeConstrainedEnabled callback!" << std::endl;
        return false;
      }
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      ambassador.enableTimeConstrained();
      std::wcout << L"enableTimeConstrained does not fail as required!" << std::endl;
      return false;
    } catch (const rti1516::TimeConstrainedAlreadyEnabled&) {
      // Ok, must do that
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    return true;
  }

  bool disableTimeConstrained(rti1516::RTIambassador& ambassador)
  {
    try {
      _lowerBoundReceiveTime.first.setInitial();
      _lowerBoundReceiveTime.second = false;
      _upperBoundReceiveTime.setFinal();
      ambassador.disableTimeConstrained();
      _timeConstrainedEnabled = false;
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }


    try {
      ambassador.disableTimeConstrained();
      std::wcout << L"disableTimeConstrained does not fail as required!" << std::endl;
      return false;
    } catch (const rti1516::TimeConstrainedIsNotEnabled&) {
      // Ok, must do that
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    return true;
  }

  virtual void
  timeConstrainedEnabled(const rti1516::LogicalTime& logicalTime)
    RTI_THROW ((rti1516::InvalidLogicalTime,
           rti1516::NoRequestToEnableTimeConstrainedWasPending,
           rti1516::FederateInternalError))
  {
    if (logicalTime < _logicalTime) {
      std::wcout << L"Time constrained enabled time grants for a time in the past!" << std::endl;
      _fail = true;
    }
    _logicalTime = logicalTime;
    _timeConstrainedEnabled = true;
    _lowerBoundReceiveTime = std::max(_lowerBoundReceiveTime, LogicalTimePair(logicalTime, false));
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  bool enableTimeRegulation(rti1516::RTIambassador& ambassador)
  {
    try {
      ambassador.disableTimeRegulation();
      std::wcout << L"disableTimeRegulation does not fail as required!" << std::endl;
      return false;
    } catch (const rti1516::TimeRegulationIsNotEnabled&) {
      // Ok, must do that
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      _lastLowerBoundSendTime.first.setInitial();
      _lastLowerBoundSendTime.second = false;
      _lowerBoundSendTime = std::max(_lowerBoundSendTime, LogicalTimePair(_logicalTime + _lookahead, false));
      _upperBoundReceiveTime.setFinal();
      ambassador.enableTimeRegulation(_lookahead);
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      ambassador.enableTimeRegulation(_lookahead);
      std::wcout << L"enableTimeRegulation does not fail as required!" << std::endl;
      return false;
    } catch (const rti1516::RequestForTimeRegulationPending&) {
      // Ok, must do that
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      Clock timeout = Clock::now() + Clock::fromSeconds(50);
      while (!_timeRegulationEnabled) {
        if (ambassador.evokeCallback(1.0))
          continue;
        if (_fail)
          return false;
        if (timeout < Clock::now()) {
          std::wcout << L"Timeout waiting for next message" << std::endl;
          return false;
        }
      }

    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      LogicalTime logicalTime;
      ambassador.queryLogicalTime(logicalTime);
      if (logicalTime != _logicalTime) {
        std::wcout << L"Queried logical time does not match the one in timeRegulationEnabled callback!" << std::endl;
        return false;
      }
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      LogicalTimeInterval lookahead;
      ambassador.queryLookahead(lookahead);
      if (lookahead != _lookahead) {
        std::wcout << L"Queried lookahead does not match the one in enableTimeRegulation!" << std::endl;
        return false;
      }
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      ambassador.enableTimeRegulation(_lookahead);
      std::wcout << L"enableTimeRegulation does not fail as required!" << std::endl;
      return false;
    } catch (const rti1516::TimeRegulationAlreadyEnabled&) {
      // Ok, must do that
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    return true;
  }

  bool disableTimeRegulation(rti1516::RTIambassador& ambassador)
  {
    try {
      if (getLogicalTimeFactoryName() == L"HLAinteger64Time")
        clearLBTS();
      ambassador.disableTimeRegulation();
      _timeRegulationEnabled = false;
      _lowerBoundSendTime.first.setInitial();
      _lowerBoundSendTime.second = false;
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    try {
      ambassador.disableTimeRegulation();
      std::wcout << L"disableTimeRegulation does not fail as required!" << std::endl;
      return false;
    } catch (const rti1516::TimeRegulationIsNotEnabled&) {
      // Ok, must do that
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    return true;
  }

  virtual void
  timeRegulationEnabled(const rti1516::LogicalTime& logicalTime)
    RTI_THROW ((rti1516::InvalidLogicalTime,
           rti1516::NoRequestToEnableTimeRegulationWasPending,
           rti1516::FederateInternalError))
  {
    if (logicalTime < _logicalTime) {
      std::wcout << L"Time regulation enabled time grants for a time in the past!" << std::endl;
      _fail = true;
    }
    _lowerBoundSendTime = std::max(_lowerBoundSendTime, LogicalTimePair(LogicalTime(logicalTime) + _lookahead, false));
    setLBTS(_lowerBoundSendTime);
    if (_timeConstrainedEnabled)
      _lowerBoundReceiveTime = std::max(_lowerBoundReceiveTime, LogicalTimePair(logicalTime, false));
    _logicalTime = logicalTime;
    _timeRegulationEnabled = true;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  bool sendTimeStampedInteraction(rti1516::RTIambassador& ambassador, const LogicalTime& logicalTime)
  {
    try {
      rti1516::ParameterHandleValueMap parameterValues;
      rti1516::VariableLengthData tag = toVariableLengthData("withTimestamp");
      parameterValues[_class0Parameter0Handle] = _logicalTime.encode();
      ambassador.sendInteraction(_interactionClassHandle0, parameterValues, tag, logicalTime);

      if (_timeRegulationEnabled) {
        // It's not ok to succeed if we try with an already passed logical time.
        if (LogicalTimePair(logicalTime, false) < _lowerBoundSendTime) {
          std::wcout << L"Accepted logical time in the past for message delivery!" << std::endl;
          return false;
        }
      }

    } catch (const rti1516::InvalidLogicalTime& e) {
      if (_timeRegulationEnabled) {
        // It's required to fail if we try with an already passed logical time.
        if (_lowerBoundSendTime <= LogicalTimePair(logicalTime, false)) {
          std::wcout << L"Not accepted message for logical time " << logicalTime
                     << L" for a lower bound send time (" << _lowerBoundSendTime.first
                     << L", " << _lowerBoundSendTime.second << L")!" << std::endl;
          return false;
        }
      } else {
        // No failure for non regulating federates
        std::wcout << L"rti1516::InvalidLogicalTime: \"" << e.what() << L"\"" << std::endl;
        return false;
      }
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    return true;
  }

  bool createObjectInstance(rti1516::RTIambassador& ambassador)
  {
    if (_objectInstanceHandle.isValid())
      return true;

    try {
      _objectInstanceHandle = ambassador.registerObjectInstance(_objectClassHandle0);
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }
    return true;
  }

  bool sendTimeStampedUpdate(rti1516::RTIambassador& ambassador, const LogicalTime& logicalTime)
  {
    if (!_objectInstanceHandle.isValid())
      return true;

    try {
      rti1516::AttributeHandleValueMap attributeValues;
      rti1516::VariableLengthData tag = toVariableLengthData("withTimestamp");
      attributeValues[_class0Attribute0Handle] = _logicalTime.encode();
      ambassador.updateAttributeValues(_objectInstanceHandle, attributeValues, tag, logicalTime);

      if (_timeRegulationEnabled) {
        // It's not ok to succeed if we try with an already passed logical time.
        if (LogicalTimePair(logicalTime, false) < _lowerBoundSendTime) {
          std::wcout << L"Accepted logical time in the past for message delivery!" << std::endl;
          return false;
        }
      }

    } catch (const rti1516::InvalidLogicalTime& e) {
      if (_timeRegulationEnabled) {
        // It's required to fail if we try with an already passed logical time.
        if (_lowerBoundSendTime <= LogicalTimePair(logicalTime, false)) {
          std::wcout << L"Not accepted message for logical time " << logicalTime
                     << L" for a lower bound send time (" << _lowerBoundSendTime.first
                     << L", " << _lowerBoundSendTime.second << L")!" << std::endl;
          return false;
        }
      } else {
        // No failure for non regulating federates
        std::wcout << L"rti1516::InvalidLogicalTime: \"" << e.what() << L"\"" << std::endl;
        return false;
      }
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    return true;
  }

  bool sendDelete(rti1516::RTIambassador& ambassador)
  {
    if (!_objectInstanceHandle.isValid())
      return true;

    try {
      rti1516::VariableLengthData tag = toVariableLengthData("withoutTimestamp");
      ambassador.deleteObjectInstance(_objectInstanceHandle, tag);
      _objectInstanceHandle = rti1516::ObjectInstanceHandle();

    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    return true;
  }

  bool sendTimeStampedDelete(rti1516::RTIambassador& ambassador, const LogicalTime& logicalTime)
  {
    if (!_objectInstanceHandle.isValid())
      return true;

    try {
      rti1516::VariableLengthData tag = toVariableLengthData("withTimestamp");
      ambassador.deleteObjectInstance(_objectInstanceHandle, tag, logicalTime);
      _objectInstanceHandle = rti1516::ObjectInstanceHandle();

      if (_timeRegulationEnabled) {
        // It's not ok to succeed if we try with an already passed logical time.
        if (LogicalTimePair(logicalTime, false) < _lowerBoundSendTime) {
          std::wcout << L"Accepted logical time in the past for message delivery!" << std::endl;
          return false;
        }
      }

    } catch (const rti1516::InvalidLogicalTime& e) {
      if (_timeRegulationEnabled) {
        // It's required to fail if we try with an already passed logical time.
        if (_lowerBoundSendTime <= LogicalTimePair(logicalTime, false)) {
          std::wcout << L"Not accepted message for logical time " << logicalTime
                     << L" for a lower bound send time (" << _lowerBoundSendTime.first
                     << L", " << _lowerBoundSendTime.second << L")!" << std::endl;
          return false;
        }
      } else {
        // No failure for non regulating federates
        std::wcout << L"rti1516::InvalidLogicalTime: \"" << e.what() << L"\"" << std::endl;
        return false;
      }
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return false;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return false;
    }

    return true;
  }

  virtual void
  timeAdvanceGrant(const rti1516::LogicalTime& logicalTime)
    RTI_THROW ((rti1516::InvalidLogicalTime,
           rti1516::JoinedFederateIsNotInTimeAdvancingState,
           rti1516::FederateInternalError))
  {
    if (!_timeAdvancePending) {
      std::wcout << L"Time advance grant without time advance pending!" << std::endl;
      _fail = true;
    }
    if (logicalTime < _logicalTime) {
      std::wcout << L"Time advance grant time grants for a time in the past!" << std::endl;
      _fail = true;
    }

    _logicalTime = logicalTime;
    _timeAdvancePending = false;

    switch (_timeAdvanceMode) {
    case TimeAdvanceRequest:
    case NextMessageRequest:
      if (_timeRegulationEnabled)
        _lowerBoundSendTime = std::max(_lastLowerBoundSendTime, LogicalTimePair(_logicalTime + _lookahead, _lookahead.isZero()));
      if (_timeConstrainedEnabled)
        _lowerBoundReceiveTime = std::max(_lowerBoundReceiveTime, LogicalTimePair(_logicalTime, true));
      break;
    case TimeAdvanceRequestAvailable:
    case NextMessageRequestAvailable:
    case FlushQueueRequest:
      if (_timeRegulationEnabled)
        _lowerBoundSendTime = std::max(_lastLowerBoundSendTime, LogicalTimePair(_logicalTime + _lookahead, false));
      if (_timeConstrainedEnabled)
        _lowerBoundReceiveTime = std::max(_lowerBoundReceiveTime, LogicalTimePair(_logicalTime, false));
      break;
    }

    switch (_timeAdvanceMode) {
    case NextMessageRequest:
    case NextMessageRequestAvailable:
    case FlushQueueRequest:
      if (!_nextMessageTimePending) {
        if (_timeAdvanceMode != FlushQueueRequest) {
          if (_logicalTime != _nextMessageTime) {
            std::wcout << L"Time advance grant time grants for a time not equal to the next message time!" << std::endl;
            _fail = true;
          }
        } else {
          if (_nextMessageTime < _logicalTime) {
            std::wcout << L"Time advance grant time grants for a time that exceeds the next message time!" << std::endl;
            _fail = true;
          }
        }
      }
      if (_advanceLogicalTime < _logicalTime) {
        std::wcout << L"Time advance grant time grants for a time beyond the requested advance time!" << std::endl;
        _fail = true;
      }
      break;
    case TimeAdvanceRequest:
    case TimeAdvanceRequestAvailable:
      if (_logicalTime != _advanceLogicalTime) {
        std::wcout << L"Time advance grant time grants for a time not equal to the requested advance time!" << std::endl;
        _fail = true;
      }
      break;
    }

    if (getLogicalTimeFactoryName() == L"HLAinteger64Time") {
      if (_timeConstrainedEnabled) {
        unsigned commonStamp = getLBTS();
        if (_lowerBoundReceiveTime.second) {
          if (commonStamp <= _lowerBoundReceiveTime.first.getTime()) {
            std::wcout << L"Time advance grant too early: lbts: " << commonStamp
                       << L", our time: " << _logicalTime.getTime() << std::endl;
            _fail = true;
          }
        } else {
          if (commonStamp < _lowerBoundReceiveTime.first.getTime()) {
            std::wcout << L"Time advance grant too early: lbts: " << commonStamp
                       << L", our time: " << _logicalTime.getTime() << std::endl;
            _fail = true;
          }
        }
      }
    }
  }

  virtual void discoverObjectInstance(rti1516::ObjectInstanceHandle, rti1516::ObjectClassHandle, const std::wstring&)
    RTI_THROW ((rti1516::CouldNotDiscover,
           rti1516::ObjectClassNotKnown,
           rti1516::FederateInternalError))
  {
  }

  virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle, const rti1516::AttributeHandleValueMap&,
                                      const rti1516::VariableLengthData& tag, rti1516::OrderType sentOrder, rti1516::TransportationType)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotSubscribed,
           rti1516::FederateInternalError))
  {
    if (_timeConstrainedEnabled && strncmp("withoutTimestamp", (const char*)tag.data(), tag.size()) != 0) {
        _fail = true;
        std::wcout << L"Got timestamp order message that was received as receive order!" << std::endl;
    }
    if (sentOrder != rti1516::RECEIVE) {
        _fail = true;
        std::wcout << L"Got receive order message that was received as timestamp order!" << std::endl;
    }
  }

  virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle objectInstanceHandle, const rti1516::AttributeHandleValueMap& attributeHandleValueMap,
                                      const rti1516::VariableLengthData& tag, rti1516::OrderType orderType, rti1516::TransportationType transportationType,
                                      const rti1516::RegionHandleSet&)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotSubscribed,
           rti1516::FederateInternalError))
  {
    reflectAttributeValues(objectInstanceHandle, attributeHandleValueMap, tag, orderType, transportationType);
  }

  virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle objectInstanceHandle, const rti1516::AttributeHandleValueMap& attributeHandleValueMap,
                                      const rti1516::VariableLengthData& tag, rti1516::OrderType sentOrder, rti1516::TransportationType transportationType,
                                      const rti1516::LogicalTime& logicalTime, rti1516::OrderType receivedOrder)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotSubscribed,
           rti1516::FederateInternalError))
  {
    if (strncmp("withTimestamp", (const char*)tag.data(), tag.size()) != 0) {
        _fail = true;
        std::wcout << L"Got receive order message that was received as timestamp order!" << std::endl;
    }
    if (receivedOrder == rti1516::TIMESTAMP && sentOrder != rti1516::TIMESTAMP) {
        _fail = true;
        std::wcout << L"Received timestamp order message that was sent as receive order!" << std::endl;
    }
    if (receivedOrder == rti1516::TIMESTAMP) {
      if (_timeConstrainedEnabled) {
        checkLogicalMessageTime(logicalTime);
      } else {
        _fail = true;
        std::wcout << L"Received timestamp order message while time constrained disabled!" << std::endl;
      }
    }
  }

  virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle objectInstanceHandle, const rti1516::AttributeHandleValueMap& attributeHandleValueMap,
                                      const rti1516::VariableLengthData& tag, rti1516::OrderType orderType, rti1516::TransportationType transportationType,
                                      const rti1516::LogicalTime& logicalTime, rti1516::OrderType receivedOrder, const rti1516::RegionHandleSet&)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotSubscribed,
           rti1516::FederateInternalError))
  {
    reflectAttributeValues(objectInstanceHandle, attributeHandleValueMap, tag, orderType, transportationType, logicalTime, receivedOrder);
  }

  virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle objectInstanceHandle, const rti1516::AttributeHandleValueMap& attributeHandleValueMap,
                                      const rti1516::VariableLengthData& tag, rti1516::OrderType sentOrder, rti1516::TransportationType transportationType,
                                      const rti1516::LogicalTime& logicalTime, rti1516::OrderType receivedOrder, rti1516::MessageRetractionHandle)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotSubscribed,
           rti1516::InvalidLogicalTime,
           rti1516::FederateInternalError))
  {
    if (strncmp("withTimestamp", (const char*)tag.data(), tag.size()) != 0) {
        _fail = true;
        std::wcout << L"Got receive order message over timestamped delivery!" << std::endl;
    }
    if (receivedOrder == rti1516::TIMESTAMP && sentOrder != rti1516::TIMESTAMP) {
        _fail = true;
        std::wcout << L"Received timestamp order message that was sent as receive order!" << std::endl;
    }
    if (receivedOrder == rti1516::TIMESTAMP) {
      if (_timeConstrainedEnabled) {
        checkLogicalMessageTime(logicalTime);
      } else {
        _fail = true;
        std::wcout << L"Received timestamp order message while time constrained disabled!" << std::endl;
      }
    }
  }

  virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle objectInstanceHandle, const rti1516::AttributeHandleValueMap& attributeHandleValueMap,
                                      const rti1516::VariableLengthData& tag, rti1516::OrderType orderType, rti1516::TransportationType transportationType,
                                      const rti1516::LogicalTime& logicalTime, rti1516::OrderType receivedOrder, rti1516::MessageRetractionHandle,
                                      const rti1516::RegionHandleSet& regionHandleSet)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotRecognized,
           rti1516::AttributeNotSubscribed,
           rti1516::InvalidLogicalTime,
           rti1516::FederateInternalError))
  {
    reflectAttributeValues(objectInstanceHandle, attributeHandleValueMap, tag, orderType, transportationType, logicalTime, receivedOrder, regionHandleSet);
  }

  virtual void
  receiveInteraction(rti1516::InteractionClassHandle, const rti1516::ParameterHandleValueMap&,
                     const rti1516::VariableLengthData& tag, rti1516::OrderType sentOrder, rti1516::TransportationType)
    RTI_THROW ((rti1516::InteractionClassNotRecognized,
           rti1516::InteractionParameterNotRecognized,
           rti1516::InteractionClassNotSubscribed,
           rti1516::FederateInternalError))
  {
    if (_timeConstrainedEnabled && strncmp("withoutTimestamp", (const char*)tag.data(), tag.size()) != 0) {
        _fail = true;
        std::wcout << L"Got timestamp order message that was received as receive order!" << std::endl;
    }
    if (sentOrder != rti1516::RECEIVE) {
        _fail = true;
        std::wcout << L"Got receive order message that was received as timestamp order!" << std::endl;
    }
  }

  virtual void
  receiveInteraction(rti1516::InteractionClassHandle theInteraction,
                     rti1516::ParameterHandleValueMap const & theParameterValues,
                     rti1516::VariableLengthData const & tag,
                     rti1516::OrderType sentOrder,
                     rti1516::TransportationType theType,
                     rti1516::LogicalTime const & theTime,
                     rti1516::OrderType receivedOrder)
    RTI_THROW ((rti1516::InteractionClassNotRecognized,
           rti1516::InteractionParameterNotRecognized,
           rti1516::InteractionClassNotSubscribed,
           rti1516::FederateInternalError))
  {
    if (strncmp("withTimestamp", (const char*)tag.data(), tag.size()) != 0) {
        _fail = true;
        std::wcout << L"Got receive order message that was received as timestamp order!" << std::endl;
    }
    if (receivedOrder == rti1516::TIMESTAMP && sentOrder != rti1516::TIMESTAMP) {
        _fail = true;
        std::wcout << L"Received timestamp order message that was sent as receive order!" << std::endl;
    }
    if (receivedOrder == rti1516::TIMESTAMP) {
      if (_timeConstrainedEnabled) {
        checkLogicalMessageTime(theTime);
      } else {
        _fail = true;
        std::wcout << L"Received timestamp order message while time constrained disabled!" << std::endl;
      }
    }
  }

  virtual void
  receiveInteraction(rti1516::InteractionClassHandle theInteraction,
                     rti1516::ParameterHandleValueMap const & theParameterValues,
                     rti1516::VariableLengthData const & tag,
                     rti1516::OrderType sentOrder,
                     rti1516::TransportationType theType,
                     rti1516::LogicalTime const & theTime,
                     rti1516::OrderType receivedOrder,
                     rti1516::MessageRetractionHandle theHandle)
    RTI_THROW ((rti1516::InteractionClassNotRecognized,
           rti1516::InteractionParameterNotRecognized,
           rti1516::InteractionClassNotSubscribed,
           rti1516::InvalidLogicalTime,
           rti1516::FederateInternalError))
  {
    if (strncmp("withTimestamp", (const char*)tag.data(), tag.size()) != 0) {
        _fail = true;
        std::wcout << L"Got receive order message over timestamped delivery!" << std::endl;
    }
    if (receivedOrder == rti1516::TIMESTAMP && sentOrder != rti1516::TIMESTAMP) {
        _fail = true;
        std::wcout << L"Received timestamp order message that was sent as receive order!" << std::endl;
    }
    if (receivedOrder == rti1516::TIMESTAMP) {
      if (_timeConstrainedEnabled) {
        checkLogicalMessageTime(theTime);
      } else {
        _fail = true;
        std::wcout << L"Received timestamp order message while time constrained disabled!" << std::endl;
      }
    }
  }

  virtual void removeObjectInstance(rti1516::ObjectInstanceHandle theObject,
                                    rti1516::VariableLengthData const & tag,
                                    rti1516::OrderType sentOrder)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::FederateInternalError))
  {
    if (_timeConstrainedEnabled && strncmp("withoutTimestamp", (const char*)tag.data(), tag.size()) != 0) {
        _fail = true;
        std::wcout << L"Got timestamp order message that was received as receive order!" << std::endl;
    }
    if (sentOrder != rti1516::RECEIVE) {
        _fail = true;
        std::wcout << L"Got receive order message that was received as timestamp order!" << std::endl;
    }
  }

  virtual void removeObjectInstance(rti1516::ObjectInstanceHandle theObject,
                                    rti1516::VariableLengthData const & tag,
                                    rti1516::OrderType sentOrder,
                                    rti1516::LogicalTime const & logicalTime,
                                    rti1516::OrderType receivedOrder)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::FederateInternalError))
  {
    if (strncmp("withTimestamp", (const char*)tag.data(), tag.size()) != 0) {
        _fail = true;
        std::wcout << L"Got receive order message that was received as timestamp order!" << std::endl;
    }
    if (receivedOrder == rti1516::TIMESTAMP && sentOrder != rti1516::TIMESTAMP) {
        _fail = true;
        std::wcout << L"Received timestamp order message that was sent as receive order!" << std::endl;
    }
    if (receivedOrder == rti1516::TIMESTAMP) {
      if (_timeConstrainedEnabled) {
        checkLogicalMessageTime(logicalTime);
      } else {
        _fail = true;
        std::wcout << L"Received timestamp order message while time constrained disabled!" << std::endl;
      }
    }
  }

  virtual void removeObjectInstance(rti1516::ObjectInstanceHandle theObject,
                                    rti1516::VariableLengthData const & tag,
                                    rti1516::OrderType sentOrder,
                                    rti1516::LogicalTime const & logicalTime,
                                    rti1516::OrderType receivedOrder,
                                    rti1516::MessageRetractionHandle theHandle)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::InvalidLogicalTime,
           rti1516::FederateInternalError))
  {
    if (strncmp("withTimestamp", (const char*)tag.data(), tag.size()) != 0) {
        _fail = true;
        std::wcout << L"Got receive order message over timestamped delivery!" << std::endl;
    }
    if (receivedOrder == rti1516::TIMESTAMP && sentOrder != rti1516::TIMESTAMP) {
        _fail = true;
        std::wcout << L"Received timestamp order message that was sent as receive order!" << std::endl;
    }
    if (receivedOrder == rti1516::TIMESTAMP) {
      if (_timeConstrainedEnabled) {
        checkLogicalMessageTime(logicalTime);
      } else {
        _fail = true;
        std::wcout << L"Received timestamp order message while time constrained disabled!" << std::endl;
      }
    }
  }

  void checkLogicalMessageTime(rti1516::LogicalTime const & logicalTime)
  {
    if (LogicalTimePair(logicalTime, false) < _lowerBoundReceiveTime) {
      _fail = true;
      std::wcout << L"Received timestamp order message with timestamp in the past: [("
                 << _lowerBoundReceiveTime.first << L", " << _lowerBoundReceiveTime.second
                 << L"), " << _upperBoundReceiveTime << L"] " << logicalTime << std::endl;
    }
    if (_upperBoundReceiveTime < logicalTime) {
      _fail = true;
      std::wcout << L"Received timestamp order message with timestamp in the future: [("
                 << _lowerBoundReceiveTime.first << L", " << _lowerBoundReceiveTime.second
                 << L"), " << _upperBoundReceiveTime << L"] " << logicalTime << std::endl;
    }

    if (_timeAdvancePending) {
      switch (_timeAdvanceMode) {
      case NextMessageRequest:
      case NextMessageRequestAvailable:
        // If in next message mode, we want only this first single messages
        if (_nextMessageTimePending) {
          _nextMessageTime = logicalTime;
          _nextMessageTimePending = false;
        } else {
          if (!(_nextMessageTime == logicalTime)) {
            std::wcout << L"Received timestamp order message in next message mode with timestamp "
                       << logicalTime << L" different than " << _nextMessageTime << L"!" << std::endl;
            _fail = true;
          }
        }
        break;
      case FlushQueueRequest:
        // The rationale paragraph E8.12 explaines that we expect the time advance
        // only up to the next delivered message in flush queue mode.
        // The argument is there that the federate might want to send a response to
        // this first message at the next possible time.
        if (logicalTime < _nextMessageTime && logicalTime <= _advanceLogicalTime) {
          _nextMessageTime = logicalTime;
          _nextMessageTimePending = false;
        }
        break;
      case TimeAdvanceRequest:
      case TimeAdvanceRequestAvailable:
        break;
      }
    }
  }

  void setLBTS(const LogicalTimePair& logicalTimePair)
  {
    if (getLogicalTimeFactoryName() != L"HLAinteger64Time")
      return;
    if (logicalTimePair.second) {
      // smallest allowed message is logical time + 1
      RTI1516TestAmbassador::setLBTS(unsigned(logicalTimePair.first.getTime() + 1));
    } else {
      // smallest allowed message is logical time
      RTI1516TestAmbassador::setLBTS(unsigned(logicalTimePair.first.getTime()));
    }
  }

private:
  TimeAdvanceMode _testTimeAdvanceMode;
  unsigned _numTimesteps;
  unsigned _numInteractions;
  unsigned _numUpdates;
  LogicalTimeInterval _lookahead;

  rti1516::InteractionClassHandle _interactionClassHandle0;
  rti1516::ParameterHandle _class0Parameter0Handle;

  rti1516::ObjectClassHandle _objectClassHandle0;
  rti1516::AttributeHandle _class0Attribute0Handle;

  rti1516::ObjectInstanceHandle _objectInstanceHandle;

  LogicalTime _logicalTime;

  bool _timeRegulationEnabled;
  LogicalTimePair _lowerBoundSendTime;
  LogicalTimePair _lastLowerBoundSendTime;

  bool _timeConstrainedEnabled;
  LogicalTimePair _lowerBoundReceiveTime;
  LogicalTime _upperBoundReceiveTime;

  TimeAdvanceMode _timeAdvanceMode;
  bool _timeAdvancePending;
  LogicalTime _advanceLogicalTime;
  bool _nextMessageTimePending;
  LogicalTime _nextMessageTime;

  bool _fail;
};

class OPENRTI_LOCAL Test : public RTITest {
public:
  Test(int argc, const char* const argv[]) :
    RTITest(argc, argv, false),
    _float(false),
    _lookahead(1),
    _timeAdvanceMode(AllTimeAdvanceRequests),
    _numTimesteps(100),
    _numInteractions(4),
    _numUpdates(4)
  {
    insertOptionString("fI:L:N:T:U:");
  }

  virtual bool processOption(char optchar, const std::string& argument)
  {
    switch (optchar) {
    case 'f':
      _float = true;
      return true;
    case 'I':
      _numInteractions = atoi(argument.c_str());
      return true;
    case 'L':
      _lookahead = atoi(argument.c_str());
      return true;
    case 'N':
      _timeAdvanceMode = TimeAdvanceMode(atoi(argument.c_str()));
      switch (_timeAdvanceMode) {
      case TimeAdvanceRequest:
      case TimeAdvanceRequestAvailable:
      case NextMessageRequest:
      case NextMessageRequestAvailable:
      case FlushQueueRequest:
      case AllTimeAdvanceRequests:
        return true;
      default:
        return false;
      }
    case 'T':
      _numTimesteps = atoi(argument.c_str());
      return true;
    case 'U':
      _numUpdates = atoi(argument.c_str());
      return true;
    default:
      return RTITest::processOption(optchar, argument);
    }
  }

  virtual Ambassador* createAmbassador(const ConstructorArgs& constructorArgs)
  {
    if (_float)
      return new TestAmbassador<HLAfloat64Time, HLAfloat64Interval>(constructorArgs, _lookahead, _timeAdvanceMode, _numTimesteps, _numInteractions, _numUpdates);
    else
      return new TestAmbassador<HLAinteger64Time, HLAinteger64Interval>(constructorArgs, _lookahead, _timeAdvanceMode, _numTimesteps, _numInteractions, _numUpdates);
  }

private:
  bool _float;
  unsigned _lookahead;
  TimeAdvanceMode _timeAdvanceMode;
  unsigned _numTimesteps;
  unsigned _numInteractions;
  unsigned _numUpdates;
};

}

int
main(int argc, char* argv[])
{
  OpenRTI::Test test(argc, argv);
  return test.exec();
}
