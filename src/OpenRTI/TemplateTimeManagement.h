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

#ifndef OpenRTI_TemplateTimeManagement_h
#define OpenRTI_TemplateTimeManagement_h

#include "TimeManagement.h"

#include "FederateHandleLowerBoundTimeStampMap.h"
#include "IntrusiveList.h"
#include "IntrusiveUnorderedMap.h"

namespace OpenRTI {

template<typename T>
class Ambassador;

template<typename T, typename L>
class OPENRTI_LOCAL TemplateTimeManagement : public TimeManagement<T> {
public:
  typedef T Traits;

  typedef typename Traits::NativeLogicalTime NativeLogicalTime;
  typedef typename Traits::NativeLogicalTimeInterval NativeLogicalTimeInterval;

  typedef L LogicalTimeFactory;
  typedef typename LogicalTimeFactory::LogicalTime LogicalTime;
  typedef typename LogicalTimeFactory::LogicalTimeInterval LogicalTimeInterval;

  // The int contains an apropriate number so that the std::less comparison on this
  // pair helps to sort messages and even the time advance requests in a appropriate way.
  // Normal time stamp order messages are scheduled in the message queues with the second
  // entry set to -1. That way they are guaranteed to happen before any tar message
  // which are scheduled with 0 for the time advance available variants and with 1 for the
  // complete time advance modes.
  // Committed lower bound timestamps are sent with the int set to wether the committed value
  // is meant to work for zero lookahead or not.

  typedef std::pair<LogicalTime, int> LogicalTimePair;

  TemplateTimeManagement(const LogicalTimeFactory& logicalTimeFactory) :
    _timeAdvanceToBeScheduled(false),
    _commitId(-17), // make sure we test wraparound regularily
    _logicalTimeFactory(logicalTimeFactory)
  {
    _logicalTime.first = _logicalTimeFactory.initialLogicalTime();
    _logicalTime.second = 0;
    _pendingLogicalTime = _logicalTime;
    _outboundLowerBoundTimeStamp = _logicalTime;
    _lastOutboundLowerBoundTimeStamp = _logicalTime;
    _committedOutboundLowerBoundTimeStamp = _logicalTime.first;
    _committedNextMessageLowerBoundTimeStamp = _logicalTime.first;
    _currentLookahead = _logicalTimeFactory.zeroLogicalTimeInterval();
    _targetLookahead = _currentLookahead;
  }
  virtual ~TemplateTimeManagement()
  {
    while (!_logicalTimeMessageListMap.empty()) {
      _logicalTimeMessageListMap.begin()->second.clear();
      _logicalTimeMessageListMap.erase(_logicalTimeMessageListMap.begin());
    }
    _receiveOrderMessages.clear();
    _messageListPool.clear();
  }

  /// This is: don't do time advance to the past
  virtual bool isLogicalTimeInThePast(const NativeLogicalTime& logicalTime)
  {
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnabled() || _committedOutboundLowerBoundTimeStamp <= _toLogicalTime(_outboundLowerBoundTimeStamp));
    return logicalTime < _logicalTimeFactory.getLogicalTime(_logicalTime.first);
  }
  /// This is: 'ok to send message with this timestamp'
  virtual bool logicalTimeAlreadyPassed(const NativeLogicalTime& logicalTime)
  {
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnabled() || _committedOutboundLowerBoundTimeStamp <= _toLogicalTime(_outboundLowerBoundTimeStamp));
    if (_outboundLowerBoundTimeStamp.second) {
      return logicalTime <= _logicalTimeFactory.getLogicalTime(_outboundLowerBoundTimeStamp.first);
    } else {
      return logicalTime < _logicalTimeFactory.getLogicalTime(_outboundLowerBoundTimeStamp.first);
    }
  }

  virtual void enableTimeRegulation(InternalAmbassador& ambassador, const NativeLogicalTimeInterval& nativeLookahead)
  {
    LogicalTimeInterval lookahead = _logicalTimeFactory.getLogicalTimeInterval(nativeLookahead);
    _enableTimeRegulation(ambassador, _logicalTime.first, lookahead);
  }

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
    OpenRTIAssert(_logicalTime.first <= logicalTime);
    OpenRTIAssert(_logicalTimeFactory.zeroLogicalTimeInterval() <= lookahead);

    InternalTimeManagement::setTimeRegulationMode(InternalTimeManagement::TimeRegulationEnablePending);

    _currentLookahead = lookahead;
    _targetLookahead = _currentLookahead;

    // Note that (8.1.6) footnote 8 tells us that enable time regulation
    // behaves like the *Available requests wrt allowed sending messages and guarantees
    // So the zero lookahead parameter does not take any effect here!!
    // Also 8.1.4 states that an already existing > restriction is not overruled by
    // a later >= restriction which would again allow for = instead of before.
    // Therefore we need the maximum here.
    _pendingLogicalTime = std::max(_logicalTime, LogicalTimePair(logicalTime, 0));

    _outboundLowerBoundTimeStamp.first = _pendingLogicalTime.first;
    _outboundLowerBoundTimeStamp.first += _currentLookahead;
    _outboundLowerBoundTimeStamp.second = 0;
    _lastOutboundLowerBoundTimeStamp = _outboundLowerBoundTimeStamp;

    // The sent out message effectively commits this
    _committedOutboundLowerBoundTimeStamp = _outboundLowerBoundTimeStamp.first;
    _committedNextMessageLowerBoundTimeStamp = _outboundLowerBoundTimeStamp.first;

    OpenRTIAssert(_timeRegulationEnableFederateHandleTimeStampMap.empty());
    OpenRTIAssert(_timeRegulationEnableFederateHandleSet.empty());

    ambassador.getFederate()->getFederateHandleSet(_timeRegulationEnableFederateHandleSet);
    // Make sure we wait for the request looping back through the root server.
    // We need to do that round trip to the root server to stay in order with newly
    // joined federates that are serialized by the root server.
    _timeRegulationEnableFederateHandleSet.insert(ambassador.getFederate()->getFederateHandle());

    SharedPtr<EnableTimeRegulationRequestMessage> request;
    request = new EnableTimeRegulationRequestMessage;
    request->setFederationHandle(ambassador.getFederate()->getFederationHandle());
    request->setFederateHandle(ambassador.getFederate()->getFederateHandle());
    request->setTimeStamp(_logicalTimeFactory.encodeLogicalTime(_outboundLowerBoundTimeStamp.first));
    request->setCommitId(_commitId);
    ambassador.send(request);

    // Once we get regulating, we need to account for being part of NextMessage deadlock detection.
    // So, ensure that everybody out there knows which commit id we have seen already ...
    _sendCommitLowerBoundTimeStampResponse(ambassador);
  }

  virtual void disableTimeRegulation(InternalAmbassador& ambassador)
  {
    OpenRTIAssert(InternalTimeManagement::getTimeRegulationEnabled());
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnablePending());
    OpenRTIAssert(!InternalTimeManagement::getTimeConstrainedEnablePending());
    OpenRTIAssert(!InternalTimeManagement::getTimeAdvancePending());

    InternalTimeManagement::setTimeRegulationMode(InternalTimeManagement::TimeRegulationDisabled);

    _currentLookahead = _targetLookahead;
    _outboundLowerBoundTimeStamp.first = _logicalTimeFactory.initialLogicalTime();
    _outboundLowerBoundTimeStamp.second = 0;
    _lastOutboundLowerBoundTimeStamp = _outboundLowerBoundTimeStamp;
    _committedOutboundLowerBoundTimeStamp = _outboundLowerBoundTimeStamp.first;
    _committedNextMessageLowerBoundTimeStamp = _outboundLowerBoundTimeStamp.first;

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

    // 8.6 tells that after getting time constrained the federate can receive messages
    // with timestamp == the granted logical time as time stamp order messages.
    // So, since this call introduces contraints on incomming messages, just set second to zero.
    _pendingLogicalTime.first = _logicalTime.first;
    _pendingLogicalTime.second = 0;

    // There are no changes to the bounds for outbound messages

    SharedPtr<TimeConstrainedEnabledMessage> message = new TimeConstrainedEnabledMessage;
    queueTimeStampedMessage(_pendingLogicalTime, *message);
  }

  virtual void disableTimeConstrained(InternalAmbassador& ambassador)
  {
    OpenRTIAssert(InternalTimeManagement::getTimeConstrainedEnabled());
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnablePending());
    OpenRTIAssert(!InternalTimeManagement::getTimeConstrainedEnablePending());
    OpenRTIAssert(!InternalTimeManagement::getTimeAdvancePending());

    InternalTimeManagement::setTimeConstrainedMode(InternalTimeManagement::TimeConstrainedDisabled);
  }

  virtual void timeAdvanceRequest(InternalAmbassador& ambassador, const NativeLogicalTime& nativeLogicalTime, InternalTimeManagement::TimeAdvanceMode timeAdvanceMode)
  {
    // FIXME, delay that into the flush queue logic to get even further
    if (timeAdvanceMode == InternalTimeManagement::FlushQueueRequest)
      ambassador.flushAndDispatchInternalMessage();

    LogicalTime logicalTime = _logicalTimeFactory.getLogicalTime(nativeLogicalTime);
    OpenRTIAssert(timeAdvanceMode != InternalTimeManagement::TimeAdvanceGranted);
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnablePending());
    OpenRTIAssert(!InternalTimeManagement::getTimeConstrainedEnablePending());
    OpenRTIAssert(!InternalTimeManagement::getTimeAdvancePending());
    OpenRTIAssert(_logicalTime.first <= logicalTime);

    // This needs to be set first as this affects the behavior of some called methods below
    InternalTimeManagement::setTimeAdvanceMode(timeAdvanceMode);

    // Determine the required pending logical time to meet our time constrained requirements
    _pendingLogicalTime = std::max(_logicalTime, LogicalTimePair(logicalTime, _getPendingTimeSecondField()));

    // Compute a new outbound logical time - if required
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnabled() || _committedOutboundLowerBoundTimeStamp <= _toLogicalTime(_outboundLowerBoundTimeStamp));
    if (InternalTimeManagement::getTimeRegulationEnabled()) {
      // Store the old value, we need that at least for next message modes later
      _lastOutboundLowerBoundTimeStamp = _outboundLowerBoundTimeStamp;
      _setOutboundLowerTimeStampAndCurrentLookahead(logicalTime, _lastOutboundLowerBoundTimeStamp);
    }
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnabled() || _committedOutboundLowerBoundTimeStamp <= _toLogicalTime(_outboundLowerBoundTimeStamp));

    OpenRTIAssert(!_timeAdvanceToBeScheduled);

    if (InternalTimeManagement::getIsAnyAdvanceRequest()) {
      if (InternalTimeManagement::getTimeRegulationEnabled())
        _sendCommitLowerBoundTimeStampIfChanged(ambassador, _outboundLowerBoundTimeStamp, TimeAdvanceAndNextMessageCommit);

      SharedPtr<TimeAdvanceGrantedMessage> message = new TimeAdvanceGrantedMessage;
      queueTimeStampedMessage(_pendingLogicalTime, *message);

    } else if (InternalTimeManagement::getIsAnyNextMessageMode()) {
      if (InternalTimeManagement::getTimeConstrainedEnabled()) {
        if (!_logicalTimeMessageListMap.empty()) {
          if (_logicalTimeMessageListMap.begin()->first.first < _pendingLogicalTime.first) {
            OpenRTIAssert(_logicalTime.first <= _logicalTimeMessageListMap.begin()->first.first);
            _pendingLogicalTime.first = _logicalTimeMessageListMap.begin()->first.first;
          }
        }

        _timeAdvanceToBeScheduled = true;
      }

      if (InternalTimeManagement::getTimeRegulationEnabled() && _timeAdvanceToBeScheduled) {
        _sendCommitLowerBoundTimeStampIfChanged(ambassador, _pendingLogicalTime.first, _targetLookahead, NextMessageCommit);
      }

      if (InternalTimeManagement::getTimeConstrainedEnabled()) {
        checkForPendingTimeAdvance(ambassador, false);
      } else {
        SharedPtr<TimeAdvanceGrantedMessage> message = new TimeAdvanceGrantedMessage;
        queueTimeStampedMessage(_pendingLogicalTime, *message);
      }
    } else if (InternalTimeManagement::getFlushQueueMode()) {
      if (InternalTimeManagement::getTimeConstrainedEnabled()) {
        if (!_logicalTimeMessageListMap.empty()) {
          if (_logicalTimeMessageListMap.begin()->first.first < _pendingLogicalTime.first) {
            OpenRTIAssert(_logicalTime.first <= _logicalTimeMessageListMap.begin()->first.first);
            _pendingLogicalTime.first = _logicalTimeMessageListMap.begin()->first.first;
          }
        }
      }

      if (InternalTimeManagement::getTimeConstrainedEnabled() && !canAdvanceTo(LogicalTimePair(_pendingLogicalTime.first, 0))) {
        OpenRTIAssert(!_federateLowerBoundMap.empty());
        _pendingLogicalTime.first = _federateLowerBoundMap.getGALT();
        OpenRTIAssert(_pendingLogicalTime.second == 0);
        _pendingLogicalTime = std::max(_logicalTime, _pendingLogicalTime);
      }

      if (InternalTimeManagement::getTimeRegulationEnabled()) {
        LogicalTime logicalTime(_pendingLogicalTime.first);
        logicalTime += _targetLookahead;
        logicalTime = std::max(logicalTime, _committedOutboundLowerBoundTimeStamp);
        _sendCommitLowerBoundTimeStampIfChanged(ambassador, logicalTime, TimeAdvanceAndNextMessageCommit);
      }

      if (InternalTimeManagement::getTimeConstrainedEnabled()) {
        _timeAdvanceToBeScheduled = true;
      } else {
        SharedPtr<TimeAdvanceGrantedMessage> message = new TimeAdvanceGrantedMessage;
        queueTimeStampedMessage(_pendingLogicalTime, *message);
      }

    } else {
      OpenRTIAssert(!"Undefined time advance mode!");
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
    logicalTime = _logicalTimeFactory.getLogicalTime(_logicalTime.first);
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

    _targetLookahead = _logicalTimeFactory.getLogicalTimeInterval(nativeLookahead);

    OpenRTIAssert(_lastOutboundLowerBoundTimeStamp == _outboundLowerBoundTimeStamp);
    _lastOutboundLowerBoundTimeStamp = _outboundLowerBoundTimeStamp;
    _setOutboundLowerTimeStampAndCurrentLookahead(_logicalTime.first, _lastOutboundLowerBoundTimeStamp);

    _sendCommitLowerBoundTimeStampIfChanged(ambassador, _outboundLowerBoundTimeStamp, TimeAdvanceAndNextMessageCommit);
  }

  virtual void queryLookahead(InternalAmbassador& ambassador, NativeLogicalTimeInterval& logicalTimeInterval)
  {
    logicalTimeInterval = _logicalTimeFactory.getLogicalTimeInterval(_currentLookahead);
  }

  virtual std::string logicalTimeToString(const NativeLogicalTime& nativeLogicalTime)
  {
    return _logicalTimeFactory.toString(nativeLogicalTime);
  }
  std::string logicalTimeToString(const LogicalTime& logicalTime)
  {
    return logicalTimeToString(_logicalTimeFactory.getLogicalTime(logicalTime));
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
    // When we have already received the root servers response, the new federate just gets the information that we are
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
        Log(Network, Warning) << "Received own EnableTimeRegulationRequestMessage without waiting for that to happen!" << std::endl;
        return;
      }
      _timeRegulationEnableFederateHandleSet.erase(ambassador.getFederate()->getFederateHandle());

      // This one checks if we are the last one this ambassador is waiting for
      // and if so, queues the callback and informs the other federates about our
      // new logical time
      checkTimeRegulationEnabled(ambassador);

    } else {
      LogicalTime logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getTimeStamp());

      // If we are in the state of a fresh joined federate which is still collecting initial information
      // we should skip sending the response.
      if (ambassador.getFederate()->getFederateHandle().valid()) {
        if (InternalTimeManagement::getTimeConstrainedEnabledOrPending()) {
          // The originating ambassador sends an initial proposal for the logical time,
          // We respond with a corrected logical time if this proposal is not sufficient

          // Note that the below already works for next message mode, when the time
          // advance is already scheduled. Then we need to respond with the time that we
          // have already guaranteed to the federate to be the only next message time.
          if (0 < _pendingLogicalTime.second) {
            if (logicalTime <= _pendingLogicalTime.first) {
              logicalTime = LogicalTimeFactory::nextAfter(_pendingLogicalTime.first);
            }
          } else {
            if (logicalTime < _pendingLogicalTime.first)
              logicalTime = _pendingLogicalTime.first;
          }
        }

        SharedPtr<EnableTimeRegulationResponseMessage> response;
        response = new EnableTimeRegulationResponseMessage;
        response->setFederationHandle(ambassador.getFederate()->getFederationHandle());
        response->setFederateHandle(message.getFederateHandle());
        response->setRespondingFederateHandle(ambassador.getFederate()->getFederateHandle());
        response->setTimeStamp(_logicalTimeFactory.encodeLogicalTime(logicalTime));
        response->setTimeStampValid(InternalTimeManagement::getTimeConstrainedEnabledOrPending());
        ambassador.send(response);
      }

      bool previousLockedByNextMessage = getLockedByNextMessage();

      _federateLowerBoundMap.insert(message.getFederateHandle(), logicalTime, logicalTime, message.getCommitId(), _commitId - 1);

      bool lockedByNextMessage = getLockedByNextMessage();
      if (previousLockedByNextMessage != lockedByNextMessage) {
        _sendLockedByNextMessageRequest(ambassador, lockedByNextMessage);
      }

      OpenRTIAssert(!_federateLowerBoundMap.empty());
    }
  }
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const EnableTimeRegulationResponseMessage& message)
  {
    if (!InternalTimeManagement::getTimeRegulationEnablePending()) {
      Log(Network, Warning) << "Received EnableTimeRegulationResponseMessage without waiting for that to happen!" << std::endl;
      return;
    }

    bool previousLockedByNextMessage = getLockedByNextMessage();

    if (message.getTimeStampValid()) {
      LogicalTime logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getTimeStamp());
      _timeRegulationEnableFederateHandleTimeStampMap[message.getRespondingFederateHandle()] = logicalTime;
    }

    _timeRegulationEnableFederateHandleSet.erase(message.getRespondingFederateHandle());

    if (previousLockedByNextMessage != getLockedByNextMessage()) {
      _sendLockedByNextMessageRequest(ambassador, getLockedByNextMessage());
    }

    // This one checks if we are the last one this ambassador is waiting for
    // and if so, queues the callback and informs the other federates about our
    // new logical time
    checkTimeRegulationEnabled(ambassador);
  }
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const DisableTimeRegulationRequestMessage& message)
  {
    bool previousLockedByNextMessage = getLockedByNextMessage();

    _federateLowerBoundMap.erase(message.getFederateHandle());

    bool lockedByNextMessage = getLockedByNextMessage();
    if (previousLockedByNextMessage != lockedByNextMessage) {
      _sendLockedByNextMessageRequest(ambassador, lockedByNextMessage);
    }

    checkForPendingTimeAdvance(ambassador);
  }

  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const CommitLowerBoundTimeStampMessage& message)
  {
    LogicalTime logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getTimeStamp());

    bool previousLockedByNextMessage = getLockedByNextMessage();

    std::pair<bool, bool> changePair = _federateLowerBoundMap.commit(message.getFederateHandle(), logicalTime, message.getCommitType(), message.getCommitId());

    if (changePair.second)
      _sendCommitLowerBoundTimeStampResponse(ambassador, message.getFederateHandle(), message.getCommitId());

    bool lockedByNextMessage = getLockedByNextMessage();
    if (previousLockedByNextMessage != lockedByNextMessage) {
      _sendLockedByNextMessageRequest(ambassador, lockedByNextMessage);
    }

    checkForPendingTimeAdvance(ambassador);
  }
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const CommitLowerBoundTimeStampResponseMessage& message)
  {
    bool previousLockedByNextMessage = getLockedByNextMessage();

    _federateLowerBoundMap.setFederateWaitCommitId(message.getSendingFederateHandle(), message.getCommitId());

    bool lockedByNextMessage = getLockedByNextMessage();
    if (previousLockedByNextMessage != lockedByNextMessage) {
      _sendLockedByNextMessageRequest(ambassador, lockedByNextMessage);
    }

    checkForPendingTimeAdvance(ambassador);
  }
  virtual void acceptInternalMessage(InternalAmbassador& ambassador, const LockedByNextMessageRequestMessage& message)
  {
    bool previousLockedByNextMessage = getLockedByNextMessage();

    _federateLowerBoundMap.setFederateLockedByNextMessage(message.getSendingFederateHandle(), message.getLockedByNextMessage());

    bool lockedByNextMessage = getLockedByNextMessage();
    if (previousLockedByNextMessage != lockedByNextMessage) {
      _sendLockedByNextMessageRequest(ambassador, lockedByNextMessage);
    }

    checkForPendingTimeAdvance(ambassador);
  }

  virtual void queueTimeStampedMessage(InternalAmbassador& ambassador, const VariableLengthData& timeStamp, const AbstractMessage& message)
  {
    queueTimeStampedMessage(ambassador, _logicalTimeFactory.decodeLogicalTime(timeStamp), message);
  }
  void queueTimeStampedMessage(InternalAmbassador& ambassador, const LogicalTime& logicalTime, const AbstractMessage& message)
  {
    // Well potentially we can savely skip both tests since only a buggy federate can send this messages
    // under these circumstance. But since we cannot rely on this on a public network, we are kind and
    // drop these messages.
    // Except in debug mode, where we want the test to fail under this conditions!
    if (InternalTimeManagement::getTimeConstrainedEnabled() && canAdvanceTo(LogicalTimePair(logicalTime, 1))) {
      Log(Network, Warning) << "Dropping illegal time stamp order message!\n"
                            << "You may communicate with a buggy federate ambassador." << std::endl;
      OpenRTIAssert(!"Received illegal timestamp order message!");
      return;
    }
    if (InternalTimeManagement::getTimeConstrainedEnabled() && LogicalTimePair(logicalTime, 1) < _logicalTime) {
      Log(Network, Warning) << "Dropping illegal time stamp order message!\n"
                            << "You may communicate with a buggy federate ambassador." << std::endl;
      OpenRTIAssert(!"Received illegal timestamp order message!");
      return;
    }
    queueTimeStampedMessage(ambassador, LogicalTimePair(logicalTime, -1), message);
  }
  void queueTimeStampedMessage(InternalAmbassador& ambassador, const LogicalTimePair& logicalTimePair, const AbstractMessage& message)
  {
    queueTimeStampedMessage(logicalTimePair, message);
    if (InternalTimeManagement::getIsAnyNextMessageMode() && InternalTimeManagement::getTimeConstrainedEnabled()) {
      if (logicalTimePair.first < _pendingLogicalTime.first) {
        OpenRTIAssert(_timeAdvanceToBeScheduled);
        OpenRTIAssert(!InternalTimeManagement::getTimeConstrainedEnabled() || _logicalTime.first <= logicalTimePair.first);
        if (_logicalTime.first <= logicalTimePair.first)
          _pendingLogicalTime.first = logicalTimePair.first;

        if (_timeAdvanceToBeScheduled && InternalTimeManagement::getTimeRegulationEnabled()) {
          _sendCommitLowerBoundTimeStampIfChanged(ambassador, _pendingLogicalTime.first, _targetLookahead, NextMessageCommit);
        }

        checkForPendingTimeAdvance(ambassador);

        OpenRTIAssert(!InternalTimeManagement::getTimeConstrainedEnabled() || _pendingLogicalTime.first <= _logicalTimeMessageListMap.begin()->first.first);
      }
    }
  }
  void queueTimeStampedMessage(const LogicalTimePair& logicalTimePair, const AbstractMessage& message)
  {
    if (_messageListPool.empty()) {
      _MessageListElement* messageListElement = new _MessageListElement(&message);
      _logicalTimeMessageListMap[logicalTimePair].push_back(*messageListElement);
      if (message.getObjectInstanceHandleForMessage().valid()) {
        messageListElement->setObjectInstanceHandle(message.getObjectInstanceHandleForMessage());
        _objectInstanceHandleMessageListElementMap.insert(*messageListElement);
      }
    } else {
      _MessageListElement& messageListElement = _messageListPool.front();
      messageListElement.unlink();
      messageListElement._message = &message;
      _logicalTimeMessageListMap[logicalTimePair].push_back(messageListElement);
      if (message.getObjectInstanceHandleForMessage().valid()) {
        messageListElement.setObjectInstanceHandle(message.getObjectInstanceHandleForMessage());
        _objectInstanceHandleMessageListElementMap.insert(messageListElement);
      }
    }
  }
  virtual void queueReceiveOrderMessage(InternalAmbassador& ambassador, const AbstractMessage& message)
  {
    if (_messageListPool.empty()) {
      _MessageListElement* messageListElement = new _MessageListElement(&message);
      _receiveOrderMessages.push_back(*messageListElement);
      if (message.getObjectInstanceHandleForMessage().valid()) {
        messageListElement->setObjectInstanceHandle(message.getObjectInstanceHandleForMessage());
        _objectInstanceHandleMessageListElementMap.insert(*messageListElement);
      }
    } else {
      _MessageListElement& messageListElement = _messageListPool.front();
      messageListElement.unlink();
      messageListElement._message = &message;
      _receiveOrderMessages.push_back(messageListElement);
      if (message.getObjectInstanceHandleForMessage().valid()) {
        messageListElement.setObjectInstanceHandle(message.getObjectInstanceHandleForMessage());
        _objectInstanceHandleMessageListElementMap.insert(messageListElement);
      }
    }
  }

  void removeFederateFromTimeManagement(InternalAmbassador& ambassador, const FederateHandle& federateHandle)
  {
    bool previousLockedByNextMessage = getLockedByNextMessage();

    _federateLowerBoundMap.erase(federateHandle);
    _timeRegulationEnableFederateHandleTimeStampMap.erase(federateHandle);
    FederateHandleSet::iterator i = _timeRegulationEnableFederateHandleSet.find(federateHandle);
    if (i != _timeRegulationEnableFederateHandleSet.end()) {
      _timeRegulationEnableFederateHandleSet.erase(i);

      // Now we should really know that we had time regulation enabled
      OpenRTIAssert(InternalTimeManagement::getTimeRegulationEnablePending());

      // This one checks if we are the last one this ambassador is waiting for
      // and if so, queues the callback and informs the other federates about our
      // new logical time
      checkTimeRegulationEnabled(ambassador);
    }

    bool lockedByNextMessage = getLockedByNextMessage();
    if (previousLockedByNextMessage != lockedByNextMessage) {
      _sendLockedByNextMessageRequest(ambassador, lockedByNextMessage);
    }

    checkForPendingTimeAdvance(ambassador);
  }

  void checkTimeRegulationEnabled(InternalAmbassador& ambassador)
  {
    if (!InternalTimeManagement::getTimeRegulationEnablePending())
      return;
    // Check if that was the last one we were waiting for
    if (!_timeRegulationEnableFederateHandleSet.empty())
      return;

    // Now, collect all logical times that other federates sent to us
    for (typename FederateHandleTimeStampMap::const_iterator i = _timeRegulationEnableFederateHandleTimeStampMap.begin();
         i != _timeRegulationEnableFederateHandleTimeStampMap.end(); ++i) {
      // This federate sent us just its logical time of the next acceptable message time.
      // That means we need to make sure that all messages we send are strictly
      // in the future wrt this sent time stamp.
      LogicalTime logicalTime = i->second;
      // Since time regulation enable pending *defines* this to be 0 we can assume this below!!
      OpenRTIAssert(_outboundLowerBoundTimeStamp.second == 0);
      if (logicalTime <= _outboundLowerBoundTimeStamp.first)
        continue;

      _outboundLowerBoundTimeStamp.first = logicalTime;
      _outboundLowerBoundTimeStamp.second = 0;
      logicalTime -= _currentLookahead;
      _pendingLogicalTime = std::max(_pendingLogicalTime, LogicalTimePair(logicalTime, 0));
    }

    _timeRegulationEnableFederateHandleTimeStampMap.clear();

    // If somebody has corrected the logical time, then there might be several
    // federates who have a too little committed time, so tell all about them
    OpenRTIAssert(_outboundLowerBoundTimeStamp.second == 0);
    _sendCommitLowerBoundTimeStamp(ambassador, _outboundLowerBoundTimeStamp.first, TimeAdvanceAndNextMessageCommit);

    // Ok, now go on ...
    SharedPtr<TimeRegulationEnabledMessage> message = new TimeRegulationEnabledMessage;
    queueTimeStampedMessage(_pendingLogicalTime, *message);
  }

  void checkForPendingTimeAdvance(InternalAmbassador& ambassador, bool allowNextMessage = true)
  {
    if (!_timeAdvanceToBeScheduled)
      return;
    if (InternalTimeManagement::getIsAnyNextMessageMode()) {

      OpenRTIAssert(!InternalTimeManagement::getTimeConstrainedEnabled() || _logicalTimeMessageListMap.empty() || _pendingLogicalTime.first <= _logicalTimeMessageListMap.begin()->first.first);

      if (allowNextMessage && getIsSaveToAdvanceToNextMessage()) {
        if (canAdvanceToNextMessage(LogicalTimePair(_pendingLogicalTime.first, 0))) {
          _timeAdvanceToBeScheduled = false;
          SharedPtr<TimeAdvanceGrantedMessage> message = new TimeAdvanceGrantedMessage;
          queueTimeStampedMessage(_pendingLogicalTime, *message);

          if (InternalTimeManagement::getTimeRegulationEnabled()) {
            _sendCommitLowerBoundTimeStampIfChanged(ambassador, _pendingLogicalTime.first, _targetLookahead, TimeAdvanceAndNextMessageCommit);
          }
        } else if (InternalTimeManagement::getTimeRegulationEnabled()) {
          OpenRTIAssert(!_federateLowerBoundMap.empty());
          OpenRTIAssert(_federateLowerBoundMap.getNextMessageGALT() <= _pendingLogicalTime.first);
          _sendCommitLowerBoundTimeStampIfChanged(ambassador, _federateLowerBoundMap.getNextMessageGALT(), _targetLookahead, TimeAdvanceCommit);
        }
      } else {
        if (canAdvanceTo(LogicalTimePair(_pendingLogicalTime.first, 0))) {
          _timeAdvanceToBeScheduled = false;
          SharedPtr<TimeAdvanceGrantedMessage> message = new TimeAdvanceGrantedMessage;
          queueTimeStampedMessage(_pendingLogicalTime, *message);

          if (InternalTimeManagement::getTimeRegulationEnabled()) {
            _sendCommitLowerBoundTimeStampIfChanged(ambassador, _pendingLogicalTime.first, _targetLookahead, TimeAdvanceAndNextMessageCommit);
          }
        } else if (InternalTimeManagement::getTimeRegulationEnabled()) {
          OpenRTIAssert(!_federateLowerBoundMap.empty());
          OpenRTIAssert(_federateLowerBoundMap.getGALT() <= _pendingLogicalTime.first);
          _sendCommitLowerBoundTimeStampIfChanged(ambassador, _federateLowerBoundMap.getGALT(), _targetLookahead, TimeAdvanceCommit);
        }
      }
    } else {
      OpenRTIAssert(InternalTimeManagement::getFlushQueueMode());
    }
  }

  void checkForPendingFlushQueue()
  {
    if (_logicalTimeMessageListMap.empty() && InternalTimeManagement::getFlushQueueMode() && _timeAdvanceToBeScheduled) {
      OpenRTIAssert(!InternalTimeManagement::getTimeConstrainedEnabled() || canAdvanceTo(_pendingLogicalTime));
      _timeAdvanceToBeScheduled = false;
      SharedPtr<TimeAdvanceGrantedMessage> message = new TimeAdvanceGrantedMessage;
      queueTimeStampedMessage(_pendingLogicalTime, *message);
    }
  }

  bool canAdvanceTo(const LogicalTimePair& logicalTimePair) const
  {
    return _federateLowerBoundMap.canAdvanceTo(logicalTimePair);
  }
  bool canAdvanceToNextMessage(const LogicalTimePair& logicalTimePair) const
  {
    return _federateLowerBoundMap.canAdvanceToNextMessage(logicalTimePair);
  }
  bool getLockedByNextMessage() const
  {
    if (!InternalTimeManagement::getTimeConstrainedEnabled())
      return false;
    if (!InternalTimeManagement::getIsAnyNextMessageMode())
      return false;
    return _federateLowerBoundMap.getLockedByNextMessage(_commitId);
  }
  bool getIsSaveToAdvanceToNextMessage() const
  {
    if (!InternalTimeManagement::getIsAnyNextMessageMode())
      return false;
    return _federateLowerBoundMap.getIsSaveToAdvanceToNextMessage(_commitId);
  }

  void _sendCommitLowerBoundTimeStampIfChanged(InternalAmbassador& ambassador, const LogicalTimePair& logicalTimePair, LowerBoundTimeStampCommitType commitType)
  {
    if (0 < logicalTimePair.second)
      _sendCommitLowerBoundTimeStampIfChanged(ambassador, LogicalTimeFactory::nextAfter(logicalTimePair.first), commitType);
    else
      _sendCommitLowerBoundTimeStampIfChanged(ambassador, logicalTimePair.first, commitType);
  }
  void _sendCommitLowerBoundTimeStampIfChanged(InternalAmbassador& ambassador, const LogicalTime& logicalTime, const LogicalTimeInterval& lookahead, LowerBoundTimeStampCommitType commitType)
  {
    LogicalTime logicalTimePlusLookahead(logicalTime);
    logicalTimePlusLookahead += lookahead;
    if (_getOutboundLowerTimeStampSecondField(lookahead))
      logicalTimePlusLookahead = LogicalTimeFactory::nextAfter(logicalTimePlusLookahead);
    logicalTimePlusLookahead = std::max(logicalTimePlusLookahead, _committedOutboundLowerBoundTimeStamp);
    _sendCommitLowerBoundTimeStampIfChanged(ambassador, logicalTimePlusLookahead, commitType);
  }
  void _sendCommitLowerBoundTimeStampIfChanged(InternalAmbassador& ambassador, const LogicalTime& logicalTime, LowerBoundTimeStampCommitType commitType)
  {
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationDisabled());
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnabled() || _committedOutboundLowerBoundTimeStamp <= _toLogicalTime(_outboundLowerBoundTimeStamp));
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnabled() || _committedOutboundLowerBoundTimeStamp <= logicalTime);
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnabled() || logicalTime <= _toLogicalTime(_outboundLowerBoundTimeStamp));
    if (logicalTime < _committedOutboundLowerBoundTimeStamp)
      return;
    if (commitType & TimeAdvanceCommit) {
      if (_committedOutboundLowerBoundTimeStamp < logicalTime) {
      } else {
        commitType = LowerBoundTimeStampCommitType(commitType & ~unsigned(TimeAdvanceCommit));
      }
    }
    if (commitType & NextMessageCommit) {
      if (_committedNextMessageLowerBoundTimeStamp != logicalTime) {
      } else {
        commitType = LowerBoundTimeStampCommitType(commitType & ~unsigned(NextMessageCommit));
      }
    }
    if (!commitType)
      return;
    _sendCommitLowerBoundTimeStamp(ambassador, logicalTime, commitType);
  }

  void _sendCommitLowerBoundTimeStamp(InternalAmbassador& ambassador, const LogicalTime& logicalTime, LowerBoundTimeStampCommitType commitType)
  {
    OpenRTIAssert(0 != commitType);
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationDisabled());
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnabled() || _committedOutboundLowerBoundTimeStamp <= _toLogicalTime(_outboundLowerBoundTimeStamp));
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnabled() || _committedOutboundLowerBoundTimeStamp <= _committedNextMessageLowerBoundTimeStamp);
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnabled() || _committedNextMessageLowerBoundTimeStamp <= _toLogicalTime(_outboundLowerBoundTimeStamp));
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnabled() || _committedOutboundLowerBoundTimeStamp <= logicalTime);
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnabled() || logicalTime <= _toLogicalTime(_outboundLowerBoundTimeStamp));

    if (commitType & TimeAdvanceCommit)
      _committedOutboundLowerBoundTimeStamp = logicalTime;
    if (commitType & NextMessageCommit) {
      _committedNextMessageLowerBoundTimeStamp = logicalTime;
      if (_committedOutboundLowerBoundTimeStamp != _committedNextMessageLowerBoundTimeStamp)
        ++_commitId;
    }

    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnabled() || _committedOutboundLowerBoundTimeStamp <= _toLogicalTime(_outboundLowerBoundTimeStamp));
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnabled() || _committedOutboundLowerBoundTimeStamp <= _committedNextMessageLowerBoundTimeStamp);
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnabled() || _committedNextMessageLowerBoundTimeStamp <= _toLogicalTime(_outboundLowerBoundTimeStamp));

    SharedPtr<CommitLowerBoundTimeStampMessage> request;
    request = new CommitLowerBoundTimeStampMessage;
    request->setFederationHandle(ambassador.getFederate()->getFederationHandle());
    request->setFederateHandle(ambassador.getFederate()->getFederateHandle());
    request->setTimeStamp(_logicalTimeFactory.encodeLogicalTime(logicalTime));
    request->setCommitType(commitType);
    request->setCommitId(_commitId);
    ambassador.send(request);
  }

  void _sendCommitLowerBoundTimeStampResponse(InternalAmbassador& ambassador)
  {
    typedef std::list<std::pair<FederateHandle, Unsigned> > FederateHandleCommitIdList;
    FederateHandleCommitIdList federateHandleCommitIdList;
    _federateLowerBoundMap.getNextMessageFederateHandleList(federateHandleCommitIdList);
    for (FederateHandleCommitIdList::const_iterator i = federateHandleCommitIdList.begin(); i != federateHandleCommitIdList.end(); ++i) {
      _sendCommitLowerBoundTimeStampResponse(ambassador, i->first, i->second);
    }
  }

  void _sendCommitLowerBoundTimeStampResponse(InternalAmbassador& ambassador, const FederateHandle& federateHandle, const Unsigned& commitId)
  {
    if (!ambassador.getFederate()->getFederateHandle().valid())
      return;
    OpenRTIAssert(_federateLowerBoundMap.getFederateIsInNextMessageModeForAssert(federateHandle));

    SharedPtr<CommitLowerBoundTimeStampResponseMessage> request;
    request = new CommitLowerBoundTimeStampResponseMessage;
    request->setFederationHandle(ambassador.getFederate()->getFederationHandle());
    request->setFederateHandle(federateHandle);
    request->setCommitId(commitId);
    request->setSendingFederateHandle(ambassador.getFederate()->getFederateHandle());
    ambassador.send(request);
  }

  void _sendLockedByNextMessageRequest(InternalAmbassador& ambassador, bool lockedByNextMessage)
  {
    SharedPtr<LockedByNextMessageRequestMessage> request;
    request = new LockedByNextMessageRequestMessage;
    request->setFederationHandle(ambassador.getFederate()->getFederationHandle());
    request->setLockedByNextMessage(lockedByNextMessage);
    request->setSendingFederateHandle(ambassador.getFederate()->getFederateHandle());
    ambassador.send(request);
  }

  int _getPendingTimeSecondField() const
  {
    if (!InternalTimeManagement::getTimeConstrainedEnabled())
      return 0;
    if (InternalTimeManagement::getIsAnyAvailableMode())
      return 0;
    return 1;
  }

  int _getOutboundLowerTimeStampSecondField(const LogicalTimeInterval& lookahead)
  {
    if (!InternalTimeManagement::getTimeRegulationEnabled())
      return 0;
    if (InternalTimeManagement::getIsAnyAvailableMode())
      return 0;
    return _logicalTimeFactory.isZeroTimeInterval(lookahead);
  }

  void _setOutboundLowerTimeStampAndCurrentLookahead(const LogicalTime& logicalTime, const LogicalTimePair& lastOutboundLowerBoundTimeStamp)
  {
    _outboundLowerBoundTimeStamp.first = logicalTime;
    _outboundLowerBoundTimeStamp.first += _targetLookahead;
    _outboundLowerBoundTimeStamp.second = _getOutboundLowerTimeStampSecondField(_targetLookahead);
    if (_outboundLowerBoundTimeStamp < lastOutboundLowerBoundTimeStamp) {
      _currentLookahead = std::min(lastOutboundLowerBoundTimeStamp.first - logicalTime, _targetLookahead);
      _outboundLowerBoundTimeStamp = lastOutboundLowerBoundTimeStamp;
    } else {
      _currentLookahead = _targetLookahead;
    }
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnabled() || _committedOutboundLowerBoundTimeStamp <= _toLogicalTime(_outboundLowerBoundTimeStamp));
    OpenRTIAssert(_targetLookahead <= _currentLookahead);
  }

  virtual void acceptCallbackMessage(Ambassador<T>& ambassador, const TimeConstrainedEnabledMessage& message)
  {
    OpenRTIAssert(InternalTimeManagement::getTimeConstrainedEnablePending());
    OpenRTIAssert(canAdvanceTo(_pendingLogicalTime));
    OpenRTIAssert(_logicalTimeMessageListMap.empty() || _pendingLogicalTime <= _logicalTimeMessageListMap.begin()->first);
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnabled() || _committedOutboundLowerBoundTimeStamp <= _toLogicalTime(_outboundLowerBoundTimeStamp));

    InternalTimeManagement::setTimeConstrainedMode(InternalTimeManagement::TimeConstrainedEnabled);
    _logicalTime = _pendingLogicalTime;
    ambassador.timeConstrainedEnabled(_logicalTimeFactory.getLogicalTime(_logicalTime.first));
  }
  virtual void acceptCallbackMessage(Ambassador<T>& ambassador, const TimeRegulationEnabledMessage& message)
  {
    OpenRTIAssert(InternalTimeManagement::getTimeRegulationEnablePending());
    OpenRTIAssert(!InternalTimeManagement::getTimeConstrainedEnabled() || canAdvanceTo(_pendingLogicalTime));
    OpenRTIAssert(!InternalTimeManagement::getTimeConstrainedEnabled() || _logicalTimeMessageListMap.empty() || _pendingLogicalTime <= _logicalTimeMessageListMap.begin()->first);
    OpenRTIAssert(_committedOutboundLowerBoundTimeStamp <= _toLogicalTime(_outboundLowerBoundTimeStamp));

    InternalTimeManagement::setTimeRegulationMode(InternalTimeManagement::TimeRegulationEnabled);
    _logicalTime = _pendingLogicalTime;
    // Note that the _outboundLowerBoundTimeStamp is already eactly computed when
    // the regulation responses are evaluated. This way we can take the exact bound value
    // here instead of the one past two times roundoff.
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnabled() || _committedOutboundLowerBoundTimeStamp <= _toLogicalTime(_outboundLowerBoundTimeStamp));
    ambassador.timeRegulationEnabled(_logicalTimeFactory.getLogicalTime(_logicalTime.first));
  }
  virtual void acceptCallbackMessage(Ambassador<T>& ambassador, const TimeAdvanceGrantedMessage& message)
  {
    OpenRTIAssert(InternalTimeManagement::getTimeAdvancePending());
    OpenRTIAssert(!InternalTimeManagement::getTimeConstrainedEnabled() || canAdvanceTo(_pendingLogicalTime));
    OpenRTIAssert(!InternalTimeManagement::getTimeConstrainedEnabled() || _logicalTimeMessageListMap.empty() || _pendingLogicalTime <= _logicalTimeMessageListMap.begin()->first);
    OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnabled() || _committedOutboundLowerBoundTimeStamp <= _toLogicalTime(_outboundLowerBoundTimeStamp));
    OpenRTIAssert(!_timeAdvanceToBeScheduled);

    // In next message mode we may need to correct back the expectations about maximum allowed message time
    if ((InternalTimeManagement::getIsAnyNextMessageMode() || InternalTimeManagement::getFlushQueueMode())&& InternalTimeManagement::getTimeRegulationEnabled()) {
      _setOutboundLowerTimeStampAndCurrentLookahead(_pendingLogicalTime.first, _lastOutboundLowerBoundTimeStamp);
      OpenRTIAssert(!InternalTimeManagement::getTimeRegulationEnabled() || _committedOutboundLowerBoundTimeStamp <= _toLogicalTime(_outboundLowerBoundTimeStamp));
    }

    _logicalTime = _pendingLogicalTime;
    InternalTimeManagement::setTimeAdvanceMode(InternalTimeManagement::TimeAdvanceGranted);
    ambassador.timeAdvanceGrant(_logicalTimeFactory.getLogicalTime(_logicalTime.first));
  }

  virtual void reflectAttributeValues(Ambassador<T>& ambassador, const Federate::ObjectClass& objectClass,
                                      const TimeStampedAttributeUpdateMessage& message)
  {
    OrderType orderType = InternalTimeManagement::getTimeStampOrderDelivery(message.getOrderType());
    LogicalTime logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getTimeStamp());
    OpenRTIAssert(orderType == RECEIVE || _logicalTime <= LogicalTimePair(logicalTime, 0));
    if (InternalTimeManagement::getFlushQueueMode()) {
      ambassador.reflectAttributeValues(objectClass, message.getObjectInstanceHandle(), message.getAttributeValues(),
                                        message.getTag(), message.getOrderType(), message.getTransportationType(),
                                        _logicalTimeFactory.getLogicalTime(logicalTime), orderType, message.getFederateHandle(),
                                        message.getMessageRetractionHandle());
    } else {
      ambassador.reflectAttributeValues(objectClass, message.getObjectInstanceHandle(), message.getAttributeValues(),
                                        message.getTag(), message.getOrderType(), message.getTransportationType(),
                                        _logicalTimeFactory.getLogicalTime(logicalTime), orderType, message.getFederateHandle());
    }
  }

  virtual void removeObjectInstance(Ambassador<T>& ambassador, const TimeStampedDeleteObjectInstanceMessage& message)
  {
    OrderType orderType = InternalTimeManagement::getTimeStampOrderDelivery(message.getOrderType());
    LogicalTime logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getTimeStamp());
    OpenRTIAssert(orderType == RECEIVE || _logicalTime <= LogicalTimePair(logicalTime, 0));
    if (InternalTimeManagement::getFlushQueueMode()) {
      ambassador.removeObjectInstance(message.getObjectInstanceHandle(), message.getTag(), message.getOrderType(),
                                      _logicalTimeFactory.getLogicalTime(logicalTime), orderType, message.getFederateHandle(),
                                      message.getMessageRetractionHandle());
    } else {
      ambassador.removeObjectInstance(message.getObjectInstanceHandle(), message.getTag(), message.getOrderType(),
                                      _logicalTimeFactory.getLogicalTime(logicalTime), orderType, message.getFederateHandle());
    }
  }

  virtual void eraseMessagesForObjectInstance(Ambassador<T>& ambassador, const ObjectInstanceHandle& objectInstanceHandle)
  {
    // Remove messages targeting this object instance from the pending messages
    typename _ObjectInstanceHandleMessageListElementMap::iterator i;
    i = _objectInstanceHandleMessageListElementMap.find(objectInstanceHandle);
    while (i != _objectInstanceHandleMessageListElementMap.end() && i->getObjectInstanceHandle() == objectInstanceHandle) {
      _MessageListElement& messageListElement = *i++;
      messageListElement.unlink();
      messageListElement._message.clear();
      _messageListPool.push_back(messageListElement);
    }
  }

  virtual void receiveInteraction(Ambassador<T>& ambassador, const Federate::InteractionClass& interactionClass,
                                  const InteractionClassHandle& interactionClassHandle, const TimeStampedInteractionMessage& message)
  {
    OrderType orderType = InternalTimeManagement::getTimeStampOrderDelivery(message.getOrderType());
    LogicalTime logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getTimeStamp());
    OpenRTIAssert(orderType == RECEIVE || _logicalTime <= LogicalTimePair(logicalTime, 0));
    if (InternalTimeManagement::getFlushQueueMode()) {
      ambassador.receiveInteraction(interactionClass, interactionClassHandle, message.getParameterValues(),
                                    message.getTag(), message.getOrderType(), message.getTransportationType(),
                                    _logicalTimeFactory.getLogicalTime(logicalTime), orderType, message.getFederateHandle(),
                                    message.getMessageRetractionHandle());
    } else {
      ambassador.receiveInteraction(interactionClass, interactionClassHandle, message.getParameterValues(),
                                    message.getTag(), message.getOrderType(), message.getTransportationType(),
                                    _logicalTimeFactory.getLogicalTime(logicalTime), orderType, message.getFederateHandle());
    }
  }

  virtual bool dispatchCallback(const AbstractMessageDispatcher& dispatcher)
  {
    if (_receiveOrderMessagesPermitted()) {
      if (!_receiveOrderMessages.empty()) {
        _MessageListElement& messageListElement = _receiveOrderMessages.front();
        messageListElement.unlink();
        SharedPtr<const AbstractMessage> message;
        message.swap(messageListElement._message);
        _messageListPool.push_back(messageListElement);

        message->dispatch(dispatcher);
        return true;
      }
    }
    checkForPendingFlushQueue();
    while (!_logicalTimeMessageListMap.empty()) {
      if (_logicalTimeMessageListMap.begin()->second.empty()) {
        _logicalTimeMessageListMap.erase(_logicalTimeMessageListMap.begin());
        checkForPendingFlushQueue();
        continue;
      }
      if (!_timeStampOrderMessagesPermitted())
        break;

      _MessageListElement& messageListElement = _logicalTimeMessageListMap.begin()->second.front();
      messageListElement.unlink();
      SharedPtr<const AbstractMessage> message;
      message.swap(messageListElement._message);
      _messageListPool.push_back(messageListElement);
      if (_logicalTimeMessageListMap.begin()->second.empty())
        _logicalTimeMessageListMap.erase(_logicalTimeMessageListMap.begin());

      message->dispatch(dispatcher);
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
    checkForPendingFlushQueue();
    while (!_logicalTimeMessageListMap.empty()) {
      if (_logicalTimeMessageListMap.begin()->second.empty()) {
        _logicalTimeMessageListMap.erase(_logicalTimeMessageListMap.begin());
        checkForPendingFlushQueue();
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
    if (!InternalTimeManagement::getTimeConstrainedEnabledOrPending())
      return true;
    if (_pendingLogicalTime < _logicalTimeMessageListMap.begin()->first)
      return false;
    return canAdvanceTo(_logicalTimeMessageListMap.begin()->first);
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

  static LogicalTime _toLogicalTime(const LogicalTimePair& logicalTimePair)
  {
    if (0 < logicalTimePair.second)
      return LogicalTimeFactory::nextAfter(logicalTimePair.first);
    else
      return logicalTimePair.first;
  }

  // The current logical time of this federate
  // This is also the current guarantee that we have alive for inbound messages
  // Its not legal anymore to deliver a timestamped message with a timestamp smaller than this
  LogicalTimePair _logicalTime;
  // If a time advance is pending, this contains the logical time of the advance call.
  LogicalTimePair _pendingLogicalTime;
  // The smallest allowed logical time for sending messages, is temporarily above the committed
  // time stamp when in next message or flush queue mode else these are the same.
  LogicalTimePair _outboundLowerBoundTimeStamp;
  // The previous value of the above. This is to make sure that we always preserve already built up constraints
  LogicalTimePair _lastOutboundLowerBoundTimeStamp;
  // The currently committed time stamp guarantee for outgoing messages
  LogicalTime _committedOutboundLowerBoundTimeStamp;
  // The currently committed next message time stamp
  LogicalTime _committedNextMessageLowerBoundTimeStamp;
  // The lookahead of this federate
  LogicalTimeInterval _currentLookahead;
  // The requested lookahead of this federate
  LogicalTimeInterval _targetLookahead;

  // map containing all the committed logical times of all known time regulating federates
  typedef FederateHandleLowerBoundTimeStampMap<LogicalTime> FederateLowerBoundMap;
  FederateLowerBoundMap _federateLowerBoundMap;

  // When we are in time regulation enable pending state, this contains the federates we need to
  // wait for to complete time regulation enabled
  FederateHandleSet _timeRegulationEnableFederateHandleSet;
  typedef std::map<FederateHandle, LogicalTime> FederateHandleTimeStampMap;
  FederateHandleTimeStampMap _timeRegulationEnableFederateHandleTimeStampMap;

  // Is true if the time advance is pending but the time advance grant is not yet scheduled in the queue
  bool _timeAdvanceToBeScheduled;

  Unsigned _commitId;

  // Holds a message to be queued into a list
  struct OPENRTI_LOCAL _MessageListElement :
         public IntrusiveList<_MessageListElement>::Hook,
         public IntrusiveUnorderedMap<ObjectInstanceHandle, _MessageListElement>::Hook {
    _MessageListElement(const AbstractMessage* message) : _message(message)
    { }
    void unlink()
    {
      IntrusiveList<_MessageListElement>::unlink(*this);
      IntrusiveUnorderedMap<ObjectInstanceHandle, _MessageListElement>::unlink(*this);
      IntrusiveUnorderedMap<ObjectInstanceHandle, _MessageListElement>::Hook::setKey(ObjectInstanceHandle());
    }
    const ObjectInstanceHandle& getObjectInstanceHandle() const
    { return IntrusiveUnorderedMap<ObjectInstanceHandle, _MessageListElement>::Hook::getKey(); }
    void setObjectInstanceHandle(const ObjectInstanceHandle& objectInstanceHandle)
    { IntrusiveUnorderedMap<ObjectInstanceHandle, _MessageListElement>::Hook::setKey(objectInstanceHandle); }
    SharedPtr<const AbstractMessage> _message;
  };
  typedef IntrusiveList<_MessageListElement> _MessageList;

  // The timestamped queued messages
  typedef std::map<LogicalTimePair, _MessageList> LogicalTimeMessageListMap;
  LogicalTimeMessageListMap _logicalTimeMessageListMap;

  // List of receive order messages ready to be queued for callback
  _MessageList _receiveOrderMessages;

  // List elements for reuse
  _MessageList _messageListPool;

  // List elements that reference a specific object instance
  typedef IntrusiveUnorderedMap<ObjectInstanceHandle, _MessageListElement> _ObjectInstanceHandleMessageListElementMap;
  _ObjectInstanceHandleMessageListElementMap _objectInstanceHandleMessageListElementMap;

  // The logical time factory required to do our job
  LogicalTimeFactory _logicalTimeFactory;
};

} // namespace OpenRTI

#endif
