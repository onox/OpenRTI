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

#include "InternalAmbassador.h"

#include "InternalTimeManagement.h"
#include "Federate.h"
#include "Exception.h"
#include "Export.h"
#include "LogStream.h"
#include "LeafServerThread.h"
#include "Message.h"

namespace OpenRTI {

class OPENRTI_LOCAL InternalAmbassador::_InternalMessageDispatchFunctor {
public:
  _InternalMessageDispatchFunctor(InternalAmbassador& basicAmbassador) :
    _basicAmbassador(basicAmbassador)
  { }
  template<typename M>
  void operator()(const M& message) const
  { _basicAmbassador.acceptInternalMessage(message); }
private:
  InternalAmbassador& _basicAmbassador;
};

InternalAmbassador::InternalAmbassador()
{
}

InternalAmbassador::~InternalAmbassador()
{
  if (_connect.valid())
    _connect->close();
  _connect.clear();
}

bool
InternalAmbassador::isConnected() const
{
  return _connect.valid();
}

void
InternalAmbassador::connect(const StringMap& parameterMap, const Clock& abstime)
{
  StringStringListMap stringStringListMap;
  for (StringMap::const_iterator j = parameterMap.begin(); j != parameterMap.end(); ++j)
    stringStringListMap[j->first].push_back(j->second);

  connect(stringStringListMap, abstime);
}

void
InternalAmbassador::connect(const StringStringListMap& parameterMap, const Clock& abstime)
{
  StringStringListMap::const_iterator i = parameterMap.find("protocol");
  std::string protocol;
  if (i != parameterMap.end() && !i->second.empty())
    protocol = i->second.front();
  else
    protocol = "thread";

  URL url;
  if (protocol == "rti") {
    i = parameterMap.find("address");
    if (i != parameterMap.end() && !i->second.empty())
      url = URL::fromProtocolAddress("rti", i->second.front());
    else
      url = URL::fromProtocolAddress("rti", "localhost");
  } else if (protocol == "pipe") {
    i = parameterMap.find("file");
    if (i != parameterMap.end() && !i->second.empty())
      url = URL::fromProtocolPath("pipe", i->second.front());
  } else if (protocol == "thread") {
    url = URL::fromProtocolAddress("thread", std::string());
  }

  _connect = LeafServerThread::connect(url, parameterMap);
}

void
InternalAmbassador::disconnect()
{
  OpenRTIAssert(_connect.valid());
  _connect->close();
  _connect.clear();
}

void
InternalAmbassador::send(const SharedPtr<const AbstractMessage>& message)
{
  OpenRTIAssert(_connect.valid());
  _connect->send(message);
}

bool
InternalAmbassador::receiveAndDispatchInternalMessage(const Clock& abstime)
{
  _InternalMessageDispatchFunctor functor(*this);
  return receiveAndDispatch(abstime, functor);
}

bool
InternalAmbassador::_receiveAndDispatch(const Clock& abstime, const AbstractMessageDispatcher& dispatcher)
{
  SharedPtr<const AbstractMessage> message = _connect->receive(abstime);
  if (!message.valid())
    return false;
  message->dispatch(dispatcher);
  return !_connect->empty();
}

void
InternalAmbassador::acceptInternalMessage(const AbstractMessage&)
{
  throw RTIinternalError("Unexpected message in internal message processing!");
}

void
InternalAmbassador::acceptInternalMessage(const ConnectionLostMessage& message)
{
}

void
InternalAmbassador::acceptInternalMessage(const EnumerateFederationExecutionsRequestMessage& message)
{
}

void
InternalAmbassador::acceptInternalMessage(const EnumerateFederationExecutionsResponseMessage& message)
{
}

// void
// InternalAmbassador::acceptInternalMessage(const InsertFederationExecutionMessage& message)
// {
// }

void
InternalAmbassador::acceptInternalMessage(const ShutdownFederationExecutionMessage& message)
{
}

void
InternalAmbassador::acceptInternalMessage(const EraseFederationExecutionMessage& message)
{
  // FIXME?!
  // deleteFederate();
}

void
InternalAmbassador::acceptInternalMessage(const ReleaseFederationHandleMessage& message)
{
}

void
InternalAmbassador::acceptInternalMessage(const JoinFederationExecutionRequestMessage& message)
{
}

void
InternalAmbassador::acceptInternalMessage(const JoinFederationExecutionResponseMessage& message)
{
  Federate* federate = getFederate();
  if (!federate)
    return;
  federate->setFederateHandle(message.getFederateHandle());
  federate->setFederateName(message.getFederateName());
  federate->setFederateType(message.getFederateType());
}

void
InternalAmbassador::acceptInternalMessage(const ResignFederationExecutionRequestMessage& message)
{
}

void
InternalAmbassador::acceptInternalMessage(const JoinFederateNotifyMessage& message)
{
  if (Federate* federate = getFederate())
    federate->insertFederate(message.getFederateHandle());
  if (InternalTimeManagement* timeManagement = getTimeManagement())
    timeManagement->acceptInternalMessage(*this, message);
}

void
InternalAmbassador::acceptInternalMessage(const ResignFederateNotifyMessage& message)
{
  if (InternalTimeManagement* timeManagement = getTimeManagement())
    timeManagement->acceptInternalMessage(*this, message);
  if (Federate* federate = getFederate())
    federate->eraseFederate(message.getFederateHandle());
}

void
InternalAmbassador::acceptInternalMessage(const RegisterFederationSynchronizationPointResponseMessage& message)
{
  queueCallback(message);
}

void
InternalAmbassador::acceptInternalMessage(const AnnounceSynchronizationPointMessage& message)
{
  queueCallback(message);
}

void
InternalAmbassador::acceptInternalMessage(const SynchronizationPointAchievedMessage& message)
{
}

void
InternalAmbassador::acceptInternalMessage(const FederationSynchronizedMessage& message)
{
  queueCallback(message);
}

void
InternalAmbassador::acceptInternalMessage(const EnableTimeRegulationRequestMessage& message)
{
  if (InternalTimeManagement* timeManagement = getTimeManagement())
    timeManagement->acceptInternalMessage(*this, message);
}

void
InternalAmbassador::acceptInternalMessage(const EnableTimeRegulationResponseMessage& message)
{
  if (InternalTimeManagement* timeManagement = getTimeManagement())
    timeManagement->acceptInternalMessage(*this, message);
}

void
InternalAmbassador::acceptInternalMessage(const DisableTimeRegulationRequestMessage& message)
{
  if (InternalTimeManagement* timeManagement = getTimeManagement())
    timeManagement->acceptInternalMessage(*this, message);
}

void
InternalAmbassador::acceptInternalMessage(const CommitLowerBoundTimeStampMessage& message)
{
  if (InternalTimeManagement* timeManagement = getTimeManagement())
    timeManagement->acceptInternalMessage(*this, message);
}

void
InternalAmbassador::acceptInternalMessage(const InsertRegionMessage& message)
{
  Federate* federate = getFederate();
  if (!federate)
    return;
  federate->insertRegions(message.getRegionHandleDimensionHandleSetPairVector());
}

void
InternalAmbassador::acceptInternalMessage(const CommitRegionMessage& message)
{
  Federate* federate = getFederate();
  if (!federate)
    return;
  federate->commitRegions(message.getRegionHandleRegionValuePairVector());
}

void
InternalAmbassador::acceptInternalMessage(const EraseRegionMessage& message)
{
  Federate* federate = getFederate();
  if (!federate)
    return;
  federate->eraseRegions(message.getRegionHandleVector());
}

void
InternalAmbassador::acceptInternalMessage(const ChangeInteractionClassPublicationMessage& message)
{
}

void
InternalAmbassador::acceptInternalMessage(const ChangeObjectClassPublicationMessage& message)
{
}

void
InternalAmbassador::acceptInternalMessage(const ChangeInteractionClassSubscriptionMessage& message)
{
}

void
InternalAmbassador::acceptInternalMessage(const ChangeObjectClassSubscriptionMessage& message)
{
}

void
InternalAmbassador::acceptInternalMessage(const RegistrationForObjectClassMessage& message)
{
}

void
InternalAmbassador::acceptInternalMessage(const TurnInteractionsOnMessage& message)
{
}

void
InternalAmbassador::acceptInternalMessage(const InteractionMessage& message)
{
  queueReceiveOrderCallback(message);
}

void
InternalAmbassador::acceptInternalMessage(const TimeStampedInteractionMessage& message)
{
  queueTimeStampedMessage(message.getTimeStamp(), message);
}

void
InternalAmbassador::acceptInternalMessage(const ObjectInstanceHandlesRequestMessage& message)
{
}

void
InternalAmbassador::acceptInternalMessage(const ObjectInstanceHandlesResponseMessage& message)
{
  Federate* federate = getFederate();
  if (!federate)
    return;
  federate->insertObjectInstanceHandleNamePairs(message.getObjectInstanceHandleNamePairVector());
}

void
InternalAmbassador::acceptInternalMessage(const ReleaseMultipleObjectInstanceNameHandlePairsMessage& message)
{
}

void
InternalAmbassador::acceptInternalMessage(const ReserveObjectInstanceNameRequestMessage& message)
{
}

void
InternalAmbassador::acceptInternalMessage(const ReserveObjectInstanceNameResponseMessage& message)
{
  queueCallback(message);
}

void
InternalAmbassador::acceptInternalMessage(const ReserveMultipleObjectInstanceNameRequestMessage& message)
{
}

void
InternalAmbassador::acceptInternalMessage(const ReserveMultipleObjectInstanceNameResponseMessage& message)
{
  queueCallback(message);
}

void
InternalAmbassador::acceptInternalMessage(const InsertObjectInstanceMessage& message)
{
  queueCallback(message);
}

void
InternalAmbassador::acceptInternalMessage(const DeleteObjectInstanceMessage& message)
{
  queueReceiveOrderCallback(message);
}

void
InternalAmbassador::acceptInternalMessage(const TimeStampedDeleteObjectInstanceMessage& message)
{
  queueTimeStampedMessage(message.getTimeStamp(), message);
}

void
InternalAmbassador::acceptInternalMessage(const AttributeUpdateMessage& message)
{
  queueReceiveOrderCallback(message);
}

void
InternalAmbassador::acceptInternalMessage(const TimeStampedAttributeUpdateMessage& message)
{
  queueTimeStampedMessage(message.getTimeStamp(), message);
}

void
InternalAmbassador::acceptInternalMessage(const RequestAttributeUpdateMessage& message)
{
  queueCallback(message);
}

void
InternalAmbassador::acceptInternalMessage(const RequestClassAttributeUpdateMessage& message)
{
  // queueCallback(message);
}

class OPENRTI_LOCAL InternalAmbassador::_CreateFederationExecutionFunctor {
public:
  _CreateFederationExecutionFunctor(InternalAmbassador& basicAmbassador) :
    _done(false),
    _responseTypeStringPair(CreateFederationExecutionResponseRTIinternalError, std::string()),
    _basicAmbassador(basicAmbassador)
  { }
  void operator()(const CreateFederationExecutionResponseMessage& message)
  {
    _responseTypeStringPair.first = message.getCreateFederationExecutionResponseType();
    _responseTypeStringPair.second = message.getExceptionString();
    _done = true;
  }
  template<typename M>
  void operator()(const M& message)
  { _basicAmbassador.acceptInternalMessage(message); }

  bool _done;
  std::pair<CreateFederationExecutionResponseType, std::string> _responseTypeStringPair;

private:
  InternalAmbassador& _basicAmbassador;
};

std::pair<CreateFederationExecutionResponseType, std::string>
InternalAmbassador::dispatchWaitCreateFederationExecutionResponse(const Clock& abstime)
{
  _CreateFederationExecutionFunctor functor(*this);
  while (!functor._done && Clock::now() <= abstime)
    receiveAndDispatch(abstime, functor);
  return functor._responseTypeStringPair;
}

class OPENRTI_LOCAL InternalAmbassador::_DestroyFederationExecutionFunctor {
public:
  _DestroyFederationExecutionFunctor(InternalAmbassador& basicAmbassador) :
    _done(false),
    _responseType(DestroyFederationExecutionResponseRTIinternalError),
    _basicAmbassador(basicAmbassador)
  { }
  void operator()(const DestroyFederationExecutionResponseMessage& message)
  {
    _responseType = message.getDestroyFederationExecutionResponseType();
    _done = true;
  }
  template<typename M>
  void operator()(const M& message) const
  { _basicAmbassador.acceptInternalMessage(message); }

  bool _done;
  DestroyFederationExecutionResponseType _responseType;

private:
  InternalAmbassador& _basicAmbassador;
};

DestroyFederationExecutionResponseType
InternalAmbassador::dispatchWaitDestroyFederationExecutionResponse(const Clock& abstime)
{
  _DestroyFederationExecutionFunctor functor(*this);
  while (!functor._done && Clock::now() <= abstime)
    receiveAndDispatch(abstime, functor);
  return functor._responseType;
}

class OPENRTI_LOCAL InternalAmbassador::_JoinFederationExecutionFunctor {
public:
  _JoinFederationExecutionFunctor(InternalAmbassador& basicAmbassador) :
    _done(false),
    _response(JoinFederationExecutionResponseFederationExecutionDoesNotExist),
    _basicAmbassador(basicAmbassador)
  { }
  void operator()(const InsertFederationExecutionMessage& message)
  {
    _basicAmbassador.acceptInternalMessage(message);
  }
  void operator()(const JoinFederationExecutionResponseMessage& message)
  {
    _basicAmbassador.acceptInternalMessage(message);
    _response = message.getJoinFederationExecutionResponseType();
    _done = true;
  }
  template<typename M>
  void operator()(const M& message) const
  { _basicAmbassador.acceptInternalMessage(message); }

  bool _done;
  JoinFederationExecutionResponseType _response;

private:
  InternalAmbassador& _basicAmbassador;
};

JoinFederationExecutionResponseType
InternalAmbassador::dispatchWaitJoinFederationExecutionResponse(const Clock& abstime)
{
  _JoinFederationExecutionFunctor functor(*this);
  while (!functor._done && Clock::now() <= abstime)
    receiveAndDispatch(abstime, functor);
  return functor._response;
}

class OPENRTI_LOCAL InternalAmbassador::_EraseFederationExecutionFunctor {
public:
  _EraseFederationExecutionFunctor(InternalAmbassador& basicAmbassador) :
    _done(false),
    _basicAmbassador(basicAmbassador)
  { }
  void operator()(const EraseFederationExecutionMessage& message)
  {
    _done = true;
  }
  void operator()(const InsertObjectInstanceMessage& message)
  {
    SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> reply;
    reply = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
    reply->setFederationHandle(message.getFederationHandle());
    reply->getObjectInstanceHandleVector().push_back(message.getObjectInstanceHandle());
    _basicAmbassador.send(reply);
  }
  void operator()(const ObjectInstanceHandlesResponseMessage& message)
  {
    SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> reply;
    reply = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
    reply->setFederationHandle(message.getFederationHandle());
    reply->getObjectInstanceHandleVector().reserve(message.getObjectInstanceHandleNamePairVector().size());
    for (ObjectInstanceHandleNamePairVector::const_iterator k = message.getObjectInstanceHandleNamePairVector().begin();
         k != message.getObjectInstanceHandleNamePairVector().end(); ++k) {
      reply->getObjectInstanceHandleVector().push_back(k->first);
    }
    _basicAmbassador.send(reply);
  }
  void operator()(const ReserveObjectInstanceNameResponseMessage& message)
  {
    if (!message.getSuccess())
      return;
    SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> reply;
    reply = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
    reply->setFederationHandle(message.getFederationHandle());
    reply->getObjectInstanceHandleVector().push_back(message.getObjectInstanceHandleNamePair().first);
    _basicAmbassador.send(reply);
  }
  void operator()(const ReserveMultipleObjectInstanceNameResponseMessage& message) const
  {
    if (!message.getSuccess())
      return;
    SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> reply;
    reply = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
    reply->setFederationHandle(message.getFederationHandle());
    reply->getObjectInstanceHandleVector().reserve(message.getObjectInstanceHandleNamePairVector().size());
    for (ObjectInstanceHandleNamePairVector::const_iterator k = message.getObjectInstanceHandleNamePairVector().begin();
         k != message.getObjectInstanceHandleNamePairVector().end(); ++k) {
      reply->getObjectInstanceHandleVector().push_back(k->first);
    }
    _basicAmbassador.send(reply);
  }
  template<typename M>
  void operator()(const M& message) const
  { _basicAmbassador.acceptInternalMessage(message); }

  bool _done;

private:
  InternalAmbassador& _basicAmbassador;
};

bool
InternalAmbassador::dispatchWaitEraseFederationExecutionResponse(const Clock& abstime)
{
  _EraseFederationExecutionFunctor functor(*this);
  while (!functor._done && Clock::now() <= abstime)
    receiveAndDispatch(abstime, functor);
  return functor._done;
}


class OPENRTI_LOCAL InternalAmbassador::_ReserveObjectInstanceNameFunctor {
public:
  _ReserveObjectInstanceNameFunctor(InternalAmbassador& basicAmbassador, const std::string& objectInstanceName) :
    _done(false),
    _basicAmbassador(basicAmbassador),
    _objectInstanceName(objectInstanceName)
  { }
  void operator()(const ReserveObjectInstanceNameResponseMessage& message)
  {
    // Intercept only those responses which match our name.
    // The rest is given to the underlying ambassador/federate.
    if (_objectInstanceName == message.getObjectInstanceHandleNamePair().second) {
      _objectInstanceHandle = message.getObjectInstanceHandleNamePair().first;
      _done = true;
    } else {
      _basicAmbassador.acceptInternalMessage(message);
    }
  }
  template<typename M>
  void operator()(const M& message) const
  { _basicAmbassador.acceptInternalMessage(message); }

  bool _done;
  ObjectInstanceHandle _objectInstanceHandle;

private:
  InternalAmbassador& _basicAmbassador;
  std::string _objectInstanceName;
};

ObjectInstanceHandle
InternalAmbassador::dispatchWaitReserveObjectInstanceName(const Clock& abstime, const std::string& objectInstanceName)
{
  Federate* federate = getFederate();
  OpenRTIAssert(federate);

  SharedPtr<ReserveObjectInstanceNameRequestMessage> request;
  request = new ReserveObjectInstanceNameRequestMessage;
  request->setFederationHandle(federate->getFederationHandle());
  request->setFederateHandle(federate->getFederateHandle());
  request->setName(objectInstanceName);
  send(request);

  _ReserveObjectInstanceNameFunctor functor(*this, objectInstanceName);
  while (!functor._done && Clock::now() <= abstime)
    receiveAndDispatch(abstime, functor);

  return functor._objectInstanceHandle;
}

void
InternalAmbassador::queueReceiveOrderCallback(const AbstractMessage& message)
{
  // If we are in time advancing state and recieve order is not allowed
  Federate* federate = getFederate();
  if (!federate)
    return;
  if (!federate->getAsynchronousDeliveryEnabled() && getTimeManagement()->getTimeAdvancePending()) {
    _receiveOrderMessages.push_back(&message);
  } else {
    queueCallback(message);
  }
}

void
InternalAmbassador::queueTimeStampedMessage(const VariableLengthData& timeStamp, const AbstractMessage& message)
{
  InternalTimeManagement* timeManagement = getTimeManagement();
  if (!timeManagement)
    return;
  if (timeManagement->getTimeConstrainedEnabledOrPending()) {
    timeManagement->queueTimeStampedMessage(*this, timeStamp, message);
  } else {
    queueCallback(message);
  }
}

}
