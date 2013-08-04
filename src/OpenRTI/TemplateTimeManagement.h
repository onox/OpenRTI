/* -*-c++-*- OpenRTI - Copyright (C) 2009-2013 Mathias Froehlich
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

#ifndef OpenRTI_TemplateTimeManagement_h
#define OpenRTI_TemplateTimeManagement_h

#include "TimeManagement.h"

#include "FederateHandleLowerBoundTimeStampMap.h"

namespace OpenRTI {

template<typename T>
class Ambassador;

template<typename T, typename L>
class TemplateTimeManagement : public TimeManagement<T> {
public:
  typedef T Traits;

  typedef typename Traits::NativeLogicalTime NativeLogicalTime;
  typedef typename Traits::NativeLogicalTimeInterval NativeLogicalTimeInterval;

  typedef L LogicalTimeFactory;
  typedef typename LogicalTimeFactory::LogicalTime LogicalTime;
  typedef typename LogicalTimeFactory::LogicalTimeInterval LogicalTimeInterval;

  // Do the maps with keys like std::pair<LogicalTime, bool>, where the unsigned is 0 when comparing with
  // strictly less and 1 when comparing with less equal. FIXME does this work??
  typedef std::pair<LogicalTime, bool> LogicalTimePair;

  using TimeManagement<T>::_timeRegulationEnablePending;
  using TimeManagement<T>::_timeRegulationEnabled;
  using TimeManagement<T>::_timeConstrainedEnablePending;
  using TimeManagement<T>::_timeConstrainedEnabled;
  using TimeManagement<T>::_timeAdvancePending;
  using TimeManagement<T>::_nextMessageMode;
  using TimeManagement<T>::_flushQueueMode;

  TemplateTimeManagement(const LogicalTimeFactory& logicalTimeFactory) :
    _logicalTimeFactory(logicalTimeFactory)
  {
    _logicalTime = _logicalTimeFactory.initialLogicalTime();
    _pendingLogicalTime.first = _logicalTime;
    _pendingLogicalTime.second = true;
    _localLowerBoundTimeStamp.first = _logicalTime;
    _localLowerBoundTimeStamp.second = true;
    _currentLookahead = _logicalTimeFactory.zeroLogicalTimeInterval();
    _targetLookahead = _currentLookahead;
  }
  virtual ~TemplateTimeManagement()
  { }

  virtual bool isLogicalTimeInThePast(const NativeLogicalTime& logicalTime)
  {
    return logicalTime <= _logicalTimeFactory.getLogicalTime(_logicalTime);
  }
  virtual bool isLogicalTimeStrictlyInThePast(const NativeLogicalTime& logicalTime)
  {
    return logicalTime < _logicalTimeFactory.getLogicalTime(_logicalTime);
  }
  virtual bool logicalTimeAlreadyPassed(const NativeLogicalTime& logicalTime)
  {
    if (_localLowerBoundTimeStamp.second) {
      return logicalTime <= _logicalTimeFactory.getLogicalTime(_localLowerBoundTimeStamp.first);
    } else {
      return logicalTime < _logicalTimeFactory.getLogicalTime(_localLowerBoundTimeStamp.first);
    }
  }



  virtual void enableTimeRegulation(Ambassador<T>& ambassador, const NativeLogicalTimeInterval& nativeLookahead)
  {
    _enableTimeRegulation(ambassador, _logicalTimeFactory.getLogicalTime(_logicalTime), nativeLookahead);
  }

  // the RTI13 variant
  virtual void enableTimeRegulation(Ambassador<T>& ambassador, const NativeLogicalTime& nativeLogicalTime, const NativeLogicalTimeInterval& nativeLookahead)
  {
    _enableTimeRegulation(ambassador, nativeLogicalTime, nativeLookahead);
  }

  void _enableTimeRegulation(Ambassador<T>& ambassador, const NativeLogicalTime& nativeLogicalTime, const NativeLogicalTimeInterval& nativeLookahead)
  {
    _timeRegulationEnablePending = true;

    _currentLookahead = _logicalTimeFactory.getLogicalTimeInterval(nativeLookahead);
    _targetLookahead = _currentLookahead;

    _pendingLogicalTime.first = _logicalTimeFactory.getLogicalTime(nativeLogicalTime);
    _pendingLogicalTime.second = _logicalTimeFactory.isZeroTimeInterval(_currentLookahead);

    _localLowerBoundTimeStamp = _pendingLogicalTime;
    _localLowerBoundTimeStamp.first += _currentLookahead;

    _timeRegulationEnableFederateHandleSet = ambassador.getFederate()->getFederateHandleSet();
    // Make sure we wait for the request looping back through the root server.
    // We need to do that round trip to the root server to stay in order with newly
    // joined federates that are serialized by the root server.
    _timeRegulationEnableFederateHandleSet.insert(ambassador.getFederate()->getFederateHandle());

    SharedPtr<EnableTimeRegulationRequestMessage> request;
    request = new EnableTimeRegulationRequestMessage;
    request->setFederationHandle(ambassador.getFederate()->getFederationHandle());
    request->setFederateHandle(ambassador.getFederate()->getFederateHandle());
    request->getTimeStamp().setLogicalTime(_logicalTimeFactory.encodeLogicalTime(_localLowerBoundTimeStamp.first));
    request->getTimeStamp().setZeroLookahead(_localLowerBoundTimeStamp.second);
    ambassador.send(request);
  }

  virtual void disableTimeRegulation(Ambassador<T>& ambassador)
  {
    _timeRegulationEnabled = false;

    SharedPtr<DisableTimeRegulationRequestMessage> request;
    request = new DisableTimeRegulationRequestMessage;
    request->setFederationHandle(ambassador.getFederate()->getFederationHandle());
    request->setFederateHandle(ambassador.getFederate()->getFederateHandle());
    ambassador.send(request);
  }

  virtual void enableTimeConstrained(Ambassador<T>& ambassador)
  {
    _timeConstrainedEnablePending = true;

    // If we wait for getting time constrained we need to wait until the federations lower bound
    // passes our logical time. Unleach this if we get beyond.
    if (_federateLowerBoundMap.canAdvanceTo(_logicalTime))
      queueTimeConstrainedEnabled(ambassador, _logicalTime);
  }

  virtual void disableTimeConstrained(Ambassador<T>& ambassador)
  {
    _timeConstrainedEnabled = false;
    // If we are in time advance pending, we are now able to advance immediately
    if (_timeAdvancePending)
      queueTimeAdvanceGranted(ambassador, _pendingLogicalTime.first);
    for (typename LogicalTimeMessageListMap::iterator i = _logicalTimeMessageListMap.begin();
         i != _logicalTimeMessageListMap.end();) {
      ambassador.queueCallbacks(i->second);
      _logicalTimeMessageListMap.erase(i++);
    }
  }

  virtual void timeAdvanceRequest(InternalAmbassador& ambassador, const NativeLogicalTime& nativeLogicalTime, bool availableMode, bool nextMessageMode)
  {
    LogicalTime logicalTime = _logicalTimeFactory.getLogicalTime(nativeLogicalTime);
    _nextMessageMode = nextMessageMode;
    _timeAdvancePending = true;
    _pendingLogicalTime.first = logicalTime;
    _pendingLogicalTime.second = !availableMode;

    // If we need to advance to match the new requested lookahead, try to increase that one
    if (_targetLookahead < _currentLookahead) {
      // Check if we would violate a previously given promise about our lower bound time stamp
      LogicalTimePair logicalTimePair(_logicalTime, _logicalTimeFactory.isZeroTimeInterval(_targetLookahead));
      logicalTimePair.first += _targetLookahead;
      if (logicalTimePair < _localLowerBoundTimeStamp) {
        _currentLookahead = _localLowerBoundTimeStamp.first - logicalTime;
      } else {
        _currentLookahead = _targetLookahead;
      }
    }

    _localLowerBoundTimeStamp.first = logicalTime;
    if (_logicalTimeFactory.isZeroTimeInterval(_currentLookahead)) {
      _localLowerBoundTimeStamp.second = true;
    } else {
      _localLowerBoundTimeStamp.first += _currentLookahead;
      _localLowerBoundTimeStamp.second = false;
    }

    if (_timeRegulationEnabled)
      sendCommitLowerBoundTimeStamp(ambassador, _localLowerBoundTimeStamp);

    if (_timeConstrainedEnabled) {
      // Case, time advance pending
      if (_nextMessageMode && !_logicalTimeMessageListMap.empty()) {
        _pendingLogicalTime.first = std::min(_pendingLogicalTime.first, _logicalTimeMessageListMap.begin()->first);
      }
      if (_federateLowerBoundMap.canAdvanceTo(_pendingLogicalTime)) {
        // Ok, nobody holding us back
        queueTimeAdvanceGranted(ambassador, _pendingLogicalTime.first);
      }
    } else {
      // If we are not time constrained, just schedule the time advance
      queueTimeAdvanceGranted(ambassador, _pendingLogicalTime.first);
    }
  }

  virtual void flushQueueRequest(Ambassador<T>& ambassador, const NativeLogicalTime& nativeLogicalTime)
  {
    Traits::throwRTIinternalError("FQR not implemented");

    LogicalTime logicalTime = _logicalTimeFactory.getLogicalTime(nativeLogicalTime);
    _timeAdvancePending = true;
    _flushQueueMode = true;

    for (typename LogicalTimeMessageListMap::iterator i = _logicalTimeMessageListMap.begin(); i != _logicalTimeMessageListMap.end();) {
      ambassador.queueCallbacks(i->second);
      _logicalTimeMessageListMap.erase(i++);
    }
  }

  virtual bool queryGALT(Ambassador<T>& ambassador, NativeLogicalTime& logicalTime)
  {
    if (_federateLowerBoundMap.empty())
      return false;
    logicalTime = _logicalTimeFactory.getLogicalTime(_federateLowerBoundMap.getGALT());
    return true;
  }

  virtual void queryLogicalTime(Ambassador<T>& ambassador, NativeLogicalTime& logicalTime)
  {
    logicalTime = _logicalTimeFactory.getLogicalTime(_logicalTime);
  }

  virtual bool queryLITS(Ambassador<T>& ambassador, NativeLogicalTime& logicalTime)
  {
    if (_logicalTimeMessageListMap.empty()) {
      if (_federateLowerBoundMap.empty()) {
        return false;
      } else {
        logicalTime = _logicalTimeFactory.getLogicalTime(_federateLowerBoundMap.getGALT());
        return true;
      }
    } else {
      if (_federateLowerBoundMap.empty()) {
        logicalTime = _logicalTimeFactory.getLogicalTime(_logicalTimeMessageListMap.begin()->first);
        return true;
      } else {
        if (_federateLowerBoundMap.getGALT() < _logicalTimeMessageListMap.begin()->first)
          logicalTime = _logicalTimeFactory.getLogicalTime(_federateLowerBoundMap.getGALT());
        else
          logicalTime = _logicalTimeFactory.getLogicalTime(_logicalTimeMessageListMap.begin()->first);
        return true;
      }
    }
  }

  virtual void modifyLookahead(Ambassador<T>& ambassador, const NativeLogicalTimeInterval& nativeLookahead)
  {
    LogicalTimeInterval lookahead = _logicalTimeFactory.getLogicalTimeInterval(nativeLookahead);
    _targetLookahead = lookahead;
    if (_currentLookahead < lookahead) {
      _currentLookahead = lookahead;

      // Now tell the other federates about or now message lower bound timestamp
      _localLowerBoundTimeStamp.first = _logicalTime;
      if (_logicalTimeFactory.isZeroTimeInterval(_currentLookahead)) {
        _localLowerBoundTimeStamp.second = true;
      } else {
        _localLowerBoundTimeStamp.first += _currentLookahead;
        _localLowerBoundTimeStamp.second = false;
      }

      sendCommitLowerBoundTimeStamp(ambassador, _localLowerBoundTimeStamp);
    }
  }

  virtual void queryLookahead(Ambassador<T>& ambassador, NativeLogicalTimeInterval& logicalTimeInterval)
  {
    logicalTimeInterval = _logicalTimeFactory.getLogicalTimeInterval(_currentLookahead);
  }

  virtual std::string logicalTimeToString(const NativeLogicalTime& nativeLogicalTime)
  {
    return _logicalTimeFactory.toString(nativeLogicalTime);
  }
  virtual std::string logicalTimeIntervalToString(const NativeLogicalTimeInterval& nativeLogicalTimeInterval)
  {
    return _logicalTimeFactory.toString(nativeLogicalTimeInterval);
  }
  virtual bool isPositiveLogicalTimeInterval(const NativeLogicalTimeInterval& nativeLogicalTimeInterval)
  {
    return _logicalTimeFactory.isPositiveTimeInterval(nativeLogicalTimeInterval);
  }
  virtual VariableLengthData encodeLogicalTime(const NativeLogicalTime& nativeLogicalTime)
  {
    return _logicalTimeFactory.encodeLogicalTime(nativeLogicalTime);
  }

  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const JoinFederateNotifyMessage& message)
  {
    // Ok, this is tricky. As long as we are waiting for the root servers reply, we need to assume that the
    // time regulation request hits the root server when this federate is already joined, which means it will
    // get the request and respond to us with a reply we need to wait for.
    // When we have already recieved the root servers response, the new federate just gets the information that we are
    // already time regulating - in which case we do not get a response from that federate.
    if (_timeRegulationEnableFederateHandleSet.count(ambassador.getFederate()->getFederateHandle())) {
      _timeRegulationEnableFederateHandleSet.insert(message.getFederateHandle());
    }
  }
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const ResignFederateNotifyMessage& message)
  {
    removeFederateFromTimeManagement(ambassador, message.getFederateHandle());
  }

  // This message is sent from federates that wish to get time regulating.
  // The federate sends its current logical time in the request.
  // If we are time constraind, we rely on the guarantee that we do not receive messages
  // from the past. This means if the requesting federate would be able to send messages
  // from the past, we need to tell that federate that it has to adjust its logical time
  // to match already established logical time guarantees.
  // Also this current federate needs to know that we will not advance further than this
  // new regulating federate is. So this call in effect establishes the logical time
  // entry in the map containing the time regulating federates and the timestamps.
  // Once the new federate has collected all its knowledge, it might commit a later time.
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const EnableTimeRegulationRequestMessage& message)
  {
    if (ambassador.getFederate()->getFederateHandle() == message.getFederateHandle()) {
      // This is our own request looping back to ourself.
      // The root server started the broadcast. This way we are in order with newly
      // joined federates who either reply with a response if and only if they joined the
      // root server before the enable time regulation request broadcast was started from
      // the root server due to our request.
      // The responses juse take the direct route back.

      if (!_timeRegulationEnablePending) {
        Log(Network, Warning) << "Recieved own EnableTimeRegulationRequestMessage without waiting for that to happen!" << std::endl;
        return;
      }
      _timeRegulationEnableFederateHandleSet.erase(ambassador.getFederate()->getFederateHandle());

      // This one checks if we are the last one this ambassador is waiting for
      // and if so, queues the callback and informs the other federates about our
      // new logical time
      checkTimeRegulationEnabled(ambassador);

    } else {
      LogicalTime logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getTimeStamp().getLogicalTime());
      bool zeroLookahead = message.getTimeStamp().getZeroLookahead(); /* FIXME ?? */
      LogicalTimePair logicalTimePair(logicalTime, zeroLookahead);

      // If we are in the state of a fresh joined federate which is still collecting initial information
      // we should skip sending the response.
      if (ambassador.getFederate()->getFederateHandle().valid()) {
        SharedPtr<EnableTimeRegulationResponseMessage> response;
        response = new EnableTimeRegulationResponseMessage;
        response->setFederationHandle(ambassador.getFederate()->getFederationHandle());
        response->setFederateHandle(message.getFederateHandle());
        response->setRespondingFederateHandle(ambassador.getFederate()->getFederateHandle());
        response->setTimeStampValid(false);

        if (_timeConstrainedEnabled || _timeConstrainedEnablePending) {
          // The originating ambassador sends an initial proposal for the logical time,
          // We respond with a corrected logical time if this proposal is not sufficient
          if (logicalTime < _logicalTime) {
            logicalTime = _logicalTime;
            response->getTimeStamp().setLogicalTime(_logicalTimeFactory.encodeLogicalTime(logicalTime));
            response->getTimeStamp().setZeroLookahead(false /*FIXME*/);
            response->setTimeStampValid(true);
          }
        }

        ambassador.send(response);
      }

      _federateLowerBoundMap.insert(message.getFederateHandle(), logicalTimePair);
    }
  }
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const EnableTimeRegulationResponseMessage& message)
  {
    if (!_timeRegulationEnablePending) {
      Log(Network, Warning) << "Recieved EnableTimeRegulationResponseMessage without waiting for that to happen!" << std::endl;
      return;
    }

    if (message.getTimeStampValid()) {
      _timeRegulationEnableFederateHandleTimeStampMap.insert(std::make_pair(message.getRespondingFederateHandle(), message.getTimeStamp()));
    }

    _timeRegulationEnableFederateHandleSet.erase(message.getRespondingFederateHandle());

    // This one checks if we are the last one this ambassador is waiting for
    // and if so, queues the callback and informs the other federates about our
    // new logical time
    checkTimeRegulationEnabled(ambassador);
  }
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const DisableTimeRegulationRequestMessage& message)
  {
    removeFederateFromTimeManagement(ambassador, message.getFederateHandle());
  }
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const CommitLowerBoundTimeStampMessage& message)
  {
    LogicalTime logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getTimeStamp().getLogicalTime());
    bool zeroLookahead = message.getTimeStamp().getZeroLookahead(); /* FIXME*/

    if (!_federateLowerBoundMap.commit(message.getFederateHandle(), LogicalTimePair(logicalTime, zeroLookahead)))
      return;

    // See if we could safely advance to the pending logical time
    if (_timeConstrainedEnablePending) {
      // If we wait for getting time constrained we need to wait until the federations lower bound
      // passes our logical time. Unleach this if we get beyond.
      if (_federateLowerBoundMap.canAdvanceTo(_logicalTime))
        queueTimeConstrainedEnabled(ambassador, _logicalTime);
    } else if (_timeConstrainedEnabled && _timeAdvancePending) {
      // Case, time advance pending
      if (_nextMessageMode && !_logicalTimeMessageListMap.empty()) {
        _pendingLogicalTime.first = std::min(_pendingLogicalTime.first, _logicalTimeMessageListMap.begin()->first);
      }
      if (_federateLowerBoundMap.canAdvanceTo(_pendingLogicalTime)) {
        // Ok, nobody holding us back
        queueTimeAdvanceGranted(ambassador, _pendingLogicalTime.first);
      }
    }
  }


  virtual void queueTimeStampedMessage(InternalAmbassador& ambassador, const VariableLengthData& timeStamp, const AbstractMessage& message)
  {
    _logicalTimeMessageListMap[_logicalTimeFactory.decodeLogicalTime(timeStamp)].push_back(&message);
  }

  void removeFederateFromTimeManagement(InternalAmbassador& ambassador, const FederateHandle& federateHandle)
  {
    _timeRegulationEnableFederateHandleSet.erase(federateHandle);
    _timeRegulationEnableFederateHandleTimeStampMap.erase(federateHandle);

    _federateLowerBoundMap.erase(federateHandle);

    if (_timeRegulationEnablePending) {
      // This one checks if we are the last one this ambassador is waiting for
      // and if so, queues the callback and informs the other federates about our
      // new logical time
      checkTimeRegulationEnabled(ambassador);
    }
    if (_timeConstrainedEnablePending) {
      // If we wait for getting time constrained we need to wait until the federations lower bound
      // passes our logical time. Unleach this if we get beyond.
      if (_federateLowerBoundMap.canAdvanceTo(_logicalTime))
        queueTimeConstrainedEnabled(ambassador, _logicalTime);
    }
    if (_timeAdvancePending) {
      // Case, time advance pending
      if (_nextMessageMode && !_logicalTimeMessageListMap.empty()) {
        _pendingLogicalTime.first = std::min(_pendingLogicalTime.first, _logicalTimeMessageListMap.begin()->first);
      }
      if (_federateLowerBoundMap.canAdvanceTo(_pendingLogicalTime)) {
        // Ok, nobody holding us back
        queueTimeAdvanceGranted(ambassador, _pendingLogicalTime.first);
      }
    }
  }

  void checkTimeRegulationEnabled(InternalAmbassador& ambassador)
  {
    // Check if that was the last one we were waiting for
    if (!_timeRegulationEnableFederateHandleSet.empty())
      return;

    // Now, collect all logical times that other federates sent to us
    for (typename FederateHandleTimeStampMap::const_iterator i = _timeRegulationEnableFederateHandleTimeStampMap.begin();
         i != _timeRegulationEnableFederateHandleTimeStampMap.end(); ++i) {
      LogicalTime logicalTime = _logicalTimeFactory.decodeLogicalTime(i->second.getLogicalTime());
      bool zeroLookahead = i->second.getZeroLookahead(); /* FIXME*/
      LogicalTimePair logicalTimePair(logicalTime, zeroLookahead);
      if (logicalTimePair <= _localLowerBoundTimeStamp)
        continue;

      _localLowerBoundTimeStamp = logicalTimePair;
      _pendingLogicalTime.first = logicalTime;
      _pendingLogicalTime.first -= _currentLookahead;
      _pendingLogicalTime.second = zeroLookahead;
    }

    _timeRegulationEnableFederateHandleTimeStampMap.clear();

    // If somebody has corrected the logical time, then there might be several
    // federates who have a too little committed time, so tell all about them
    sendCommitLowerBoundTimeStamp(ambassador, _localLowerBoundTimeStamp);

    // Ok, now go on ...
    queueTimeRegulationEnabled(ambassador, _pendingLogicalTime.first);
  }
  void sendCommitLowerBoundTimeStamp(InternalAmbassador& ambassador, const LogicalTimePair& logicalTimePair)
  {
    SharedPtr<CommitLowerBoundTimeStampMessage> request;
    request = new CommitLowerBoundTimeStampMessage;
    request->setFederationHandle(ambassador.getFederate()->getFederationHandle());
    request->setFederateHandle(ambassador.getFederate()->getFederateHandle());
    request->getTimeStamp().setLogicalTime(_logicalTimeFactory.encodeLogicalTime(logicalTimePair.first));
    request->getTimeStamp().setZeroLookahead(logicalTimePair.second);
    ambassador.send(request);
  }
  void queueTimeConstrainedEnabled(InternalAmbassador& ambassador, const LogicalTime& logicalTime)
  {
    SharedPtr<TimeConstrainedEnabledMessage> message = new TimeConstrainedEnabledMessage;
    message->setLogicalTime(_logicalTimeFactory.encodeLogicalTime(logicalTime));
    ambassador.queueCallback(message);
  }
  void queueTimeRegulationEnabled(InternalAmbassador& ambassador, const LogicalTime& logicalTime)
  {
    SharedPtr<TimeRegulationEnabledMessage> message = new TimeRegulationEnabledMessage;
    message->setLogicalTime(_logicalTimeFactory.encodeLogicalTime(logicalTime));
    ambassador.queueCallback(message);
  }
  void queueTimeAdvanceGranted(InternalAmbassador& ambassador, const LogicalTime& logicalTime)
  {
    // Flush all timestamped messages up to the given logical time
    for (typename LogicalTimeMessageListMap::iterator i = _logicalTimeMessageListMap.begin(); i != _logicalTimeMessageListMap.end();) {
      if (logicalTime < i->first)
        break;
      ambassador.queueCallbacks(i->second);
      _logicalTimeMessageListMap.erase(i++);
    }
    // Queue the time advance granted
    SharedPtr<TimeAdvanceGrantedMessage> message = new TimeAdvanceGrantedMessage;
    message->setLogicalTime(_logicalTimeFactory.encodeLogicalTime(logicalTime));
    ambassador.queueCallback(message);
    // Queue the may be accumulated ro callbacks that are hold back until now
    ambassador.flushReceiveOrderMessages();
  }

  virtual void acceptCallbackMessage(Ambassador<T>& ambassador, const TimeConstrainedEnabledMessage& message)
  {
    _timeConstrainedEnablePending = false;
    _timeConstrainedEnabled = true;
    _logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getLogicalTime());
    ambassador.timeConstrainedEnabled(_logicalTimeFactory.getLogicalTime(_logicalTime));
  }
  virtual void acceptCallbackMessage(Ambassador<T>& ambassador, const TimeRegulationEnabledMessage& message)
  {
    _timeRegulationEnablePending = false;
    _timeRegulationEnabled = true;
    _logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getLogicalTime());
    ambassador.timeRegulationEnabled(_logicalTimeFactory.getLogicalTime(_logicalTime));
  }
  virtual void acceptCallbackMessage(Ambassador<T>& ambassador, const TimeAdvanceGrantedMessage& message)
  {
    _logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getLogicalTime());
    _pendingLogicalTime.first = _logicalTime;
    _timeAdvancePending = false;
    _flushQueueMode = false;
    ambassador.timeAdvanceGrant(_logicalTimeFactory.getLogicalTime(_logicalTime));
  }

  virtual void reflectAttributeValues(Ambassador<T>& ambassador, const Federate::ObjectClass& objectClass, bool flushQueueMode,
                                      bool timeConstrainedEnabled, const TimeStampedAttributeUpdateMessage& message)
  {
    ambassador.reflectAttributeValues(objectClass, flushQueueMode, timeConstrainedEnabled, message,
                                    _logicalTimeFactory.getLogicalTime(_logicalTimeFactory.decodeLogicalTime(message.getTimeStamp())));
  }

  virtual void removeObjectInstance(Ambassador<T>& ambassador, bool flushQueueMode, bool timeConstrainedEnabled,
                                    const TimeStampedDeleteObjectInstanceMessage& message)
  {
    ambassador.removeObjectInstance(flushQueueMode, timeConstrainedEnabled, message,
                                    _logicalTimeFactory.getLogicalTime(_logicalTimeFactory.decodeLogicalTime(message.getTimeStamp())));
  }

  virtual void receiveInteraction(Ambassador<T>& ambassador, const InteractionClassHandle& interactionClassHandle,
                                  const Federate::InteractionClass& interactionClass, bool flushQueueMode,
                                  bool timeConstrainedEnabled, const TimeStampedInteractionMessage& message)
  {
    ambassador.receiveInteraction(interactionClassHandle, interactionClass, flushQueueMode,
                                  timeConstrainedEnabled, message,
                                  _logicalTimeFactory.getLogicalTime(_logicalTimeFactory.decodeLogicalTime(message.getTimeStamp())));
  }

  // The current logical time of this federate
  LogicalTime _logicalTime;
  // If a time advance is pending, this contains the logical time of the advance call.
  LogicalTimePair _pendingLogicalTime;
  // The smallest allowed logical time for sending messages
  LogicalTimePair _localLowerBoundTimeStamp;
  // The lookahead of this federate
  LogicalTimeInterval _currentLookahead;
  LogicalTimeInterval _targetLookahead;

  // The logical time factory required to do our job
  LogicalTimeFactory _logicalTimeFactory;

  // When we are in time regulation enable pending state, this contains the federates we need to
  // wait for to complete time regulation enabled
  FederateHandleSet _timeRegulationEnableFederateHandleSet;
  typedef std::map<FederateHandle, TimeStamp> FederateHandleTimeStampMap;
  FederateHandleTimeStampMap _timeRegulationEnableFederateHandleTimeStampMap;

  // map containing all the committed logical times of all known time regulating federates
  typedef FederateHandleLowerBoundTimeStampMap<LogicalTime> FederateLowerBoundMap;
  FederateLowerBoundMap _federateLowerBoundMap;

  // The timestamped queued messages
  typedef std::list<SharedPtr<const AbstractMessage> > MessageList2;
  typedef std::map<LogicalTime, MessageList2> LogicalTimeMessageListMap;
  LogicalTimeMessageListMap _logicalTimeMessageListMap;
};

} // namespace OpenRTI

#endif
