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
class OPENRTI_API TemplateTimeManagement : public TimeManagement<T> {
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

  TemplateTimeManagement(const LogicalTimeFactory& logicalTimeFactory) :
    _logicalTimeFactory(logicalTimeFactory)
  {
    _logicalTime = _logicalTimeFactory.initialLogicalTime();
    _pendingLogicalTime = _logicalTime;
    _localLowerBoundTimeStamp.first = _logicalTime;
    _localLowerBoundTimeStamp.second = true;
    _committedLowerBoundTimeStamp = _localLowerBoundTimeStamp;
    _currentLookahead = _logicalTimeFactory.zeroLogicalTimeInterval();
    _targetLookahead = _currentLookahead;
  }
  virtual ~TemplateTimeManagement()
  { }

  virtual bool isLogicalTimeInThePast(const NativeLogicalTime& logicalTime)
  {
    OpenRTIAssert(_committedLowerBoundTimeStamp <= _localLowerBoundTimeStamp);
    return logicalTime < _logicalTimeFactory.getLogicalTime(_logicalTime);
  }
  virtual bool logicalTimeAlreadyPassed(const NativeLogicalTime& logicalTime)
  {
    OpenRTIAssert(_committedLowerBoundTimeStamp <= _localLowerBoundTimeStamp);
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
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnabledOrPending());
    OpenRTIAssert(!InternalTimeManagement::getTimeConstrainedEnablePending());
    OpenRTIAssert(!InternalTimeManagement::getTimeAdvancePending());
    OpenRTIAssert(_logicalTime <= logicalTime);
    OpenRTIAssert(_logicalTimeFactory.zeroLogicalTimeInterval() <= lookahead);

    InternalTimeManagement::setTimeRegulationMode(InternalTimeManagement::TimeRegulationEnablePending);

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
    OpenRTIAssert(InternalTimeManagement::getTimeRegulationEnabled());
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnablePending());
    OpenRTIAssert(!InternalTimeManagement::getTimeConstrainedEnablePending());
    OpenRTIAssert(!InternalTimeManagement::getTimeAdvancePending());

    InternalTimeManagement::setTimeRegulationMode(InternalTimeManagement::TimeRegulationDisabled);

    SharedPtr<DisableTimeRegulationRequestMessage> request;
    request = new DisableTimeRegulationRequestMessage;
    request->setFederationHandle(ambassador.getFederate()->getFederationHandle());
    request->setFederateHandle(ambassador.getFederate()->getFederateHandle());
    ambassador.send(request);
  }

  virtual void enableTimeConstrained(InternalAmbassador& ambassador)
  {
    OpenRTIAssert(!InternalTimeManagement::getTimeConstrainedEnabledOrPending());
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnablePending());
    OpenRTIAssert(!InternalTimeManagement::getTimeAdvancePending());

    InternalTimeManagement::setTimeConstrainedMode(InternalTimeManagement::TimeConstrainedEnablePending);

    SharedPtr<TimeConstrainedEnabledMessage> message = new TimeConstrainedEnabledMessage;
    queueTimeStampedMessage(LogicalTimePair(_logicalTime, 1), *message);
  }

  virtual void disableTimeConstrained(InternalAmbassador& ambassador)
  {
    OpenRTIAssert(InternalTimeManagement::getTimeConstrainedEnabled());
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnablePending());
    OpenRTIAssert(!InternalTimeManagement::getTimeConstrainedEnablePending());
    OpenRTIAssert(!InternalTimeManagement::getTimeAdvancePending());

    InternalTimeManagement::setTimeConstrainedMode(InternalTimeManagement::TimeConstrainedDisabled);
  }

  void setLocalLowerBoundTimeStampAndCurrentLookahead(const LogicalTime& logicalTime, const LogicalTimeInterval& lookahead)
  {
    LogicalTimePair localLowerBoundTimeStamp(logicalTime, _logicalTimeFactory.isZeroTimeInterval(lookahead));
    if (!localLowerBoundTimeStamp.second) {
      localLowerBoundTimeStamp.first += lookahead;
    }

    // Check if we would violate a previously given guarantee about the lower bound timestamp
    if (localLowerBoundTimeStamp < _localLowerBoundTimeStamp) {
      // if so, adjust the _current lookahead and leave the lower bound timestamp alone
      _currentLookahead = _localLowerBoundTimeStamp.first - localLowerBoundTimeStamp.first;
    } else {
      if (_currentLookahead != lookahead)
        _currentLookahead = lookahead;
      _localLowerBoundTimeStamp = localLowerBoundTimeStamp;
    }
  }

  virtual void timeAdvanceRequest(InternalAmbassador& ambassador, const NativeLogicalTime& nativeLogicalTime, InternalTimeManagement::TimeAdvanceMode timeAdvanceMode)
  {
    OpenRTIAssert(timeAdvanceMode != InternalTimeManagement::TimeAdvanceGranted);
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnablePending());
    OpenRTIAssert(!InternalTimeManagement::getTimeConstrainedEnablePending());
    OpenRTIAssert(!InternalTimeManagement::getTimeAdvancePending());
    OpenRTIAssert(_logicalTime <= _logicalTimeFactory.getLogicalTime(nativeLogicalTime));

    /// FIXME no next message mode implemented as of today!!!
    OpenRTIAssert(timeAdvanceMode != InternalTimeManagement::NextMessageRequest);
    OpenRTIAssert(timeAdvanceMode != InternalTimeManagement::NextMessageRequestAvailable);
    OpenRTIAssert(timeAdvanceMode != InternalTimeManagement::FlushQueueRequest);

    LogicalTime logicalTime = _logicalTimeFactory.getLogicalTime(nativeLogicalTime);
    InternalTimeManagement::setTimeAdvanceMode(timeAdvanceMode);
    _pendingLogicalTime = logicalTime;

    setLocalLowerBoundTimeStampAndCurrentLookahead(_pendingLogicalTime, _targetLookahead);

    if (InternalTimeManagement::getTimeRegulationEnabled())
      sendCommitLowerBoundTimeStampIfNewer(ambassador, _localLowerBoundTimeStamp);

    // Queue the time advance granted
    SharedPtr<TimeAdvanceGrantedMessage> message = new TimeAdvanceGrantedMessage;
    switch (timeAdvanceMode) {
    case InternalTimeManagement::TimeAdvanceRequest:
    case InternalTimeManagement::NextMessageRequest:
      queueTimeStampedMessage(LogicalTimePair(_pendingLogicalTime, 1), *message);
      break;
    case InternalTimeManagement::TimeAdvanceRequestAvailable:
    case InternalTimeManagement::NextMessageRequestAvailable:
      queueTimeStampedMessage(LogicalTimePair(_pendingLogicalTime, 0), *message);
      break;
    case InternalTimeManagement::FlushQueueRequest:
      queueTimeStampedMessage(LogicalTimePair(_logicalTimeFactory.finalLogicalTime(), 1), *message);
      break;
    default:
      break;
    }
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
    OpenRTIAssert(InternalTimeManagement::getTimeRegulationEnabled());
    OpenRTIAssert(!InternalTimeManagement::getTimeAdvancePending());
    OpenRTIAssert(!InternalTimeManagement::getTimeConstrainedEnablePending());
    OpenRTIAssert(_logicalTimeFactory.zeroLogicalTimeInterval() <= _logicalTimeFactory.getLogicalTimeInterval(nativeLookahead));

    LogicalTimeInterval lookahead = _logicalTimeFactory.getLogicalTimeInterval(nativeLookahead);
    _targetLookahead = lookahead;
    setLocalLowerBoundTimeStampAndCurrentLookahead(_logicalTime, _targetLookahead);
    sendCommitLowerBoundTimeStampIfNewer(ambassador, _localLowerBoundTimeStamp);
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

      if (!InternalTimeManagement::getTimeRegulationEnablePending()) {
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

        if (InternalTimeManagement::getTimeConstrainedEnabledOrPending()) {
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
      OpenRTIAssert(InternalTimeManagement::getTimeConstrainedDisabled() || _federateLowerBoundMap.canAdvanceTo(_logicalTime));
    }
  }
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const EnableTimeRegulationResponseMessage& message)
  {
    if (!InternalTimeManagement::getTimeRegulationEnablePending()) {
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
    OpenRTIAssert(!InternalTimeManagement::getTimeConstrainedEnabled() || _federateLowerBoundMap.canAdvanceTo(_logicalTime));
    _federateLowerBoundMap.commit(message.getFederateHandle(), LogicalTimePair(logicalTime, zeroLookahead));
    OpenRTIAssert(!InternalTimeManagement::getTimeConstrainedEnabled() || _federateLowerBoundMap.canAdvanceTo(_logicalTime));
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
    OpenRTIAssert(!InternalTimeManagement::getTimeConstrainedEnabled() || _logicalTime < logicalTime);
    if (InternalTimeManagement::getTimeConstrainedEnabled() && logicalTime <= _logicalTime) {
      Log(Network, Warning) << "Dropping illegal time stamp order message!\n"
                            << "You may communicate with a buggy federate ambassador." << std::endl;
      return;
    }
#endif
    queueTimeStampedMessage(LogicalTimePair(logicalTime, -1), message);
  }
  void queueTimeStampedMessage(const LogicalTimePair& logicalTimePair, const AbstractMessage& message)
  {
    if (_messageListPool.empty()) {
      _logicalTimeMessageListMap[logicalTimePair].push_back(&message);
    } else {
      MessageList& messageList = _logicalTimeMessageListMap[logicalTimePair];
      messageList.splice(messageList.end(), _messageListPool, _messageListPool.begin());
      messageList.back() = &message;
    }
  }
  virtual void queueReceiveOrderMessage(InternalAmbassador& ambassador, const AbstractMessage& message)
  {
    if (_messageListPool.empty()) {
      _receiveOrderMessages.push_back(&message);
    } else {
      _receiveOrderMessages.splice(_receiveOrderMessages.end(), _messageListPool, _messageListPool.begin());
      _receiveOrderMessages.back() = &message;
    }
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
    OpenRTIAssert(InternalTimeManagement::getTimeRegulationEnablePending());

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
    sendCommitLowerBoundTimeStampIfNewer(ambassador, _localLowerBoundTimeStamp);

    // Ok, now go on ...
    SharedPtr<TimeRegulationEnabledMessage> message = new TimeRegulationEnabledMessage;
    queueTimeStampedMessage(LogicalTimePair(_pendingLogicalTime, 1), *message);
  }

  void sendCommitLowerBoundTimeStampIfNewer(InternalAmbassador& ambassador, const LogicalTimePair& logicalTimePair)
  {
    OpenRTIAssert(_committedLowerBoundTimeStamp <= _localLowerBoundTimeStamp);
    if (logicalTimePair <= _committedLowerBoundTimeStamp)
      return;
    _committedLowerBoundTimeStamp = logicalTimePair;
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
    InternalTimeManagement::setTimeConstrainedMode(InternalTimeManagement::TimeConstrainedEnabled);
    _logicalTime = _pendingLogicalTime;
    OpenRTIAssert(_federateLowerBoundMap.canAdvanceTo(_logicalTime));
    OpenRTIAssert(_logicalTimeMessageListMap.empty() || _logicalTime < _logicalTimeMessageListMap.begin()->first.first || _logicalTimeMessageListMap.begin()->second.size() <= 1);
    ambassador.timeConstrainedEnabled(_logicalTimeFactory.getLogicalTime(_logicalTime));
  }
  virtual void acceptCallbackMessage(Ambassador<T>& ambassador, const TimeRegulationEnabledMessage& message)
  {
    InternalTimeManagement::setTimeRegulationMode(InternalTimeManagement::TimeRegulationEnabled);
    _logicalTime = _pendingLogicalTime;
    ambassador.timeRegulationEnabled(_logicalTimeFactory.getLogicalTime(_logicalTime));
  }
  virtual void acceptCallbackMessage(Ambassador<T>& ambassador, const TimeAdvanceGrantedMessage& message)
  {
    _logicalTime = _pendingLogicalTime;
    OpenRTIAssert(InternalTimeManagement::getFlushQueueMode() || _federateLowerBoundMap.canAdvanceTo(_logicalTime));
    InternalTimeManagement::setTimeAdvanceMode(InternalTimeManagement::TimeAdvanceGranted);
    ambassador.timeAdvanceGrant(_logicalTimeFactory.getLogicalTime(_logicalTime));
  }

  virtual void reflectAttributeValues(Ambassador<T>& ambassador, const Federate::ObjectClass& objectClass,
                                      const TimeStampedAttributeUpdateMessage& message)
  {
    bool isTimeStampedOrderDelivery = InternalTimeManagement::getTimeStampedOrderDelivery(message.getOrderType());
    LogicalTime logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getTimeStamp());
    OpenRTIAssert(!isTimeStampedOrderDelivery || _logicalTime < logicalTime);
    ambassador.reflectAttributeValues(objectClass, InternalTimeManagement::getFlushQueueMode(), isTimeStampedOrderDelivery,
                                      message, _logicalTimeFactory.getLogicalTime(logicalTime));
  }

  virtual void removeObjectInstance(Ambassador<T>& ambassador, const TimeStampedDeleteObjectInstanceMessage& message)
  {
    bool isTimeStampedOrderDelivery = InternalTimeManagement::getTimeStampedOrderDelivery(message.getOrderType());
    LogicalTime logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getTimeStamp());
    OpenRTIAssert(!isTimeStampedOrderDelivery || _logicalTime < logicalTime);
    ambassador.removeObjectInstance(InternalTimeManagement::getFlushQueueMode(), isTimeStampedOrderDelivery, message,
                                    _logicalTimeFactory.getLogicalTime(logicalTime));
  }

  virtual void receiveInteraction(Ambassador<T>& ambassador, const InteractionClassHandle& interactionClassHandle,
                                  const Federate::InteractionClass& interactionClass, const TimeStampedInteractionMessage& message)
  {
    bool isTimeStampedOrderDelivery = InternalTimeManagement::getTimeStampedOrderDelivery(message.getOrderType());
    LogicalTime logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getTimeStamp());
    OpenRTIAssert(!isTimeStampedOrderDelivery || _logicalTime < logicalTime);
    ambassador.receiveInteraction(interactionClassHandle, interactionClass, InternalTimeManagement::getFlushQueueMode(),
                                  isTimeStampedOrderDelivery, message, _logicalTimeFactory.getLogicalTime(logicalTime));
  }

  virtual bool dispatchCallback(const AbstractMessageDispatcher& dispatcher)
  {
    if (_receiveOrderMessagesPermitted()) {
      if (!_receiveOrderMessages.empty()) {
        SharedPtr<const AbstractMessage> message;
        message.swap(_receiveOrderMessages.front());
        _messageListPool.splice(_messageListPool.begin(), _receiveOrderMessages, _receiveOrderMessages.begin());
        message->dispatch(dispatcher);
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
      SharedPtr<const AbstractMessage> message;
      message.swap(_logicalTimeMessageListMap.begin()->second.front());
      _messageListPool.splice(_messageListPool.begin(), _logicalTimeMessageListMap.begin()->second, _logicalTimeMessageListMap.begin()->second.begin());
      message->dispatch(dispatcher);
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
    if (InternalTimeManagement::getFlushQueueMode())
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
  LogicalTimePair _committedLowerBoundTimeStamp;
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

  // List elements for reuse
  MessageList _messageListPool;
};

} // namespace OpenRTI

#endif
