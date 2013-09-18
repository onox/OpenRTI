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

  // The int contains an apropriate number so that the std::less comparison on this
  // pair helps to sort messages and even thje time advance requests in a appropriate way.
  // Normal time stamp order messages are scheduled in the message queues with the second
  // entry set to -1. That way they are guaranteed to happen before any tar message
  // which are scheduled with 0 for the time advance available variants and with 1 for the
  // compete time advance modes.
  // Committed lower bound timestamps are sent with the int set to wether the committed value
  // is meant to work for zero lookahead or not.
  typedef std::pair<LogicalTime, int> LogicalTimePair;

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
    _pendingLogicalTime = _logicalTime;
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



  virtual void enableTimeRegulation(InternalAmbassador& ambassador, const NativeLogicalTimeInterval& nativeLookahead)
  {
    LogicalTimeInterval lookahead = _logicalTimeFactory.getLogicalTimeInterval(nativeLookahead);
    _enableTimeRegulation(ambassador, _logicalTime, lookahead);
  }

  // the RTI13 variant
  virtual void enableTimeRegulation(InternalAmbassador& ambassador, const NativeLogicalTime& nativeLogicalTime, const NativeLogicalTimeInterval& nativeLookahead)
  {
    LogicalTime logicalTime = _logicalTimeFactory.getLogicalTime(nativeLogicalTime);
    LogicalTimeInterval lookahead = _logicalTimeFactory.getLogicalTimeInterval(nativeLookahead);
    _enableTimeRegulation(ambassador, logicalTime, lookahead);
  }

  void _enableTimeRegulation(InternalAmbassador& ambassador, const LogicalTime& logicalTime, const LogicalTimeInterval& lookahead)
  {
    _timeRegulationEnablePending = true;

    _currentLookahead = lookahead;
    _targetLookahead = _currentLookahead;

    _pendingLogicalTime = logicalTime;

    _localLowerBoundTimeStamp.first = _pendingLogicalTime;
    _localLowerBoundTimeStamp.first += _currentLookahead;
    _localLowerBoundTimeStamp.second = _logicalTimeFactory.isZeroTimeInterval(_currentLookahead);

    OpenRTIAssert(_timeRegulationEnableFederateHandleTimeStampMap.empty());
    OpenRTIAssert(_timeRegulationEnableFederateHandleSet.empty());

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

  virtual void disableTimeRegulation(InternalAmbassador& ambassador)
  {
    _timeRegulationEnabled = false;

    SharedPtr<DisableTimeRegulationRequestMessage> request;
    request = new DisableTimeRegulationRequestMessage;
    request->setFederationHandle(ambassador.getFederate()->getFederationHandle());
    request->setFederateHandle(ambassador.getFederate()->getFederateHandle());
    ambassador.send(request);
  }

  virtual void enableTimeConstrained(InternalAmbassador& ambassador)
  {
    _timeConstrainedEnablePending = true;

    SharedPtr<TimeConstrainedEnabledMessage> message = new TimeConstrainedEnabledMessage;
    message->setLogicalTime(_logicalTimeFactory.encodeLogicalTime(_logicalTime));
    _logicalTimeMessageListMap[LogicalTimePair(_logicalTime, 1)].push_back(message);
  }

  virtual void disableTimeConstrained(InternalAmbassador& ambassador)
  {
    _timeConstrainedEnabled = false;
  }

  virtual void timeAdvanceRequest(InternalAmbassador& ambassador, const NativeLogicalTime& nativeLogicalTime, bool availableMode, bool nextMessageMode, bool flushQueue)
  {
    LogicalTime logicalTime = _logicalTimeFactory.getLogicalTime(nativeLogicalTime);
    /// FIXME no next message mode implemented as of today!!!
    _nextMessageMode = nextMessageMode;
    _flushQueueMode = flushQueue;
    _timeAdvancePending = true;
    _pendingLogicalTime = logicalTime;

#ifndef _NDEBUG
    LogicalTimePair oldLBTS = _localLowerBoundTimeStamp;
#endif

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
      _localLowerBoundTimeStamp.second = 1;
    } else {
      _localLowerBoundTimeStamp.first += _currentLookahead;
      _localLowerBoundTimeStamp.second = 0;
    }

#ifndef _NDEBUG
    OpenRTIAssert(oldLBTS <= _localLowerBoundTimeStamp);
#endif

    if (_timeRegulationEnabled)
      sendCommitLowerBoundTimeStamp(ambassador, _localLowerBoundTimeStamp);

    // Queue the time advance granted
    SharedPtr<TimeAdvanceGrantedMessage> message = new TimeAdvanceGrantedMessage;
    message->setLogicalTime(_logicalTimeFactory.encodeLogicalTime(_pendingLogicalTime));
    if (flushQueue)
      _logicalTimeMessageListMap[LogicalTimePair(_logicalTimeFactory.finalLogicalTime(), 1)].push_back(message);
    else if (availableMode)
      _logicalTimeMessageListMap[LogicalTimePair(_pendingLogicalTime, 0)].push_back(message);
    else
      _logicalTimeMessageListMap[LogicalTimePair(_pendingLogicalTime, 1)].push_back(message);
  }

  virtual bool queryGALT(InternalAmbassador& ambassador, NativeLogicalTime& logicalTime)
  {
    if (_federateLowerBoundMap.empty())
      return false;
    logicalTime = _logicalTimeFactory.getLogicalTime(_federateLowerBoundMap.getGALT());
    return true;
  }

  virtual void queryLogicalTime(InternalAmbassador& ambassador, NativeLogicalTime& logicalTime)
  {
    logicalTime = _logicalTimeFactory.getLogicalTime(_logicalTime);
  }

  virtual bool queryLITS(InternalAmbassador& ambassador, NativeLogicalTime& logicalTime)
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
        logicalTime = _logicalTimeFactory.getLogicalTime(_logicalTimeMessageListMap.begin()->first.first);
        return true;
      } else {
        if (_federateLowerBoundMap.getGALT() < _logicalTimeMessageListMap.begin()->first.first)
          logicalTime = _logicalTimeFactory.getLogicalTime(_federateLowerBoundMap.getGALT());
        else
          logicalTime = _logicalTimeFactory.getLogicalTime(_logicalTimeMessageListMap.begin()->first.first);
        return true;
      }
    }
  }

  virtual void modifyLookahead(InternalAmbassador& ambassador, const NativeLogicalTimeInterval& nativeLookahead)
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

  virtual void queryLookahead(InternalAmbassador& ambassador, NativeLogicalTimeInterval& logicalTimeInterval)
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
      // The responses just takes the direct route back.

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
      LogicalTimePair logicalTimePair(logicalTime, message.getTimeStamp().getZeroLookahead());

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
          if (logicalTime <= _logicalTime) {
            logicalTimePair.first = _logicalTime;
            logicalTimePair.second = true;
            response->setTimeStamp(_logicalTimeFactory.encodeLogicalTime(_logicalTime));
            response->setTimeStampValid(true);
          }
        }

        ambassador.send(response);
      }

      _federateLowerBoundMap.insert(message.getFederateHandle(), logicalTimePair);
      OpenRTIAssert(!_federateLowerBoundMap.empty());
      OpenRTIAssert(!(_timeConstrainedEnabled || _timeConstrainedEnablePending) || _federateLowerBoundMap.canAdvanceTo(_logicalTime));
    }
  }
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const EnableTimeRegulationResponseMessage& message)
  {
    if (!_timeRegulationEnablePending) {
      Log(Network, Warning) << "Recieved EnableTimeRegulationResponseMessage without waiting for that to happen!" << std::endl;
      return;
    }

    if (message.getTimeStampValid()) {
      LogicalTime logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getTimeStamp());
      _timeRegulationEnableFederateHandleTimeStampMap[message.getRespondingFederateHandle()] = logicalTime;
    }

    _timeRegulationEnableFederateHandleSet.erase(message.getRespondingFederateHandle());

    // This one checks if we are the last one this ambassador is waiting for
    // and if so, queues the callback and informs the other federates about our
    // new logical time
    checkTimeRegulationEnabled(ambassador);
  }
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const DisableTimeRegulationRequestMessage& message)
  {
    _federateLowerBoundMap.erase(message.getFederateHandle());
    _timeRegulationEnableFederateHandleTimeStampMap.erase(message.getFederateHandle());
  }
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const CommitLowerBoundTimeStampMessage& message)
  {
    LogicalTime logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getTimeStamp().getLogicalTime());
    bool zeroLookahead = message.getTimeStamp().getZeroLookahead();
    OpenRTIAssert(!_timeConstrainedEnabled || _federateLowerBoundMap.canAdvanceTo(_logicalTime));
    _federateLowerBoundMap.commit(message.getFederateHandle(), LogicalTimePair(logicalTime, zeroLookahead));
    OpenRTIAssert(!_timeConstrainedEnabled || _federateLowerBoundMap.canAdvanceTo(_logicalTime));
  }

  virtual void queueTimeStampedMessage(InternalAmbassador& ambassador, const VariableLengthData& timeStamp, const AbstractMessage& message)
  {
    queueTimeStampedMessage(_logicalTimeFactory.decodeLogicalTime(timeStamp), message);
  }
  void queueTimeStampedMessage(const LogicalTime& logicalTime, const AbstractMessage& message)
  {
    // Well potentially we can savely skip both tests since only a buggy federate can send this messages
    // under these circumstance. But since we cannot rely on this on a public network, we are kind and
    // drop these messages.
    // Except in debug mode, where we wat the test to fail under this conditions!
#ifdef _NDEBUG
    if (_federateLowerBoundMap.canAdvanceTo(logicalTime)) {
      Log(Network, Warning) << "Dropping illegal time stamp order message!\n"
                            << "You may communicate with a buggy federate ambassador." << std::endl;
      return;
    }
    OpenRTIAssert(!_timeConstrainedEnabled || _logicalTime < logicalTime);
    if (_timeConstrainedEnabled && logicalTime <= _logicalTime) {
      Log(Network, Warning) << "Dropping illegal time stamp order message!\n"
                            << "You may communicate with a buggy federate ambassador." << std::endl;
      return;
    }
#endif
    _logicalTimeMessageListMap[LogicalTimePair(logicalTime, -1)].push_back(&message);
  }
  virtual void queueReceiveOrderMessage(InternalAmbassador& ambassador, const AbstractMessage& message)
  {
    _receiveOrderMessages.push_back(&message);
  }

  void removeFederateFromTimeManagement(InternalAmbassador& ambassador, const FederateHandle& federateHandle)
  {
    _federateLowerBoundMap.erase(federateHandle);
    _timeRegulationEnableFederateHandleTimeStampMap.erase(federateHandle);
    FederateHandleSet::iterator i = _timeRegulationEnableFederateHandleSet.find(federateHandle);
    if (i == _timeRegulationEnableFederateHandleSet.end())
      return;
    _timeRegulationEnableFederateHandleSet.erase(i);

    // Now we should really know that we had time regulation enabled
    OpenRTIAssert(_timeRegulationEnablePending);

    // This one checks if we are the last one this ambassador is waiting for
    // and if so, queues the callback and informs the other federates about our
    // new logical time
    checkTimeRegulationEnabled(ambassador);
  }

  void checkTimeRegulationEnabled(InternalAmbassador& ambassador)
  {
    // Check if that was the last one we were waiting for
    if (!_timeRegulationEnableFederateHandleSet.empty())
      return;

#ifndef _NDEBUG
    LogicalTimePair oldLBTS = _localLowerBoundTimeStamp;
#endif

    // Now, collect all logical times that other federates sent to us
    for (typename FederateHandleTimeStampMap::const_iterator i = _timeRegulationEnableFederateHandleTimeStampMap.begin();
         i != _timeRegulationEnableFederateHandleTimeStampMap.end(); ++i) {
      // This federate sent us just its logical time.
      // That means we need to make sure that all messages we send are strictly
      // in the future wrt this sent timestep.
      LogicalTimePair logicalTimePair(i->second, 0);
      if (logicalTimePair < _localLowerBoundTimeStamp)
        continue;

      // Ok, here is some room in between, but for now this is ok by the standard
      _localLowerBoundTimeStamp.first = logicalTimePair.first;
      _localLowerBoundTimeStamp.first += _currentLookahead;
      _pendingLogicalTime = logicalTimePair.first;
    }

    _timeRegulationEnableFederateHandleTimeStampMap.clear();

#ifndef _NDEBUG
    OpenRTIAssert(oldLBTS <= _localLowerBoundTimeStamp);
#endif

    // If somebody has corrected the logical time, then there might be several
    // federates who have a too little committed time, so tell all about them
    sendCommitLowerBoundTimeStamp(ambassador, _localLowerBoundTimeStamp);

    // Ok, now go on ...
    SharedPtr<TimeRegulationEnabledMessage> message = new TimeRegulationEnabledMessage;
    message->setLogicalTime(_logicalTimeFactory.encodeLogicalTime(_pendingLogicalTime));
    _logicalTimeMessageListMap[LogicalTimePair(_pendingLogicalTime, 1)].push_back(message);
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

  virtual void acceptCallbackMessage(Ambassador<T>& ambassador, const TimeConstrainedEnabledMessage& message)
  {
    _timeConstrainedEnablePending = false;
    _timeConstrainedEnabled = true;
    OpenRTIAssert(_logicalTime <= _logicalTimeFactory.decodeLogicalTime(message.getLogicalTime()));
    _logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getLogicalTime());
    OpenRTIAssert(_pendingLogicalTime == _logicalTime);
    OpenRTIAssert(_federateLowerBoundMap.canAdvanceTo(_logicalTime));
    OpenRTIAssert(_logicalTimeMessageListMap.empty() || _logicalTime < _logicalTimeMessageListMap.begin()->first.first || _logicalTimeMessageListMap.begin()->second.size() <= 1);
    ambassador.timeConstrainedEnabled(_logicalTimeFactory.getLogicalTime(_logicalTime));
  }
  virtual void acceptCallbackMessage(Ambassador<T>& ambassador, const TimeRegulationEnabledMessage& message)
  {
    OpenRTIAssert(_logicalTime <= _logicalTimeFactory.decodeLogicalTime(message.getLogicalTime()));
    _timeRegulationEnablePending = false;
    _timeRegulationEnabled = true;
    OpenRTIAssert(_logicalTime <= _logicalTimeFactory.decodeLogicalTime(message.getLogicalTime()));
    _logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getLogicalTime());
    OpenRTIAssert(_pendingLogicalTime == _logicalTime);
    ambassador.timeRegulationEnabled(_logicalTimeFactory.getLogicalTime(_logicalTime));
  }
  virtual void acceptCallbackMessage(Ambassador<T>& ambassador, const TimeAdvanceGrantedMessage& message)
  {
    OpenRTIAssert(_logicalTime <= _logicalTimeFactory.decodeLogicalTime(message.getLogicalTime()));
    _logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getLogicalTime());
    OpenRTIAssert(_pendingLogicalTime == _logicalTime);
    _timeAdvancePending = false;
    _flushQueueMode = false;
    OpenRTIAssert(_flushQueueMode || _federateLowerBoundMap.canAdvanceTo(_logicalTime));
    ambassador.timeAdvanceGrant(_logicalTimeFactory.getLogicalTime(_logicalTime));
  }

  virtual void reflectAttributeValues(Ambassador<T>& ambassador, const Federate::ObjectClass& objectClass,
                                      const TimeStampedAttributeUpdateMessage& message)
  {
    LogicalTime logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getTimeStamp());
    ambassador.reflectAttributeValues(objectClass, _flushQueueMode, _timeConstrainedEnabled && message.getOrderType() == TIMESTAMP, message,
                                    _logicalTimeFactory.getLogicalTime(logicalTime));
  }

  virtual void removeObjectInstance(Ambassador<T>& ambassador, const TimeStampedDeleteObjectInstanceMessage& message)
  {
    LogicalTime logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getTimeStamp());
    ambassador.removeObjectInstance(_flushQueueMode, _timeConstrainedEnabled && message.getOrderType() == TIMESTAMP, message,
                                    _logicalTimeFactory.getLogicalTime(logicalTime));
  }

  virtual void receiveInteraction(Ambassador<T>& ambassador, const InteractionClassHandle& interactionClassHandle,
                                  const Federate::InteractionClass& interactionClass, const TimeStampedInteractionMessage& message)
  {
    LogicalTime logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getTimeStamp());

    OpenRTIAssert(!_timeConstrainedEnabled || message.getOrderType() != TIMESTAMP || _logicalTime < logicalTime);

    ambassador.receiveInteraction(interactionClassHandle, interactionClass, _flushQueueMode,
                                  _timeConstrainedEnabled && message.getOrderType() == TIMESTAMP, message,
                                  _logicalTimeFactory.getLogicalTime(logicalTime));
  }

  virtual bool dispatchCallback(const AbstractMessageDispatcher& dispatcher)
  {
    if (_receiveOrderMessagesPermitted()) {
      if (!_receiveOrderMessages.empty()) {
        _receiveOrderMessages.front()->dispatch(dispatcher);
        _receiveOrderMessages.pop_front();
        return true;
      }
    }
    while (!_logicalTimeMessageListMap.empty()) {
      if (_logicalTimeMessageListMap.begin()->second.empty()) {
        _logicalTimeMessageListMap.erase(_logicalTimeMessageListMap.begin());
        continue;
      }
      if (!_timeStampOrderMessagesPermitted())
        break;
      _logicalTimeMessageListMap.begin()->second.front()->dispatch(dispatcher);
      _logicalTimeMessageListMap.begin()->second.pop_front();
      if (_logicalTimeMessageListMap.begin()->second.empty())
        _logicalTimeMessageListMap.erase(_logicalTimeMessageListMap.begin());
      return true;
    }
    return false;
  }

  virtual bool callbackMessageAvailable()
  {
    if (_receiveOrderMessagesPermitted()) {
      if (!_receiveOrderMessages.empty()) {
        return true;
      }
    }
    while (!_logicalTimeMessageListMap.empty()) {
      if (_logicalTimeMessageListMap.begin()->second.empty()) {
        _logicalTimeMessageListMap.erase(_logicalTimeMessageListMap.begin());
        continue;
      }
      if (!_timeStampOrderMessagesPermitted())
        break;
      return true;
    }
    return false;
  }

  bool _timeStampOrderMessagesPermitted() const
  {
    if (_flushQueueMode)
      return true;
    if (!InternalTimeManagement::getTimeConstrainedEnabled())
      return true;
    if (_pendingLogicalTime < _logicalTimeMessageListMap.begin()->first.first)
      return false;
    return _federateLowerBoundMap.canAdvanceTo(_logicalTimeMessageListMap.begin()->first.first);
  }

  bool _receiveOrderMessagesPermitted() const
  {
    if (!InternalTimeManagement::getTimeConstrainedEnabled())
      return true;
    if (InternalTimeManagement::getAsynchronousDeliveryEnabled())
      return true;
    if (InternalTimeManagement::getTimeAdvancePending())
      return true;
    return false;
  }

  // The current logical time of this federate
  LogicalTime _logicalTime;
  // If a time advance is pending, this contains the logical time of the advance call.
  LogicalTime _pendingLogicalTime;
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
  typedef std::map<FederateHandle, LogicalTime> FederateHandleTimeStampMap;
  FederateHandleTimeStampMap _timeRegulationEnableFederateHandleTimeStampMap;

  // map containing all the committed logical times of all known time regulating federates
  typedef FederateHandleLowerBoundTimeStampMap<LogicalTime> FederateLowerBoundMap;
  FederateLowerBoundMap _federateLowerBoundMap;

  // The timestamped queued messages
  typedef std::map<LogicalTimePair, MessageList> LogicalTimeMessageListMap;
  LogicalTimeMessageListMap _logicalTimeMessageListMap;

  // List of receive order messages ready to be queued for callback
  MessageList _receiveOrderMessages;
};

} // namespace OpenRTI

#endif
