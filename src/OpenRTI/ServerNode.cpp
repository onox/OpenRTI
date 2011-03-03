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
#include "ObjectModel.h"
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
  ConnectHandle insertParentMessageSender(const SharedPtr<AbstractMessageSender>& messageSender)
  {
    OpenRTIAssert(!_parentServerConnectHandle.valid());
    _parentServerConnectHandle = insertMessageSender(messageSender);
    return _parentServerConnectHandle;
  }

  ConnectHandle insertMessageSender(const SharedPtr<AbstractMessageSender>& messageSender)
  {
    if (!messageSender.valid())
      return ConnectHandle();
    ConnectHandle connectHandle = _connectHandleAllocator.get();
    OpenRTIAssert(_messageSenderMap.find(connectHandle) == _messageSenderMap.end());
    _messageSenderMap[connectHandle] = messageSender;
    return connectHandle;
  }
  void removeMessageSender(const ConnectHandle& connectHandle)
  {
    MessageSenderMap::iterator i = _messageSenderMap.find(connectHandle);
    OpenRTIAssert(i != _messageSenderMap.end());
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
    i->second->send(message);
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
      i->second->send(message);
    }
  }

  SharedPtr<AbstractMessageSender> getMessageSender(const ConnectHandle& connectHandle) const
  {
    MessageSenderMap::const_iterator i = _messageSenderMap.find(connectHandle);
    OpenRTIAssert(i != _messageSenderMap.end());
    return i->second;
  }

private:
  /// Connect management.

  /// If we have an upstream server this one contains a vaild connect handle.
  ConnectHandle _parentServerConnectHandle;

  /// The server class has several connects.
  /// Each connect can have multiple federates attached.
  typedef std::map<ConnectHandle, SharedPtr<AbstractMessageSender> > MessageSenderMap;
  MessageSenderMap _messageSenderMap;

  HandleAllocator<ConnectHandle> _connectHandleAllocator;
};


class OPENRTI_LOCAL SynchronizationState {
public:
  SynchronizationState() :
    _addJoiningFederates(true)
  { }

  void set(const FederateHandleSet& participatingFederates, const FederateHandleSet& joinedFederates)
  {
    if (participatingFederates.empty()) {
      _participatingFederates = joinedFederates;
      _waitFederates = joinedFederates;
      _addJoiningFederates = true;
    } else {
      _participatingFederates = participatingFederates;
      _waitFederates = participatingFederates;
      _addJoiningFederates = false;
    }
  }
  // Use this to build up the initial connect handle set
  void insert(const FederateHandle& federateHandle)
  {
    _participatingFederates.insert(federateHandle);
    _waitFederates.insert(federateHandle);
  }
  void insert(const FederateHandleSet& federateHandleSet)
  {
    _participatingFederates.insert(federateHandleSet.begin(), federateHandleSet.end());
    _waitFederates.insert(federateHandleSet.begin(), federateHandleSet.end());
  }
  // Call when a federate resigns
  void removeFederate(const FederateHandle& federateHandle)
  {
    _participatingFederates.erase(federateHandle);
    _waitFederates.erase(federateHandle);
  }

  VariableLengthData& getTag()
  { return _tag; }
  const VariableLengthData& getTag() const
  { return _tag; }

// private:
  bool _addJoiningFederates;
  VariableLengthData _tag;
  // The set of federates that was given in the RegisterFederationSynchronizationPoint.
  // If this is empty all joined federates should be taken instead.
  FederateHandleSet _participatingFederates;
  // The set of federates we need to wait for until the synchronization point could be announced to the parent.
  // This one must be filled with the the active federate handles in any case
  FederateHandleSet _waitFederates;
};

/// Hmm, should that be the Federation ???
class OPENRTI_LOCAL FederationServer : public Federation {
public:
  FederationServer(const std::wstring& name, const FederationHandle& handle, const SharedPtr<const ServerOptions>& serverOptions) :
    Federation(name, handle),
    _serverOptions(serverOptions)
  {
  }

  /// Get the rti servers name
  const std::wstring& getServerName() const
  { return _serverOptions->getServerName(); }
  /// Get the rti servers path
  const std::wstring& getServerPath() const
  { return _serverOptions->getServerPath(); }

  /// Returns true if this is a root server
  bool isRootServer() const
  { return !_parentServerConnectHandle.valid(); }

  bool isFederateNameInUse(const std::wstring& name) const
  { return _federateNameSet.find(name) != _federateNameSet.end(); }

  void accept(const ConnectHandle& connectHandle, JoinFederationExecutionRequestMessage* message)
  {
    // This function needs to be successful since the calling function has already inserted the connect here
    // and does not undo that FIXME: rethink?!

    OpenRTIAssert(isRootServer());

    // Respond with Success
    SharedPtr<JoinFederationExecutionResponseMessage> response;
    response = new JoinFederationExecutionResponseMessage;
    response->setJoinFederationExecutionResponseType(JoinFederationExecutionResponseSuccess);

    // ... insert a new federate ...
    FederateHandle federateHandle = insertFederate(connectHandle, message->getFederateType(), message->getFederateName());
    OpenRTIAssert(federateHandle.valid());
    // FIXME
    std::wstring federateName = *(_federateHandleFederateDataMap.find(federateHandle)->second._stringSetIterator);

    /// FIXME shall we get that message from the federation server???
    response->setFederationHandle(getHandle());
    response->setFederateType(message->getFederateType());
    response->setFederateName(federateName);
    response->setFederateHandle(federateHandle);
    send(connectHandle, response);

    // send all children the notification about the new federate except the one that gets the join response
    SharedPtr<JoinFederateNotifyMessage> notify = new JoinFederateNotifyMessage;
    notify->setFederationHandle(response->getFederationHandle());
    notify->setFederateHandle(response->getFederateHandle());
    notify->setFederateType(response->getFederateType());
    notify->setFederateName(federateName);
    broadcastToChildren(connectHandle, notify);

    // For those sync request that are automatically extended to new federates, send the announcement
    for (SyncronizationLabelStateMap::iterator i = _syncronizationLabelStateMap.begin();
         i != _syncronizationLabelStateMap.end(); ++i) {
      // only those with an auto expanding federate set
      if (!i->second._addJoiningFederates)
        continue;

      i->second.insert(federateHandle);

      SharedPtr<AnnounceSynchronizationPointMessage> announce;
      announce = new AnnounceSynchronizationPointMessage;
      announce->setFederationHandle(getHandle());
      announce->setLabel(i->first);
      announce->setTag(i->second.getTag());
      announce->getFederateHandleSet().insert(federateHandle);
      announce->setAddJoiningFederates(i->second._addJoiningFederates);
      broadcastToChildren(announce);
    }
  }
  void accept(const ConnectHandle& connectHandle, const ConnectHandle& requestConnectHandle, JoinFederationExecutionResponseMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());

    FederateHandle federateHandle = message->getFederateHandle();
    OpenRTIAssert(federateHandle.valid());

    insertFederate(requestConnectHandle, message->getFederateType(), message->getFederateName(), federateHandle);

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
    FederateHandleFederateDataMap::iterator i = _federateHandleFederateDataMap.find(federateHandle);
    if (i == _federateHandleFederateDataMap.end())
      throw MessageError(L"Recieved ResignFederationExecutionRequestMessage for invalid federate handle!");

    // already done so ... just waiting for the response
    if (i->second._resignPending)
      return;

    // We need to skip resource allocations in the future
    i->second._resignPending = true;

    // remove from time management
    FederateHandleTimeStampMap::iterator k = _federateHandleTimeStampMap.find(federateHandle);
    if (k != _federateHandleTimeStampMap.end()) {
      _federateHandleTimeStampMap.erase(k);
      SharedPtr<DisableTimeRegulationRequestMessage> request = new DisableTimeRegulationRequestMessage;
      request->setFederationHandle(getHandle());
      request->setFederateHandle(federateHandle);
      broadcast(connectHandle, request);
    }

    ConnectHandleConnectDataMap::iterator l = _connectHandleConnectDataMap.find(connectHandle);
    if (l != _connectHandleConnectDataMap.end()) {
      // FIXME
      if (l->second._federateHandleSet.size() == 1 && *l->second._federateHandleSet.begin() == federateHandle) {
        // FIXME this is in effect something like removeConnect!!!
        for (ObjectInstanceHandleDataMap::iterator k = _objectInstanceHandleDataMap.begin(); k != _objectInstanceHandleDataMap.end();) {
          unreferenceObjectInstanceHandle(k++, connectHandle);
        }
      }
    }

    // FIXME?!!
    for (ObjectInstanceMap::const_iterator j = _objectInstanceMap.begin();
         j != _objectInstanceMap.end(); ++j) {
      if (message->getFederateHandle() != j->second->getOwnerFederateHandle())
        continue;
      j->second->setOwnerFederateHandle(FederateHandle());
      j->second->setOwnerConnectHandle(ConnectHandle());
    }

    // If we are a root server ...
    if (isRootServer()) {
      // ... and respond with Success
      /// FIXME shall we get that message from the federation server???
      SharedPtr<ResignFederationExecutionResponseMessage> response;
      response = new ResignFederationExecutionResponseMessage;
      response->setFederationHandle(getHandle());
      response->setFederateHandle(federateHandle);
      send(connectHandle, response);

      SharedPtr<ResignFederateNotifyMessage> notify = new ResignFederateNotifyMessage;
      notify->setFederateHandle(federateHandle);
      notify->setFederationHandle(getHandle());
      broadcastToChildren(connectHandle, notify);

      // Remove the federate locally
      removeFederate(federateHandle);
    }
    // If we have an upstream connect, mark this request as pending and ask the parent server
    else {
      send(_parentServerConnectHandle, message);
    }
  }
  void accept(const ConnectHandle& connectHandle, ResignFederationExecutionResponseMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    FederateHandle federateHandle = message->getFederateHandle();
    if (!federateHandle.valid())
      throw MessageError(L"Recieved ResignFederationExecutionResponseMessage for invalid federate handle!");

    // Get the connect where this federate was sitting up to now
    FederateHandleFederateDataMap::const_iterator j = _federateHandleFederateDataMap.find(federateHandle);
    ConnectHandle requestConnectHandle;
    // Can happen when the resign was forced due to a dropped connection
    if (j != _federateHandleFederateDataMap.end())
      requestConnectHandle = j->second._connectHandle;

    // send all children the notification about the new federate except the one that gets the response
    SharedPtr<ResignFederateNotifyMessage> notify = new ResignFederateNotifyMessage;
    notify->setFederateHandle(federateHandle);
    notify->setFederationHandle(getHandle());
    broadcastToChildren(requestConnectHandle, notify);

    // Report downstream to the originator, if there is still something to report
    if (requestConnectHandle.valid())
      send(requestConnectHandle, message);

    // Do some work locally
    removeFederate(federateHandle);
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

  FederateHandle insertFederate(const ConnectHandle& connectHandle, const std::wstring& federateType,
                                std::wstring federateName, const FederateHandle& federateHandle = FederateHandle())
  {
    // Either we allocate a new federate, then the connect must still be alive
    // or we insert an already died federate and just keep everything in order for a currect the resing request sequence
    OpenRTIAssert(connectHandle.valid() || federateHandle.valid());
    OpenRTIAssert(federateName.empty() || _federateNameSet.find(federateName) == _federateNameSet.end());
    // OpenRTIAssert(!federateHandle.valid() || federateName.compare(0, 3, L"HLA") == 0);

    FederateHandleAllocator::Candidate candidate(_federateHandleAllocator, federateHandle);

    OpenRTIAssert(_federateHandleFederateDataMap.find(candidate.get()) == _federateHandleFederateDataMap.end());

    // generate a unique name if there is none given
    if (federateName.empty()) {
      std::wstringstream ss;
      ss << "HLAfederate" << candidate.get().getHandle();
      federateName = ss.str();
    }
    OpenRTIAssert(_federateNameSet.find(federateName) == _federateNameSet.end());

    // Register that we reach this federate through this connect
    StringSet::iterator stringSetIterator = _federateNameSet.insert(federateName).first;
    FederateHandleFederateDataMap::iterator i;
    i = _federateHandleFederateDataMap.insert(FederateHandleFederateDataMap::value_type(candidate.get(), FederateData(connectHandle, stringSetIterator))).first;
    if (connectHandle.valid()) {
      _connectHandleConnectDataMap[connectHandle]._federateHandleSet.insert(candidate.get());
    } else {
      i->second._resignPending = true;
    }
    i->second._federateType = federateType;

    return candidate.take();
  }

  void removeFederate(const FederateHandle& federateHandle)
  {
    if (!federateHandle.valid())
      return;

    // The time management stuff
    _federateHandleTimeStampMap.erase(federateHandle);

    // Remove from syncronization state
    for (SyncronizationLabelStateMap::iterator k = _syncronizationLabelStateMap.begin();
         k != _syncronizationLabelStateMap.end(); ++k) {
      k->second.removeFederate(federateHandle);
    }

    // Remove from connects
    FederateHandleFederateDataMap::iterator i = _federateHandleFederateDataMap.find(federateHandle);
    OpenRTIAssert(i != _federateHandleFederateDataMap.end());
    ConnectHandle connectHandle = i->second._connectHandle;
    ConnectHandleConnectDataMap::iterator j = _connectHandleConnectDataMap.find(connectHandle);
    if (j != _connectHandleConnectDataMap.end()) {
      j->second._federateHandleSet.erase(federateHandle);
      // remove all child connects if there is no joined federate left
      if (connectHandle != _parentServerConnectHandle && j->second._federateHandleSet.empty()) {

        if (j->second._messageSender.valid()) {
          SharedPtr<EraseFederationExecutionMessage> eraseMessage = new EraseFederationExecutionMessage;
          eraseMessage->setFederationHandle(getHandle());
          j->second._messageSender->send(eraseMessage);
        }

        // FIXME this is in effect something like removeConnect!!!
        _connectHandleConnectDataMap.erase(j);
      }
    }
    _federateNameSet.erase(i->second._stringSetIterator);
    _federateHandleFederateDataMap.erase(i);

    // Give back the handle to the allocator
    _federateHandleAllocator.put(federateHandle);
  }

  void accept(const ConnectHandle& connectHandle, JoinFederateNotifyMessage* message)
  {
    FederateHandle federateHandle = message->getFederateHandle();
    if (_federateHandleFederateDataMap.find(federateHandle) != _federateHandleFederateDataMap.end())
      throw MessageError(L"Received JoinFederateNotify for already known federate!");
    insertFederate(connectHandle, message->getFederateType(), message->getFederateName(), federateHandle);
    broadcastToChildren(message);
  }
  void accept(const ConnectHandle& connectHandle, ResignFederateNotifyMessage* message)
  {
    FederateHandle federateHandle = message->getFederateHandle();
    if (_federateHandleFederateDataMap.find(federateHandle) == _federateHandleFederateDataMap.end())
      throw MessageError(L"Received ResignFederateNotify for unknown federate!");
    broadcastToChildren(message);
    removeFederate(federateHandle);
  }
  void accept(const ConnectHandle& connectHandle, ChangeDefaultResignActionMessage* message)
  {
    FederateHandle federateHandle = message->getFederateHandle();
    FederateHandleFederateDataMap::iterator i = _federateHandleFederateDataMap.find(federateHandle);
    if (i == _federateHandleFederateDataMap.end())
      throw MessageError(L"Received ChangeDefaultResignActionMessage for unknown federate!");
    i->second._defaultResignAction = message->getResignAction();
    broadcast(connectHandle, message);
  }

  // Synchronization labels
  void accept(const ConnectHandle& connectHandle, RegisterFederationSynchronizationPointMessage* message)
  {
    // Labels must be ckecked by the ambassador. So what arrives here with an empty label must be some kind of error.
    if (message->getLabel().empty())
      throw MessageError(L"Received empty label in RegisterFederationSynchronizationPointMessage!");

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
      for (FederateHandleFederateDataMap::const_iterator j = _federateHandleFederateDataMap.begin();
           j != _federateHandleFederateDataMap.end(); ++j)
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
        std::set_intersection(j->second._federateHandleSet.begin(), j->second._federateHandleSet.end(),
                              i->second._waitFederates.begin(), i->second._waitFederates.end(),
                              std::inserter(federateHandleSet, federateHandleSet.end()));
        if (federateHandleSet.empty())
          continue;

        SharedPtr<AnnounceSynchronizationPointMessage> announce;
        announce = new AnnounceSynchronizationPointMessage;
        announce->setFederationHandle(getHandle());
        announce->setLabel(message->getLabel());
        announce->setTag(i->second._tag);
        announce->setAddJoiningFederates(i->second._addJoiningFederates);
        announce->setFederateHandleSet(federateHandleSet);
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
      throw MessageError(L"Received empty label in RegisterFederationSynchronizationPointResponseMessage!");

    send(message->getFederateHandle(), message);
  }

  void accept(const ConnectHandle& connectHandle, AnnounceSynchronizationPointMessage* message)
  {
    if (message->getLabel().empty())
      throw MessageError(L"Received empty label in AnnounceSynchronizationPointMessage!");

    SyncronizationLabelStateMap::iterator i = _syncronizationLabelStateMap.find(message->getLabel());
    if (i == _syncronizationLabelStateMap.end()) {
      // label is new, create one
      i = _syncronizationLabelStateMap.insert(SyncronizationLabelStateMap::value_type(message->getLabel(), SynchronizationState())).first;
      i->second._tag = message->getTag();
      i->second._addJoiningFederates = message->getAddJoiningFederates();
    } else {
      // label is already there
      if (!i->second._addJoiningFederates)
        MessageError(L"Receiving incremental synchronization point update for fixed federate handle synchronization point!");
      if (!message->getAddJoiningFederates())
        MessageError(L"Receiving incremental synchronization point update for fixed federate handle synchronization point!");
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
      std::set_intersection(j->second._federateHandleSet.begin(), j->second._federateHandleSet.end(),
                            message->getFederateHandleSet().begin(), message->getFederateHandleSet().end(),
                            std::inserter(federateHandleSet, federateHandleSet.end()));
      if (federateHandleSet.empty())
        continue;

      i->second.insert(federateHandleSet);

      SharedPtr<AnnounceSynchronizationPointMessage> announce;
      announce = new AnnounceSynchronizationPointMessage;
      announce->setFederationHandle(getHandle());
      announce->setLabel(message->getLabel());
      announce->setTag(message->getTag());
      announce->setAddJoiningFederates(message->getAddJoiningFederates());
      announce->setFederateHandleSet(federateHandleSet);
      send(j->first, announce);
    }
  }
  void accept(const ConnectHandle& connectHandle, SynchronizationPointAchievedMessage* message)
  {
    SyncronizationLabelStateMap::iterator i = _syncronizationLabelStateMap.find(message->getLabel());
    if (i == _syncronizationLabelStateMap.end())
      throw MessageError(L"SynchronizationPointAchievedMessage for unknown label!");

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
      throw MessageError(L"FederateSynchronizedMessage for unknown label.");

    // Cycle over all child connects and send announcements with the appropriate handle sets
    for (ConnectHandleConnectDataMap::const_iterator j = _connectHandleConnectDataMap.begin();
	 j != _connectHandleConnectDataMap.end(); ++j) {
      if (j->first == _parentServerConnectHandle)
	continue;
      if (!j->first.valid())
	continue;

      // Build the intersection of the federate handles in the message and the ones in the connect.
      FederateHandleSet federateHandleSet;
      std::set_intersection(j->second._federateHandleSet.begin(), j->second._federateHandleSet.end(),
                            message->getFederateHandleSet().begin(), message->getFederateHandleSet().end(),
                            std::inserter(federateHandleSet, federateHandleSet.end()));
      if (federateHandleSet.empty())
        continue;

      SharedPtr<FederationSynchronizedMessage> synchronized;
      synchronized = new FederationSynchronizedMessage;
      synchronized->setFederationHandle(getHandle());
      synchronized->setLabel(message->getLabel());
      synchronized->setFederateHandleSet(federateHandleSet);
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
      throw MessageError(L"ChangeInteractionClassSubscriptionMessage for unknown InteractionClass!");
    // Change publication type for this connect ...
    PropagationTypeConnectHandlePair propagationConnectPair;
    propagationConnectPair = interactionClass->setSubscriptionType(connectHandle, message->getSubscriptionType());
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

      if (message->getSubscriptionType() != Unsubscribed) {
        for (ObjectInstanceList::const_iterator j = objectClass->getObjectInstanceList().begin();
             j != objectClass->getObjectInstanceList().end(); ++j) {
          ObjectAttribute* objectAttribute = (*j)->getAttribute(*i);
          if (!objectAttribute) // FIXME: is this an error??
            continue;

          // Don't add the owner to the list of connect handles that recieve this attribute
          if (objectAttribute->getOwnerConnectHandle() == connectHandle)
            continue;

          // Insert the connect handle into the recieving connects
          if (!objectAttribute->_recieveingConnects.insert(connectHandle).second)
            continue;

          if (*i == AttributeHandle(0)) {
            // And in case of the attribute to delete, tell about those objects
            // FIXME: not always, let the object know if it is already pushed into this connect ...
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
              attributeState.setFederateHandle((*k)->getOwnerFederateHandle());
              request->getAttributeStateVector().push_back(attributeState);
            }
            if (connectHandle != _parentServerConnectHandle) {
              if (_objectInstanceHandleDataMap.find((*j)->getHandle()) == _objectInstanceHandleDataMap.end()) {
                insertObjectInstanceHandle((*j)->getHandle(), (*j)->getName(), connectHandle);
                _objectInstanceHandleAllocator.take((*j)->getHandle());
              } else
                referenceObjectInstanceHandle((*j)->getHandle(), connectHandle);
            }
            send(connectHandle, request);
          }
        }
      } else {
        ObjectInstanceHandleSet objectInstanceHandleSet;
        for (ObjectInstanceList::const_iterator j = objectClass->getObjectInstanceList().begin();
             j != objectClass->getObjectInstanceList().end(); ++j) {
          ObjectAttribute* objectAttribute = (*j)->getAttribute(*i);
          if (!objectAttribute) // FIXME: is this an error??
            continue;

          // Don't add the owner to the list of connect handles that recieve this attribute
          if (objectAttribute->getOwnerConnectHandle() == connectHandle)
            continue;

          // Erase the connect handle from the recieving connects
          if (objectAttribute->_recieveingConnects.erase(connectHandle) == 0)
            continue;

          // FIXME: no ???
          // FIXME unreference object instances that get unsubscribed somehow ...

          if (*i == AttributeHandle(0)) {
            objectInstanceHandleSet.insert((*j)->getHandle());
          }
        }
        for (ObjectInstanceHandleSet::const_iterator i = objectInstanceHandleSet.begin();
             i != objectInstanceHandleSet.end(); ++i) {
          // Shall we erase these objects from the branch then???
          SharedPtr<DeleteObjectInstanceMessage> message = new DeleteObjectInstanceMessage;
          message->setFederationHandle(getHandle());
          message->setObjectInstanceHandle(*i);
          send(connectHandle, message);

          // if (connectHandle != _parentServerConnectHandle) {
          //   unreferenceObjectInstanceHandle(*i, connectHandle);
          // }
        }
      }
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
      // Provide some object instance handles to a federate.
      // This is to be completely asyncronous in the registerObjectInstance call.
      SharedPtr<ObjectInstanceHandlesResponseMessage> response;
      response = new ObjectInstanceHandlesResponseMessage;
      response->setFederationHandle(message->getFederationHandle());
      FederateHandle federateHandle = message->getFederateHandle();
      response->setFederateHandle(federateHandle);
      unsigned count = message->getCount();
      response->getObjectInstanceHandleNamePairVector().reserve(count);
      FederateHandleFederateDataMap::iterator i = _federateHandleFederateDataMap.find(federateHandle);
      while (count--) {
        ObjectInstanceHandle objectInstanceHandle = _objectInstanceHandleAllocator.get();
        std::wstringstream stream;
        stream << L"HLAobjectInstance" << objectInstanceHandle.getHandle();
        ObjectInstanceHandleNamePair objectInstanceHandleNamePair(objectInstanceHandle, stream.str());
        insertObjectInstanceHandle(objectInstanceHandleNamePair.first, objectInstanceHandleNamePair.second, connectHandle);
        response->getObjectInstanceHandleNamePairVector().push_back(objectInstanceHandleNamePair);
      }
      send(connectHandle, response);
    } else {
      send(_parentServerConnectHandle, message);
    }
  }
  void accept(const ConnectHandle& connectHandle, ObjectInstanceHandlesResponseMessage* message)
  {
    FederateHandle federateHandle = message->getFederateHandle();
    FederateHandleFederateDataMap::iterator i = _federateHandleFederateDataMap.find(federateHandle);
    if (i == _federateHandleFederateDataMap.end()) {
      throw MessageError(L"Got ObjectInstanceHandlesResponseMessage for an unknown federate!");
    } else if (!i->second._connectHandle.valid() || i->second._resignPending) {
      // Can happen, may be it has resigned/is died in between but the response is already underway
      // If so, then just ignore, the upstream server needs to release them
    } else {
      for (ObjectInstanceHandleNamePairVector::const_iterator k = message->getObjectInstanceHandleNamePairVector().begin();
           k != message->getObjectInstanceHandleNamePairVector().end(); ++k) {
        _objectInstanceHandleAllocator.take(k->first);
        insertObjectInstanceHandle(k->first, k->second, i->second._connectHandle);
      }

      send(i->second._connectHandle, message);
    }
  }
  void accept(const ConnectHandle& connectHandle, ReleaseMultipleObjectInstanceNameHandlePairsMessage* message)
  {
    for (ObjectInstanceHandleVector::const_iterator j = message->getObjectInstanceHandleVector().begin();
         j != message->getObjectInstanceHandleVector().end(); ++j) {
      unreferenceObjectInstanceHandle(*j, connectHandle);
    }
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
    if (isRootServer()) {
      FederateHandle federateHandle = message->getFederateHandle();
      FederateHandleFederateDataMap::iterator i = _federateHandleFederateDataMap.find(federateHandle);
      if (i == _federateHandleFederateDataMap.end())
        throw MessageError(L"Got ReserveObjectInstanceNameRequestMessage for an unknown federate!");

      // names starting with HLA are reserved for the RTI, a correct programmed ambassador does not request these
      if (message->getName().compare(0, 3, L"HLA") == 0)
        throw MessageError(L"Got ReserveObjectInstanceNameRequestMessage with name starting with HLA.");

      SharedPtr<ReserveObjectInstanceNameResponseMessage> response;
      response = new ReserveObjectInstanceNameResponseMessage;
      response->setFederationHandle(message->getFederationHandle());
      response->setFederateHandle(message->getFederateHandle());
      if (_objectInstanceNameSet.find(message->getName()) == _objectInstanceNameSet.end()) {
        ObjectInstanceHandleNamePair objectInstanceHandleNamePair(_objectInstanceHandleAllocator.get(), message->getName());
        insertObjectInstanceHandle(objectInstanceHandleNamePair.first, objectInstanceHandleNamePair.second, connectHandle);
        response->setObjectInstanceHandleNamePair(objectInstanceHandleNamePair);
        response->setSuccess(true);
      } else {
        ObjectInstanceHandleNamePair objectInstanceHandleNamePair(ObjectInstanceHandle(), message->getName());
        response->setObjectInstanceHandleNamePair(objectInstanceHandleNamePair);
        response->setSuccess(false);
      }
      send(message->getFederateHandle(), response);
    } else {
      send(_parentServerConnectHandle, message);
    }
  }
  void accept(const ConnectHandle& connectHandle, ReserveObjectInstanceNameResponseMessage* message)
  {
    FederateHandle federateHandle = message->getFederateHandle();
    FederateHandleFederateDataMap::iterator i = _federateHandleFederateDataMap.find(federateHandle);
    if (i == _federateHandleFederateDataMap.end()) {
      throw MessageError(L"Got ReserveObjectInstanceNameResponseMessage for an unknown federate!");
    } else if (!i->second._connectHandle.valid() || i->second._resignPending) {
      // Can happen, may be it has resigned/is died in between but the response is already underway
      // If so, then release the reservations.
    } else {
      if (message->getSuccess()) {
        insertObjectInstanceHandle(message->getObjectInstanceHandleNamePair().first,
                                   message->getObjectInstanceHandleNamePair().second, i->second._connectHandle);
        _objectInstanceHandleAllocator.take(message->getObjectInstanceHandleNamePair().first);
      }
      send(i->second._connectHandle, message);
    }
  }
  void accept(const ConnectHandle& connectHandle, ReserveMultipleObjectInstanceNameRequestMessage* message)
  {
    if (isRootServer()) {
      FederateHandle federateHandle = message->getFederateHandle();
      FederateHandleFederateDataMap::iterator i = _federateHandleFederateDataMap.find(federateHandle);
      if (i == _federateHandleFederateDataMap.end())
        throw MessageError(L"Got ReserveObjectInstanceNameRequestMessage for an unknown federate!");

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
        // names starting with HLA are reserved for the RTI
        if (i->compare(0, 3, L"HLA") == 0)
          throw MessageError(L"ReserveObjectInstanceNameRequestMessage with name starting with HLA - that must be handled in the ambassador.");
        // We do not need to check against the object names since the automatic generated object names do not collide by design
        // and the reserved names are tested here
        if (_objectInstanceNameSet.find(*i) == _objectInstanceNameSet.end())
          continue;
        response->setSuccess(false);
      }
      // If none of them is reserved or in use, reservem them all
      if (response->getSuccess()) {
        for (ObjectInstanceHandleNamePairVector::iterator j = response->getObjectInstanceHandleNamePairVector().begin();
             j != response->getObjectInstanceHandleNamePairVector().end(); ++j) {
          j->first = _objectInstanceHandleAllocator.get();
          insertObjectInstanceHandle(j->first, j->second, connectHandle);
        }
      }
      send(message->getFederateHandle(), response);
    } else {
      send(_parentServerConnectHandle, message);
    }
  }
  void accept(const ConnectHandle& connectHandle, ReserveMultipleObjectInstanceNameResponseMessage* message)
  {
    FederateHandle federateHandle = message->getFederateHandle();
    FederateHandleFederateDataMap::iterator i = _federateHandleFederateDataMap.find(federateHandle);
    if (i == _federateHandleFederateDataMap.end()) {
      throw MessageError(L"Got ReserveMultipleObjectInstanceNameResponseMessage for an unknown federate!");
    } else if (!i->second._connectHandle.valid() || i->second._resignPending) {
      // Can happen, may be it has resigned/is died in between but the response is already underway
      // If so, then release the reservations.
    } else {
      if (message->getSuccess()) {
        for (ObjectInstanceHandleNamePairVector::const_iterator k = message->getObjectInstanceHandleNamePairVector().begin();
             k != message->getObjectInstanceHandleNamePairVector().end(); ++k) {
          insertObjectInstanceHandle(k->first, k->second, i->second._connectHandle);
          _objectInstanceHandleAllocator.take(k->first);
        }
      }
      send(i->second._connectHandle, message);
    }
  }


  // Object instance messages
  void accept(const ConnectHandle& connectHandle, InsertObjectInstanceMessage* message)
  {
    ObjectClass* objectClass = getObjectClass(message->getObjectClassHandle());
    if (!objectClass)
      throw MessageError(L"InsertObjectInstanceMessage for unknown ObjectClass.");

    ObjectInstanceHandle objectInstanceHandle = message->getObjectInstanceHandle();
    SharedPtr<ObjectInstance> objectInstance = new ObjectInstance(message->getName(), objectInstanceHandle, objectClass);
    for (size_t i = 0; i < message->getAttributeStateVector().size(); ++i) {
      ObjectAttribute* attribute = objectInstance->getAttribute(message->getAttributeStateVector()[i].getAttributeHandle());
      if (!attribute)
        continue;
      attribute->setOwnerFederateHandle(message->getAttributeStateVector()[i].getFederateHandle());
      attribute->setOwnerConnectHandle(connectHandle);
      // FIXME
      attribute->_recieveingConnects.erase(connectHandle);
    }
    insertObjectInstance(objectInstance);

    // FIXME Improove this with preevaluated sets:
    // std::map<FederateHandle,ConnectHandleSet> ...
    // or may be to reduce the N in O(log(N))
    // std::map<ConnectHandle,ConnectHandleSet> ...
    ConnectHandleSet connectHandleSet;
    while (objectClass) {
      ObjectClassAttribute* attribute = objectClass->getPrivilegeToDeleteAttribute();
      if (attribute && attribute->getSubscriptionType() != Unsubscribed) {
        connectHandleSet.insert(attribute->getSubscribedConnectHandleSet().begin(), attribute->getSubscribedConnectHandleSet().end());
      }
      objectClass = objectClass->getParentObjectClass();
    }
    connectHandleSet.erase(connectHandle);
    objectInstance->getPrivilegeToDeleteAttribute()->_recieveingConnects = connectHandleSet;

    for (ConnectHandleSet::iterator i = connectHandleSet.begin(); i != connectHandleSet.end(); ++i) {
      if (*i == _parentServerConnectHandle)
        continue;
      if (_objectInstanceHandleDataMap.find(objectInstanceHandle) == _objectInstanceHandleDataMap.end()) {
        _objectInstanceHandleAllocator.take(message->getObjectInstanceHandle());
        insertObjectInstanceHandle(message->getObjectInstanceHandle(), message->getName(), *i);
      } else
        referenceObjectInstanceHandle(message->getObjectInstanceHandle(), *i);
    }

    send(connectHandleSet, message);
  }
  void accept(const ConnectHandle& connectHandle, DeleteObjectInstanceMessage* message)
  {
    ObjectInstance* objectInstance = getObjectInstance(message->getObjectInstanceHandle());
    if (!objectInstance)
      throw MessageError(L"DeleteObjectInstanceMessage for unknown ObjectInstance.");

    ObjectClass* objectClass = objectInstance->getObjectClass();
    if (!objectClass)
      throw MessageError(L"DeleteObjectInstanceMessage for unknown ObjectClass.");

    // send that to all servers that have seen that object instance at some time
    // OpenRTIAssert(objectInstance->getPrivilegeToDeleteAttribute()->_recieveingConnects.count(connectHandle) == 0);
    // send(objectInstance->getPrivilegeToDeleteAttribute()->_recieveingConnects, message);
    send(objectInstance->getPrivilegeToDeleteAttribute()->_recieveingConnects, connectHandle, message);
    // Ok, here ... we need to unsubscribe these node that no longer need that

    // currently without message retraction this is true ...
    eraseObjectInstance(objectInstance);
  }
  void accept(const ConnectHandle& connectHandle, LocalDeleteObjectInstanceMessage* message)
  {
    ObjectInstance* objectInstance = getObjectInstance(message->getObjectInstanceHandle());
    if (!objectInstance)
      return;
      // throw MessageError(L"LocalDeleteObjectInstanceMessage for unknown ObjectInstance.");
    // FIXME only propagate further, if nobody here else is interrested in this object.
    // Which means, need to track attributes that might be no longer needed for this object
    // May be this message should be more: 'switch of updates for object instance'
    // Then during this messages way, propagate as long as there are any attributes to skip left.
    ConnectHandle ownerConnectHandle = objectInstance->getOwnerConnectHandle();
    send(ownerConnectHandle, message);
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
    send(interactionClass->getSubscribedConnectHandleSet(), connectHandle, message);
  }
  void accept(const ConnectHandle& connectHandle, TimeStampedInteractionMessage* message)
  {
    InteractionClass* interactionClass = getInteractionClass(message->getInteractionClassHandle());
    // This might happen with FOM modules
    if (!interactionClass)
      return;
    // Send to all subscribed connects except the originating one
    send(interactionClass->getSubscribedConnectHandleSet(), connectHandle, message);
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
      throw MessageError(L"Received RequestClassAttributeUpdateMessage for unknown object class!");

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

  bool hasJoinedFederatesForConnect(const ConnectHandle& connectHandle) const
  {
    ConnectHandleConnectDataMap::const_iterator i = _connectHandleConnectDataMap.find(connectHandle);
    if (i == _connectHandleConnectDataMap.end())
      return false;
    return !i->second._federateHandleSet.empty();
  }

  void insertParentConnect(const ConnectHandle& connectHandle, const SharedPtr<AbstractMessageSender>& messageSender)
  {
    OpenRTIAssert(connectHandle.valid());
    OpenRTIAssert(!_parentServerConnectHandle.valid());
    _parentServerConnectHandle = connectHandle;
    if (_connectHandleConnectDataMap.find(connectHandle) != _connectHandleConnectDataMap.end())
      return;
    _connectHandleConnectDataMap[connectHandle]._messageSender = messageSender;
  }
  void insertConnect(const ConnectHandle& connectHandle, const SharedPtr<AbstractMessageSender>& messageSender)
  {
    OpenRTIAssert(connectHandle.valid());
    if (_connectHandleConnectDataMap.find(connectHandle) != _connectHandleConnectDataMap.end()) {
      OpenRTIAssert(_connectHandleConnectDataMap.find(connectHandle)->second._messageSender == messageSender);
      return;
    }
    _connectHandleConnectDataMap[connectHandle]._messageSender = messageSender;

    SharedPtr<InsertFederationExecutionMessage> message = new InsertFederationExecutionMessage;
    message->setFederationHandle(getHandle());
    message->setFederationName(getName());
    message->setLogicalTimeFactoryName(getLogicalTimeFactoryName());
    // FIXME push them as required
    message->setFOMModuleList(getModuleList());
    messageSender->send(message);

    for (FederateHandleFederateDataMap::const_iterator i = _federateHandleFederateDataMap.begin();
         i != _federateHandleFederateDataMap.end(); ++i) {
      SharedPtr<JoinFederateNotifyMessage> notify = new JoinFederateNotifyMessage;
      notify->setFederationHandle(getHandle());
      notify->setFederateHandle(i->first);
      notify->setFederateType(i->second._federateType);
      notify->setFederateName(*(i->second._stringSetIterator));
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
    if (_parentServerConnectHandle == connectHandle)
      _parentServerConnectHandle = ConnectHandle();

    // Unsubscribe this connect
    unsubscribeConnect(connectHandle);

    // FIXME avoid this loop over all objects
    // FIXME avoid precollecting these in the object handle set
    ObjectInstanceHandleSet objectInstanceHandleSet;
    for (ObjectInstanceMap::const_iterator j = getObjectInstanceMap().begin(); j != getObjectInstanceMap().end(); ++j) {
      if (j->second->getOwnerConnectHandle() != connectHandle)
        continue;
      FederateHandle federateHandle = j->second->getOwnerFederateHandle();
      if (!federateHandle.valid())
        continue;
      FederateHandleFederateDataMap::iterator k = _federateHandleFederateDataMap.find(j->second->getOwnerFederateHandle());
      OpenRTIAssert(k != _federateHandleFederateDataMap.end());

      // delete object instances if requested
      ResignAction resignAction = k->second._defaultResignAction;
      bool deleteObject = resignAction == DELETE_OBJECTS ||
        resignAction == DELETE_OBJECTS_THEN_DIVEST || resignAction == CANCEL_THEN_DELETE_THEN_DIVEST;
      // FIXME: currently we do not have ownership management - so, if the owner dies the object needs to die too
      deleteObject = true;

      if (deleteObject) {
        objectInstanceHandleSet.insert(j->second->getHandle());
      } else {
        j->second->setOwnerFederateHandle(FederateHandle());
        j->second->setOwnerConnectHandle(ConnectHandle());
      }
    }
    for (ObjectInstanceHandleSet::iterator j = objectInstanceHandleSet.begin();
         j != objectInstanceHandleSet.end(); ++j) {
      SharedPtr<DeleteObjectInstanceMessage> request;
      request = new DeleteObjectInstanceMessage;
      request->setFederationHandle(getHandle());
      request->setObjectInstanceHandle(*j);
      accept(connectHandle, request.get());
      if (connectHandle != _parentServerConnectHandle)
        unreferenceObjectInstanceHandle(*j, connectHandle);
    }

    if (connectHandle != _parentServerConnectHandle) {
      for (ObjectInstanceHandleDataMap::iterator j = _objectInstanceHandleDataMap.begin(); j != _objectInstanceHandleDataMap.end();) {
        unreferenceObjectInstanceHandle(j++, connectHandle);
      }
    }

    // Unpublish this connect
    unpublishConnect(connectHandle);

    ConnectHandleConnectDataMap::iterator i = _connectHandleConnectDataMap.find(connectHandle);
    if (i != _connectHandleConnectDataMap.end()) {
      FederateHandleSet federateHandleSet = i->second._federateHandleSet;
      for (FederateHandleSet::iterator j = federateHandleSet.begin(); j != federateHandleSet.end(); ++j) {
        Log(FederationServer, Info) << "Resigning federate " << ucsToLocale(j->toString()) << " because of closed connection!" << std::endl;
        SharedPtr<ResignFederationExecutionRequestMessage> message = new ResignFederationExecutionRequestMessage;
        message->setFederationHandle(getHandle());
        message->setFederateHandle(*j);
        FederateHandleFederateDataMap::iterator k = _federateHandleFederateDataMap.find(*j);
        if (k != _federateHandleFederateDataMap.end()) {
          k->second._connectHandle = ConnectHandle();
        }
        accept(connectHandle, message.get());
      }

      if (i->second._messageSender.valid()) {
        SharedPtr<EraseFederationExecutionMessage> eraseMessage = new EraseFederationExecutionMessage;
        eraseMessage->setFederationHandle(getHandle());
        i->second._messageSender->send(eraseMessage);
      }

      // Finally remove what is referencing the old connect handle
      _connectHandleConnectDataMap.erase(i);
    }

    Federation::removeConnect(connectHandle);

    // FIXME if the removed connection is the parent and we have a resign pending, respond as if we were the root
  }

  bool hasJoinedFederates() const
  {
    OpenRTIAssert(_federateHandleFederateDataMap.empty() == (!_federateHandleAllocator.used()));
    return !_federateHandleFederateDataMap.empty();
  }

  bool hasJoinedChildren() const
  {
    for (ConnectHandleConnectDataMap::const_iterator i = _connectHandleConnectDataMap.begin();
	 i != _connectHandleConnectDataMap.end(); ++i) {
      if (_parentServerConnectHandle.valid() && i->first == _parentServerConnectHandle)
	continue;
      // Ok, even the still pending resign request for invalid connect handles is treated as valid child.
      return true;
    }
    return false;
  }

  void broadcastToChildren(const SharedPtr<AbstractMessage>& message) const
  {
    for (ConnectHandleConnectDataMap::const_iterator i = _connectHandleConnectDataMap.begin();
	 i != _connectHandleConnectDataMap.end(); ++i) {
      if (!i->first.valid())
	continue;
      if (i->first == _parentServerConnectHandle)
	continue;
      i->second._messageSender->send(message);
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
      i->second._messageSender->send(message);
    }
  }

  void broadcastToChildren(const FederateHandleSet& federateHandleSet, const SharedPtr<AbstractMessage>& message) const
  {
    ConnectHandleSet broadcastSet;
    for (FederateHandleSet::const_iterator i = federateHandleSet.begin(); i != federateHandleSet.end(); ++i) {
      FederateHandleFederateDataMap::const_iterator j = _federateHandleFederateDataMap.find(*i);
      if (j == _federateHandleFederateDataMap.end())
        continue;
      if (!j->second._connectHandle.valid())
        continue;
      if (j->second._connectHandle == _parentServerConnectHandle)
        continue;
      broadcastSet.insert(j->second._connectHandle);
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
      i->second._messageSender->send(message);
    }
  }
  void send(const ConnectHandle& connectHandle, const SharedPtr<AbstractMessage>& message) const
  {
    ConnectHandleConnectDataMap::const_iterator i = _connectHandleConnectDataMap.find(connectHandle);
    OpenRTIAssert(i != _connectHandleConnectDataMap.end());
    OpenRTIAssert(i->second._messageSender.valid());
    i->second._messageSender->send(message);
  }
  void send(const FederateHandle& federateHandle, const SharedPtr<AbstractMessage>& message) const
  {
    FederateHandleFederateDataMap::const_iterator i = _federateHandleFederateDataMap.find(federateHandle);
    if (i == _federateHandleFederateDataMap.end())
      return;
    if (!i->second._connectHandle.valid())
      return;
    send(i->second._connectHandle, message);
  }

  bool isChildFederate(const FederateHandle& federateHandle) const
  {
    FederateHandleFederateDataMap::const_iterator i;
    i = _federateHandleFederateDataMap.find(federateHandle);
    if (i == _federateHandleFederateDataMap.end())
      return false;
    if (i->second._connectHandle == _parentServerConnectHandle)
      return false;
    return true;
  }

  ConnectHandle getConnectHandle(const FederateHandle& federateHandle) const
  {
    FederateHandleFederateDataMap::const_iterator i = _federateHandleFederateDataMap.find(federateHandle);
    if (i == _federateHandleFederateDataMap.end())
      return ConnectHandle();
    return i->second._connectHandle;
  }

  /// The rti servers options
  SharedPtr<const ServerOptions> _serverOptions;

  /// Synchronizatoin labels are tracked here
  typedef std::map<std::wstring, SynchronizationState> SyncronizationLabelStateMap;
  SyncronizationLabelStateMap _syncronizationLabelStateMap;

  /// Contains the committed timestamps, we will need them for a join response
  FederateHandleTimeStampMap _federateHandleTimeStampMap;

  /// track object instance handles
  ObjectInstanceHandleAllocator _objectInstanceHandleAllocator;

  // Ok, this is not held in the ObjectInstance ??!! the problem is that this is just the
  // knowledge about the object handle and name. Not yet any object class is known.
  // Hmm, may be we should be able to create object instances without object class instead??
  struct ObjectInstanceData {
    ObjectInstanceData() {}
    ObjectInstanceData(const StringSet::iterator& stringSetIterator) : _stringSetIterator(stringSetIterator) {}
    // The child connect handles that reference this object instance handle
    ConnectHandleSet _connectHandleSet;
    // Hmm, this is the other possibility
    // SharedPtr<ObjectInstance> _objectInstance;
    // FIXME collapse that with the name/handle resources above
    StringSet::iterator _stringSetIterator;
  };
  typedef std::map<ObjectInstanceHandle, ObjectInstanceData> ObjectInstanceHandleDataMap;
  ObjectInstanceHandleDataMap _objectInstanceHandleDataMap;
  StringSet _objectInstanceNameSet;

  void insertObjectInstanceHandle(const ObjectInstanceHandle& objectInstanceHandle, const std::wstring& name,
                                  const ConnectHandle& connectHandle)
  {
    OpenRTIAssert(connectHandle != _parentServerConnectHandle);
    OpenRTIAssert(_objectInstanceHandleDataMap.find(objectInstanceHandle) == _objectInstanceHandleDataMap.end());
    OpenRTIAssert(_connectHandleConnectDataMap.find(connectHandle) != _connectHandleConnectDataMap.end());
    OpenRTIAssert(!_connectHandleConnectDataMap[connectHandle]._federateHandleSet.empty());
    StringSet::iterator i = _objectInstanceNameSet.insert(name).first;
    typedef ObjectInstanceHandleDataMap::value_type value_type;
    ObjectInstanceHandleDataMap::iterator j;
    j = _objectInstanceHandleDataMap.insert(value_type(objectInstanceHandle, ObjectInstanceData(i))).first;
    j->second._connectHandleSet.insert(connectHandle);
    // FIXME can put that into a better api with a default argument for the instance handle and a returned allocated handle for example
    // _objectInstanceHandleAllocator.take(objectInstanceHandle);
  }
  void referenceObjectInstanceHandle(const ObjectInstanceHandle& objectInstanceHandle, const ConnectHandle& connectHandle)
  {
    OpenRTIAssert(connectHandle != _parentServerConnectHandle);
    ObjectInstanceHandleDataMap::iterator i = _objectInstanceHandleDataMap.find(objectInstanceHandle);
    OpenRTIAssert(i != _objectInstanceHandleDataMap.end());
    i->second._connectHandleSet.insert(connectHandle);
  }
  void unreferenceObjectInstanceHandle(ObjectInstanceHandleDataMap::iterator i, const ConnectHandle& connectHandle)
  {
    OpenRTIAssert(connectHandle != _parentServerConnectHandle);
    OpenRTIAssert(i != _objectInstanceHandleDataMap.end());
    i->second._connectHandleSet.erase(connectHandle);

    ObjectInstance* objectInstance = getObjectInstance(i->first);
    if (objectInstance)
      objectInstance->removeConnect(connectHandle);

    if (!i->second._connectHandleSet.empty())
      return;

    if (objectInstance)
      eraseObjectInstance(objectInstance);

    if (_parentServerConnectHandle.valid()) {
      SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> message;
      message = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
      message->setFederationHandle(getHandle());
      message->getObjectInstanceHandleVector().push_back(i->first);
      send(_parentServerConnectHandle, message);
    }

    _objectInstanceHandleAllocator.put(i->first);

    _objectInstanceNameSet.erase(i->second._stringSetIterator);
    _objectInstanceHandleDataMap.erase(i);
  }
  void unreferenceObjectInstanceHandle(const ObjectInstanceHandle& objectInstanceHandle, const ConnectHandle& connectHandle)
  {
    unreferenceObjectInstanceHandle(_objectInstanceHandleDataMap.find(objectInstanceHandle), connectHandle);
  }

  ConnectHandle _parentServerConnectHandle;

  /// The pool of federate handles that the server has available.
  FederateHandleAllocator _federateHandleAllocator;

  // The FederateHandle <-> federate data mappings
  struct FederateData {
    FederateData(const ConnectHandle& connectHandle, const StringSet::iterator& stringSetIterator) :
      _connectHandle(connectHandle),
      _stringSetIterator(stringSetIterator),
      _defaultResignAction(CANCEL_THEN_DELETE_THEN_DIVEST),
      _resignPending(false)
    { }
    ConnectHandle _connectHandle;
    std::wstring _federateType;
    StringSet::iterator _stringSetIterator;
    ResignAction _defaultResignAction;
    bool _resignPending;
  };
  typedef std::map<FederateHandle, FederateData> FederateHandleFederateDataMap;
  FederateHandleFederateDataMap _federateHandleFederateDataMap;
  StringSet _federateNameSet;

  // The ConnectHandle <-> connect data mappings
  struct ConnectData {
    SharedPtr<AbstractMessageSender> _messageSender;
    // FIXME make this an iterator to the FederateData above
    FederateHandleSet _federateHandleSet;
    // FIXME
    // ObjectInstanceHandleSet _ownedObjectInstanceHandleSet;
  };
  typedef std::map<ConnectHandle, ConnectData> ConnectHandleConnectDataMap;
  ConnectHandleConnectDataMap _connectHandleConnectDataMap;
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
      Log(FederationServer, Warning) << getServerPath() << ": Received " << message->getTypeName()
                                     << " for unknown federation id: " << ucsToLocale(message->getFederationHandle().toString())
                                     << "!" << std::endl;
      throw MessageError(getServerPath() + std::wstring(L" received ") + localeToUcs(message->getTypeName())
                         + L" for unknown federation id: " + message->getFederationHandle().toString() + L"!");
    }
    i->second->acceptFederationMessage(connectHandle, message);
  }
  template<typename M>
  void acceptUpstreamFederationMessage(const ConnectHandle& connectHandle, M* message)
  {
    OpenRTIAssert(connectHandle.valid());
    if (connectHandle == _serverConnectSet.getParentConnectHandle())
      throw MessageError(std::wstring(L"Received ") + localeToUcs(message->getTypeName()) + L" through the parent connect!");
    acceptFederationMessage(connectHandle, message);
  }
  template<typename M>
  void acceptDownstreamFederationMessage(const ConnectHandle& connectHandle, M* message)
  {
    OpenRTIAssert(connectHandle.valid());
    if (connectHandle != _serverConnectSet.getParentConnectHandle())
      throw MessageError(std::wstring(L"Received ") + localeToUcs(message->getTypeName()) + L" through a child connect!");
    acceptFederationMessage(connectHandle, message);
  }

  // If the parent connect dies, tell this all children
  void accept(const ConnectHandle& connectHandle, ConnectionLostMessage* message)
  {
    if (connectHandle != _serverConnectSet.getParentConnectHandle())
      throw MessageError(std::wstring(L"Received ") + localeToUcs(message->getTypeName()) + L" through a child connect!");
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

          Log(FederationServer, Info) << "Create federation execution \""
                                      << ucsToLocale(message->getFederationExecution()) << "\"." << std::endl;

          // ... and respond with Success
          SharedPtr<CreateFederationExecutionResponseMessage> response;
          response = new CreateFederationExecutionResponseMessage;
          response->setCreateFederationExecutionResponseType(CreateFederationExecutionResponseSuccess);
          _serverConnectSet.send(connectHandle, response);

        } catch (const Exception& e) {
          Log(FederationServer, Info) << "Caught Exception creating federation execution \"" << e.getReason() << "\"." << std::endl;
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
        throw MessageError(L"Received CreateExecutionRequestMessage through the parent connect!");

      _pendingMessageList.push_back(ConnectHandleMessagePair(connectHandle, message));
      _serverConnectSet.sendToParent(message);
    }
  }
  void accept(const ConnectHandle& connectHandle, CreateFederationExecutionResponseMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    // Such a response must originate from the parent.
    if (connectHandle != _serverConnectSet.getParentConnectHandle())
      throw MessageError(L"Received CreateExecutionResponseMessage through a child connect!");

    // need to have a connect handle to resond to
    if (_pendingMessageList.empty())
      throw MessageError(L"No pending CreateExecutionRequestMessage but received CreateFederationExecutionResponseMessage!");

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
        Log(FederationServer, Info) << "DestroyFederationExecutionRequestMessage faild for unknown federation named \""
                                    << ucsToLocale(message->getFederationExecution()) << "\"!" << std::endl;
        // Ok, have an inactive federation, destroy needs to fail
        SharedPtr<DestroyFederationExecutionResponseMessage> response;
        response = new DestroyFederationExecutionResponseMessage;
        response->setDestroyFederationExecutionResponseType(DestroyFederationExecutionResponseFederationExecutionDoesNotExist);
        _serverConnectSet.send(connectHandle, response);
      } else {

        // Federates currently joined?
        if (i->second->hasJoinedFederates()) {
          Log(FederationServer, Info) << "DestroyFederationExecutionRequestMessage faild for \""
                                      << ucsToLocale(message->getFederationExecution()) << "\", federates joined!" << std::endl;
          // federades there, so, no
          SharedPtr<DestroyFederationExecutionResponseMessage> response;
          response = new DestroyFederationExecutionResponseMessage;
          response->setDestroyFederationExecutionResponseType(DestroyFederationExecutionResponseFederatesCurrentlyJoined);
          _serverConnectSet.send(connectHandle, response);
        } else {
          // Successful destroy
          eraseFederation(i);

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
        throw MessageError(L"Received DestroyExecutionRequestMessage through the parent connect!");

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
      throw MessageError(L"Received DestroyExecutionResponseMessage through a child connect!");

    // need to have a connect handle to resond to
    if (_pendingMessageList.empty())
      throw MessageError(L"No pending DestroyExecutionResponseMessage even in FederateDestroyPending state!");

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
        throw MessageError(L"Received EnumerateFederationExecutionsRequestMessage through the parent connect!");

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
      throw MessageError(L"Received EnumerateFederationExecutionsResponseMessage through a child connect!");

    // need to have a connect handle to resond to
    if (_pendingMessageList.empty())
      throw MessageError(L"No pending EnumerateFederationExecutionsResponseMessage!");

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

    FederationHandle federationHandle = message->getFederationHandle();
    if (!federationHandle.valid())
      throw MessageError("Received InsertFederationExecutionMessage with invalid federation handle!");
    if (_federationServerMap.find(federationHandle) != _federationServerMap.end())
      throw MessageError("Received InsertFederationExecutionMessage for an already existing federation!");
    if (_federationServerMap.find(message->getFederationName()) != _federationServerMap.end())
      throw MessageError("Received InsertFederationExecutionMessage for an already existing federation!");

    FederationServerMap::iterator i = insertFederation(message->getFederationName(), federationHandle);
    i->second->setLogicalTimeFactoryName(message->getLogicalTimeFactoryName());
    i->second->insert(message->getFOMModuleList());
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
    if (_federationServerMap.find(federationHandle) == _federationServerMap.end())
      throw MessageError("Received EraseFederationExecutionMessage for a non existing federation!");

    // Find the federation and erase it
    eraseFederation(federationHandle);
  }

  // The Join messages
  void accept(const ConnectHandle& connectHandle, JoinFederationExecutionRequestMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());

    // The ambassador already needs to care for that. So, if we get that here, drop the connection.
    if (message->getFederateName().compare(0, 3, L"HLA") == 0)
      throw MessageError(L"Got JoinFederationExecutionRequestMessage with name starting with HLA.");

    // If we are a root server ...
    if (isRootServer()) {

      FederationServerMap::const_iterator i = _federationServerMap.find(message->getFederationExecution());
      if (i == _federationServerMap.end()) {
        // FederationExecutionDoesNotExist ...
        Log(FederationServer, Info) << "JoinFederationExecutionRequestMessage faild for unknown federation named \""
                                    << ucsToLocale(message->getFederationExecution()) << "\"!" << std::endl;
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
	i->second->insertConnect(connectHandle, _serverConnectSet.getMessageSender(connectHandle));
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
      throw MessageError(L"Received JoinFederationExecutionResponseMessage through a child connect!");

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
        throw MessageError(L"Received successful JoinFederationExecutionResponseMessage with an invalid federate handle!");
      if (!message->getFederationHandle().valid())
        throw MessageError(L"Received successful JoinFederationExecutionResponseMessage with an invalid federation handle!");
      FederationServerMap::iterator i = _federationServerMap.find(message->getFederationHandle());
      if (i == _federationServerMap.end())
        throw MessageError(L"Received successful JoinFederationExecutionResponseMessage with an unknown federation handle!");

      if (requestConnectHandle.valid())
        i->second->insertConnect(requestConnectHandle, _serverConnectSet.getMessageSender(requestConnectHandle));
      i->second->accept(connectHandle, requestConnectHandle, message);
    }

    // remove the pending message
    _pendingMessageList.pop_front();
  }

  void accept(const ConnectHandle& connectHandle, ResignFederationExecutionRequestMessage* message)
  { acceptUpstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, ResignFederationExecutionResponseMessage* message)
  { acceptDownstreamFederationMessage(connectHandle, message); }

  // Message to inform federates about newly joined federates
  void accept(const ConnectHandle& connectHandle, JoinFederateNotifyMessage* message)
  { acceptDownstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, ResignFederateNotifyMessage* message)
  { acceptDownstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, ChangeDefaultResignActionMessage* message)
  { acceptFederationMessage(connectHandle, message); }

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
  void accept(const ConnectHandle& connectHandle, LocalDeleteObjectInstanceMessage* message)
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
  { throw MessageError(L"Received unexpected message???"); }

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
    FunctorMessageDispatcher<DispatchFunctor> dispatcher(DispatchFunctor(*this, connectHandle));
    message.dispatch(dispatcher);
  }

  ConnectHandle insertConnect(const SharedPtr<AbstractMessageSender>& messageSender)
  {
    return _serverConnectSet.insertMessageSender(messageSender);
  }
  ConnectHandle insertParentConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& parentOptions)
  {
    _serverOptions->setParentOptionMap(parentOptions);
    return _serverConnectSet.insertParentMessageSender(messageSender);
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
      // Notify all children about the lost connection.
      _serverConnectSet.broadcastToChildren(new ConnectionLostMessage);

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

  const std::wstring& getServerPath() const
  { return _serverOptions->getServerPath(); }

private:
  // Federations in a server
  // Map federation execution names and handles to federation servers.
  typedef NameHandleMap<FederationHandle, FederationServer> FederationServerMap;
  FederationServerMap _federationServerMap;

  FederationServerMap::iterator insertFederation(const std::wstring& name)
  {
    OpenRTIAssert(isRootServer());
    OpenRTIAssert(_federationServerMap.find(name) == _federationServerMap.end());

    FederationHandle federationHandle = _federationHandleAllocator.get();
    SharedPtr<FederationServer> federationServer = new FederationServer(name, federationHandle, _serverOptions);
    return _federationServerMap.insert(FederationServerMap::value_type(federationHandle, federationServer)).first;
  }
  FederationServerMap::iterator insertFederation(const std::wstring& name, const FederationHandle& federationHandle)
  {
    OpenRTIAssert(federationHandle.valid());
    OpenRTIAssert(_federationServerMap.find(name) == _federationServerMap.end());
    OpenRTIAssert(_federationServerMap.find(federationHandle) == _federationServerMap.end());

    _federationHandleAllocator.take(federationHandle);
    SharedPtr<FederationServer> federationServer = new FederationServer(name, federationHandle, _serverOptions);
    if (!isRootServer())
      federationServer->insertParentConnect(_serverConnectSet.getParentConnectHandle(),
                                            _serverConnectSet.getMessageSender(_serverConnectSet.getParentConnectHandle()));
    return _federationServerMap.insert(FederationServerMap::value_type(federationHandle, federationServer)).first;
  }
  void eraseFederation(const FederationHandle& federationHandle)
  {
    eraseFederation(_federationServerMap.find(federationHandle));
  }
  void eraseFederation(FederationServerMap::iterator i)
  {
    OpenRTIAssert(i != _federationServerMap.end());
    OpenRTIAssert(!i->second->hasJoinedChildren());

    Log(FederationServer, Info) << getServerPath() << ": Destroyed federation execution in child server for \""
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

const std::wstring&
ServerNode::getServerName() const
{
  return _serverMessageDispatcher->getServerOptions().getServerName();
}

void
ServerNode::setServerName(const std::wstring& name)
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
ServerNode::insertConnect(const SharedPtr<AbstractMessageSender>& messageSender)
{
  return _serverMessageDispatcher->insertConnect(messageSender);
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
