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

#include "ServerNode.h"

#include <algorithm>
#include <iterator>
#include "Message.h"
#include "AbstractMessageSender.h"
#include "LogStream.h"
#include "NameHandleMap.h"
#include "ServerObjectModel.h"
#include "ServerOptions.h"

namespace OpenRTI {

/// Class to handle the output connections attached to the server
/// Since we have a tree like topology in the servers one connection might carry plenty federates
/// This class handles this and maps connects to federates and back.
// FIXME should go away
class OPENRTI_LOCAL ServerConnectSet {
public:

  // FIXME:
  // need a little more concept behind this:
  // May be the ConnectHandle (->NodeHandle) which represents the network nodes you can reach with a set of connects.
  // And a SocketHandle(? - better name required) that identifies the output chanels where messages should go in the end.

  bool hasParentConnect() const
  { return _parentServerConnectHandle.valid(); }

  bool hasChildConnects() const
  {
    if (hasParentConnect())
      return 1 < _messageSenderMap.size();
    else
      return !_messageSenderMap.empty();
  }

  const ConnectHandle& getParentConnectHandle() const
  { return _parentServerConnectHandle; }
  ConnectHandle insertParentMessageSender(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& parentOptions)
  {
    OpenRTIAssert(!_parentServerConnectHandle.valid());
    _parentServerConnectHandle = insertMessageSender(messageSender, parentOptions);
    return _parentServerConnectHandle;
  }

  ConnectHandle insertMessageSender(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& clientOptions)
  {
    if (!messageSender.valid())
      return ConnectHandle();
    ConnectHandle connectHandle = _connectHandleAllocator.get();
    OpenRTIAssert(_messageSenderMap.find(connectHandle) == _messageSenderMap.end());
    MessageSenderMap::iterator i = _messageSenderMap.insert(MessageSenderMap::value_type(connectHandle, ConnectData())).first;
    i->second._messageSender = messageSender;
    StringStringListMap::const_iterator j = clientOptions.find("serverName");
    if (j != clientOptions.end() && !j->second.empty()) {
      i->second._name = j->second.front();
    } else {
      i->second._name = connectHandle.toString();
    }
    Log(ServerConnect, Info) << "Inserting connect \"" << i->second._name << "\"." << std::endl;
    return connectHandle;
  }
  void removeMessageSender(const ConnectHandle& connectHandle)
  {
    MessageSenderMap::iterator i = _messageSenderMap.find(connectHandle);
    OpenRTIAssert(i != _messageSenderMap.end());
    Log(ServerConnect, Info) << "Removing connect \"" << i->second._name << "\"." << std::endl;
    _messageSenderMap.erase(i);
    if (_parentServerConnectHandle == connectHandle)
      _parentServerConnectHandle = ConnectHandle();
    _connectHandleAllocator.put(connectHandle);
  }

  /// send message to the given connect
  void send(const ConnectHandle& connectHandle, const SharedPtr<AbstractMessage>& message) const
  {
    MessageSenderMap::const_iterator i = _messageSenderMap.find(connectHandle);
    OpenRTIAssert(i != _messageSenderMap.end());
    i->second._messageSender->send(message);
  }
  /// send upstream to the parent server
  void sendToParent(const SharedPtr<AbstractMessage>& message) const
  {
    send(_parentServerConnectHandle, message);
  }
  void broadcastToChildren(const SharedPtr<AbstractMessage>& message) const
  {
    for (MessageSenderMap::const_iterator i = _messageSenderMap.begin(); i != _messageSenderMap.end(); ++i) {
      if (i->first == _parentServerConnectHandle)
        continue;
      i->second._messageSender->send(message);
    }
  }

  SharedPtr<AbstractMessageSender> getMessageSender(const ConnectHandle& connectHandle) const
  {
    MessageSenderMap::const_iterator i = _messageSenderMap.find(connectHandle);
    OpenRTIAssert(i != _messageSenderMap.end());
    return i->second._messageSender;
  }
  const std::string& getName(const ConnectHandle& connectHandle) const
  {
    MessageSenderMap::const_iterator i = _messageSenderMap.find(connectHandle);
    OpenRTIAssert(i != _messageSenderMap.end());
    return i->second._name;
  }

private:
  /// Connect management.

  /// If we have an upstream server this one contains a vaild connect handle.
  ConnectHandle _parentServerConnectHandle;

  /// The server class has several connects.
  /// Each connect can have multiple federates attached.
  struct ConnectData {
    SharedPtr<AbstractMessageSender> _messageSender;
    std::string _name;
  };

  typedef std::map<ConnectHandle, ConnectData> MessageSenderMap;
  MessageSenderMap _messageSenderMap;

  HandleAllocator<ConnectHandle> _connectHandleAllocator;
};

/// Hmm, should that be the Federation ???
class OPENRTI_LOCAL FederationServer : public ServerObjectModel {
public:
  FederationServer(const std::string& name, const FederationHandle& handle, const SharedPtr<const ServerOptions>& serverOptions) :
    /// FIXME make this a member instead of a base
    ServerObjectModel(name, handle),
    _serverOptions(serverOptions)
  {
  }

  /// Get the rti servers name
  const std::string& getServerName() const
  { return _serverOptions->getServerName(); }
  /// Get the rti servers path
  const std::string& getServerPath() const
  { return _serverOptions->getServerPath(); }

  void accept(const ConnectHandle& connectHandle, JoinFederationExecutionRequestMessage* message)
  {
    // This function needs to be successful since the calling function has already inserted the connect here
    // and does not undo that FIXME: rethink?!

    OpenRTIAssert(isRootServer());

    // The ambassador needs to handle this already
    if (message->getFederateName().compare(0, 3, "HLA") == 0)
      throw MessageError("Got JoinFederationExecutionRequestMessage with name starting with HLA.");

    if (isFederateNameInUse(message->getFederateName())) {
      SharedPtr<JoinFederationExecutionResponseMessage> response;
      response = new JoinFederationExecutionResponseMessage;
      response->setJoinFederationExecutionResponseType(JoinFederationExecutionResponseFederateNameAlreadyInUse);
      send(connectHandle, response);
      return;
    }

    // Respond with Success
    SharedPtr<JoinFederationExecutionResponseMessage> response;
    response = new JoinFederationExecutionResponseMessage;
    response->setJoinFederationExecutionResponseType(JoinFederationExecutionResponseSuccess);

    // ... insert a new federate ...
    Federate* federate = insertFederate(connectHandle, message->getFederateName());
    OpenRTIAssert(federate);
    federate->_federateType = message->getFederateType();

    /// FIXME shall we get that message from the federation server???
    response->setFederationHandle(getHandle());
    response->setFederateType(message->getFederateType());
    response->setFederateName(federate->getName());
    response->setFederateHandle(federate->getHandle());
    send(connectHandle, response);

    // send all children the notification about the new federate except the one that gets the join response
    SharedPtr<JoinFederateNotifyMessage> notify = new JoinFederateNotifyMessage;
    notify->setFederationHandle(response->getFederationHandle());
    notify->setFederateHandle(response->getFederateHandle());
    notify->setFederateType(response->getFederateType());
    notify->setFederateName(federate->getName());
    broadcastToChildren(connectHandle, notify);

    // For those sync request that are automatically extended to new federates, send the announcement
    for (SyncronizationLabelStateMap::iterator j = _syncronizationLabelStateMap.begin();
         j != _syncronizationLabelStateMap.end(); ++j) {
      // only those with an auto expanding federate set
      if (!j->second._addJoiningFederates)
        continue;

      j->second.insert(federate->getHandle());

      SharedPtr<AnnounceSynchronizationPointMessage> announce;
      announce = new AnnounceSynchronizationPointMessage;
      announce->setFederationHandle(getHandle());
      announce->setLabel(j->first);
      announce->setTag(j->second.getTag());
      announce->getFederateHandleSet().insert(federate->getHandle());
      announce->setAddJoiningFederates(j->second._addJoiningFederates);
      broadcastToChildren(announce);
    }
  }
  void accept(const ConnectHandle& connectHandle, const ConnectHandle& requestConnectHandle, JoinFederationExecutionResponseMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());

    FederateHandle federateHandle = message->getFederateHandle();
    OpenRTIAssert(federateHandle.valid());

    Federate* federate = insertFederate(requestConnectHandle, message->getFederateName(), federateHandle);
    OpenRTIAssert(federate);
    federate->_federateType = message->getFederateType();

    if (requestConnectHandle.valid()) {
      // The requesting server needs to know about us, just forward here
      send(requestConnectHandle, message);

    } else {
      // if the connect is died in between, resign now again
      SharedPtr<ResignFederationExecutionRequestMessage> resign = new ResignFederationExecutionRequestMessage;
      resign->setFederationHandle(getHandle());
      resign->setFederateHandle(message->getFederateHandle());
      send(connectHandle, resign);
    }

    // send all children the notification about the new federate except the one that gets the join response
    SharedPtr<JoinFederateNotifyMessage> notify = new JoinFederateNotifyMessage;
    notify->setFederationHandle(message->getFederationHandle());
    notify->setFederateHandle(message->getFederateHandle());
    notify->setFederateType(message->getFederateType());
    notify->setFederateName(message->getFederateName());
    broadcastToChildren(requestConnectHandle, notify);
  }


  void accept(const ConnectHandle& connectHandle, ResignFederationExecutionRequestMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    FederateHandle federateHandle = message->getFederateHandle();
    FederateHandleFederateMap::iterator i = _federateHandleFederateMap.find(federateHandle);
    if (i == _federateHandleFederateMap.end())
      throw MessageError("Recieved ResignFederationExecutionRequestMessage for invalid federate handle!");

    // already done so ... just waiting for the response
    if (i->second->_resignPending)
      return;

    // We need to skip resource allocations in the future
    i->second->_resignPending = true;

    // remove from time management
    FederateHandleTimeStampMap::iterator k = _federateHandleTimeStampMap.find(federateHandle);
    if (k != _federateHandleTimeStampMap.end()) {
      _federateHandleTimeStampMap.erase(k);
      SharedPtr<DisableTimeRegulationRequestMessage> request = new DisableTimeRegulationRequestMessage;
      request->setFederationHandle(getHandle());
      request->setFederateHandle(federateHandle);
      broadcast(connectHandle, request);
    }

    // If we are a root server ...
    if (isRootServer()) {
      // ... and respond with Success
      /// FIXME shall we get that message from the federation server???
      SharedPtr<ResignFederateNotifyMessage> notify = new ResignFederateNotifyMessage;
      notify->setFederateHandle(federateHandle);
      notify->setFederationHandle(getHandle());
      broadcastToChildren(notify);

      // Remove the federate locally
      eraseFederate(federateHandle);

      // Remove a connect that runs idle
      if (!hasJoinedFederatesForConnect(connectHandle))
        eraseFederationExecutionAtConnect(connectHandle);
    }
    // If we have an upstream connect, mark this request as pending and ask the parent server
    else {
      send(_parentServerConnectHandle, message);
    }
  }

  void pushPublications(const ConnectHandle& connectHandle)
  {
    for (InteractionClassVector::const_iterator i = _interactionClassVector.begin(); i != _interactionClassVector.end(); ++i) {
      const InteractionClass* interactionClass = i->get();
      if (!interactionClass)
        continue;
      if (Unpublished == interactionClass->getPublicationType())
        continue;

      SharedPtr<ChangeInteractionClassPublicationMessage> message;
      message = new ChangeInteractionClassPublicationMessage;
      message->setFederationHandle(getHandle());
      message->setPublicationType(Published);
      message->setInteractionClassHandle(interactionClass->getHandle());
      send(connectHandle, message);
    }

    // Object classes
    for (ObjectClassVector::const_iterator i = _objectClassVector.begin(); i != _objectClassVector.end(); ++i) {
      const ObjectClass* objectClass = i->get();
      if (!objectClass)
        continue;

      AttributeHandleVector attributeHandleVector;
      for (ObjectClassAttributeVector::const_iterator j = objectClass->getObjectClassAttributeVector().begin();
           j != objectClass->getObjectClassAttributeVector().end(); ++j) {
        const ObjectClassAttribute* attribute = j->get();
        if (!attribute)
          continue;
        if (Unpublished == attribute->getPublicationType())
          continue;

        if (attributeHandleVector.empty())
          attributeHandleVector.reserve(objectClass->getObjectClassAttributeVector().size());
        attributeHandleVector.push_back(attribute->getHandle());
      }
      if (attributeHandleVector.empty())
        continue;

      SharedPtr<ChangeObjectClassPublicationMessage> message;
      message = new ChangeObjectClassPublicationMessage;
      message->setFederationHandle(getHandle());
      message->setPublicationType(Published);
      message->setObjectClassHandle(objectClass->getHandle());
      message->getAttributeHandles().swap(attributeHandleVector);
      send(connectHandle, message);
    }
  }

  void eraseFederationExecutionAtConnect(const ConnectHandle& connectHandle)
  {
    for (FederateHandleFederateMap::const_iterator i = _federateHandleFederateMap.begin();
         i != _federateHandleFederateMap.end(); ++i) {
      SharedPtr<ResignFederateNotifyMessage> notify = new ResignFederateNotifyMessage;
      notify->setFederationHandle(getHandle());
      notify->setFederateHandle(i->first);
      send(connectHandle, notify);
    }

    SharedPtr<EraseFederationExecutionMessage> response;
    response = new EraseFederationExecutionMessage;
    response->setFederationHandle(getHandle());
    send(connectHandle, response);

    setInActive(connectHandle);
  }

  void broadcastEraseFederationExecution()
  {
    for (ConnectHandleConnectDataMap::iterator i = _connectHandleConnectDataMap.begin();
         i != _connectHandleConnectDataMap.end(); ++i) {
      if (i->first == _parentServerConnectHandle)
        continue;
      eraseFederationExecutionAtConnect(i->first);
    }
  }

  void accept(const ConnectHandle& connectHandle, JoinFederateNotifyMessage* message)
  {
    FederateHandle federateHandle = message->getFederateHandle();
    if (_federateHandleFederateMap.find(federateHandle) != _federateHandleFederateMap.end())
      // /// FIXME implement a clear for erase'?!
      // return;
      throw MessageError("Received JoinFederateNotify for already known federate!");
    Federate* federate = insertFederate(connectHandle, message->getFederateName(), federateHandle);
    OpenRTIAssert(federate);
    federate->_federateType = message->getFederateType();
    broadcastToChildren(message);
  }
  void accept(const ConnectHandle& connectHandle, ResignFederateNotifyMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    FederateHandle federateHandle = message->getFederateHandle();
    if (_federateHandleFederateMap.find(federateHandle) == _federateHandleFederateMap.end())
      throw MessageError("Received ResignFederateNotify for unknown federate!");
    broadcastToChildren(message);

    // FIXME can we simplify this and remove the federate on upstream resign?
    FederateHandleFederateMap::iterator i = _federateHandleFederateMap.find(federateHandle);
    ConnectHandle federateConnectHandle = i->second->getConnectHandle();
    eraseFederate(i);
    if (hasJoinedFederatesForConnect(federateConnectHandle))
      return;
    // Can happen that we ask for shotdown that is already underway
    if (!getActive(federateConnectHandle))
      return;

    if (federateConnectHandle == _parentServerConnectHandle)
      return;

    /// FIXME do this only if a shutdown request is pending!
    eraseFederationExecutionAtConnect(federateConnectHandle);
  }

  // Synchronization labels
  void accept(const ConnectHandle& connectHandle, RegisterFederationSynchronizationPointMessage* message)
  {
    // Labels must be ckecked by the ambassador. So what arrives here with an empty label must be some kind of error.
    if (message->getLabel().empty())
      throw MessageError("Received empty label in RegisterFederationSynchronizationPointMessage!");

    // If we are a root server ...
    if (isRootServer()) {
      // label is already there
      if (_syncronizationLabelStateMap.find(message->getLabel()) != _syncronizationLabelStateMap.end()) {
        SharedPtr<RegisterFederationSynchronizationPointResponseMessage> response;
        response = new RegisterFederationSynchronizationPointResponseMessage;
        response->setFederationHandle(getHandle());
        response->setLabel(message->getLabel());
        response->setFederateHandle(message->getFederateHandle());
        response->setRegisterFederationSynchronizationPointResponseType(RegisterFederationSynchronizationPointResponseLabelNotUnique);
        send(connectHandle, response);
        return;
      }

      SyncronizationLabelStateMap::iterator i;
      i = _syncronizationLabelStateMap.insert(SyncronizationLabelStateMap::value_type(message->getLabel(), SynchronizationState())).first;
      // FIXME, avoid that extra copy
      FederateHandleSet joinedFederates;
      for (FederateHandleFederateMap::const_iterator j = _federateHandleFederateMap.begin();
           j != _federateHandleFederateMap.end(); ++j)
        joinedFederates.insert(joinedFederates.end(), j->first);
      i->second.set(message->getFederateHandleSet(), joinedFederates);
      i->second._tag = message->getTag();

      // Respond to the originator
      SharedPtr<RegisterFederationSynchronizationPointResponseMessage> response;
      response = new RegisterFederationSynchronizationPointResponseMessage;
      response->setFederationHandle(getHandle());
      response->setLabel(message->getLabel());
      response->setFederateHandle(message->getFederateHandle());
      response->setRegisterFederationSynchronizationPointResponseType(RegisterFederationSynchronizationPointResponseSuccess);
      send(connectHandle, response);

      // Cycle over all child connects and send announcements with the appropriate handle sets
      for (ConnectHandleConnectDataMap::const_iterator j = _connectHandleConnectDataMap.begin();
           j != _connectHandleConnectDataMap.end(); ++j) {
        if (!j->first.valid())
          continue;
        // Build the intersection of the federate handles in the message and the ones in the connect.
        FederateHandleSet federateHandleSet;
        for (FederateList::const_iterator k = j->second->_federateList.begin();
             k != j->second->_federateList.end(); ++k) {
          if (i->second._waitFederates.find((*k)->getHandle()) == i->second._waitFederates.end())
            continue;
          federateHandleSet.insert((*k)->getHandle());
        }
        if (federateHandleSet.empty())
          continue;

        SharedPtr<AnnounceSynchronizationPointMessage> announce;
        announce = new AnnounceSynchronizationPointMessage;
        announce->setFederationHandle(getHandle());
        announce->setLabel(message->getLabel());
        announce->setTag(i->second._tag);
        announce->setAddJoiningFederates(i->second._addJoiningFederates);
        announce->getFederateHandleSet().swap(federateHandleSet);
        send(j->first, announce);
      }
    }
    // If we have an upstream connect, mark this request as pending and ask the parent server
    else {
      // ... ask your father
      send(_parentServerConnectHandle, message);
    }
  }
  void accept(const ConnectHandle& connectHandle, RegisterFederationSynchronizationPointResponseMessage* message)
  {
    if (message->getLabel().empty())
      throw MessageError("Received empty label in RegisterFederationSynchronizationPointResponseMessage!");

    send(message->getFederateHandle(), message);
  }

  void accept(const ConnectHandle& connectHandle, AnnounceSynchronizationPointMessage* message)
  {
    if (message->getLabel().empty())
      throw MessageError("Received empty label in AnnounceSynchronizationPointMessage!");

    SyncronizationLabelStateMap::iterator i = _syncronizationLabelStateMap.find(message->getLabel());
    if (i == _syncronizationLabelStateMap.end()) {
      // label is new, create one
      i = _syncronizationLabelStateMap.insert(SyncronizationLabelStateMap::value_type(message->getLabel(), SynchronizationState())).first;
      i->second._tag = message->getTag();
      i->second._addJoiningFederates = message->getAddJoiningFederates();
    } else {
      // label is already there
      if (!i->second._addJoiningFederates)
        MessageError("Receiving incremental synchronization point update for fixed federate handle synchronization point!");
      if (!message->getAddJoiningFederates())
        MessageError("Receiving incremental synchronization point update for fixed federate handle synchronization point!");
    }

    // Cycle over all child connects and send announcements with the appropriate handle sets
    for (ConnectHandleConnectDataMap::const_iterator j = _connectHandleConnectDataMap.begin();
	 j != _connectHandleConnectDataMap.end(); ++j) {
      if (j->first == _parentServerConnectHandle)
	continue;
      if (!j->first.valid())
	continue;

      // Build the intersection of the federate handles in the message and the ones in the connect.
      FederateHandleSet federateHandleSet;
      for (FederateList::const_iterator k = j->second->_federateList.begin();
           k != j->second->_federateList.end(); ++k) {
        if (message->getFederateHandleSet().find((*k)->getHandle()) == message->getFederateHandleSet().end())
          continue;
        federateHandleSet.insert((*k)->getHandle());
      }
      if (federateHandleSet.empty())
        continue;

      i->second.insert(federateHandleSet);

      SharedPtr<AnnounceSynchronizationPointMessage> announce;
      announce = new AnnounceSynchronizationPointMessage;
      announce->setFederationHandle(getHandle());
      announce->setLabel(message->getLabel());
      announce->setTag(message->getTag());
      announce->setAddJoiningFederates(message->getAddJoiningFederates());
      announce->getFederateHandleSet().swap(federateHandleSet);
      send(j->first, announce);
    }
  }
  void accept(const ConnectHandle& connectHandle, SynchronizationPointAchievedMessage* message)
  {
    SyncronizationLabelStateMap::iterator i = _syncronizationLabelStateMap.find(message->getLabel());
    if (i == _syncronizationLabelStateMap.end())
      throw MessageError("SynchronizationPointAchievedMessage for unknown label!");

    for (FederateHandleSet::const_iterator j = message->getFederateHandleSet().begin();
         j != message->getFederateHandleSet().end(); ++j) {
      i->second._waitFederates.erase(*j);
    }
    if (i->second._waitFederates.empty()) {
      if (isRootServer()) {
        SharedPtr<FederationSynchronizedMessage> response;
        response = new FederationSynchronizedMessage;
        response->setFederationHandle(getHandle());
        response->setLabel(message->getLabel());
        response->setFederateHandleSet(i->second._participatingFederates);
        broadcastToChildren(response->getFederateHandleSet(), response);
        _syncronizationLabelStateMap.erase(i);
      } else {
        SharedPtr<SynchronizationPointAchievedMessage> notify;
        notify = new SynchronizationPointAchievedMessage;
        notify->setFederationHandle(getHandle());
        notify->setLabel(message->getLabel());
        notify->setFederateHandleSet(i->second._participatingFederates);
        send(_parentServerConnectHandle, notify);
      }
    }
  }
  void accept(const ConnectHandle& connectHandle, FederationSynchronizedMessage* message)
  {
    SyncronizationLabelStateMap::iterator i = _syncronizationLabelStateMap.find(message->getLabel());
    if (i == _syncronizationLabelStateMap.end())
      throw MessageError("FederateSynchronizedMessage for unknown label.");

    // Cycle over all child connects and send announcements with the appropriate handle sets
    for (ConnectHandleConnectDataMap::const_iterator j = _connectHandleConnectDataMap.begin();
	 j != _connectHandleConnectDataMap.end(); ++j) {
      if (j->first == _parentServerConnectHandle)
	continue;
      if (!j->first.valid())
	continue;

      // Build the intersection of the federate handles in the message and the ones in the connect.
      FederateHandleSet federateHandleSet;
      for (FederateList::const_iterator k = j->second->_federateList.begin();
           k != j->second->_federateList.end(); ++k) {
        if (message->getFederateHandleSet().find((*k)->getHandle()) == message->getFederateHandleSet().end())
          continue;
        federateHandleSet.insert((*k)->getHandle());
      }
      if (federateHandleSet.empty())
        continue;

      SharedPtr<FederationSynchronizedMessage> synchronized;
      synchronized = new FederationSynchronizedMessage;
      synchronized->setFederationHandle(getHandle());
      synchronized->setLabel(message->getLabel());
      synchronized->getFederateHandleSet().swap(federateHandleSet);
      send(j->first, synchronized);
    }
    _syncronizationLabelStateMap.erase(i);
  }

  // Time management
  void accept(const ConnectHandle& connectHandle, EnableTimeRegulationRequestMessage* message)
  {
    if (isRootServer()) {
      // Note that this message really loops back to the requestor.
      // The requestor needs to know which federates he needs to wait for.
      // see an explanation in the ambassaror's time management code
      _federateHandleTimeStampMap[message->getFederateHandle()] = message->getTimeStamp();
      broadcastToChildren(message);
    } else {
      if (connectHandle == _parentServerConnectHandle) {
        _federateHandleTimeStampMap[message->getFederateHandle()] = message->getTimeStamp();
        broadcastToChildren(message);
      } else {
        send(_parentServerConnectHandle, message);
        // FIXME:
        // If we really want to work well with a parent vanishing in between, we need to
        // store the outstanding requests here and on loosing connection to a parent complete
        // the role as root server ...
      }
    }
  }
  void accept(const ConnectHandle&, EnableTimeRegulationResponseMessage* message)
  {
    send(message->getFederateHandle(), message);
  }
  void accept(const ConnectHandle& connectHandle, DisableTimeRegulationRequestMessage* message)
  {
    // Don't bail out on anything. If the federate dies in between, we might need to clean up somehow
    broadcast(connectHandle, message);
    _federateHandleTimeStampMap.erase(message->getFederateHandle());
  }
  void accept(const ConnectHandle& connectHandle, CommitLowerBoundTimeStampMessage* message)
  {
    _federateHandleTimeStampMap[message->getFederateHandle()] = message->getTimeStamp();
    // send to all time constrainted connects except to where it originates
    // Hmm, send to all federates. The problem is that non time constrained federates
    // must be able to query the GALT for itself, which is only possible if they know the time advances
    // of each regulating federate, thus just broadcast
    broadcast(connectHandle, message);
  }

  // (un)publish messages for interactions
  void accept(const ConnectHandle& connectHandle, ChangeInteractionClassPublicationMessage* message)
  {
    InteractionClass* interactionClass = getInteractionClass(message->getInteractionClassHandle());
    if (!interactionClass)
      return;
    // Change publication type for this connect ...
    PropagationTypeConnectHandlePair propagationConnectPair;
    propagationConnectPair = interactionClass->setPublicationType(connectHandle, message->getPublicationType());
    // ... and propagate further if required.
    switch (propagationConnectPair.first) {
    case PropagateBroadcast:
      broadcast(connectHandle, message);
      break;
    case PropagateSend:
      send(propagationConnectPair.second, message);
      break;
    case PropagateNone:
      break;
    }

    // See if and how we should respond to that publication
    SubscriptionType subscriptionType = interactionClass->getSubscriptionTypeToConnect(connectHandle);
    if (subscriptionType != Unsubscribed) {
      SharedPtr<ChangeInteractionClassSubscriptionMessage> subscription;
      subscription = new ChangeInteractionClassSubscriptionMessage;
      subscription->setFederationHandle(getHandle());
      subscription->setInteractionClassHandle(interactionClass->getHandle());
      if (message->getPublicationType() == Published) {
        subscription->setSubscriptionType(subscriptionType);
      } else {
        subscription->setSubscriptionType(Unsubscribed);
      }
      send(connectHandle, subscription);
    }
  }
  void accept(const ConnectHandle& connectHandle, ChangeObjectClassPublicationMessage* message)
  {
    ObjectClass* objectClass = getObjectClass(message->getObjectClassHandle());
    if (!objectClass)
      return;

    // Build up the lists where to propagate what
    AttributeHandleVector broadcastAttributeHandles;
    std::map<ConnectHandle, AttributeHandleVector> sendAttributeHandlesMap;
    AttributeHandleVector passiveSubscribeAttributeHandles;
    AttributeHandleVector activeSubscribeAttributeHandles;
    for (AttributeHandleVector::const_iterator i = message->getAttributeHandles().begin();
         i != message->getAttributeHandles().end(); ++i) {
      ObjectClassAttribute* attribute = objectClass->getAttribute(*i);
      if (!attribute)
        continue;
      PropagationTypeConnectHandlePair propagationConnectPair;
      propagationConnectPair = attribute->setPublicationType(connectHandle, message->getPublicationType());
      switch (propagationConnectPair.first) {
      case PropagateBroadcast:
        broadcastAttributeHandles.push_back(*i);
        break;
      case PropagateSend:
        sendAttributeHandlesMap[propagationConnectPair.second].push_back(*i);
        break;
      case PropagateNone:
        break;
      }

      // See if and how we should respond to that publication
      SubscriptionType subscriptionType = attribute->getSubscriptionTypeToConnect(connectHandle);
      switch (subscriptionType) {
      case Unsubscribed:
        break;
      case SubscribedPassive:
        passiveSubscribeAttributeHandles.push_back(*i);
        break;
      case SubscribedActive:
        activeSubscribeAttributeHandles.push_back(*i);
        break;
      }
    }
    if (!broadcastAttributeHandles.empty()) {
      SharedPtr<ChangeObjectClassPublicationMessage> request;
      request = new ChangeObjectClassPublicationMessage;
      request->setFederationHandle(message->getFederationHandle());
      request->setObjectClassHandle(message->getObjectClassHandle());
      request->getAttributeHandles().swap(broadcastAttributeHandles);
      request->setPublicationType(message->getPublicationType());
      broadcast(connectHandle, request);
    }
    for (std::map<ConnectHandle, AttributeHandleVector>::iterator i = sendAttributeHandlesMap.begin();
         i != sendAttributeHandlesMap.end(); ++i) {
      SharedPtr<ChangeObjectClassPublicationMessage> request;
      request = new ChangeObjectClassPublicationMessage;
      request->setFederationHandle(message->getFederationHandle());
      request->setObjectClassHandle(message->getObjectClassHandle());
      request->getAttributeHandles().swap(i->second);
      request->setPublicationType(message->getPublicationType());
      send(i->first, request);
    }

    // Already existing subscriptions need to be sent to the new publishing connect
    if (!passiveSubscribeAttributeHandles.empty()) {
      SharedPtr<ChangeObjectClassSubscriptionMessage> subscription;
      subscription = new ChangeObjectClassSubscriptionMessage;
      subscription->setFederationHandle(getHandle());
      subscription->setObjectClassHandle(objectClass->getHandle());
      if (message->getPublicationType() == Published) {
        subscription->setSubscriptionType(SubscribedPassive);
      } else {
        subscription->setSubscriptionType(Unsubscribed);
      }
      subscription->getAttributeHandles().swap(passiveSubscribeAttributeHandles);
      send(connectHandle, subscription);
    }
    if (!activeSubscribeAttributeHandles.empty()) {
      SharedPtr<ChangeObjectClassSubscriptionMessage> subscription;
      subscription = new ChangeObjectClassSubscriptionMessage;
      subscription->setFederationHandle(getHandle());
      subscription->setObjectClassHandle(objectClass->getHandle());
      if (message->getPublicationType() == Published) {
        subscription->setSubscriptionType(SubscribedActive);
      } else {
        subscription->setSubscriptionType(Unsubscribed);
      }
      subscription->getAttributeHandles().swap(activeSubscribeAttributeHandles);
      send(connectHandle, subscription);
    }
  }

  void unpublishConnect(const ConnectHandle& connectHandle)
  {
    // Unpublish this connect by fake unpublish messages?!!
    for (InteractionClassVector::const_iterator j = getInteractionClassVector().begin();
         j != getInteractionClassVector().end(); ++j) {
      if ((*j)->getPublicationType(connectHandle) == Unpublished)
        continue;
      SharedPtr<ChangeInteractionClassPublicationMessage> message = new ChangeInteractionClassPublicationMessage;
      message->setFederationHandle(getHandle());
      message->setPublicationType(Unpublished);
      message->setInteractionClassHandle((*j)->getHandle());
      accept(connectHandle, message.get());
    }
    for (ObjectClassVector::const_iterator j = getObjectClassVector().begin();
         j != getObjectClassVector().end(); ++j) {
      AttributeHandleVector attributeHandleVector;
      for (ObjectClassAttributeVector::const_iterator k = (*j)->getObjectClassAttributeVector().begin();
           k != (*j)->getObjectClassAttributeVector().end(); ++k) {
        if ((*k)->getPublicationType(connectHandle) == Unpublished)
          continue;
        attributeHandleVector.reserve((*j)->getObjectClassAttributeVector().size());
        attributeHandleVector.push_back((*k)->getHandle());
      }
      if (attributeHandleVector.empty())
        continue;
      SharedPtr<ChangeObjectClassPublicationMessage> message = new ChangeObjectClassPublicationMessage;
      message->setFederationHandle(getHandle());
      message->setPublicationType(Unpublished);
      message->setObjectClassHandle((*j)->getHandle());
      message->getAttributeHandles().swap(attributeHandleVector);
      accept(connectHandle, message.get());
    }
  }


  // (un)subscription messages for interactions
  void accept(const ConnectHandle& connectHandle, ChangeInteractionClassSubscriptionMessage* message)
  {
    InteractionClass* interactionClass = getInteractionClass(message->getInteractionClassHandle());
    if (!interactionClass)
      throw MessageError("ChangeInteractionClassSubscriptionMessage for unknown InteractionClass!");
    // Change publication type for this connect ...
    PropagationTypeConnectHandlePair propagationConnectPair;
    propagationConnectPair = interactionClass->setSubscriptionType(connectHandle, message->getSubscriptionType());
    // Update the receiving connect handle set
    interactionClass->updateCumulativeSubscription(connectHandle);
    // ... and propagate further if required.
    switch (propagationConnectPair.first) {
    case PropagateBroadcast:
      send(interactionClass->getPublishingConnectHandleSet(), connectHandle, message);
      break;
    case PropagateSend:
      send(propagationConnectPair.second, message);
      break;
    case PropagateNone:
      break;
    }
  }

  // (un)subscription messages for object classes
  void accept(const ConnectHandle& connectHandle, ChangeObjectClassSubscriptionMessage* message)
  {
    ObjectClass* objectClass = getObjectClass(message->getObjectClassHandle());
    if (!objectClass)
      return;

    ConnectData* connect = getConnect(connectHandle);
    OpenRTIAssert(connect);

    ObjectInstanceList objectInstanceList;
    std::map<ConnectHandle, AttributeHandleVector> sendAttributeHandlesMap;
    for (std::vector<AttributeHandle>::const_iterator i = message->getAttributeHandles().begin();
         i != message->getAttributeHandles().end(); ++i) {
      ObjectClassAttribute* attribute = objectClass->getAttribute(*i);
      if (!attribute)
        continue;
      PropagationTypeConnectHandlePair propagationConnectPair;
      propagationConnectPair = attribute->setSubscriptionType(connectHandle, message->getSubscriptionType());
      switch (propagationConnectPair.first) {
      case PropagateNone:
        break;
      case PropagateBroadcast:
        for (ConnectHandleSet::const_iterator j = attribute->getPublishingConnectHandleSet().begin();
             j != attribute->getPublishingConnectHandleSet().end(); ++j) {
          if (*j == connectHandle)
            continue;
          sendAttributeHandlesMap[*j].push_back(*i);
        }
        break;
      case PropagateSend:
        if (attribute->getPublicationType(propagationConnectPair.second) == Published)
          sendAttributeHandlesMap[propagationConnectPair.second].push_back(*i);
        break;
      }

      objectClass->updateCumulativeSubscription(connectHandle, *i, objectInstanceList);
    }

    // Now propagate the subscription change further
    for (std::map<ConnectHandle, AttributeHandleVector>::iterator i = sendAttributeHandlesMap.begin();
         i != sendAttributeHandlesMap.end(); ++i) {
      SharedPtr<ChangeObjectClassSubscriptionMessage> request;
      request = new ChangeObjectClassSubscriptionMessage;
      request->setFederationHandle(message->getFederationHandle());
      request->setObjectClassHandle(message->getObjectClassHandle());
      request->getAttributeHandles().swap(i->second);
      request->setSubscriptionType(message->getSubscriptionType());
      send(i->first, request);
    }

    // Insert all object instances that are now new to this connect
    if (message->getSubscriptionType() != Unsubscribed) {
      for (ObjectInstanceList::const_iterator j = objectInstanceList.begin(); j != objectInstanceList.end(); ++j) {
        SharedPtr<InsertObjectInstanceMessage> request = new InsertObjectInstanceMessage;
        request->setFederationHandle(getHandle());
        request->setObjectInstanceHandle((*j)->getHandle());
        request->setObjectClassHandle((*j)->getObjectClass()->getHandle());
        request->setName((*j)->getName());
        request->getAttributeStateVector().reserve((*j)->getHandleObjectAttributeVector().size());
        for (HandleObjectAttributeVector::const_iterator k = (*j)->getHandleObjectAttributeVector().begin();
             k != (*j)->getHandleObjectAttributeVector().end(); ++k) {
          AttributeState attributeState;
          attributeState.setAttributeHandle((*k)->getHandle());
          request->getAttributeStateVector().push_back(attributeState);
        }
        if (connectHandle != _parentServerConnectHandle) {
          ObjectInstance* objectInstance = getObjectInstance((*j)->getHandle());
          if (!objectInstance)
            objectInstance = insertObjectInstanceHandle((*j)->getHandle(), (*j)->getName());
          objectInstance->referenceObjectInstance(connect);
        }
        connect->send(request);
      }
    }
  }
  void unsubscribeConnect(const ConnectHandle& connectHandle)
  {
    // Unsubscribe this connect by fake unsubscribe messages?!!
    for (InteractionClassVector::const_iterator j = getInteractionClassVector().begin();
         j != getInteractionClassVector().end(); ++j) {
      if ((*j)->getSubscriptionType(connectHandle) == Unsubscribed)
        continue;
      SharedPtr<ChangeInteractionClassSubscriptionMessage> message = new ChangeInteractionClassSubscriptionMessage;
      message->setFederationHandle(getHandle());
      message->setSubscriptionType(Unsubscribed);
      message->setInteractionClassHandle((*j)->getHandle());
      accept(connectHandle, message.get());
    }
    for (ObjectClassVector::const_iterator j = getObjectClassVector().begin();
         j != getObjectClassVector().end(); ++j) {
      AttributeHandleVector attributeHandleVector;
      for (ObjectClassAttributeVector::const_iterator k = (*j)->getObjectClassAttributeVector().begin();
           k != (*j)->getObjectClassAttributeVector().end(); ++k) {
        if ((*k)->getSubscriptionType(connectHandle) == Unsubscribed)
          continue;
        attributeHandleVector.reserve((*j)->getObjectClassAttributeVector().size());
        attributeHandleVector.push_back((*k)->getHandle());
      }
      if (attributeHandleVector.empty())
        continue;
      SharedPtr<ChangeObjectClassSubscriptionMessage> message = new ChangeObjectClassSubscriptionMessage;
      message->setFederationHandle(getHandle());
      message->setSubscriptionType(Unsubscribed);
      message->setObjectClassHandle((*j)->getHandle());
      message->getAttributeHandles().swap(attributeHandleVector);
      accept(connectHandle, message.get());
    }
  }

  // ObjectInstance handle management,
  // These are maintained at the root server. Clients can request handles from the root server
  // the root server then send a requested amount of unused object instance handles to the client.
  // The reponse is sent to the requesting federate and each server node on the way registers a
  // refrence of the recieving connect handle to this object instance handle.
  // An ambassador requests a bunch of handles at join time. Then, on object creation,
  // the ambassador has very likely some free handles available. So in effect we even have the
  // object instance registration without any latency. With every new registered object, the
  // Ambassador also requests a new instance handle to keep the pool of available handles in about the
  // same size. Only if latency is high and object registration rate is high also, the ambassador might block
  // on the registerObjectInstance call until a new free handle arrives.
  void accept(const ConnectHandle& connectHandle, ObjectInstanceHandlesRequestMessage* message)
  {
    if (isRootServer()) {
      ConnectData* connect = getConnect(connectHandle);

      // Provide some object instance handles to a federate.
      // This is to be completely asyncronous in the registerObjectInstance call.
      SharedPtr<ObjectInstanceHandlesResponseMessage> response;
      response = new ObjectInstanceHandlesResponseMessage;
      response->setFederationHandle(message->getFederationHandle());
      FederateHandle federateHandle = message->getFederateHandle();
      response->setFederateHandle(federateHandle);
      unsigned count = message->getCount();
      response->getObjectInstanceHandleNamePairVector().reserve(count);
      FederateHandleFederateMap::iterator i = _federateHandleFederateMap.find(federateHandle);
      while (count--) {
        ObjectInstance* objectInstance = insertObjectInstanceHandle();
        objectInstance->referenceObjectInstance(connect);
        response->getObjectInstanceHandleNamePairVector().push_back(ObjectInstanceHandleNamePair(objectInstance->getHandle(), objectInstance->getName()));
      }
      connect->send(response);
    } else {
      send(_parentServerConnectHandle, message);
    }
  }
  void accept(const ConnectHandle& connectHandle, ObjectInstanceHandlesResponseMessage* message)
  {
    FederateHandle federateHandle = message->getFederateHandle();
    Federate* federate = getFederate(federateHandle);
    if (!federate)
      throw MessageError("Got ObjectInstanceHandlesResponseMessage for an unknown federate!");

    ConnectData* federateConnect = federate->_connect;
    if (!federateConnect || federate->_resignPending) {
      // Can happen, may be it has resigned/is died in between but the response is already underway
      // If so, then just ignore, the upstream server needs to release them
    } else {
      ConnectHandle federateConnectHandle = federateConnect->getHandle();
      for (ObjectInstanceHandleNamePairVector::const_iterator k = message->getObjectInstanceHandleNamePairVector().begin();
           k != message->getObjectInstanceHandleNamePairVector().end(); ++k) {
        ObjectInstance* objectInstance = insertObjectInstanceHandle(k->first, k->second);
        objectInstance->referenceObjectInstance(federateConnect);
      }

      federateConnect->send(message);
    }
  }
  void accept(const ConnectHandle& connectHandle, ReleaseMultipleObjectInstanceNameHandlePairsMessage* message)
  {
    ConnectData* connect = getConnect(connectHandle);
    OpenRTIAssert(connect);

    SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> releaseMessage;
    for (ObjectInstanceHandleVector::const_iterator i = message->getObjectInstanceHandleVector().begin();
         i != message->getObjectInstanceHandleVector().end(); ++i) {
      ObjectInstance* objectInstance = getObjectInstance(*i);
      if (!objectInstance)
        throw MessageError("Got ReleaseMultipleObjectInstanceNameHandlePairsMessage for an unknown object instance!");
      if (!objectInstance->unreferenceObjectInstance(connect))
        continue;
      eraseObjectInstanceHandle(objectInstance);
      if (!_parentServerConnectHandle.valid())
        continue;
      if (!releaseMessage.valid()) {
        releaseMessage = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
        releaseMessage->setFederationHandle(getHandle());
        releaseMessage->getObjectInstanceHandleVector().reserve(message->getObjectInstanceHandleVector().size());
      }
      releaseMessage->getObjectInstanceHandleVector().push_back(*i);
    }
    if (releaseMessage.valid())
      send(_parentServerConnectHandle, releaseMessage);
  }

  // ObjectInstance name management
  // Object instance names are also coordinated by the root server.
  // FIXME: we would need to propagate and track these messages into the whole tree.
  // Just in case a disconnected server needs to know a consistent state.
  // Solved: A server just tracks what its children an itself has reserved.
  // So in case a branch is split away, reservations of now unreachable branches are not visible in the
  // current server node.
  // FIXME: should work for the instance handles too. As long as the root server is authoritive, it will know all,
  // Once the root is no longer reachable, it is sufficient to know what is here and below ...
  void accept(const ConnectHandle& connectHandle, ReserveObjectInstanceNameRequestMessage* message)
  {
    FederateHandle federateHandle = message->getFederateHandle();
    Federate* federate = getFederate(federateHandle);
    if (!federate)
      throw MessageError("Got ReserveObjectInstanceNameRequestMessage for an unknown federate!");
    // names starting with HLA are reserved for the RTI, a correct programmed ambassador does not request these
    if (message->getName().compare(0, 3, "HLA") == 0)
      throw MessageError("Got ReserveObjectInstanceNameRequestMessage with name starting with HLA.");

    if (isRootServer()) {
      ConnectData* connect = federate->_connect;
      OpenRTIAssert(connect);

      SharedPtr<ReserveObjectInstanceNameResponseMessage> response;
      response = new ReserveObjectInstanceNameResponseMessage;
      response->setFederationHandle(getHandle());
      response->setFederateHandle(federateHandle);
      if (!isObjectNameInUse(message->getName())) {
        ObjectInstance* objectInstance = insertObjectInstanceHandle(message->getName());
        objectInstance->referenceObjectInstance(connect);
        response->setObjectInstanceHandleNamePair(ObjectInstanceHandleNamePair(objectInstance->getHandle(), objectInstance->getName()));
        response->setSuccess(true);
      } else {
        ObjectInstanceHandleNamePair objectInstanceHandleNamePair(ObjectInstanceHandle(), message->getName());
        response->setObjectInstanceHandleNamePair(objectInstanceHandleNamePair);
        response->setSuccess(false);
      }
      connect->send(response);
    } else {
      send(_parentServerConnectHandle, message);
    }
  }
  void accept(const ConnectHandle& connectHandle, ReserveObjectInstanceNameResponseMessage* message)
  {
    FederateHandle federateHandle = message->getFederateHandle();
    Federate* federate = getFederate(federateHandle);
    if (!federate)
      throw MessageError("Got ReserveObjectInstanceNameResponseMessage for an unknown federate!");

    ConnectData* federateConnect = federate->_connect;
    if (!federateConnect || federate->_resignPending) {
      // Can happen, may be it has resigned/is died in between but the response is already underway
      // If so, then release the reservations.
    } else {
      if (message->getSuccess()) {
        ObjectInstance* objectInstance = insertObjectInstanceHandle(message->getObjectInstanceHandleNamePair().first,
                                                                    message->getObjectInstanceHandleNamePair().second);
        objectInstance->referenceObjectInstance(federateConnect);
      }
      federateConnect->send(message);
    }
  }
  void accept(const ConnectHandle& connectHandle, ReserveMultipleObjectInstanceNameRequestMessage* message)
  {
    FederateHandle federateHandle = message->getFederateHandle();
    Federate* federate = getFederate(federateHandle);
    if (!federate)
      throw MessageError("Got ReserveMultipleObjectInstanceNameRequestMessage for an unknown federate!");
    // names starting with HLA are reserved for the RTI, a correct programmed ambassador does not request these
    for (StringVector::const_iterator i = message->getNameList().begin(); i != message->getNameList().end(); ++i) {
      if (i->compare(0, 3, "HLA") == 0)
        throw MessageError("ReserveMultipleObjectInstanceNameRequestMessage with name starting with HLA.");
    }

    if (isRootServer()) {
      ConnectData* connect = federate->_connect;
      OpenRTIAssert(connect);

      SharedPtr<ReserveMultipleObjectInstanceNameResponseMessage> response;
      response = new ReserveMultipleObjectInstanceNameResponseMessage;
      response->setFederationHandle(message->getFederationHandle());
      response->setFederateHandle(message->getFederateHandle());
      response->getObjectInstanceHandleNamePairVector().reserve(message->getNameList().size());
      response->setSuccess(true);
      for (StringVector::const_iterator i = message->getNameList().begin(); i != message->getNameList().end(); ++i) {
        ObjectInstanceHandleNamePair objectInstanceHandleNamePair;
        objectInstanceHandleNamePair.second = *i;
        response->getObjectInstanceHandleNamePairVector().push_back(objectInstanceHandleNamePair);
        if (!isObjectNameInUse(*i))
          continue;
        response->setSuccess(false);
      }
      // If none of them is reserved or in use, reserve them all
      if (response->getSuccess()) {
        for (ObjectInstanceHandleNamePairVector::iterator j = response->getObjectInstanceHandleNamePairVector().begin();
             j != response->getObjectInstanceHandleNamePairVector().end(); ++j) {
          ObjectInstance* objectInstance = insertObjectInstanceHandle(j->second);
          objectInstance->referenceObjectInstance(connect);
          j->first = objectInstance->getHandle();
        }
      }
      connect->send(response);
    } else {
      send(_parentServerConnectHandle, message);
    }
  }
  void accept(const ConnectHandle& connectHandle, ReserveMultipleObjectInstanceNameResponseMessage* message)
  {
    FederateHandle federateHandle = message->getFederateHandle();
    Federate* federate = getFederate(federateHandle);
    if (!federate)
      throw MessageError("Got ReserveMultipleObjectInstanceNameResponseMessage for an unknown federate!");

    ConnectData* federateConnect = federate->_connect;
    if (!federateConnect || federate->_resignPending) {
      // Can happen, may be it has resigned/is died in between but the response is already underway
      // If so, then release the reservations.
    } else {
      if (message->getSuccess()) {
        for (ObjectInstanceHandleNamePairVector::const_iterator k = message->getObjectInstanceHandleNamePairVector().begin();
             k != message->getObjectInstanceHandleNamePairVector().end(); ++k) {
          ObjectInstance* objectInstance = insertObjectInstanceHandle(k->first, k->second);
          objectInstance->referenceObjectInstance(federateConnect);
        }
      }

      federateConnect->send(message);
    }
  }


  // Object instance messages
  void accept(const ConnectHandle& connectHandle, InsertObjectInstanceMessage* message)
  {
    ObjectInstanceHandle objectInstanceHandle = message->getObjectInstanceHandle();
    ObjectClassHandle objectClassHandle = message->getObjectClassHandle();
    ObjectClass* objectClass = getObjectClass(objectClassHandle);
    if (!objectClass)
      throw MessageError("InsertObjectInstanceMessage for unknown ObjectClass.");

    // FIXME Improove this with preevaluated sets:
    // std::map<ConnectHandle,ConnectHandleSet> ...
    ObjectInstance* objectInstance = getObjectInstance(objectInstanceHandle);
    ObjectClassAttribute* privilegeToDeleteAttribute = objectClass->getPrivilegeToDeleteAttribute();
    if (privilegeToDeleteAttribute) {
      for (ConnectHandleSet::iterator j = privilegeToDeleteAttribute->_cumulativeSubscribedConnectHandleSet.begin();
           j != privilegeToDeleteAttribute->_cumulativeSubscribedConnectHandleSet.end(); ++j) {
        if (*j == _parentServerConnectHandle)
          continue;
        if (*j == connectHandle)
          continue;
        if (!objectInstance)
          objectInstance = insertObjectInstanceHandle(objectInstanceHandle, message->getName());
        objectInstance->referenceObjectInstance(getConnect(*j));
      }
    }

    // If still unreferenced, ignore the insert and unref again in the parent
    // this can happen if we subscribed and unsubscribed at the server before we recieved the insert that is triggered by the subscribe request.
    if (!objectInstance) {
      OpenRTIAssert(connectHandle == _parentServerConnectHandle);

      SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> message;
      message = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
      message->setFederationHandle(getHandle());
      message->getObjectInstanceHandleVector().push_back(objectInstanceHandle);

      send(_parentServerConnectHandle, message);

    } else {
      OpenRTIAssert(!objectInstance->_connectHandleObjectInstanceConnectMap.empty());

      objectInstance->setObjectClass(objectClass);
      for (size_t j = 0; j < message->getAttributeStateVector().size(); ++j) {
        ObjectAttribute* attribute = objectInstance->getAttribute(message->getAttributeStateVector()[j].getAttributeHandle());
        attribute->setOwnerConnectHandle(connectHandle);
      }

      send(objectInstance->getPrivilegeToDeleteAttribute()->_recieveingConnects, message);
    }
  }
  void accept(const ConnectHandle& connectHandle, DeleteObjectInstanceMessage* message)
  {
    // If the object class is already unsubscribed, we might still get delete instance or update messages
    // That are sent by the owner at a time the subscription was still there.
    // So This is not an error. FIXME: if we do explicit instance removal in parent to child order we can
    // make that am error again.
    ObjectInstance* objectInstance = getObjectInstance(message->getObjectInstanceHandle());
    if (!objectInstance)
      return;

    // send that to all servers that have seen that object instance at some time
    OpenRTIAssert(objectInstance->getPrivilegeToDeleteAttribute()->_recieveingConnects.count(connectHandle) == 0);
    send(objectInstance->getPrivilegeToDeleteAttribute()->_recieveingConnects, message);
  }

  void accept(const ConnectHandle& connectHandle, AttributeUpdateMessage* message)
  {
    ObjectInstance* objectInstance = getObjectInstance(message->getObjectInstanceHandle());
    if (!objectInstance)
      return;

    // FIXME: this can be improoved:
    // An update message always stems from a single federate, which means that we should already be able to
    // have a preprepared list of connect handle sets associated with a maximum attribute handle value for this message.
    // If this list is trivial we do not need to make any copy of the message, just broadcast to this set of connects.
    // May be each object instance can have a callback attached which either does complex copy stuff or just forwards as possible?

    typedef std::map<ConnectHandle, AttributeValueVector> ConnectHandleAttributeValueVectorMap;
    ConnectHandleAttributeValueVectorMap connectHandleAttributeValueVectorMap;
    for (AttributeValueVector::const_iterator i = message->getAttributeValues().begin();
         i != message->getAttributeValues().end(); ++i) {
      ObjectAttribute* objectAttribute = objectInstance->getAttribute(i->getAttributeHandle());
      if (!objectAttribute)
        continue;
      for (ConnectHandleSet::const_iterator j = objectAttribute->_recieveingConnects.begin();
           j != objectAttribute->_recieveingConnects.end(); ++j) {
        connectHandleAttributeValueVectorMap[*j].reserve(message->getAttributeValues().size());
        connectHandleAttributeValueVectorMap[*j].push_back(*i);
      }
    }

    for (ConnectHandleAttributeValueVectorMap::iterator i = connectHandleAttributeValueVectorMap.begin();
          i != connectHandleAttributeValueVectorMap.end(); ++i) {
      SharedPtr<AttributeUpdateMessage> update = new AttributeUpdateMessage;
      update->setFederationHandle(getHandle());
      update->setObjectInstanceHandle(message->getObjectInstanceHandle());
      update->setTag(message->getTag());
      update->setTransportationType(message->getTransportationType());
      update->getAttributeValues().swap(i->second);
      send(i->first, update);
    }
  }
  void accept(const ConnectHandle& connectHandle, TimeStampedAttributeUpdateMessage* message)
  {
    ObjectInstance* objectInstance = getObjectInstance(message->getObjectInstanceHandle());
    if (!objectInstance)
      return;

    // See the above improovements for the AttributeUpdateMessage FIXME

    typedef std::map<ConnectHandle, AttributeValueVector> ConnectHandleAttributeValueVectorMap;
    ConnectHandleAttributeValueVectorMap connectHandleAttributeValueVectorMap;
    for (AttributeValueVector::const_iterator i = message->getAttributeValues().begin();
         i != message->getAttributeValues().end(); ++i) {
      ObjectAttribute* objectAttribute = objectInstance->getAttribute(i->getAttributeHandle());
      if (!objectAttribute)
        continue;
      for (ConnectHandleSet::const_iterator j = objectAttribute->_recieveingConnects.begin();
           j != objectAttribute->_recieveingConnects.end(); ++j) {
        connectHandleAttributeValueVectorMap[*j].reserve(message->getAttributeValues().size());
        connectHandleAttributeValueVectorMap[*j].push_back(*i);
      }
    }

    for (ConnectHandleAttributeValueVectorMap::iterator i = connectHandleAttributeValueVectorMap.begin();
          i != connectHandleAttributeValueVectorMap.end(); ++i) {
      SharedPtr<TimeStampedAttributeUpdateMessage> update = new TimeStampedAttributeUpdateMessage;
      update->setFederationHandle(getHandle());
      update->setObjectInstanceHandle(message->getObjectInstanceHandle());
      update->setTag(message->getTag());
      update->setTimeStamp(message->getTimeStamp());
      update->setMessageRetractionHandle(message->getMessageRetractionHandle());
      update->setTransportationType(message->getTransportationType());
      update->getAttributeValues().swap(i->second);
      send(i->first, update);
    }
  }


  // Send interactions due to the noted rounting tables
  void accept(const ConnectHandle& connectHandle, InteractionMessage* message)
  {
    InteractionClass* interactionClass = getInteractionClass(message->getInteractionClassHandle());
    // This might happen with FOM modules
    if (!interactionClass)
      return;
    // Send to all subscribed connects except the originating one
    send(interactionClass->_cumulativeSubscribedConnectHandleSet, connectHandle, message);
  }
  void accept(const ConnectHandle& connectHandle, TimeStampedInteractionMessage* message)
  {
    InteractionClass* interactionClass = getInteractionClass(message->getInteractionClassHandle());
    // This might happen with FOM modules
    if (!interactionClass)
      return;
    // Send to all subscribed connects except the originating one
    send(interactionClass->_cumulativeSubscribedConnectHandleSet, connectHandle, message);
  }

  void accept(const ConnectHandle& connectHandle, RequestAttributeUpdateMessage* message)
  {
    typedef std::map<ConnectHandle, SharedPtr<RequestAttributeUpdateMessage> > ConnectMessageMap;

    ObjectInstanceHandle objectInstanceHandle = message->getObjectInstanceHandle();
    ObjectInstance* objectInstance = getObjectInstance(objectInstanceHandle);
    if (!objectInstance)
      return;

    // Find the server connects that own the attributes and build up a message for those
    ConnectMessageMap connectMessageMap;
    for (AttributeHandleVector::const_iterator i = message->getAttributeHandles().begin();
         i != message->getAttributeHandles().end(); ++i) {
      const ObjectAttribute* objectAttribute = objectInstance->getAttribute(*i);
      if (!objectAttribute) // FIXME this is more an error ...
        continue;
      // The connect handle that owns this attribute
      ConnectHandle connectHandle = objectAttribute->getOwnerConnectHandle();
      if (!connectHandle.valid())
        continue;
      ConnectMessageMap::iterator k = connectMessageMap.find(connectHandle);
      if (k == connectMessageMap.end()) {
        k = connectMessageMap.insert(ConnectMessageMap::value_type(connectHandle, new RequestAttributeUpdateMessage)).first;
        k->second->setFederationHandle(getHandle());
        k->second->setObjectInstanceHandle(objectInstanceHandle);
        k->second->setTag(message->getTag());
        k->second->getAttributeHandles().reserve(message->getAttributeHandles().size());
      }
      k->second->getAttributeHandles().push_back(*i);
    }
    for (ConnectMessageMap::iterator i = connectMessageMap.begin(); i != connectMessageMap.end(); ++i) {
      send(i->first, i->second);
    }
  }
  void accept(const ConnectHandle& connectHandle, RequestClassAttributeUpdateMessage* message)
  {
    typedef std::map<ConnectHandle, SharedPtr<RequestClassAttributeUpdateMessage> > ConnectMessageMap;

    ObjectInstanceHandle objectClassHandle = message->getObjectClassHandle();
    ObjectClass* objectClass = getObjectClass(objectClassHandle);
    if (!objectClass)
      throw MessageError("Received RequestClassAttributeUpdateMessage for unknown object class!");

    // Find the server connects that own the attributes and build up a message for those
    ConnectMessageMap connectMessageMap;
    for (AttributeHandleVector::const_iterator i = message->getAttributeHandles().begin();
         i != message->getAttributeHandles().end(); ++i) {
      // FIXME: improove that, less allocations, Probably use std::list of connect handles, insert, sort, unique, pool of handle elements
      ConnectHandleSet connectHandleSet;
      for (ObjectInstanceList::const_iterator l = objectClass->getObjectInstanceList().begin();
           l != objectClass->getObjectInstanceList().end(); ++l) {
        const ObjectAttribute* objectAttribute = (*l)->getAttribute(*i);
        if (!objectAttribute) // FIXME this is more an error ...
          continue;
        // The connect handle that owns this attribute
        ConnectHandle connectHandle = objectAttribute->getOwnerConnectHandle();
        if (!connectHandle.valid())
          continue;
        connectHandleSet.insert(connectHandle);
      }
      for (ConnectHandleSet::const_iterator l = connectHandleSet.begin();
           l != connectHandleSet.end(); ++l) {
        ConnectMessageMap::iterator k = connectMessageMap.find(*l);
        if (k == connectMessageMap.end()) {
          k = connectMessageMap.insert(ConnectMessageMap::value_type(connectHandle, new RequestClassAttributeUpdateMessage)).first;
          k->second->setFederationHandle(getHandle());
          k->second->setObjectClassHandle(objectClassHandle);
          k->second->setTag(message->getTag());
          k->second->getAttributeHandles().reserve(message->getAttributeHandles().size());
        }
        k->second->getAttributeHandles().push_back(*i);
      }
    }
    for (ConnectMessageMap::iterator i = connectMessageMap.begin(); i != connectMessageMap.end(); ++i) {
      send(i->first, i->second);
    }
  }

  template<typename M>
  void acceptFederationMessage(const ConnectHandle& connectHandle, M* message)
  {
    OpenRTIAssert(_connectHandleConnectDataMap.find(connectHandle) != _connectHandleConnectDataMap.end());
    accept(connectHandle, message);
  }


  // utilities for connecthandle/federatehandle handling

  void insertParentConnect(const ConnectHandle& connectHandle, const SharedPtr<AbstractMessageSender>& messageSender, const std::string& name)
  {
    ConnectData* connectData = ServerObjectModel::getOrInsertParentConnect(connectHandle);
    OpenRTIAssert(connectData);
    OpenRTIAssert(!connectData->_messageSender.valid());
    connectData->_messageSender = messageSender;
    connectData->_name = name;
  }
  void insertConnect(const ConnectHandle& connectHandle, const SharedPtr<AbstractMessageSender>& messageSender, const std::string& name)
  {
    ConnectData* connectData = ServerObjectModel::getOrInsertConnect(connectHandle);
    OpenRTIAssert(connectData);
    if (connectData->_messageSender.valid()) {
      OpenRTIAssert(connectData->_messageSender == messageSender);
      return;
    }

    connectData->_messageSender = messageSender;
    connectData->_name = name;

    SharedPtr<InsertFederationExecutionMessage> message = new InsertFederationExecutionMessage;
    message->setFederationHandle(getHandle());
    message->setFederationName(getName());
    message->setLogicalTimeFactoryName(getLogicalTimeFactoryName());
    // FIXME add the server options

    // FIXME push them as required
    message->setFOMModuleList(getModuleList());
    messageSender->send(message);

    /// FIXME currently these are all flushed when an EraseFederationExecutionMessage is recieved.
    /// FIXME Make that more explicit????
    for (FederateHandleFederateMap::const_iterator i = _federateHandleFederateMap.begin();
         i != _federateHandleFederateMap.end(); ++i) {
      SharedPtr<JoinFederateNotifyMessage> notify = new JoinFederateNotifyMessage;
      notify->setFederationHandle(getHandle());
      notify->setFederateHandle(i->first);
      notify->setFederateType(i->second->_federateType);
      notify->setFederateName(i->second->getName());
      messageSender->send(notify);
    }

    for (FederateHandleTimeStampMap::const_iterator i = _federateHandleTimeStampMap.begin();
         i != _federateHandleTimeStampMap.end(); ++i) {
      SharedPtr<EnableTimeRegulationRequestMessage> enable = new EnableTimeRegulationRequestMessage;
      enable->setFederationHandle(getHandle());
      enable->setFederateHandle(i->first);
      enable->setTimeStamp(i->second);
      messageSender->send(enable);
    }

    pushPublications(connectHandle);
  }

  // Should be called when a connection dies
  void removeConnect(const ConnectHandle& connectHandle)
  {
    ConnectHandleConnectDataMap::iterator i = _connectHandleConnectDataMap.find(connectHandle);
    if (i == _connectHandleConnectDataMap.end())
      return;

    resignConnect(connectHandle);
    eraseConnect(connectHandle);
  }

  // Should be called when a connection dies
  void resignConnect(const ConnectHandle& connectHandle)
  {
    ConnectData* connect = getConnect(connectHandle);
    OpenRTIAssert(connect);

    // Unsubscribe this connect
    unsubscribeConnect(connectHandle);

    // FIXME avoid this loop over all objects
    // FIXME avoid precollecting these in the object handle set
    ObjectInstanceHandleSet objectInstanceHandleSet;
    for (ObjectInstanceHandleObjectInstanceMap::iterator i = _objectInstanceHandleObjectInstanceMap.begin();
         i != _objectInstanceHandleObjectInstanceMap.end(); ++i) {
      if (i->second->getOwnerConnectHandle() != connectHandle)
        continue;

      // FIXME: currently we do not have ownership management - so, if the owner dies the object needs to die too
      bool deleteObject = true;

      if (deleteObject) {
        objectInstanceHandleSet.insert(i->first);
      } else {
        i->second->setOwnerConnectHandle(ConnectHandle());
      }
    }
    for (ObjectInstanceHandleSet::iterator j = objectInstanceHandleSet.begin();
         j != objectInstanceHandleSet.end(); ++j) {
      SharedPtr<DeleteObjectInstanceMessage> request;
      request = new DeleteObjectInstanceMessage;
      request->setFederationHandle(getHandle());
      request->setObjectInstanceHandle(*j);
      accept(connectHandle, request.get());
    }

    if (connectHandle != _parentServerConnectHandle) {
      SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> releaseMessage;
      for (ObjectInstanceConnectList::iterator j = connect->_objectInstanceConnectList.begin();
           j != connect->_objectInstanceConnectList.end();) {
        ObjectInstanceConnect* objectInstanceConnect = *(j++);
        ObjectInstance* objectInstance = objectInstanceConnect->getObjectInstance();
        if (!objectInstance->unreferenceObjectInstance(objectInstanceConnect))
          continue;
        ObjectInstanceHandle objectInstanceHandle = objectInstance->getHandle();
        eraseObjectInstanceHandle(objectInstance);
        if (!_parentServerConnectHandle.valid())
          continue;
        if (!releaseMessage.valid()) {
          releaseMessage = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
          releaseMessage->setFederationHandle(getHandle());
          releaseMessage->getObjectInstanceHandleVector().reserve(_objectInstanceHandleObjectInstanceMap.size());
        }
        releaseMessage->getObjectInstanceHandleVector().push_back(objectInstanceHandle);
      }
      if (releaseMessage.valid())
        send(_parentServerConnectHandle, releaseMessage);
      OpenRTIAssert(connect->_objectInstanceConnectList.empty());
    }

    // Unpublish this connect
    unpublishConnect(connectHandle);

    ConnectHandleConnectDataMap::iterator i = _connectHandleConnectDataMap.find(connectHandle);
    if (i != _connectHandleConnectDataMap.end()) {
      for (FederateList::iterator j = i->second->_federateList.begin(); j != i->second->_federateList.end();) {
        Federate* federate = *(j++);
        i->second->eraseFederate(federate);
        Log(ServerFederate, Info) << getServerPath() << ": Resigning federate " << federate->getHandle()
                                  << " because of closed connection!" << std::endl;
        SharedPtr<ResignFederationExecutionRequestMessage> message = new ResignFederationExecutionRequestMessage;
        message->setFederationHandle(getHandle());
        message->setFederateHandle(federate->getHandle());
        accept(connectHandle, message.get());
      }
      OpenRTIAssert(i->second->_federateList.empty());
    }

    ServerObjectModel::removeConnect(connectHandle);

    // FIXME if the removed connection is the parent and we have a resign pending, respond as if we were the root
  }


  void broadcastToChildren(const SharedPtr<AbstractMessage>& message) const
  {
    for (ConnectHandleConnectDataMap::const_iterator i = _connectHandleConnectDataMap.begin();
	 i != _connectHandleConnectDataMap.end(); ++i) {
      if (!i->first.valid())
	continue;
      if (i->first == _parentServerConnectHandle)
	continue;
      i->second->send(message);
    }
  }

  void broadcastToChildren(const ConnectHandle& connectHandle, const SharedPtr<AbstractMessage>& message) const
  {
    for (ConnectHandleConnectDataMap::const_iterator i = _connectHandleConnectDataMap.begin();
	 i != _connectHandleConnectDataMap.end(); ++i) {
      if (!i->first.valid())
	continue;
      if (i->first == _parentServerConnectHandle)
	continue;
      if (i->first == connectHandle)
	continue;
      i->second->send(message);
    }
  }

  void broadcastToChildren(const FederateHandleSet& federateHandleSet, const SharedPtr<AbstractMessage>& message) const
  {
    ConnectHandleSet broadcastSet;
    for (FederateHandleSet::const_iterator i = federateHandleSet.begin(); i != federateHandleSet.end(); ++i) {
      FederateHandleFederateMap::const_iterator j = _federateHandleFederateMap.find(*i);
      if (j == _federateHandleFederateMap.end())
        continue;
      Federate* federate = j->second.get();
      if (!federate)
        continue;
      ConnectHandle connectHandle = federate->getConnectHandle();
      if (!connectHandle.valid())
        continue;
      if (connectHandle == _parentServerConnectHandle)
        continue;
      broadcastSet.insert(connectHandle);
    }
    send(broadcastSet, message);
  }

  // send to all in the set except the additionally given one
  void send(const ConnectHandleSet& connectHandleSet, const ConnectHandle& connectHandle, const SharedPtr<AbstractMessage>& message) const
  {
    // FIXME is currently O(n*log(n)) can be O(n)
    for (ConnectHandleSet::const_iterator i = connectHandleSet.begin(); i != connectHandleSet.end(); ++i) {
      if (*i == connectHandle)
	continue;
      send(*i, message);
    }
  }

  void send(const ConnectHandleSet& connectHandleSet, const SharedPtr<AbstractMessage>& message) const
  {
    // FIXME is currently O(n*log(n)) can be O(n)
    for (ConnectHandleSet::const_iterator i = connectHandleSet.begin(); i != connectHandleSet.end(); ++i)
      send(*i, message);
  }

  void broadcast(const ConnectHandle& connectHandle, const SharedPtr<AbstractMessage>& message) const
  {
    for (ConnectHandleConnectDataMap::const_iterator i = _connectHandleConnectDataMap.begin();
	 i != _connectHandleConnectDataMap.end(); ++i) {
      if (!i->first.valid())
	continue;
      if (i->first == connectHandle)
	continue;
      i->second->send(message);
    }
  }
  void send(const ConnectHandle& connectHandle, const SharedPtr<AbstractMessage>& message) const
  {
    // ConnectData* connect = getConnect(connectHandle);
    ConnectHandleConnectDataMap::const_iterator i = _connectHandleConnectDataMap.find(connectHandle);
    OpenRTIAssert(i != _connectHandleConnectDataMap.end());
    i->second->send(message);
  }
  void send(const FederateHandle& federateHandle, const SharedPtr<AbstractMessage>& message) const
  {
    FederateHandleFederateMap::const_iterator i = _federateHandleFederateMap.find(federateHandle);
    if (i == _federateHandleFederateMap.end())
      return;
    ConnectData* connect = i->second->_connect;
    if (!connect)
      return;
    connect->send(message);
  }

  /// The rti servers options
  SharedPtr<const ServerOptions> _serverOptions;
};


class OPENRTI_LOCAL ServerMessageDispatcher : public Referenced {
public:
  ServerMessageDispatcher(const SharedPtr<ServerOptions>& serverOptions) :
    _serverOptions(serverOptions)
  { }

  template<typename M>
  void acceptFederationMessage(const ConnectHandle& connectHandle, M* message)
  {
    FederationServerMap::const_iterator i = _federationServerMap.find(message->getFederationHandle());
    if (i == _federationServerMap.end()) {
      Log(ServerFederation, Warning) << getServerPath() << ": Received " << message->getTypeName()
                                     << " for unknown federation id: " << message->getFederationHandle()
                                     << "!" << std::endl;
      throw MessageError(getServerPath() + std::string(" received ") + message->getTypeName()
                         + " for unknown federation id: " + message->getFederationHandle().toString() + "!");
    }
    i->second->acceptFederationMessage(connectHandle, message);
  }
  template<typename M>
  void acceptUpstreamFederationMessage(const ConnectHandle& connectHandle, M* message)
  {
    OpenRTIAssert(connectHandle.valid());
    if (connectHandle == _serverConnectSet.getParentConnectHandle())
      throw MessageError(std::string("Received ") + message->getTypeName() + " through the parent connect!");
    acceptFederationMessage(connectHandle, message);
  }
  template<typename M>
  void acceptDownstreamFederationMessage(const ConnectHandle& connectHandle, M* message)
  {
    OpenRTIAssert(connectHandle.valid());
    if (connectHandle != _serverConnectSet.getParentConnectHandle())
      throw MessageError(std::string("Received ") + message->getTypeName() + " through a child connect!");
    acceptFederationMessage(connectHandle, message);
  }

  // If the parent connect dies, tell this all children
  void accept(const ConnectHandle& connectHandle, ConnectionLostMessage* message)
  {
    // Throw away these kind of messages when they originate from a child.
    // This can happen since a connect just sends this message on socket problems.
    // But the socket connect does not know if it is a parent connect or not.
    if (connectHandle != _serverConnectSet.getParentConnectHandle())
      return;
    _serverConnectSet.broadcastToChildren(message);
  }

  // Create messages
  void accept(const ConnectHandle& connectHandle, CreateFederationExecutionRequestMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    // If we are a root server ...
    if (isRootServer()) {
      // Check if it is already there?
      FederationServerMap::const_iterator i = _federationServerMap.find(message->getFederationExecution());
      if (i != _federationServerMap.end()) {
        // Ok, have an active federation, create needs to fail
        SharedPtr<CreateFederationExecutionResponseMessage> response;
        response = new CreateFederationExecutionResponseMessage;
        response->setCreateFederationExecutionResponseType(CreateFederationExecutionResponseFederationExecutionAlreadyExists);
        _serverConnectSet.send(connectHandle, response);
      } else {
        try {
          // Successful create
          FederationHandleAllocator::Candidate candidate(_federationHandleAllocator);
          FederationHandle federationHandle = candidate.get();

          SharedPtr<FederationServer> federationServer;
          federationServer = new FederationServer(message->getFederationExecution(), federationHandle, _serverOptions);
          federationServer->setLogicalTimeFactoryName(message->getLogicalTimeFactoryName());

          FOMModuleHandleSet fomModuleHandleSet;
          fomModuleHandleSet = federationServer->_fomModuleSet.insertModuleList(message->getFOMStringModuleList());
          FOMModuleList moduleList = federationServer->_fomModuleSet.getModuleList();
          federationServer->insert(moduleList);

          // register this one
          candidate.take();
          _federationServerMap.insert(FederationServerMap::value_type(federationHandle, federationServer));

          Log(ServerFederation, Info) << getServerPath() << ": Create federation execution \""
                                      << message->getFederationExecution() << "\"." << std::endl;

          // ... and respond with Success
          SharedPtr<CreateFederationExecutionResponseMessage> response;
          response = new CreateFederationExecutionResponseMessage;
          response->setCreateFederationExecutionResponseType(CreateFederationExecutionResponseSuccess);
          _serverConnectSet.send(connectHandle, response);

        } catch (const Exception& e) {
          Log(ServerFederation, Info) << getServerPath() << ": Caught Exception creating federation execution \""
                                      << e.getReason() << "\"." << std::endl;
          SharedPtr<CreateFederationExecutionResponseMessage> response;
          response = new CreateFederationExecutionResponseMessage;
          response->setCreateFederationExecutionResponseType(CreateFederationExecutionResponseRTIinternalError);
          response->setExceptionString(e.getReason());
          _serverConnectSet.send(connectHandle, response);
        }
      }
    }
    // If we have an upstream connect, mark this request as pending and ask the parent server
    else {
      if (connectHandle == _serverConnectSet.getParentConnectHandle())
        throw MessageError("Received CreateExecutionRequestMessage through the parent connect!");

      _pendingMessageList.push_back(ConnectHandleMessagePair(connectHandle, message));
      _serverConnectSet.sendToParent(message);
    }
  }
  void accept(const ConnectHandle& connectHandle, CreateFederationExecutionResponseMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    // Such a response must originate from the parent.
    if (connectHandle != _serverConnectSet.getParentConnectHandle())
      throw MessageError("Received CreateExecutionResponseMessage through a child connect!");

    // need to have a connect handle to resond to
    if (_pendingMessageList.empty())
      throw MessageError("No pending CreateExecutionRequestMessage but received CreateFederationExecutionResponseMessage!");

    // Report downstream to the originator
    if (_pendingMessageList.front().first.valid())
        _serverConnectSet.send(_pendingMessageList.front().first, message);
    // remove the pending message that succeeded
    _pendingMessageList.pop_front();
  }

  // Destroy messages
  void accept(const ConnectHandle& connectHandle, DestroyFederationExecutionRequestMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    // If we are a root server ...
    if (isRootServer()) {

      // Check if it is there?
      FederationServerMap::iterator i = _federationServerMap.find(message->getFederationExecution());
      if (i == _federationServerMap.end()) {
        Log(ServerFederation, Debug) << getServerPath()
                                     << ": DestroyFederationExecutionRequestMessage faild for unknown federation named \""
                                     << message->getFederationExecution() << "\"!" << std::endl;
        // Ok, have an inactive federation, destroy needs to fail
        SharedPtr<DestroyFederationExecutionResponseMessage> response;
        response = new DestroyFederationExecutionResponseMessage;
        response->setDestroyFederationExecutionResponseType(DestroyFederationExecutionResponseFederationExecutionDoesNotExist);
        _serverConnectSet.send(connectHandle, response);
      } else {

        // Federates currently joined?
        if (i->second->hasJoinedFederates()) {
          Log(ServerFederation, Debug) << getServerPath() << ": DestroyFederationExecutionRequestMessage faild for \""
                                       << message->getFederationExecution() << "\", federates joined!" << std::endl;
          // federades there, so, no
          SharedPtr<DestroyFederationExecutionResponseMessage> response;
          response = new DestroyFederationExecutionResponseMessage;
          response->setDestroyFederationExecutionResponseType(DestroyFederationExecutionResponseFederatesCurrentlyJoined);
          _serverConnectSet.send(connectHandle, response);
        } else {
          // Successful destroy
          Log(ServerFederation, Info) << getServerPath() << ": DestroyFederationExecutionRequestMessage succeeded for \""
                                      << message->getFederationExecution() << "\"!" << std::endl;

          if (i->second->hasChildConnects()) {
            // ... we are not the lower most server node that still knows that federation,
            // so forward this to all children.
            i->second->broadcastEraseFederationExecution();
            eraseFederationName(i);
          } else {
            // ... we are the lower most server node that still knows about that federation,
            // so just throw away the federation handle.
            eraseFederation(i);
          }

          // ... and just respond with Success
          SharedPtr<DestroyFederationExecutionResponseMessage> response;
          response = new DestroyFederationExecutionResponseMessage;
          response->setDestroyFederationExecutionResponseType(DestroyFederationExecutionResponseSuccess);
          _serverConnectSet.send(connectHandle, response);
        }
      }
    }
    // If we have an upstream connect, mark this request as pending and ask the parent server
    else {
      if (connectHandle == _serverConnectSet.getParentConnectHandle())
        throw MessageError("Received DestroyExecutionRequestMessage through the parent connect!");

      // ... ask your father
      _pendingMessageList.push_back(ConnectHandleMessagePair(connectHandle, message));
      _serverConnectSet.sendToParent(message);
    }
  }
  void accept(const ConnectHandle& connectHandle, DestroyFederationExecutionResponseMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    // Such a response must originate from the parent.
    if (connectHandle != _serverConnectSet.getParentConnectHandle())
      throw MessageError("Received DestroyExecutionResponseMessage through a child connect!");

    // need to have a connect handle to resond to
    if (_pendingMessageList.empty())
      throw MessageError("No pending DestroyExecutionResponseMessage even in FederateDestroyPending state!");

    // Report downstream to the originator
    if (_pendingMessageList.front().first.valid())
      _serverConnectSet.send(_pendingMessageList.front().first, message);
    // remove the pending message that succeeded
    _pendingMessageList.pop_front();
  }


  // Enumerate federations
  void accept(const ConnectHandle& connectHandle, EnumerateFederationExecutionsRequestMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    // If we are a root server ...
    if (isRootServer()) {
      SharedPtr<EnumerateFederationExecutionsResponseMessage> response;
      response->getFederationExecutionInformationVector().reserve(_federationServerMap.size());
      for (FederationServerMap::iterator i = _federationServerMap.begin(); i != _federationServerMap.end(); ++i) {
        FederationExecutionInformation federationExecutionInformation;
        federationExecutionInformation.setFederationExecutionName(i->second->getName());
        federationExecutionInformation.setLogicalTimeFactoryName(i->second->getLogicalTimeFactoryName());
        response->getFederationExecutionInformationVector().push_back(federationExecutionInformation);
      }
      _serverConnectSet.send(connectHandle, response);
    }
    // If we have an upstream connect, mark this request as pending and ask the parent server
    else {
      if (connectHandle == _serverConnectSet.getParentConnectHandle())
        throw MessageError("Received EnumerateFederationExecutionsRequestMessage through the parent connect!");

      // ... ask your father
      _pendingMessageList.push_back(ConnectHandleMessagePair(connectHandle, message));
      _serverConnectSet.sendToParent(message);
    }
  }
  void accept(const ConnectHandle& connectHandle, EnumerateFederationExecutionsResponseMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    // Such a response must originate from the parent.
    if (connectHandle != _serverConnectSet.getParentConnectHandle())
      throw MessageError("Received EnumerateFederationExecutionsResponseMessage through a child connect!");

    // need to have a connect handle to resond to
    if (_pendingMessageList.empty())
      throw MessageError("No pending EnumerateFederationExecutionsResponseMessage!");

    // Report downstream to the originator
    if (_pendingMessageList.front().first.valid())
      _serverConnectSet.send(_pendingMessageList.front().first, message);
    // remove the pending message that succeeded
    _pendingMessageList.pop_front();
  }

  // Insert a new federation into the server node
  void accept(const ConnectHandle& connectHandle, InsertFederationExecutionMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    if (connectHandle != _serverConnectSet.getParentConnectHandle())
      throw MessageError("Received InsertFederationExecutionMessage through a child connect!");

    if (_federationServerMap.find(message->getFederationName()) != _federationServerMap.end())
      throw MessageError("Received InsertFederationExecutionMessage for an already existing federation!");
    FederationHandle federationHandle = message->getFederationHandle();
    if (!federationHandle.valid())
      throw MessageError("Received InsertFederationExecutionMessage with invalid federation handle!");
    FederationServerMap::iterator i = _federationServerMap.find(federationHandle);
    // FIXME: revisit, how to handle this
    // if (i != _federationServerMap.end())
    //   throw MessageError("Received InsertFederationExecutionMessage for an already existing federation!");
    if (i != _federationServerMap.end()) {
      // reinsert the already existing datastructure to get the index by name back
      FederationServerMap::value_type v(*i);
      _federationServerMap.insert(v);
      return;
    }

    i = insertFederation(message->getFederationName(), federationHandle);
    i->second->setLogicalTimeFactoryName(message->getLogicalTimeFactoryName());
    i->second->insert(message->getFOMModuleList());
    // FIXME add the server options
  }
  // A child server or an ambassador sends this request to be removed from the federation execution.
  void accept(const ConnectHandle& connectHandle, ShutdownFederationExecutionMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    // Such a response must originate from the parent.
    if (connectHandle == _serverConnectSet.getParentConnectHandle())
      throw MessageError("Received ShutdownFederationExecutionMessage through a parent connect!");

    FederationHandle federationHandle = message->getFederationHandle();
    if (!federationHandle.valid())
      throw MessageError("Received ShutdownFederationExecutionMessage with invalid federation handle!");
    FederationServerMap::iterator i = _federationServerMap.find(federationHandle);
    if (i == _federationServerMap.end())
      throw MessageError("Received ShutdownFederationExecutionMessage for a non existing federation!");
    if (!i->second->hasChildConnect(connectHandle))
      throw MessageError("Received ShutdownFederationExecutionMessage for a federation not knowing this connect!");

    // Check if the federation has again gained a new federate in between
    if (i->second->hasJoinedFederatesForConnect(connectHandle))
      return;
    // Can happen that we ask for shotdown that is already underway
    if (!i->second->getActive(connectHandle))
      return;

    i->second->eraseFederationExecutionAtConnect(connectHandle);
  }
  // Erase a new federation from the server node
  void accept(const ConnectHandle& connectHandle, EraseFederationExecutionMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    // Such a response must originate from the parent.
    if (connectHandle != _serverConnectSet.getParentConnectHandle())
      throw MessageError("Received EraseFederationExecutionMessage through a child connect!");

    FederationHandle federationHandle = message->getFederationHandle();
    if (!federationHandle.valid())
      throw MessageError("Received EraseFederationExecutionMessage with invalid federation handle!");
    FederationServerMap::iterator i = _federationServerMap.find(federationHandle);
    if (i == _federationServerMap.end())
      throw MessageError("Received EraseFederationExecutionMessage for a non existing federation!");

    // Two cases ...
    if (i->second->hasChildConnects()) {
      // ... we are not the lower most server node that still knows that federation,
      // so forward this to all children.
      i->second->broadcastEraseFederationExecution();
      eraseFederationName(i);
    } else {
      // ... we are the lower most server node that still knows about that federation,
      // so respond with releasing the federation handle.
      SharedPtr<ReleaseFederationHandleMessage> response;
      response = new ReleaseFederationHandleMessage;
      response->setFederationHandle(federationHandle);
      _serverConnectSet.sendToParent(response);
      eraseFederation(i);
    }
  }
  // Erase a federation handle from the server node, this is part of the two way shutdown with a child
  void accept(const ConnectHandle& connectHandle, ReleaseFederationHandleMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    // Such a response must originate from the parent.
    if (connectHandle == _serverConnectSet.getParentConnectHandle())
      throw MessageError("Received ReleaseFederationHandleMessage through a parent connect!");

    FederationHandle federationHandle = message->getFederationHandle();
    if (!federationHandle.valid())
      throw MessageError("Received ReleaseFederationHandleMessage with invalid federation handle!");
    FederationServerMap::iterator i = _federationServerMap.find(federationHandle);
    if (i == _federationServerMap.end())
      throw MessageError("Received ReleaseFederationHandleMessage for a non existing federation!");

    Log(ServerFederation, Info) << getServerPath() << ": ReleaseFederationHandleMessage for federation named \""
                                << i->second->getName() << "\"!" << std::endl;

    // If we are still not the last one, wait until this happens
    // FIXME, move that into the erase connect in some way - may be with a new function
    // FIXME See if this is a real problem
    if (!i->second->hasChildConnect(connectHandle))
      return;
    if (i->second->getActive(connectHandle))
      return;
    i->second->eraseConnect(connectHandle);
    if (i->second->hasChildConnects())
      return;
    // Only release those federations that we were asked to release
    // FIXME better test for that ...
    if (_federationServerMap.find(i->second->getName()) != _federationServerMap.end())
      return;

    if (isRootServer())
      return;
    _serverConnectSet.sendToParent(message);
    eraseFederation(i);
  }

  // The Join messages
  void accept(const ConnectHandle& connectHandle, JoinFederationExecutionRequestMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());

    // The ambassador already needs to care for that. So, if we get that here, drop the connection.
    if (message->getFederateName().compare(0, 3, "HLA") == 0)
      throw MessageError("Got JoinFederationExecutionRequestMessage with name starting with HLA.");

    // If we are a root server ...
    if (isRootServer()) {

      FederationServerMap::const_iterator i = _federationServerMap.find(message->getFederationExecution());
      if (i == _federationServerMap.end()) {
        // FederationExecutionDoesNotExist ...
        Log(ServerFederate, Info) << getServerPath()
                                  << ": JoinFederationExecutionRequestMessage faild for unknown federation named \""
                                  << message->getFederationExecution() << "\"!" << std::endl;
        SharedPtr<JoinFederationExecutionResponseMessage> response;
        response = new JoinFederationExecutionResponseMessage;
        response->setJoinFederationExecutionResponseType(JoinFederationExecutionResponseFederationExecutionDoesNotExist);
        _serverConnectSet.send(connectHandle, response);

      } else if (!message->getFederateName().empty() && i->second->isFederateNameInUse(message->getFederateName())) {
        SharedPtr<JoinFederationExecutionResponseMessage> response;
        response = new JoinFederationExecutionResponseMessage;
        response->setJoinFederationExecutionResponseType(JoinFederationExecutionResponseFederateNameAlreadyInUse);
        _serverConnectSet.send(connectHandle, response);

      } else {
        // Insert a new connect
	i->second->insertConnect(connectHandle, _serverConnectSet.getMessageSender(connectHandle), _serverConnectSet.getName(connectHandle));
        // Process the join request
        i->second->accept(connectHandle, message);
      }
    }
    // If we have an upstream connect, mark this request as pending and ask the parent server
    else {
      // ... ask your father
      _pendingMessageList.push_back(ConnectHandleMessagePair(connectHandle, message));
      _serverConnectSet.sendToParent(message);
    }
  }
  void accept(const ConnectHandle& connectHandle, JoinFederationExecutionResponseMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    // Such a response must originate from the parent.
    if (connectHandle != _serverConnectSet.getParentConnectHandle())
      throw MessageError("Received JoinFederationExecutionResponseMessage through a child connect!");

    ConnectHandle requestConnectHandle = _pendingMessageList.front().first;

    if (message->getJoinFederationExecutionResponseType() != JoinFederationExecutionResponseSuccess) {
      // Join failed.
      // An invalid requestConnectHandle means that the connect is died in between.
      if (requestConnectHandle.valid())
        _serverConnectSet.send(requestConnectHandle, message);
    } else {
      // Success!

      // Check the messages content somehow
      if (!message->getFederateHandle().valid())
        throw MessageError("Received successful JoinFederationExecutionResponseMessage with an invalid federate handle!");
      if (!message->getFederationHandle().valid())
        throw MessageError("Received successful JoinFederationExecutionResponseMessage with an invalid federation handle!");
      FederationServerMap::iterator i = _federationServerMap.find(message->getFederationHandle());
      if (i == _federationServerMap.end())
        throw MessageError("Received successful JoinFederationExecutionResponseMessage with an unknown federation handle!");

      if (requestConnectHandle.valid())
        i->second->insertConnect(requestConnectHandle, _serverConnectSet.getMessageSender(requestConnectHandle), _serverConnectSet.getName(requestConnectHandle));
      i->second->accept(connectHandle, requestConnectHandle, message);
    }

    // remove the pending message
    _pendingMessageList.pop_front();
  }

  void accept(const ConnectHandle& connectHandle, ResignFederationExecutionRequestMessage* message)
  { acceptUpstreamFederationMessage(connectHandle, message); }

  // Message to inform federates about newly joined federates
  void accept(const ConnectHandle& connectHandle, JoinFederateNotifyMessage* message)
  { acceptDownstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, ResignFederateNotifyMessage* message)
  { acceptDownstreamFederationMessage(connectHandle, message); }

  // Synchronization labels
  void accept(const ConnectHandle& connectHandle, RegisterFederationSynchronizationPointMessage* message)
  { acceptUpstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, RegisterFederationSynchronizationPointResponseMessage* message)
  { acceptDownstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, AnnounceSynchronizationPointMessage* message)
  { acceptDownstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, SynchronizationPointAchievedMessage* message)
  { acceptUpstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, FederationSynchronizedMessage* message)
  { acceptDownstreamFederationMessage(connectHandle, message); }

  // Time Management
  void accept(const ConnectHandle& connectHandle, EnableTimeRegulationRequestMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, EnableTimeRegulationResponseMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, DisableTimeRegulationRequestMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, CommitLowerBoundTimeStampMessage* message)
  { acceptFederationMessage(connectHandle, message); }

  // Publications
  void accept(const ConnectHandle& connectHandle, ChangeInteractionClassPublicationMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, ChangeObjectClassPublicationMessage* message)
  { acceptFederationMessage(connectHandle, message); }

  // Subscriptions
  void accept(const ConnectHandle& connectHandle, ChangeInteractionClassSubscriptionMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, ChangeObjectClassSubscriptionMessage* message)
  { acceptFederationMessage(connectHandle, message); }

  // ObjectInstance handle management
  void accept(const ConnectHandle& connectHandle, ObjectInstanceHandlesRequestMessage* message)
  { acceptUpstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, ObjectInstanceHandlesResponseMessage* message)
  { acceptDownstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, ReleaseMultipleObjectInstanceNameHandlePairsMessage* message)
  { acceptUpstreamFederationMessage(connectHandle, message); }

  // ObjectInstance name management
  void accept(const ConnectHandle& connectHandle, ReserveObjectInstanceNameRequestMessage* message)
  { acceptUpstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, ReserveObjectInstanceNameResponseMessage* message)
  { acceptDownstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, ReserveMultipleObjectInstanceNameRequestMessage* message)
  { acceptUpstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, ReserveMultipleObjectInstanceNameResponseMessage* message)
  { acceptDownstreamFederationMessage(connectHandle, message); }

  // Object instance messages
  void accept(const ConnectHandle& connectHandle, InsertObjectInstanceMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, DeleteObjectInstanceMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, AttributeUpdateMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, TimeStampedAttributeUpdateMessage* message)
  { acceptFederationMessage(connectHandle, message); }

  // InteractionMessages
  void accept(const ConnectHandle& connectHandle, InteractionMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, TimeStampedInteractionMessage* message)
  { acceptFederationMessage(connectHandle, message); }

  // AttributeUpdateRequest messages
  void accept(const ConnectHandle& connectHandle, RequestAttributeUpdateMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, RequestClassAttributeUpdateMessage* message)
  { acceptFederationMessage(connectHandle, message); }

  void accept(const ConnectHandle&, AbstractMessage* message)
  { throw MessageError("Received unexpected message???"); }

  class DispatchFunctor {
  public:
    DispatchFunctor(ServerMessageDispatcher& serverMessageDispatcher, const ConnectHandle& connectHandle) :
      _serverMessageDispatcher(serverMessageDispatcher), _connectHandle(connectHandle)
    { }
    template<typename M>
    void operator()(M& message) const
    { _serverMessageDispatcher.accept(_connectHandle, &message); }
  private:
    ServerMessageDispatcher& _serverMessageDispatcher;
    ConnectHandle _connectHandle;
  };

  void dispatch(AbstractMessage& message, const ConnectHandle& connectHandle)
  {
    Log(ServerMessage, Debug3) << getServerPath() << ": Received " << message << "!" << std::endl;
    FunctorMessageDispatcher<DispatchFunctor> dispatcher(DispatchFunctor(*this, connectHandle));
    message.dispatch(dispatcher);
  }

  ConnectHandle insertConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& clientOptions)
  {
    return _serverConnectSet.insertMessageSender(messageSender, clientOptions);
  }
  ConnectHandle insertParentConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& parentOptions)
  {
    _serverOptions->setParentOptionMap(parentOptions);
    return _serverConnectSet.insertParentMessageSender(messageSender, parentOptions);
  }
  void removeConnect(const ConnectHandle& connectHandle)
  {
    bool isParent = connectHandle == _serverConnectSet.getParentConnectHandle();

    // Remove that from the federations.
    for (FederationServerMap::const_iterator i = _federationServerMap.begin();
         i != _federationServerMap.end(); ++i)
      i->second->removeConnect(connectHandle);

    // And remove it here
    _serverConnectSet.removeMessageSender(connectHandle);

    if (isParent) {
      // Replay messages that we forwarded to the parent server to ourself.
      // We are the parent now ...
      for (ConnectHandleMessagePairList::iterator i = _pendingMessageList.begin();
           i != _pendingMessageList.end(); ++i) {
        if (!i->first.valid())
          continue;
        dispatch(*i->second, i->first);
      }
    } else {
      // For all pending messages here, set the originating handle to invalid.
      // So we should not try to return the messages further but still do not loose track of
      // the responses
      for (ConnectHandleMessagePairList::iterator i = _pendingMessageList.begin();
           i != _pendingMessageList.end(); ++i) {
        if (i->first != connectHandle)
          continue;
        i->first = ConnectHandle();
      }
    }
  }

  bool hasChildConnects() const
  { return _serverConnectSet.hasChildConnects(); }
  bool isRootServer() const
  { return !_serverConnectSet.hasParentConnect(); }

  /// FIXME ????
  bool isRunning() const
  // { return _serverConnectSet.hasChildConnects() || !_federationServerMap.empty(); }
  { return _serverConnectSet.hasChildConnects(); }

  ServerOptions& getServerOptions()
  { return *_serverOptions; }
  const ServerOptions& getServerOptions() const
  { return *_serverOptions; }

  const std::string& getServerPath() const
  { return _serverOptions->getServerPath(); }

private:
  // Federations in a server
  // Map federation execution names and handles to federation servers.
  typedef NameHandleMap<FederationHandle, FederationServer> FederationServerMap;
  FederationServerMap _federationServerMap;

  FederationServerMap::iterator insertFederation(const std::string& name)
  {
    OpenRTIAssert(isRootServer());
    OpenRTIAssert(_federationServerMap.find(name) == _federationServerMap.end());

    FederationHandle federationHandle = _federationHandleAllocator.get();
    SharedPtr<FederationServer> federationServer = new FederationServer(name, federationHandle, _serverOptions);
    return _federationServerMap.insert(FederationServerMap::value_type(federationHandle, federationServer)).first;
  }
  FederationServerMap::iterator insertFederation(const std::string& name, const FederationHandle& federationHandle)
  {
    OpenRTIAssert(federationHandle.valid());
    OpenRTIAssert(_federationServerMap.find(name) == _federationServerMap.end());
    OpenRTIAssert(_federationServerMap.find(federationHandle) == _federationServerMap.end());

    _federationHandleAllocator.take(federationHandle);
    SharedPtr<FederationServer> federationServer = new FederationServer(name, federationHandle, _serverOptions);
    OpenRTIAssert(!isRootServer());
    federationServer->insertParentConnect(_serverConnectSet.getParentConnectHandle(),
                                          _serverConnectSet.getMessageSender(_serverConnectSet.getParentConnectHandle()),
                                          _serverConnectSet.getName(_serverConnectSet.getParentConnectHandle()));
    return _federationServerMap.insert(FederationServerMap::value_type(federationHandle, federationServer)).first;
  }
  void eraseFederationName(const FederationHandle& federationHandle)
  {
    eraseFederationName(_federationServerMap.find(federationHandle));
  }
  void eraseFederationName(FederationServerMap::iterator i)
  {
    OpenRTIAssert(i != _federationServerMap.end());
    OpenRTIAssert(!i->second->hasJoinedChildren());

    Log(ServerFederation, Info) << getServerPath() << ": Destroyed federation execution in child server for \""
                                << i->second->getName() << "\"!" << std::endl;

    _federationServerMap.eraseName(i);
  }
  void eraseFederation(const FederationHandle& federationHandle)
  {
    eraseFederation(_federationServerMap.find(federationHandle));
  }
  void eraseFederation(FederationServerMap::iterator i)
  {
    OpenRTIAssert(i != _federationServerMap.end());
    OpenRTIAssert(!i->second->hasJoinedChildren());

    Log(ServerFederation, Info) << getServerPath() << ": Released FederationHandle in child server for \""
                                << i->second->getName() << "\"!" << std::endl;

    _federationHandleAllocator.put(i->first);
    _federationServerMap.erase(i);
  }

  /// Messages that require a response from the root server.
  /// We store a list of these pending messages to
  /// 1. know where to respond.
  /// 2. to be able to replay these messages if the parent dies
  /// Note that the connect handles are invalidated once a connect dies while a response
  /// is pending. This way we can keep track of the responses even when some reqests get
  /// invalid before they completed.
  typedef std::pair<ConnectHandle, SharedPtr<AbstractMessage> > ConnectHandleMessagePair;
  typedef std::list<ConnectHandleMessagePair> ConnectHandleMessagePairList;
  ConnectHandleMessagePairList _pendingMessageList;

  // The handle allocator helper class of this server.
  // Tracks allocations from the parent server as well as allocates
  // handles if this is a root server.
  FederationHandleAllocator _federationHandleAllocator;

  // The set of connects where to send messages to
  ServerConnectSet _serverConnectSet;

  // The ServerOptions for this server component
  SharedPtr<ServerOptions> _serverOptions;
};

ServerNode::ServerNode(const SharedPtr<ServerOptions>& serverOptions) :
  _serverMessageDispatcher(new ServerMessageDispatcher(serverOptions))
{
}

ServerNode::~ServerNode()
{
}

const std::string&
ServerNode::getServerName() const
{
  return _serverMessageDispatcher->getServerOptions().getServerName();
}

void
ServerNode::setServerName(const std::string& name)
{
  _serverMessageDispatcher->getServerOptions().setServerName(name);
}

bool
ServerNode::isRunning() const
{
  return _serverMessageDispatcher->isRunning();
}

const ServerOptions&
ServerNode::getServerOptions() const
{
  return _serverMessageDispatcher->getServerOptions();
}

ConnectHandle
ServerNode::insertConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& clientOptions)
{
  return _serverMessageDispatcher->insertConnect(messageSender, clientOptions);
}

ConnectHandle
ServerNode::insertParentConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& parentOptions)
{
  return _serverMessageDispatcher->insertParentConnect(messageSender, parentOptions);
}

void
ServerNode::removeConnect(const ConnectHandle& connectHandle)
{
  _serverMessageDispatcher->removeConnect(connectHandle);
}

void
ServerNode::dispatchMessage(AbstractMessage& message, const ConnectHandle& connectHandle)
{
  _serverMessageDispatcher->dispatch(message, connectHandle);
}

} // namespace OpenRTI
