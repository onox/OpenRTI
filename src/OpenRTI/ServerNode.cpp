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

#include "ServerNode.h"

#include <algorithm>
#include <iterator>
#include "Message.h"
#include "AbstractMessageSender.h"
#include "LogStream.h"
#include "ServerModel.h"
#include "ServerOptions.h"

namespace OpenRTI {

class OPENRTI_LOCAL FederationServer : public ServerModel::Federation {
public:
  FederationServer(ServerModel::Node& serverNode) :
    ServerModel::Federation(serverNode),
    _parentPermitTimeRegulation(true)
  { }
  virtual ~FederationServer()
  { }

  void setParentConfigurationParameterMap(const ConfigurationParameterMap& configurationParameterMap)
  {
    ConfigurationParameterMap::const_iterator i;
    // time regulation is by default permittet, but may be denied due to parent server policy
    i = configurationParameterMap.find("permitTimeRegulation");
    if (i != configurationParameterMap.end() && !i->second.empty()) {
      _parentPermitTimeRegulation = (i->second.front() == "true");
    } else {
      _parentPermitTimeRegulation = true;
    }
  }

  void accept(const ConnectHandle& connectHandle, const InsertModulesMessage* message)
  {
    broadcastToChildren(message);
    insert(message->getFOMModuleList());
  }

  void accept(const ConnectHandle& connectHandle, const JoinFederationExecutionRequestMessage* message)
  {
    OpenRTIAssert(isRootServer());

    if (!message->getFederateName().empty() && isFederateNameInUse(message->getFederateName())) {
      SharedPtr<JoinFederationExecutionResponseMessage> response;
      response = new JoinFederationExecutionResponseMessage;
      response->setJoinFederationExecutionResponseType(JoinFederationExecutionResponseFederateNameAlreadyInUse);
      response->setExceptionString(message->getFederateName());
      getServerNode().send(connectHandle, response);
      return;
    }

    // Try to extend the object model
    if (!message->getFOMStringModuleList().empty()) {
      ModuleHandleVector moduleHandleVector;
      try {
        insert(moduleHandleVector, message->getFOMStringModuleList());
      } catch (OpenRTI::Exception& e) {
        SharedPtr<JoinFederationExecutionResponseMessage> response;
        response = new JoinFederationExecutionResponseMessage;
        response->setJoinFederationExecutionResponseType(JoinFederationExecutionResponseInconsistentFDD);
        response->setExceptionString(e.what());
        getServerNode().send(connectHandle, response);
        return;
      }

      if (!moduleHandleVector.empty()) {
        SharedPtr<InsertModulesMessage> insertModules;
        insertModules = new InsertModulesMessage;
        getModuleList(insertModules->getFOMModuleList(), moduleHandleVector);
        insertModules->setFederationHandle(getFederationHandle());
        broadcastToChildren(insertModules);
      }
    }

    // ... insert a new federate ...
    ServerModel::Federate* federate = new ServerModel::Federate(*this);
    federate->setName(message->getFederateName());
    federate->setFederateType(message->getFederateType());
    insert(*federate);

    // Survived so far, insert a new federation connect
    ServerModel::NodeConnect* nodeConnect = getServerNode().getNodeConnect(connectHandle);
    OpenRTIAssert(nodeConnect);
    ServerModel::FederationConnect* federationConnect = getOrInsertConnect(*nodeConnect);
    OpenRTIAssert(federationConnect);
    federationConnect->insert(*federate);
    pushFederation(connectHandle);

    // Respond with Success
    SharedPtr<JoinFederationExecutionResponseMessage> response;
    response = new JoinFederationExecutionResponseMessage;
    response->setJoinFederationExecutionResponseType(JoinFederationExecutionResponseSuccess);
    response->setFederationHandle(getFederationHandle());
    response->setFederateHandle(federate->getFederateHandle());
    response->setFederateName(federate->getName());
    response->setFederateType(federate->getFederateType());
    send(connectHandle, response);

    // send all children the notification about the new federate except the one that gets the join response
    SharedPtr<JoinFederateNotifyMessage> notify = new JoinFederateNotifyMessage;
    notify->setFederationHandle(getFederationHandle());
    notify->setFederateHandle(federate->getFederateHandle());
    notify->setFederateName(federate->getName());
    notify->setFederateType(federate->getFederateType());
    broadcastToChildren(connectHandle, notify);

    // For those sync request that are automatically extended to new federates, send the announcement
    for (ServerModel::Synchronization::NameMap::iterator j = _synchronizationNameSynchronizationMap.begin();
         j != _synchronizationNameSynchronizationMap.end(); ++j) {
      // only those with an auto expanding federate set
      if (!j->getAddJoiningFederates())
        continue;

      j->insert(*federate);

      SharedPtr<AnnounceSynchronizationPointMessage> announce;
      announce = new AnnounceSynchronizationPointMessage;
      announce->setFederationHandle(getFederationHandle());
      announce->setLabel(j->getLabel());
      announce->setTag(j->getTag());
      announce->getFederateHandleVector().push_back(federate->getFederateHandle());
      announce->setAddJoiningFederates(j->getAddJoiningFederates());
      broadcastToChildren(announce);
    }
  }
  void accept(const ConnectHandle& connectHandle, const ConnectHandle& requestConnectHandle, const JoinFederationExecutionResponseMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());

    FederateHandle federateHandle = message->getFederateHandle();
    OpenRTIAssert(federateHandle.valid());

    ServerModel::Federate* federate = insertFederate(requestConnectHandle, message->getFederateName(), federateHandle);
    OpenRTIAssert(federate);
    federate->setFederateType(message->getFederateType());

    // send all children the notification about the new federate except the one that gets the join response
    SharedPtr<JoinFederateNotifyMessage> notify = new JoinFederateNotifyMessage;
    notify->setFederationHandle(message->getFederationHandle());
    notify->setFederateHandle(message->getFederateHandle());
    notify->setFederateType(message->getFederateType());
    notify->setFederateName(message->getFederateName());
    broadcastToChildren(requestConnectHandle, notify);
  }


  void accept(const ConnectHandle& connectHandle, const ResignFederationExecutionRequestMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    FederateHandle federateHandle = message->getFederateHandle();
    ServerModel::Federate* federate = getFederate(federateHandle);
    if (!federate)
      throw MessageError("Received ResignFederationExecutionRequestMessage for invalid federate handle!");

    // already done so ... just waiting for the response
    if (federate->getResignPending())
      return;

    // We need to skip resource allocations in the future
    federate->setResignPending(true);

    // remove from time management
    if (federate->getIsTimeRegulating()) {
      eraseTimeRegulating(*federate);
      SharedPtr<DisableTimeRegulationRequestMessage> request = new DisableTimeRegulationRequestMessage;
      request->setFederationHandle(getFederationHandle());
      request->setFederateHandle(federateHandle);
      broadcast(connectHandle, request);
    }

    for (ServerModel::SynchronizationFederate::FirstList::iterator k = federate->getSynchronizationFederateList().begin();
         k != federate->getSynchronizationFederateList().end();) {
      if (!k->getSynchronization().getIsWaitingFor(federate->getFederateHandle()))
        ++k;
      else {
        SharedPtr<SynchronizationPointAchievedMessage> achieved;
        achieved = new SynchronizationPointAchievedMessage;
        achieved->setFederationHandle(getFederationHandle());
        achieved->setLabel(k->getSynchronization().getLabel());
        achieved->getFederateHandleBoolPairVector().push_back(FederateHandleBoolPair(federate->getFederateHandle(), false));
        ++k;
        accept(connectHandle, achieved.get());
      }
    }

    // May be we need to put object deletion and that here and remove the second resign message again

    // If we are a root server ...
    if (isRootServer()) {
      // ... and respond with Success
      /// FIXME shall we get that message from the federation server???
      SharedPtr<ResignFederateNotifyMessage> notify = new ResignFederateNotifyMessage;
      notify->setFederateHandle(federateHandle);
      notify->setFederationHandle(getFederationHandle());
      broadcastToChildren(notify);

      // Remove the federate locally
      eraseFederate(*federate);

      // Remove a connect that runs idle
      ServerModel::FederationConnect* federationConnect = getFederationConnect(connectHandle);
      if (federationConnect && !federationConnect->getHasFederates())
        eraseFederationExecutionAtConnect(connectHandle);
    }
    // If we have an upstream connect, mark this request as pending and ask the parent server
    else {
      /// FIXME in case of a dying parent, we need to reevaluate this too
      sendToParent(message);
    }
  }

  void accept(const ConnectHandle& connectHandle, const ResignFederationExecutionLeafRequestMessage* message)
  {
    // We can use asserts here since this message is only produced by a federate ambassador which is what we have with us in the same shared library
    OpenRTIAssert(connectHandle.valid());
    ServerModel::FederationConnect* federationConnect = getFederationConnect(connectHandle);
    OpenRTIAssert(federationConnect);

    FederateHandle federateHandle = message->getFederateHandle();
    OpenRTIAssert(federationConnect->getFederateList().size() == 1);
    OpenRTIAssert(federationConnect->getFederateList().front().getFederateHandle() == federateHandle);

    // Since this server node is executing the resign setting this here should
    // be sufficient.
    ServerModel::Federate* federate = getFederate(federateHandle);
    OpenRTIAssert(federate);
    federate->setResignAction(message->getResignAction());

    resignConnect(connectHandle);
  }

  void pushPublications(const ConnectHandle& connectHandle)
  {
    for (ServerModel::InteractionClass::HandleMap::iterator i = getInteractionClassHandleInteractionClassMap().begin();
         i != getInteractionClassHandleInteractionClassMap().end(); ++i) {
      const ServerModel::InteractionClass* interactionClass = i.get();
      if (!interactionClass)
        continue;
      if (Unpublished == interactionClass->getPublicationType())
        continue;

      SharedPtr<ChangeInteractionClassPublicationMessage> message;
      message = new ChangeInteractionClassPublicationMessage;
      message->setFederationHandle(getFederationHandle());
      message->setPublicationType(Published);
      message->setInteractionClassHandle(interactionClass->getInteractionClassHandle());
      send(connectHandle, message);
    }

    // Object classes
    for (ServerModel::ObjectClass::HandleMap::iterator i = getObjectClassHandleObjectClassMap().begin();
         i != getObjectClassHandleObjectClassMap().end(); ++i) {
      AttributeHandleVector attributeHandleVector;
      for (ServerModel::ClassAttribute::HandleMap::iterator j = i->getAttributeHandleClassAttributeMap().begin();
           j != i->getAttributeHandleClassAttributeMap().end(); ++j) {
        if (Unpublished == j->getPublicationType())
          continue;
        if (attributeHandleVector.empty())
          attributeHandleVector.reserve(i->getAttributeHandleClassAttributeMap().size());
        attributeHandleVector.push_back(j->getAttributeHandle());
      }
      if (attributeHandleVector.empty())
        continue;

      SharedPtr<ChangeObjectClassPublicationMessage> message;
      message = new ChangeObjectClassPublicationMessage;
      message->setFederationHandle(getFederationHandle());
      message->setPublicationType(Published);
      message->setObjectClassHandle(i->getObjectClassHandle());
      message->getAttributeHandles().swap(attributeHandleVector);
      send(connectHandle, message);
    }
  }

  void eraseFederationExecutionAtConnect(const ConnectHandle& connectHandle)
  {
    ServerModel::FederationConnect* federationConnect = getFederationConnect(connectHandle);
    if (!federationConnect)
      return;
    if (federationConnect->getIsParentConnect())
      return;
    OpenRTIAssert(!federationConnect->getHasFederates());

    for (ServerModel::Federate::HandleMap::const_iterator i = getFederateHandleFederateMap().begin();
         i != getFederateHandleFederateMap().end(); ++i) {
      SharedPtr<ResignFederateNotifyMessage> notify = new ResignFederateNotifyMessage;
      notify->setFederationHandle(getFederationHandle());
      notify->setFederateHandle(i->getFederateHandle());
      federationConnect->send(notify);
    }

    SharedPtr<EraseFederationExecutionMessage> response;
    response = new EraseFederationExecutionMessage;
    response->setFederationHandle(getFederationHandle());
    federationConnect->send(response);

    federationConnect->setActive(false);
  }

  void broadcastEraseFederationExecution()
  {
    for (ServerModel::FederationConnect::HandleMap::iterator i = getConnectHandleFederationConnectMap().begin();
         i != getConnectHandleFederationConnectMap().end(); ++i) {
      if (i->getIsParentConnect())
        continue;
      eraseFederationExecutionAtConnect(i->getConnectHandle());
    }
  }

  void accept(const ConnectHandle& connectHandle, const JoinFederateNotifyMessage* message)
  {
    FederateHandle federateHandle = message->getFederateHandle();
    if (getFederate(federateHandle))
      throw MessageError("Received JoinFederateNotify for already known federate!");
    ServerModel::Federate* federate = insertFederate(connectHandle, message->getFederateName(), federateHandle);
    OpenRTIAssert(federate);
    federate->setFederateType(message->getFederateType());
    broadcastToChildren(message);
  }
  void accept(const ConnectHandle& connectHandle, const ResignFederateNotifyMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    ServerModel::Federate* federate = getFederate(message->getFederateHandle());
    if (!federate)
      throw MessageError("Received ResignFederateNotify for unknown federate!");
    broadcastToChildren(message);

    // FIXME can we simplify this and remove the federate on upstream resign?
    ServerModel::FederationConnect* federationConnect = federate->getFederationConnect();
    eraseFederate(*federate);
    // Can happen that we ask for shutdown that is already underway
    if (!federationConnect)
      return;
    if (!federationConnect->getActive())
      return;
    if (federationConnect->getHasFederates())
      return;

    if (federationConnect->getIsParentConnect())
      return;

    /// FIXME do this only if a shutdown request is pending!
    eraseFederationExecutionAtConnect(federationConnect->getConnectHandle());
  }

  // Policy how to handle implicit resign
  void accept(const ConnectHandle& connectHandle, const ChangeAutomaticResignDirectiveMessage* message)
  {
    ServerModel::Federate* federate = getFederate(message->getFederateHandle());
    if (!federate)
      throw MessageError("Got ChangeAutomaticResignDirectiveMessage for an unknown federate!");
    federate->setResignAction(message->getResignAction());
    broadcast(connectHandle, message);
  }

  // Synchronization labels
  void accept(const ConnectHandle& connectHandle, const RegisterFederationSynchronizationPointMessage* message)
  {
    // Labels must be ckecked by the ambassador. So what arrives here with an empty label must be some kind of error.
    if (message->getLabel().empty())
      throw MessageError("Received empty label in RegisterFederationSynchronizationPointMessage!");

    // If we are a root server ...
    if (isRootServer()) {
      // label is already there
      if (_synchronizationNameSynchronizationMap.find(message->getLabel()) != _synchronizationNameSynchronizationMap.end()) {
        SharedPtr<RegisterFederationSynchronizationPointResponseMessage> response;
        response = new RegisterFederationSynchronizationPointResponseMessage;
        response->setFederationHandle(getFederationHandle());
        response->setLabel(message->getLabel());
        response->setFederateHandle(message->getFederateHandle());
        response->setRegisterFederationSynchronizationPointResponseType(RegisterFederationSynchronizationPointResponseLabelNotUnique);
        send(connectHandle, response);
        return;
      }

      ServerModel::Synchronization* synchronization;
      synchronization = new ServerModel::Synchronization;
      synchronization->setLabel(message->getLabel());
      ServerModel::Synchronization::NameMap::iterator i;
      i = _synchronizationNameSynchronizationMap.insert(*synchronization);

      if (message->getFederateHandleVector().empty()) {
        // In this case add all known federates and the future ones also
        i->setAddJoiningFederates(true);
        for (ServerModel::Federate::HandleMap::iterator j = getFederateHandleFederateMap().begin();
             j != getFederateHandleFederateMap().end(); ++j) {
          i->insert(*j);
        }

      } else {
        // Add the ones noted in the message
        i->setAddJoiningFederates(false);
        for (FederateHandleVector::const_iterator j = message->getFederateHandleVector().begin();
             j != message->getFederateHandleVector().end(); ++j) {
          ServerModel::Federate* federate = getFederate(*j);
          if (!federate)
            continue;
          i->insert(*federate);
        }
      }
      i->setTag(message->getTag());

      // Respond to the originator
      SharedPtr<RegisterFederationSynchronizationPointResponseMessage> response;
      response = new RegisterFederationSynchronizationPointResponseMessage;
      response->setFederationHandle(getFederationHandle());
      response->setLabel(message->getLabel());
      response->setFederateHandle(message->getFederateHandle());
      response->setRegisterFederationSynchronizationPointResponseType(RegisterFederationSynchronizationPointResponseSuccess);
      send(connectHandle, response);

      // Cycle over all child connects and send announcements with the appropriate handle sets

      for (ServerModel::FederationConnect::HandleMap::iterator j = getConnectHandleFederationConnectMap().begin();
           j != getConnectHandleFederationConnectMap().end(); ++j) {
        // Build the intersection of the federate handles in the message and the ones in the connect.
        FederateHandleVector federateHandleVector;
        federateHandleVector.reserve(j->getFederateList().size());
        for (ServerModel::Federate::FirstList::const_iterator k = j->getFederateList().begin();
             k != j->getFederateList().end(); ++k) {
          if (!i->getIsWaitingFor(k->getFederateHandle()))
            continue;
          federateHandleVector.push_back(k->getFederateHandle());
        }
        if (federateHandleVector.empty())
          continue;

        SharedPtr<AnnounceSynchronizationPointMessage> announce;
        announce = new AnnounceSynchronizationPointMessage;
        announce->setFederationHandle(getFederationHandle());
        announce->setLabel(message->getLabel());
        announce->setTag(i->getTag());
        announce->setAddJoiningFederates(i->getAddJoiningFederates());
        announce->getFederateHandleVector().swap(federateHandleVector);
        send(j->getConnectHandle(), announce);
      }
    }
    // If we have an upstream connect, mark this request as pending and ask the parent server
    else {
      // ... ask your father
      /// FIXME in case of a dying parent, we need to reevaluate this too
      sendToParent(message);
    }
  }
  void accept(const ConnectHandle& connectHandle, const RegisterFederationSynchronizationPointResponseMessage* message)
  {
    if (message->getLabel().empty())
      throw MessageError("Received empty label in RegisterFederationSynchronizationPointResponseMessage!");

    send(message->getFederateHandle(), message);
  }

  void accept(const ConnectHandle& connectHandle, const AnnounceSynchronizationPointMessage* message)
  {
    if (message->getLabel().empty())
      throw MessageError("Received empty label in AnnounceSynchronizationPointMessage!");

    ServerModel::Synchronization::NameMap::iterator i = _synchronizationNameSynchronizationMap.find(message->getLabel());
    if (i == _synchronizationNameSynchronizationMap.end()) {
      // label is new, create one
      ServerModel::Synchronization* synchronization;
      synchronization = new ServerModel::Synchronization;
      synchronization->setLabel(message->getLabel());
      i = _synchronizationNameSynchronizationMap.insert(*synchronization);
      i->setTag(message->getTag());
      i->setAddJoiningFederates(message->getAddJoiningFederates());
    } else {
      // label is already there
      if (!i->getAddJoiningFederates())
        throw MessageError("Receiving incremental synchronization point update for fixed federate handle synchronization point!");
      if (!message->getAddJoiningFederates())
        throw MessageError("Receiving incremental synchronization point update for fixed federate handle synchronization point!");
    }

    // Distribute the announce messages across the appropriate connects
    // First collect the federate handle sets by connect ...
    std::map<ConnectHandle, FederateHandleVector> connectHandleFederateHandleVectorMap;
    for (FederateHandleVector::const_iterator j = message->getFederateHandleVector().begin();
           j != message->getFederateHandleVector().end(); ++j) {
        ServerModel::Federate* federate = getFederate(*j);
        if (!federate)
          continue;
        ServerModel::FederationConnect* federationConnect = federate->getFederationConnect();
        if (!federationConnect)
          continue;
        if (federationConnect->getIsParentConnect())
          continue;
        ConnectHandle federateConnectHandle = federationConnect->getConnectHandle();
        if (!federateConnectHandle.valid())
          continue;
        FederateHandleVector& federateHandleVector = connectHandleFederateHandleVectorMap[federateConnectHandle];
        if (federateHandleVector.empty())
          federateHandleVector.reserve(message->getFederateHandleVector().size());
        federateHandleVector.push_back(*j);
        i->insert(*federate);
    }

    // ... then send them out throught the connect.
    for (std::map<ConnectHandle, FederateHandleVector>::iterator j = connectHandleFederateHandleVectorMap.begin();
         j != connectHandleFederateHandleVectorMap.end(); ++j) {
      SharedPtr<AnnounceSynchronizationPointMessage> announce;
      announce = new AnnounceSynchronizationPointMessage;
      announce->setFederationHandle(getFederationHandle());
      announce->setLabel(message->getLabel());
      announce->setTag(message->getTag());
      announce->setAddJoiningFederates(message->getAddJoiningFederates());
      announce->getFederateHandleVector().swap(j->second);
      send(j->first, announce);
    }
  }
  void accept(const ConnectHandle& connectHandle, const SynchronizationPointAchievedMessage* message)
  {
    ServerModel::Synchronization::NameMap::iterator i = _synchronizationNameSynchronizationMap.find(message->getLabel());
    if (i == _synchronizationNameSynchronizationMap.end())
      throw MessageError("SynchronizationPointAchievedMessage for unknown label!");

    for (FederateHandleBoolPairVector::const_iterator j = message->getFederateHandleBoolPairVector().begin();
         j != message->getFederateHandleBoolPairVector().end(); ++j) {
      i->achieved(j->first, j->second);
    }
    if (i->_waitingFederateSyncronizationMap.empty()) {
      if (isRootServer()) {
        SharedPtr<FederationSynchronizedMessage> response;
        response = new FederationSynchronizedMessage;
        response->setFederationHandle(getFederationHandle());
        response->setLabel(message->getLabel());
        response->getFederateHandleBoolPairVector().reserve(i->_achievedFederateSyncronizationMap.size());
        FederateHandleVector federateHandleVector;
        for (ServerModel::SynchronizationFederate::HandleMap::iterator j = i->_achievedFederateSyncronizationMap.begin();
             j != i->_achievedFederateSyncronizationMap.end(); ++j) {
          federateHandleVector.push_back(j->getFederateHandle());
          response->getFederateHandleBoolPairVector().push_back(FederateHandleBoolPair(j->getFederateHandle(), j->getSuccessful()));
        }
        broadcastToChildren(federateHandleVector, response);
        ServerModel::Synchronization::NameMap::erase(*i);
      } else {
        SharedPtr<SynchronizationPointAchievedMessage> achieved;
        achieved = new SynchronizationPointAchievedMessage;
        achieved->setFederationHandle(getFederationHandle());
        achieved->setLabel(message->getLabel());
        achieved->getFederateHandleBoolPairVector().reserve(i->_achievedFederateSyncronizationMap.size());
        for (ServerModel::SynchronizationFederate::HandleMap::iterator j = i->_achievedFederateSyncronizationMap.begin();
             j != i->_achievedFederateSyncronizationMap.end(); ++j) {
          achieved->getFederateHandleBoolPairVector().push_back(FederateHandleBoolPair(j->getFederateHandle(), j->getSuccessful()));
        }
        sendToParent(achieved);
      }
    }
  }
  void accept(const ConnectHandle& connectHandle, const FederationSynchronizedMessage* message)
  {
    ServerModel::Synchronization::NameMap::iterator i = _synchronizationNameSynchronizationMap.find(message->getLabel());
    if (i == _synchronizationNameSynchronizationMap.end())
      throw MessageError("FederateSynchronizedMessage for unknown label.");

    // Distribute the synchronized messages across the appropriate connects
    // First collect the federate handle sets by connect ...
    std::map<ConnectHandle, FederateHandleBoolPairVector> connectHandleFederateHandleVectorMap;
    for (FederateHandleBoolPairVector::const_iterator j = message->getFederateHandleBoolPairVector().begin();
         j != message->getFederateHandleBoolPairVector().end(); ++j) {
      ServerModel::Federate* federate = getFederate(j->first);
      if (!federate)
        continue;
      ServerModel::FederationConnect* federationConnect = federate->getFederationConnect();
      if (!federationConnect)
        continue;
      if (federationConnect->getIsParentConnect())
        continue;
      ConnectHandle federateConnectHandle = federationConnect->getConnectHandle();
      if (!federateConnectHandle.valid())
        continue;
      FederateHandleBoolPairVector& federateHandleBoolPairVector = connectHandleFederateHandleVectorMap[federateConnectHandle];
      if (federateHandleBoolPairVector.empty())
        federateHandleBoolPairVector.reserve(message->getFederateHandleBoolPairVector().size());
      federateHandleBoolPairVector.push_back(*j);
    }

    // ... then send them out throught the connect.
    for (std::map<ConnectHandle, FederateHandleBoolPairVector>::iterator j = connectHandleFederateHandleVectorMap.begin();
         j != connectHandleFederateHandleVectorMap.end(); ++j) {
      SharedPtr<FederationSynchronizedMessage> synchronized;
      synchronized = new FederationSynchronizedMessage;
      synchronized->setFederationHandle(getFederationHandle());
      synchronized->setLabel(message->getLabel());
      synchronized->getFederateHandleBoolPairVector().swap(j->second);
      send(j->first, synchronized);
    }

    ServerModel::Synchronization::NameMap::erase(*i);
  }

  // Time management
  void accept(const ConnectHandle& connectHandle, const EnableTimeRegulationRequestMessage* message)
  {
    ServerModel::Federate* federate = getFederate(message->getFederateHandle());
    if (!federate)
      throw MessageError("EnableTimeRegulationRequestMessage from unknown Federate!");

    ServerModel::FederationConnect* federationConnect = getFederationConnect(connectHandle);
    OpenRTIAssert(federationConnect);
    // OpenRTIAssert(federationConnect->getConnectHandle() == connectHandle);

    // A correctly programmed ambassador already denies the enable request.
    // So this is an error terminating the connection if somebody asks for that if it should not do so.
    if (!federationConnect->getPermitTimeRegulation())
      throw MessageError("EnableTimeRegulationRequestMessage from unauthorized connect!");

    if (isRootServer() || federationConnect->getIsParentConnect()) {
      // Note that this message really loops back to the requestor.
      // The requestor needs to know which federates he needs to wait for.
      // see an explanation in the ambassador's time management code

      if (federate->getIsTimeRegulating())
        throw MessageError("EnableTimeRegulationRequestMessage for already time regulaitng federate!");
      insertTimeRegulating(*federate);
      federate->setTimeAdvanceTimeStamp(message->getTimeStamp());
      federate->setNextMessageTimeStamp(message->getTimeStamp());
      federate->setCommitId(message->getCommitId());
      broadcastToChildren(message);
    } else {
      sendToParent(message);
      // FIXME:
      // If we really want to work well with a parent vanishing in between, we need to
      // store the outstanding requests here and on loosing connection to a parent complete
      // the role as root server ...
    }
  }
  void accept(const ConnectHandle&, const EnableTimeRegulationResponseMessage* message)
  {
    send(message->getFederateHandle(), message);
  }
  void accept(const ConnectHandle& connectHandle, const DisableTimeRegulationRequestMessage* message)
  {
    ServerModel::Federate* federate = getFederate(message->getFederateHandle());
    if (!federate)
      throw MessageError("DisableTimeRegulationRequestMessage from unknown Federate!");
    if (!federate->getIsTimeRegulating())
      throw MessageError("DisableTimeRegulationRequestMessage for non time regulating Federate!");
    // Don't bail out on anything. If the federate dies in between, we might need to clean up somehow
    broadcast(connectHandle, message);
    eraseTimeRegulating(*federate);
  }
  void accept(const ConnectHandle& connectHandle, const CommitLowerBoundTimeStampMessage* message)
  {
    ServerModel::Federate* federate = getFederate(message->getFederateHandle());
    if (!federate)
      throw MessageError("Received CommitLowerBoundTimeStampMessage from unknown Federate!");
    if (!federate->getIsTimeRegulating())
      throw MessageError("Received CommitLowerBoundTimeStampMessage for non time regulating Federate!");

    switch (message->getCommitType()) {
    case TimeAdvanceCommit:
    case TimeAdvanceAndNextMessageCommit:
      federate->setTimeAdvanceTimeStamp(message->getTimeStamp());
      federate->setCommitId(message->getCommitId());
      break;
    case NextMessageCommit:
      break;
    }
    switch (message->getCommitType()) {
    case TimeAdvanceAndNextMessageCommit:
    case NextMessageCommit:
      federate->setNextMessageTimeStamp(message->getTimeStamp());
      federate->setCommitId(message->getCommitId());
      break;
    case TimeAdvanceCommit:
      break;
    }

    // send to all time constrainted connects except to where it originates
    // Hmm, send to all federates. The problem is that non time constrained federates
    // must be able to query the GALT for itself, which is only possible if they know the time advances
    // of each regulating federate, thus just broadcast
    broadcast(connectHandle, message);
  }
  void accept(const ConnectHandle& connectHandle, const CommitLowerBoundTimeStampResponseMessage* message)
  {
    send(message->getFederateHandle(), message);
  }
  void accept(const ConnectHandle& connectHandle, const LockedByNextMessageRequestMessage* message)
  {
    // Only time regulating federates are interrested in this message.
    // May be we should at one point track and store this connect handle set.
    for (ServerModel::FederationConnect::SecondList::iterator i = getTimeRegulatingFederationConnectList().begin();
         i != getTimeRegulatingFederationConnectList().end(); ++i) {
      if (connectHandle == i->getConnectHandle())
        continue;
      i->send(message);
    }
  }


  // Regions
  void accept(const ConnectHandle& connectHandle, const InsertRegionMessage* message)
  {
    for (RegionHandleDimensionHandleSetPairVector::const_iterator i = message->getRegionHandleDimensionHandleSetPairVector().begin();
         i != message->getRegionHandleDimensionHandleSetPairVector().end(); ++i) {
      ServerModel::Region* region = getOrCreateRegion(i->first);
      if (!region)
        throw MessageError("InsertegionMessage for unknown Federate!");
      region->_dimensionHandleSet = i->second;
    }

    broadcast(connectHandle, message);
  }
  void accept(const ConnectHandle& connectHandle, const CommitRegionMessage* message)
  {
    for (RegionHandleRegionValuePairVector::const_iterator i = message->getRegionHandleRegionValuePairVector().begin();
         i != message->getRegionHandleRegionValuePairVector().end(); ++i) {
      ServerModel::Region* region = getOrCreateRegion(i->first);
      if (!region)
        throw MessageError("CommitRegionMessage for unknown Region!");
      region->_regionValue = i->second;
    }

    broadcast(connectHandle, message);
  }
  void accept(const ConnectHandle& connectHandle, const EraseRegionMessage* message)
  {
    for (RegionHandleVector::const_iterator i = message->getRegionHandleVector().begin();
         i != message->getRegionHandleVector().end(); ++i) {
      ServerModel::Region* region = getRegion(*i);
      if (!region)
        throw MessageError("EraseRegionMessage for unknown Region!");
      delete region;
    }

    broadcast(connectHandle, message);
  }

  // (un)publish messages for interactions
  void accept(const ConnectHandle& connectHandle, const ChangeInteractionClassPublicationMessage* message)
  {
    ServerModel::InteractionClass* interactionClass = getInteractionClass(message->getInteractionClassHandle());
    if (!interactionClass)
      return;
    // Change publication type for this connect ...
    ServerModel::PropagationTypeConnectHandlePair propagationConnectPair;
    propagationConnectPair = interactionClass->setPublicationType(connectHandle, message->getPublicationType());
    // ... and propagate further if required.
    switch (propagationConnectPair.first) {
    case ServerModel::PropagateBroadcast:
      broadcast(connectHandle, message);
      break;
    case ServerModel::PropagateSend:
      send(propagationConnectPair.second, message);
      break;
    case ServerModel::PropagateNone:
      break;
    }

    // See if and how we should respond to that publication
    SubscriptionType subscriptionType = interactionClass->getSubscriptionTypeToConnect(connectHandle);
    if (subscriptionType != Unsubscribed) {
      SharedPtr<ChangeInteractionClassSubscriptionMessage> subscription;
      subscription = new ChangeInteractionClassSubscriptionMessage;
      subscription->setFederationHandle(getFederationHandle());
      subscription->setInteractionClassHandle(interactionClass->getInteractionClassHandle());
      if (message->getPublicationType() == Published) {
        subscription->setSubscriptionType(subscriptionType);
      } else {
        subscription->setSubscriptionType(Unsubscribed);
      }
      send(connectHandle, subscription);
    }
  }
  void accept(const ConnectHandle& connectHandle, const ChangeObjectClassPublicationMessage* message)
  {
    ServerModel::ObjectClass* objectClass = ServerModel::Federation::getObjectClass(message->getObjectClassHandle());
    if (!objectClass)
      return;

    // Build up the lists where to propagate what
    AttributeHandleVector broadcastAttributeHandles;
    std::map<ConnectHandle, AttributeHandleVector> sendAttributeHandlesMap;
    AttributeHandleVector passiveSubscribeAttributeHandles;
    AttributeHandleVector activeSubscribeAttributeHandles;
    for (AttributeHandleVector::const_iterator i = message->getAttributeHandles().begin();
         i != message->getAttributeHandles().end(); ++i) {
      ServerModel::ClassAttribute* attribute = objectClass->getClassAttribute(*i);
      if (!attribute)
        continue;
      ServerModel::PropagationTypeConnectHandlePair propagationConnectPair;
      propagationConnectPair = attribute->setPublicationType(connectHandle, message->getPublicationType());
      switch (propagationConnectPair.first) {
      case ServerModel::PropagateBroadcast:
        broadcastAttributeHandles.push_back(*i);
        break;
      case ServerModel::PropagateSend:
        sendAttributeHandlesMap[propagationConnectPair.second].push_back(*i);
        break;
      case ServerModel::PropagateNone:
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
      subscription->setFederationHandle(getFederationHandle());
      subscription->setObjectClassHandle(objectClass->getObjectClassHandle());
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
      subscription->setFederationHandle(getFederationHandle());
      subscription->setObjectClassHandle(objectClass->getObjectClassHandle());
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
    for (ServerModel::InteractionClass::HandleMap::iterator j = getInteractionClassHandleInteractionClassMap().begin();
         j != getInteractionClassHandleInteractionClassMap().end(); ++j) {
      if (j->getPublicationType(connectHandle) == Unpublished)
        continue;
      SharedPtr<ChangeInteractionClassPublicationMessage> message = new ChangeInteractionClassPublicationMessage;
      message->setFederationHandle(getFederationHandle());
      message->setPublicationType(Unpublished);
      message->setInteractionClassHandle(j->getInteractionClassHandle());
      accept(connectHandle, message.get());
    }
    for (ServerModel::ObjectClass::HandleMap::iterator j = getObjectClassHandleObjectClassMap().begin();
         j != getObjectClassHandleObjectClassMap().end(); ++j) {
      AttributeHandleVector attributeHandleVector;
      for (ServerModel::ClassAttribute::HandleMap::iterator k = j->getAttributeHandleClassAttributeMap().begin();
           k != j->getAttributeHandleClassAttributeMap().end(); ++k) {
        if (k->getPublicationType(connectHandle) == Unpublished)
          continue;
        attributeHandleVector.reserve(j->getAttributeHandleClassAttributeMap().size());
        attributeHandleVector.push_back(k->getAttributeHandle());
      }
      if (attributeHandleVector.empty())
        continue;
      SharedPtr<ChangeObjectClassPublicationMessage> message = new ChangeObjectClassPublicationMessage;
      message->setFederationHandle(getFederationHandle());
      message->setPublicationType(Unpublished);
      message->setObjectClassHandle(j->getObjectClassHandle());
      message->getAttributeHandles().swap(attributeHandleVector);
      accept(connectHandle, message.get());
    }
  }


  // (un)subscription messages for interactions
  void accept(const ConnectHandle& connectHandle, const ChangeInteractionClassSubscriptionMessage* message)
  {
    ServerModel::InteractionClass* interactionClass = getInteractionClass(message->getInteractionClassHandle());
    if (!interactionClass)
      throw MessageError("ChangeInteractionClassSubscriptionMessage for unknown InteractionClass!");
    // Change publication type for this connect ...
    ServerModel::PropagationTypeConnectHandlePair propagationConnectPair;
    propagationConnectPair = interactionClass->setSubscriptionType(connectHandle, message->getSubscriptionType());
    // Update the receiving connect handle set
    interactionClass->updateCumulativeSubscription(connectHandle);
    // ... and propagate further if required.
    switch (propagationConnectPair.first) {
    case ServerModel::PropagateBroadcast:
      send(interactionClass->getPublishingConnectHandleSet(), connectHandle, message);
      break;
    case ServerModel::PropagateSend:
      send(propagationConnectPair.second, message);
      break;
    case ServerModel::PropagateNone:
      break;
    }
  }

  // (un)subscription messages for object classes
  void accept(const ConnectHandle& connectHandle, const ChangeObjectClassSubscriptionMessage* message)
  {
    ServerModel::ObjectClass* objectClass = ServerModel::Federation::getObjectClass(message->getObjectClassHandle());
    if (!objectClass)
      return;

    ServerModel::FederationConnect* federationConnect = getFederationConnect(connectHandle);
    OpenRTIAssert(federationConnect);

    ServerModel::ObjectClass::ObjectInstanceList objectInstanceList;
    std::map<ConnectHandle, AttributeHandleVector> sendAttributeHandlesMap;
    for (std::vector<AttributeHandle>::const_iterator i = message->getAttributeHandles().begin();
         i != message->getAttributeHandles().end(); ++i) {
      ServerModel::ClassAttribute* attribute = objectClass->getClassAttribute(*i);
      if (!attribute)
        continue;
      ServerModel::PropagationTypeConnectHandlePair propagationConnectPair;
      propagationConnectPair = attribute->setSubscriptionType(connectHandle, message->getSubscriptionType());
      switch (propagationConnectPair.first) {
      case ServerModel::PropagateNone:
        break;
      case ServerModel::PropagateBroadcast:
        for (ConnectHandleSet::const_iterator j = attribute->getPublishingConnectHandleSet().begin();
             j != attribute->getPublishingConnectHandleSet().end(); ++j) {
          if (*j == connectHandle)
            continue;
          sendAttributeHandlesMap[*j].push_back(*i);
        }
        break;
      case ServerModel::PropagateSend:
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
      for (ServerModel::ObjectClass::ObjectInstanceList::iterator j = objectInstanceList.begin(); j != objectInstanceList.end(); ++j) {
        SharedPtr<InsertObjectInstanceMessage> request = new InsertObjectInstanceMessage;
        request->setFederationHandle(getFederationHandle());
        request->setObjectInstanceHandle((*j)->getObjectInstanceHandle());
        request->setObjectClassHandle((*j)->getObjectClass()->getObjectClassHandle());
        request->setName((*j)->getName());
        request->getAttributeStateVector().reserve((*j)->getAttributeHandleInstanceAttributeMap().size());
        for (ServerModel::InstanceAttribute::HandleMap::iterator k = (*j)->getAttributeHandleInstanceAttributeMap().begin();
             k != (*j)->getAttributeHandleInstanceAttributeMap().end(); ++k) {
          AttributeState attributeState;
          attributeState.setAttributeHandle(k->getAttributeHandle());
          request->getAttributeStateVector().push_back(attributeState);
        }
        if (!isParentConnect(connectHandle)) {
          (*j)->reference(*federationConnect);
        }
        federationConnect->send(request);
      }
    }
  }
  void unsubscribeConnect(const ConnectHandle& connectHandle)
  {
    // Unsubscribe this connect by fake unsubscribe messages?!!
    for (ServerModel::InteractionClass::HandleMap::iterator j = getInteractionClassHandleInteractionClassMap().begin();
         j != getInteractionClassHandleInteractionClassMap().end(); ++j) {
      if (j->getSubscriptionType(connectHandle) == Unsubscribed)
        continue;
      SharedPtr<ChangeInteractionClassSubscriptionMessage> message = new ChangeInteractionClassSubscriptionMessage;
      message->setFederationHandle(getFederationHandle());
      message->setSubscriptionType(Unsubscribed);
      message->setInteractionClassHandle(j->getInteractionClassHandle());
      accept(connectHandle, message.get());
    }
    for (ServerModel::ObjectClass::HandleMap::iterator j = getObjectClassHandleObjectClassMap().begin();
         j != getObjectClassHandleObjectClassMap().end(); ++j) {
      AttributeHandleVector attributeHandleVector;
      for (ServerModel::ClassAttribute::HandleMap::iterator k = j->getAttributeHandleClassAttributeMap().begin();
           k != j->getAttributeHandleClassAttributeMap().end(); ++k) {
        if (k->getSubscriptionType(connectHandle) == Unsubscribed)
          continue;
        attributeHandleVector.reserve(j->getAttributeHandleClassAttributeMap().size());
        attributeHandleVector.push_back(k->getAttributeHandle());
      }
      if (attributeHandleVector.empty())
        continue;
      SharedPtr<ChangeObjectClassSubscriptionMessage> message = new ChangeObjectClassSubscriptionMessage;
      message->setFederationHandle(getFederationHandle());
      message->setSubscriptionType(Unsubscribed);
      message->setObjectClassHandle(j->getObjectClassHandle());
      message->getAttributeHandles().swap(attributeHandleVector);
      accept(connectHandle, message.get());
    }
  }

  // ObjectInstance handle management,
  // These are maintained at the root server. Clients can request handles from the root server
  // the root server then sends the requested amount of unused object instance handles to the client.
  // The reponse is sent to the requesting federate and each server node on the way registers a
  // refrence of the receiving connect handle to this object instance handle.
  // An ambassador requests a bunch of handles at join time. Then, on object creation,
  // the ambassador has very likely some free handles available. So in effect we even have the
  // object instance registration without any latency. With every new registered object, the
  // Ambassador also requests a new instance handle to keep the pool of available handles in about the
  // same size. Only if latency is high and object registration rate is high also, the ambassador might block
  // on the registerObjectInstance call until a new free handle arrives.
  void accept(const ConnectHandle& connectHandle, const ObjectInstanceHandlesRequestMessage* message)
  {
    if (isRootServer()) {
      ServerModel::FederationConnect* federationConnect = getFederationConnect(connectHandle);

      // Provide some object instance handles to a federate.
      // This is to be completely asyncronous in the registerObjectInstance call.
      SharedPtr<ObjectInstanceHandlesResponseMessage> response;
      response = new ObjectInstanceHandlesResponseMessage;
      response->setFederationHandle(message->getFederationHandle());
      FederateHandle federateHandle = message->getFederateHandle();
      response->setFederateHandle(federateHandle);
      unsigned count = message->getCount();
      response->getObjectInstanceHandleNamePairVector().reserve(count);
      while (count--) {
        ServerModel::ObjectInstance* objectInstance = insertObjectInstance(ObjectInstanceHandle(), std::string());
        objectInstance->reference(*federationConnect);
        response->getObjectInstanceHandleNamePairVector().push_back(ObjectInstanceHandleNamePair(objectInstance->getObjectInstanceHandle(), objectInstance->getName()));
      }
      federationConnect->send(response);
    } else {
      /// FIXME in case of a dying parent, we need to reevaluate this too
      sendToParent(message);
    }
  }
  void accept(const ConnectHandle& connectHandle, const ObjectInstanceHandlesResponseMessage* message)
  {
    FederateHandle federateHandle = message->getFederateHandle();
    ServerModel::Federate* federate = getFederate(federateHandle);
    if (!federate)
      throw MessageError("Got ObjectInstanceHandlesResponseMessage for an unknown federate!");

    ServerModel::FederationConnect* federationConnect = federate->getFederationConnect();
    if (!federationConnect || federate->getResignPending()) {
      // Can happen, may be it has resigned/is died in between but the response is already underway
      // If so, then just ignore, the upstream server needs to release them
    } else {
      for (ObjectInstanceHandleNamePairVector::const_iterator k = message->getObjectInstanceHandleNamePairVector().begin();
           k != message->getObjectInstanceHandleNamePairVector().end(); ++k) {
        ServerModel::ObjectInstance* objectInstance = insertObjectInstance(k->first, k->second);
        objectInstance->reference(*federationConnect);
      }

      federationConnect->send(message);
    }
  }
  void accept(const ConnectHandle& connectHandle, const ReleaseMultipleObjectInstanceNameHandlePairsMessage* message)
  {
    SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> releaseMessage;
    for (ObjectInstanceHandleVector::const_iterator i = message->getObjectInstanceHandleVector().begin();
         i != message->getObjectInstanceHandleVector().end(); ++i) {
      ServerModel::ObjectInstance* objectInstance = getObjectInstance(*i);
      if (!objectInstance)
        throw MessageError("Got ReleaseMultipleObjectInstanceNameHandlePairsMessage for an unknown object instance!");
      for (ServerModel::InstanceAttribute::HandleMap::iterator j = objectInstance->getAttributeHandleInstanceAttributeMap().begin();
           j != objectInstance->getAttributeHandleInstanceAttributeMap().end(); ++j) {
        j->removeConnect(connectHandle);
      }
      if (!objectInstance->unreference(connectHandle))
        continue;
      erase(*objectInstance);
      if (isRootServer())
        continue;
      if (!releaseMessage.valid()) {
        releaseMessage = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
        releaseMessage->setFederationHandle(getFederationHandle());
        releaseMessage->getObjectInstanceHandleVector().reserve(message->getObjectInstanceHandleVector().size());
      }
      releaseMessage->getObjectInstanceHandleVector().push_back(*i);
    }
    if (releaseMessage.valid())
      sendToParent(releaseMessage);
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
  void accept(const ConnectHandle& connectHandle, const ReserveObjectInstanceNameRequestMessage* message)
  {
    FederateHandle federateHandle = message->getFederateHandle();
    ServerModel::Federate* federate = getFederate(federateHandle);
    if (!federate)
      throw MessageError("Got ReserveObjectInstanceNameRequestMessage for an unknown federate!");
    // names starting with HLA are reserved for the RTI, a correct programmed ambassador does not request these
    if (message->getName().compare(0, 3, "HLA") == 0)
      throw MessageError("Got ReserveObjectInstanceNameRequestMessage with name starting with HLA.");

    if (isRootServer()) {
      ServerModel::FederationConnect* federationConnect = federate->getFederationConnect();
      OpenRTIAssert(federationConnect);

      SharedPtr<ReserveObjectInstanceNameResponseMessage> response;
      response = new ReserveObjectInstanceNameResponseMessage;
      response->setFederationHandle(getFederationHandle());
      response->setFederateHandle(federateHandle);
      if (!isObjectInstanceNameInUse(message->getName())) {
        ServerModel::ObjectInstance* objectInstance = insertObjectInstance(ObjectInstanceHandle(), message->getName());
        objectInstance->reference(*federationConnect);
        response->setObjectInstanceHandleNamePair(ObjectInstanceHandleNamePair(objectInstance->getObjectInstanceHandle(), objectInstance->getName()));
        response->setSuccess(true);
      } else {
        ObjectInstanceHandleNamePair objectInstanceHandleNamePair(ObjectInstanceHandle(), message->getName());
        response->setObjectInstanceHandleNamePair(objectInstanceHandleNamePair);
        response->setSuccess(false);
      }
      federationConnect->send(response);
    } else {
      /// FIXME in case of a dying parent, we need to reevaluate this too
      sendToParent(message);
    }
  }
  void accept(const ConnectHandle& connectHandle, const ReserveObjectInstanceNameResponseMessage* message)
  {
    FederateHandle federateHandle = message->getFederateHandle();
    ServerModel::Federate* federate = getFederate(federateHandle);
    if (!federate)
      throw MessageError("Got ReserveObjectInstanceNameResponseMessage for an unknown federate!");

    ServerModel::FederationConnect* federateConnect = federate->getFederationConnect();
    if (!federateConnect || federate->getResignPending()) {
      // Can happen, may be it has resigned/is died in between but the response is already underway
      // If so, then release the reservations.
    } else {
      if (message->getSuccess()) {
        ServerModel::ObjectInstance* objectInstance = insertObjectInstance(message->getObjectInstanceHandleNamePair().first,
                                                                           message->getObjectInstanceHandleNamePair().second);
        objectInstance->reference(*federateConnect);
      }
      federateConnect->send(message);
    }
  }
  void accept(const ConnectHandle& connectHandle, const ReserveMultipleObjectInstanceNameRequestMessage* message)
  {
    FederateHandle federateHandle = message->getFederateHandle();
    ServerModel::Federate* federate = getFederate(federateHandle);
    if (!federate)
      throw MessageError("Got ReserveMultipleObjectInstanceNameRequestMessage for an unknown federate!");
    // names starting with HLA are reserved for the RTI, a correct programmed ambassador does not request these
    for (StringVector::const_iterator i = message->getNameList().begin(); i != message->getNameList().end(); ++i) {
      if (i->compare(0, 3, "HLA") == 0)
        throw MessageError("ReserveMultipleObjectInstanceNameRequestMessage with name starting with HLA.");
    }

    if (isRootServer()) {
      ServerModel::FederationConnect* federationConnect = federate->getFederationConnect();
      OpenRTIAssert(federationConnect);

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
        if (!isObjectInstanceNameInUse(*i))
          continue;
        response->setSuccess(false);
      }
      // If none of them is reserved or in use, reserve them all
      if (response->getSuccess()) {
        for (ObjectInstanceHandleNamePairVector::iterator j = response->getObjectInstanceHandleNamePairVector().begin();
             j != response->getObjectInstanceHandleNamePairVector().end(); ++j) {
          ServerModel::ObjectInstance* objectInstance = insertObjectInstance(ObjectInstanceHandle(), j->second);
          objectInstance->reference(*federationConnect);
          j->first = objectInstance->getObjectInstanceHandle();
        }
      }
      federationConnect->send(response);
    } else {
      /// FIXME in case of a dying parent, we need to reevaluate this too
      sendToParent(message);
    }
  }
  void accept(const ConnectHandle& connectHandle, const ReserveMultipleObjectInstanceNameResponseMessage* message)
  {
    FederateHandle federateHandle = message->getFederateHandle();
    ServerModel::Federate* federate = getFederate(federateHandle);
    if (!federate)
      throw MessageError("Got ReserveMultipleObjectInstanceNameResponseMessage for an unknown federate!");

    ServerModel::FederationConnect* federateConnect = federate->getFederationConnect();
    if (!federateConnect || federate->getResignPending()) {
      // Can happen, may be it has resigned/is died in between but the response is already underway
      // If so, then release the reservations.
    } else {
      if (message->getSuccess()) {
        for (ObjectInstanceHandleNamePairVector::const_iterator k = message->getObjectInstanceHandleNamePairVector().begin();
             k != message->getObjectInstanceHandleNamePairVector().end(); ++k) {
          ServerModel::ObjectInstance* objectInstance = insertObjectInstance(k->first, k->second);
          objectInstance->reference(*federateConnect);
        }
      }

      federateConnect->send(message);
    }
  }


  // Object instance messages
  void accept(const ConnectHandle& connectHandle, const InsertObjectInstanceMessage* message)
  {
    ObjectInstanceHandle objectInstanceHandle = message->getObjectInstanceHandle();
    ObjectClassHandle objectClassHandle = message->getObjectClassHandle();
    ServerModel::ObjectClass* objectClass = ServerModel::Federation::getObjectClass(objectClassHandle);
    if (!objectClass)
      throw MessageError("InsertObjectInstanceMessage for unknown ObjectClass.");

    // FIXME Improove this with preevaluated sets:
    // std::map<ConnectHandle,ConnectHandleSet> ...
    ServerModel::ObjectInstance* objectInstance = getObjectInstance(objectInstanceHandle);
    ServerModel::ClassAttribute* privilegeToDeleteAttribute = objectClass->getPrivilegeToDeleteClassAttribute();
    if (privilegeToDeleteAttribute) {
      for (ConnectHandleSet::iterator j = privilegeToDeleteAttribute->_cumulativeSubscribedConnectHandleSet.begin();
           j != privilegeToDeleteAttribute->_cumulativeSubscribedConnectHandleSet.end(); ++j) {
        if (isParentConnect(*j))
          continue;
        if (*j == connectHandle)
          continue;
        if (!objectInstance)
          objectInstance = insertObjectInstance(objectInstanceHandle, message->getName());
        ServerModel::FederationConnect* federationConnect = getFederationConnect(*j);
        if (!federationConnect)
          continue;
        objectInstance->reference(*federationConnect);
      }
    }

    // If still unreferenced, ignore the insert and unref again in the parent
    // this can happen if we subscribed and unsubscribed at the server before we received the insert that is triggered by the subscribe request.
    if (!objectInstance) {
      OpenRTIAssert(isParentConnect(connectHandle));

      SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> message;
      message = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
      message->setFederationHandle(getFederationHandle());
      message->getObjectInstanceHandleVector().push_back(objectInstanceHandle);

      sendToParent(message);

    } else {
      OpenRTIAssert(!objectInstance->getConnectHandleObjectInstanceConnectMap().empty());

      objectInstance->setObjectClass(objectClass);
      for (size_t j = 0; j < message->getAttributeStateVector().size(); ++j) {
        ServerModel::InstanceAttribute* instanceAttribute = objectInstance->getInstanceAttribute(message->getAttributeStateVector()[j].getAttributeHandle());
        instanceAttribute->setOwnerConnectHandle(connectHandle);
      }

      send(objectInstance->getPrivilegeToDeleteInstanceAttribute()->_receivingConnects, message);
    }
  }
  void accept(const ConnectHandle& connectHandle, const DeleteObjectInstanceMessage* message)
  {
    // If the object class is already unsubscribed, we might still get delete instance or update messages
    // That are sent by the owner at a time the subscription was still there.
    // So This is not an error. FIXME: if we do explicit instance removal in parent to child order we can
    // make that am error again.
    ServerModel::ObjectInstance* objectInstance = getObjectInstance(message->getObjectInstanceHandle());
    if (!objectInstance)
      return;

    // send that to all servers that have seen that object instance at some time
    OpenRTIAssert(objectInstance->getPrivilegeToDeleteInstanceAttribute()->_receivingConnects.count(connectHandle) == 0);
    send(objectInstance->getPrivilegeToDeleteInstanceAttribute()->_receivingConnects, message);
  }
  void accept(const ConnectHandle& connectHandle, const TimeStampedDeleteObjectInstanceMessage* message)
  {
    // If the object class is already unsubscribed, we might still get delete instance or update messages
    // That are sent by the owner at a time the subscription was still there.
    // So This is not an error. FIXME: if we do explicit instance removal in parent to child order we can
    // make that am error again.
    ServerModel::ObjectInstance* objectInstance = getObjectInstance(message->getObjectInstanceHandle());
    if (!objectInstance)
      return;

    // send that to all servers that have seen that object instance at some time
    OpenRTIAssert(objectInstance->getPrivilegeToDeleteInstanceAttribute()->_receivingConnects.count(connectHandle) == 0);
    send(objectInstance->getPrivilegeToDeleteInstanceAttribute()->_receivingConnects, message);
  }

  void accept(const ConnectHandle& connectHandle, const AttributeUpdateMessage* message)
  {
    ServerModel::ObjectInstance* objectInstance = getObjectInstance(message->getObjectInstanceHandle());
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
      ServerModel::InstanceAttribute* instanceAttribute = objectInstance->getInstanceAttribute(i->getAttributeHandle());
      if (!instanceAttribute)
        continue;
      for (ConnectHandleSet::const_iterator j = instanceAttribute->_receivingConnects.begin();
           j != instanceAttribute->_receivingConnects.end(); ++j) {
        connectHandleAttributeValueVectorMap[*j].reserve(message->getAttributeValues().size());
        connectHandleAttributeValueVectorMap[*j].push_back(*i);
      }
    }

    for (ConnectHandleAttributeValueVectorMap::iterator i = connectHandleAttributeValueVectorMap.begin();
          i != connectHandleAttributeValueVectorMap.end(); ++i) {
      SharedPtr<AttributeUpdateMessage> update = new AttributeUpdateMessage;
      update->setFederationHandle(getFederationHandle());
      update->setFederateHandle(message->getFederateHandle());
      update->setObjectInstanceHandle(message->getObjectInstanceHandle());
      update->setTag(message->getTag());
      update->setTransportationType(message->getTransportationType());
      update->getAttributeValues().swap(i->second);
      send(i->first, update);
    }
  }
  void accept(const ConnectHandle& connectHandle, const TimeStampedAttributeUpdateMessage* message)
  {
    ServerModel::ObjectInstance* objectInstance = getObjectInstance(message->getObjectInstanceHandle());
    if (!objectInstance)
      return;

    // See the above improovements for the AttributeUpdateMessage FIXME

    typedef std::map<ConnectHandle, AttributeValueVector> ConnectHandleAttributeValueVectorMap;
    ConnectHandleAttributeValueVectorMap connectHandleAttributeValueVectorMap;
    for (AttributeValueVector::const_iterator i = message->getAttributeValues().begin();
         i != message->getAttributeValues().end(); ++i) {
      ServerModel::InstanceAttribute* instanceAttribute = objectInstance->getInstanceAttribute(i->getAttributeHandle());
      if (!instanceAttribute)
        continue;
      for (ConnectHandleSet::const_iterator j = instanceAttribute->_receivingConnects.begin();
           j != instanceAttribute->_receivingConnects.end(); ++j) {
        connectHandleAttributeValueVectorMap[*j].reserve(message->getAttributeValues().size());
        connectHandleAttributeValueVectorMap[*j].push_back(*i);
      }
    }

    for (ConnectHandleAttributeValueVectorMap::iterator i = connectHandleAttributeValueVectorMap.begin();
          i != connectHandleAttributeValueVectorMap.end(); ++i) {
      SharedPtr<TimeStampedAttributeUpdateMessage> update = new TimeStampedAttributeUpdateMessage;
      update->setFederationHandle(getFederationHandle());
      update->setFederateHandle(message->getFederateHandle());
      update->setObjectInstanceHandle(message->getObjectInstanceHandle());
      update->setTag(message->getTag());
      update->setTimeStamp(message->getTimeStamp());
      update->setMessageRetractionHandle(message->getMessageRetractionHandle());
      update->setOrderType(message->getOrderType());
      update->setTransportationType(message->getTransportationType());
      update->getAttributeValues().swap(i->second);
      send(i->first, update);
    }
  }


  // Send interactions due to the noted rounting tables
  void accept(const ConnectHandle& connectHandle, const InteractionMessage* message)
  {
    ServerModel::InteractionClass* interactionClass = getInteractionClass(message->getInteractionClassHandle());
    if (!interactionClass)
      throw MessageError("Received InteractionMessage for unknown interaction class!");
    // Send to all subscribed connects except the originating one
    for (ConnectHandleSet::const_iterator i = interactionClass->_cumulativeSubscribedConnectHandleSet.begin();
         i != interactionClass->_cumulativeSubscribedConnectHandleSet.end(); ++i) {
      if (*i == connectHandle)
        continue;
      ServerModel::InteractionClass* currentInteractionClass = interactionClass;
      while (currentInteractionClass) {
        if (currentInteractionClass->getSubscriptionType(*i) != Unsubscribed) {
          if (currentInteractionClass == interactionClass) {
            send(*i, message);
          } else {
            SharedPtr<InteractionMessage> message2 = new InteractionMessage;
            message2->setFederationHandle(message->getFederationHandle());
            message2->setFederateHandle(message->getFederateHandle());
            message2->setTransportationType(message->getTransportationType());
            message2->setTag(message->getTag());
            message2->getParameterValues().reserve(message->getParameterValues().size());
            for (ParameterValueVector::const_iterator j = message->getParameterValues().begin();
                 j != message->getParameterValues().end(); ++j) {
              if (!currentInteractionClass->getClassParameter(j->getParameterHandle()))
                continue;
              message2->getParameterValues().push_back(*j);
            }
            message2->setInteractionClassHandle(currentInteractionClass->getInteractionClassHandle());
            send(*i, message2);
          }
          break;
        }
        currentInteractionClass = currentInteractionClass->getParentInteractionClass();
      }
    }
  }
  void accept(const ConnectHandle& connectHandle, const TimeStampedInteractionMessage* message)
  {
    ServerModel::InteractionClass* interactionClass = getInteractionClass(message->getInteractionClassHandle());
    if (!interactionClass)
      throw MessageError("Received TimeStampedInteractionMessage for unknown interaction class!");
    // Send to all subscribed connects except the originating one
    for (ConnectHandleSet::const_iterator i = interactionClass->_cumulativeSubscribedConnectHandleSet.begin();
         i != interactionClass->_cumulativeSubscribedConnectHandleSet.end(); ++i) {
      if (*i == connectHandle)
        continue;
      ServerModel::InteractionClass* currentInteractionClass = interactionClass;
      while (currentInteractionClass) {
        if (currentInteractionClass->getSubscriptionType(*i) != Unsubscribed) {
          if (currentInteractionClass == interactionClass) {
            send(*i, message);
          } else {
            SharedPtr<TimeStampedInteractionMessage> message2 = new TimeStampedInteractionMessage;
            message2->setFederationHandle(message->getFederationHandle());
            message2->setFederateHandle(message->getFederateHandle());
            message2->setOrderType(message->getOrderType());
            message2->setTransportationType(message->getTransportationType());
            message2->setTag(message->getTag());
            message2->setTimeStamp(message->getTimeStamp());
            message2->setMessageRetractionHandle(message->getMessageRetractionHandle());
            message2->getParameterValues().reserve(message->getParameterValues().size());
            for (ParameterValueVector::const_iterator j = message->getParameterValues().begin();
                 j != message->getParameterValues().end(); ++j) {
              if (!currentInteractionClass->getClassParameter(j->getParameterHandle()))
                continue;
              message2->getParameterValues().push_back(*j);
            }
            message2->setInteractionClassHandle(currentInteractionClass->getInteractionClassHandle());
            send(*i, message2);
          }
          break;
        }
        currentInteractionClass = currentInteractionClass->getParentInteractionClass();
      }
    }
  }

  void accept(const ConnectHandle& connectHandle, const RequestAttributeUpdateMessage* message)
  {
    typedef std::map<ConnectHandle, SharedPtr<RequestAttributeUpdateMessage> > ConnectMessageMap;

    ObjectInstanceHandle objectInstanceHandle = message->getObjectInstanceHandle();
    ServerModel::ObjectInstance* objectInstance = getObjectInstance(objectInstanceHandle);
    if (!objectInstance)
      return;

    // Find the server connects that own the attributes and build up a message for those
    ConnectMessageMap connectMessageMap;
    for (AttributeHandleVector::const_iterator i = message->getAttributeHandles().begin();
         i != message->getAttributeHandles().end(); ++i) {
      ServerModel::InstanceAttribute* instanceAttribute = objectInstance->getInstanceAttribute(*i);
      if (!instanceAttribute) // FIXME this is more an error ...
        continue;
      // The connect handle that owns this attribute
      ConnectHandle connectHandle = instanceAttribute->getOwnerConnectHandle();
      if (!connectHandle.valid())
        continue;
      ConnectMessageMap::iterator k = connectMessageMap.find(connectHandle);
      if (k == connectMessageMap.end()) {
        k = connectMessageMap.insert(ConnectMessageMap::value_type(connectHandle, new RequestAttributeUpdateMessage)).first;
        k->second->setFederationHandle(getFederationHandle());
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
  void accept(const ConnectHandle& connectHandle, const RequestClassAttributeUpdateMessage* message)
  {
    ObjectClassHandle objectClassHandle = message->getObjectClassHandle();
    ServerModel::ObjectClass* objectClass = ServerModel::Federation::getObjectClass(objectClassHandle);
    if (!objectClass)
      throw MessageError("Received RequestClassAttributeUpdateMessage for unknown object class!");

    ConnectHandleSet connectHandleSet;
    objectClass->accumulateAllPublications(connectHandleSet);
    send(connectHandleSet, connectHandle, message);
  }

  template<typename M>
  void acceptFederationMessage(const ConnectHandle& connectHandle, const M* message)
  {
    OpenRTIAssert(getFederationConnect(connectHandle));
    accept(connectHandle, message);
  }


  ServerModel::FederationConnect* getOrInsertConnect(ServerModel::NodeConnect& nodeConnect)
  {
    OpenRTIAssert(nodeConnect.getConnectHandle().valid());
    ServerModel::FederationConnect* federationConnect = getFederationConnect(nodeConnect.getConnectHandle());
    if (federationConnect)
      return federationConnect;

    federationConnect = new ServerModel::FederationConnect(*this, nodeConnect);
    ServerModel::Federation::insert(*federationConnect);
    nodeConnect.insert(*federationConnect);
    if (nodeConnect.getIsParentConnect())
      federationConnect->setActive(true);
    return federationConnect;
  }

  void pushFederation(const ConnectHandle& connectHandle)
  {
    ServerModel::FederationConnect* federationConnect = getFederationConnect(connectHandle);
    OpenRTIAssert(federationConnect);

    if (federationConnect->getActive())
      return;
    federationConnect->setActive(true);

    OpenRTIAssert(!federationConnect->getIsParentConnect());

    bool permitTimeRegulation = _parentPermitTimeRegulation && getServerNode().getServerOptions().getPermitTimeRegulation();
    federationConnect->setPermitTimeRegulation(permitTimeRegulation);

    SharedPtr<InsertFederationExecutionMessage> message = new InsertFederationExecutionMessage;
    message->setFederationHandle(getFederationHandle());
    message->setFederationName(getName());
    message->setLogicalTimeFactoryName(getLogicalTimeFactoryName());
    if (!federationConnect->getPermitTimeRegulation())
      message->getConfigurationParameterMap()["permitTimeRegulation"].push_back("false");
    // FIXME add the server options
    federationConnect->send(message);

    SharedPtr<InsertModulesMessage> insertModules;
    insertModules = new InsertModulesMessage;
    insertModules->setFederationHandle(getFederationHandle());
    getModuleList(insertModules->getFOMModuleList());
    federationConnect->send(insertModules);

    /// FIXME currently these are all flushed when an EraseFederationExecutionMessage is received.
    /// FIXME Make that more explicit????
    for (ServerModel::Federate::HandleMap::const_iterator i = getFederateHandleFederateMap().begin();
         i != getFederateHandleFederateMap().end(); ++i) {
      if (i->getConnectHandle() == connectHandle)
        continue;
      SharedPtr<JoinFederateNotifyMessage> notify = new JoinFederateNotifyMessage;
      notify->setFederationHandle(getFederationHandle());
      notify->setFederateHandle(i->getFederateHandle());
      notify->setFederateType(i->getFederateType());
      notify->setFederateName(i->getName());
      federationConnect->send(notify);
    }

    for (ServerModel::FederationConnect::SecondList::iterator i = getTimeRegulatingFederationConnectList().begin();
         i != getTimeRegulatingFederationConnectList().end(); ++i) {
      for (ServerModel::Federate::SecondList::iterator j = i->getTimeRegulatingFederateList().begin();
           j != i->getTimeRegulatingFederateList().end(); ++j) {
        SharedPtr<EnableTimeRegulationRequestMessage> enable = new EnableTimeRegulationRequestMessage;
        enable->setFederationHandle(getFederationHandle());
        enable->setFederateHandle(j->getFederateHandle());
        enable->setTimeStamp(j->getTimeAdvanceTimeStamp());
        enable->setCommitId(j->getCommitId());
        federationConnect->send(enable);

        SharedPtr<CommitLowerBoundTimeStampMessage> commit = new CommitLowerBoundTimeStampMessage;
        commit->setFederationHandle(getFederationHandle());
        commit->setFederateHandle(j->getFederateHandle());
        commit->setTimeStamp(j->getNextMessageTimeStamp());
        commit->setCommitType(NextMessageCommit);
        commit->setCommitId(j->getCommitId());
        federationConnect->send(commit);
      }
    }

    pushPublications(federationConnect->getConnectHandle());

    for (ServerModel::Federate::HandleMap::iterator i = getFederateHandleFederateMap().begin();
         i != getFederateHandleFederateMap().end(); ++i) {
      if (i->getConnectHandle() == connectHandle)
        continue;
      if (i->getRegionHandleRegionMap().empty())
        continue;
      std::size_t count = i->getRegionHandleRegionMap().size();
      SharedPtr<InsertRegionMessage> insertRegionMessage;
      insertRegionMessage = new InsertRegionMessage;
      insertRegionMessage->setFederationHandle(getFederationHandle());
      insertRegionMessage->getRegionHandleDimensionHandleSetPairVector().reserve(count);
      SharedPtr<CommitRegionMessage> commitRegionMessage;
      commitRegionMessage = new CommitRegionMessage;
      commitRegionMessage->setFederationHandle(getFederationHandle());
      commitRegionMessage->getRegionHandleRegionValuePairVector().reserve(count);

      for (ServerModel::Region::HandleMap::iterator j = i->getRegionHandleRegionMap().begin();
           j != i->getRegionHandleRegionMap().end(); ++j) {
        RegionHandle regionHandle(i->getFederateHandle(), j->getRegionHandle());
        insertRegionMessage->getRegionHandleDimensionHandleSetPairVector().push_back(RegionHandleDimensionHandleSetPair(regionHandle, j->_dimensionHandleSet));
        commitRegionMessage->getRegionHandleRegionValuePairVector().push_back(RegionHandleRegionValuePair(regionHandle, j->_regionValue));
      }
      federationConnect->send(insertRegionMessage);
      federationConnect->send(commitRegionMessage);
    }
  }


  // Should be called when a connection dies
  void removeConnect(const ConnectHandle& connectHandle)
  {
    ServerModel::FederationConnect* federationConnect = getFederationConnect(connectHandle);
    if (!federationConnect)
      return;

    resignConnect(connectHandle);
    ServerModel::Federation::removeConnect(connectHandle);
    eraseConnect(connectHandle);
  }

  // Should be called when a connection dies
  void resignConnect(const ConnectHandle& connectHandle)
  {
    ServerModel::FederationConnect* connect = getFederationConnect(connectHandle);
    OpenRTIAssert(connect);

    // Unsubscribe this connect
    unsubscribeConnect(connectHandle);

    // FIXME avoid this loop over all objects
    // FIXME avoid precollecting these in the object handle set
    ObjectInstanceHandleSet objectInstanceHandleSet;
    for (ServerModel::ObjectInstance::HandleMap::iterator i = getObjectInstanceHandleObjectInstanceMap().begin();
         i != getObjectInstanceHandleObjectInstanceMap().end(); ++i) {
      if (i->getOwnerConnectHandle() != connectHandle)
        continue;

      // FIXME: currently we do not have ownership management - so, if the owner dies the object needs to die too
      // bool deleteObjects = resignAction == DELETE_OBJECTS ||
      //   resignAction == DELETE_OBJECTS_THEN_DIVEST || resignAction == CANCEL_THEN_DELETE_THEN_DIVEST;
      bool deleteObject = true;

      if (deleteObject) {
        objectInstanceHandleSet.insert(i->getObjectInstanceHandle());
      } else {
        i->setOwnerConnectHandle(ConnectHandle());
      }
    }
    for (ObjectInstanceHandleSet::iterator j = objectInstanceHandleSet.begin();
         j != objectInstanceHandleSet.end(); ++j) {
      SharedPtr<DeleteObjectInstanceMessage> request;
      request = new DeleteObjectInstanceMessage;
      request->setFederationHandle(getFederationHandle());
      request->setObjectInstanceHandle(*j);
      accept(connectHandle, request.get());
    }

    // Release all object references
    if (!isParentConnect(connectHandle)) {
      SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> releaseMessage;
      for (ServerModel::ObjectInstanceConnect::FirstList::iterator j = connect->getObjectInstanceConnectList().begin();
           j != connect->getObjectInstanceConnectList().end();) {
        // This is unreferencing the object instance.
        // If nobody else references finally release the object instance handle.
        ServerModel::ObjectInstance& objectInstance = j->getObjectInstance();
        for (ServerModel::InstanceAttribute::HandleMap::iterator k = objectInstance.getAttributeHandleInstanceAttributeMap().begin();
             k != objectInstance.getAttributeHandleInstanceAttributeMap().end(); ++k) {
          k->removeConnect(connectHandle);
        }
        j = connect->getObjectInstanceConnectList().erase(j);
        if (!objectInstance.getConnectHandleObjectInstanceConnectMap().empty())
          continue;
        ObjectInstanceHandle objectInstanceHandle = objectInstance.getObjectInstanceHandle();
        erase(objectInstance);
        if (isRootServer())
          continue;
        if (!releaseMessage.valid()) {
          releaseMessage = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
          releaseMessage->setFederationHandle(getFederationHandle());
          releaseMessage->getObjectInstanceHandleVector().reserve(getObjectInstanceHandleObjectInstanceMap().size());
        }
        releaseMessage->getObjectInstanceHandleVector().push_back(objectInstanceHandle);
      }
      if (releaseMessage.valid())
        sendToParent(releaseMessage);
      OpenRTIAssert(connect->getObjectInstanceConnectList().empty());
    }

    // Unpublish this connect
    unpublishConnect(connectHandle);

    ServerModel::FederationConnect* federationConnect =getFederationConnect(connectHandle);
    if (federationConnect) {
      for (ServerModel::Federate::FirstList::iterator j = federationConnect->getFederateList().begin();
           j != federationConnect->getFederateList().end();) {
        ServerModel::Federate* federate = (j++).get();
        OpenRTIAssert(federate);

        // Release region references
        if (!federate->getRegionHandleRegionMap().empty()) {
          SharedPtr<EraseRegionMessage> eraseRegionMessage = new EraseRegionMessage;
          eraseRegionMessage->setFederationHandle(getFederationHandle());
          eraseRegionMessage->getRegionHandleVector().reserve(federate->getRegionHandleRegionMap().size());
          for (ServerModel::Region::HandleMap::iterator k = federate->getRegionHandleRegionMap().begin();
               k != federate->getRegionHandleRegionMap().end(); ++k) {
            RegionHandle regionHandle(federate->getFederateHandle(), k->getRegionHandle());
            eraseRegionMessage->getRegionHandleVector().push_back(regionHandle);
          }
          accept(connectHandle, eraseRegionMessage.get());
        }

        // Remove from time management, needs to happen before the federation connect is removed
        if (federate->getIsTimeRegulating()) {
          eraseTimeRegulating(*federate);
          SharedPtr<DisableTimeRegulationRequestMessage> request = new DisableTimeRegulationRequestMessage;
          request->setFederationHandle(getFederationHandle());
          request->setFederateHandle(federate->getFederateHandle());
          broadcast(connectHandle, request);
        }

        for (ServerModel::SynchronizationFederate::FirstList::iterator k = federate->getSynchronizationFederateList().begin();
             k != federate->getSynchronizationFederateList().end();) {
          if (!k->getSynchronization().getIsWaitingFor(federate->getFederateHandle()))
            ++k;
          else {
            SharedPtr<SynchronizationPointAchievedMessage> achieved;
            achieved = new SynchronizationPointAchievedMessage;
            achieved->setFederationHandle(getFederationHandle());
            achieved->setLabel(k->getSynchronization().getLabel());
            achieved->getFederateHandleBoolPairVector().push_back(FederateHandleBoolPair(federate->getFederateHandle(), false));
            ++k;
            accept(connectHandle, achieved.get());
          }
        }

        // Remove from connects
        federationConnect->erase(*federate);
        Log(ServerFederate, Info) << getServerPath() << ": Resigning federate " << federate->getFederateHandle()
                                  << " because of closed connection!" << std::endl;
        SharedPtr<ResignFederationExecutionRequestMessage> message = new ResignFederationExecutionRequestMessage;
        message->setFederationHandle(getFederationHandle());
        message->setFederateHandle(federate->getFederateHandle());
        accept(connectHandle, message.get());
      }
      OpenRTIAssert(federationConnect->getFederateList().empty());
    }

    // FIXME if the removed connection is the parent and we have a resign pending, respond as if we were the root
  }

  /// Erase a connect from the object model.
  /// Precondition is that the connect is already idle
  void eraseConnect(const ConnectHandle& connectHandle)
  {
    ServerModel::FederationConnect* federationConnect = getFederationConnect(connectHandle);
    OpenRTIAssert(federationConnect);
    OpenRTIAssert(federationConnect->getIsParentConnect() || !federationConnect->getHasFederates());

    // Finally remove what is referencing the old connect handle
    // ServerModel::Federation::erase(*federationConnect);
    delete federationConnect;
  }

  ServerModel::Federate* insertFederate(const ConnectHandle& connectHandle, const std::string& federateName,
                                        const FederateHandle& federateHandle)
  {
    // Either we allocate a new federate, then the connect must still be alive
    // or we insert an already died federate and just keep everything in order for a currect the resing request sequence
    OpenRTIAssert(!isFederateNameInUse(federateName));
    OpenRTIAssert(!getFederate(federateHandle));

    // Register that we reach this federate through this connect
    ServerModel::Federate* federate = new ServerModel::Federate(*this);
    federate->setName(federateName);
    federate->setFederateHandle(federateHandle);
    ServerModel::Federation::insert(*federate);

    ServerModel::FederationConnect* federationConnect = getFederationConnect(connectHandle);
    if (federationConnect) {
      federationConnect->insert(*federate);
    } else {
      federate->setResignPending(true);
    }

    return federate;
  }

  void eraseFederate(ServerModel::Federate& federate)
  {
    // The time management stuff
    if (federate.getIsTimeRegulating())
      eraseTimeRegulating(federate);

    // Remove from connects
    ServerModel::FederationConnect* federationConnect = federate.getFederationConnect();
    if (federationConnect)
      federationConnect->erase(federate);

    ServerModel::Federation::erase(federate);
  }

  using ServerModel::Federation::broadcastToChildren;
  void broadcastToChildren(const FederateHandleVector& federateHandleVector, const SharedPtr<const AbstractMessage>& message)
  {
    ConnectHandleSet broadcastSet;
    for (FederateHandleVector::const_iterator i = federateHandleVector.begin(); i != federateHandleVector.end(); ++i) {
      ServerModel::Federate* federate = getFederate(*i);
      if (!federate)
        continue;
      ServerModel::FederationConnect* federationConnect = federate->getFederationConnect();
      if (!federationConnect)
        continue;
      if (federationConnect->getIsParentConnect())
        continue;
      ConnectHandle connectHandle = federationConnect->getConnectHandle();
      if (!connectHandle.valid())
        continue;
      broadcastSet.insert(connectHandle);
    }
    send(broadcastSet, message);
  }

  using ServerModel::Federation::send;
  // send to all in the set except the additionally given one
  void send(const ConnectHandleSet& connectHandleSet, const ConnectHandle& connectHandle, const SharedPtr<const AbstractMessage>& message)
  {
    // FIXME is currently O(n*log(n)) can be O(n)
    for (ConnectHandleSet::const_iterator i = connectHandleSet.begin(); i != connectHandleSet.end(); ++i) {
      if (*i == connectHandle)
	continue;
      send(*i, message);
    }
  }

  void send(const ConnectHandleSet& connectHandleSet, const SharedPtr<const AbstractMessage>& message)
  {
    // FIXME is currently O(n*log(n)) can be O(n)
    for (ConnectHandleSet::const_iterator i = connectHandleSet.begin(); i != connectHandleSet.end(); ++i)
      send(*i, message);
  }

  /// The parents policy if we are allowed to get time regulating
  bool _parentPermitTimeRegulation;
};

class OPENRTI_LOCAL ServerMessageDispatcher : public ServerModel::Node {
public:
  /// We have some stateless upstream messages that get handled in the root server,
  /// We remember these and know where to send them back.
  template<typename M>
  void forwardUpstreamMessage(const ConnectHandle& connectHandle, const M* message)
  {
    OpenRTIAssert(connectHandle.valid());
    // Must be upstream here
    if (isParentConnect(connectHandle))
      throw MessageError(std::string("Received ")  + message->getTypeName() + " through the parent connect!");
    if (!isRootServer()) {
      // Remember the pending message ...
      _pendingMessageList.push_back(ConnectHandleMessagePair(connectHandle, message));
      // ... and go ask your father
      sendToParent(message);
    } else {
      // If we are the root server, process this message
      // Note that this is the callback that needs to be implemented for this method to be useful
      acceptAsRoot(connectHandle, message);
    }
  }
  template<typename M>
  void forwardDownstreamMessage(const ConnectHandle& connectHandle, const M* message)
  {
    OpenRTIAssert(connectHandle.valid());
    // Must be downstream here
    if (!isParentConnect(connectHandle))
      throw MessageError(std::string("Received ") + message->getTypeName() + " through a child connect!");
    // need to have a connect handle to resond to
    if (_pendingMessageList.empty())
      throw MessageError(std::string("No pending ") + message->getTypeName() + "!");
    // report downstream to the originator
    if (_pendingMessageList.front().first.valid())
      send(_pendingMessageList.front().first, message);
    // remove the pending message that succeeded
    _pendingMessageList.pop_front();
  }

  // If the parent connect dies, tell this all children
  void accept(const ConnectHandle& connectHandle, const ConnectionLostMessage* message)
  {
    // Throw away these kind of messages when they originate from a child.
    // This can happen since a connect just sends this message on socket problems.
    // But the socket connect does not know if it is a parent connect or not.
    if (!isParentConnect(connectHandle))
      return;
    broadcastToChildren(message);
  }

  // Create messages
  void acceptAsRoot(const ConnectHandle& connectHandle, const CreateFederationExecutionRequestMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    OpenRTIAssert(isRootServer());
    // Check if it is already there?
    if (getFederationExecutionAlreadyExists(message->getFederationExecution())) {
      // Ok, have an active federation, create needs to fail
      SharedPtr<CreateFederationExecutionResponseMessage> response;
      response = new CreateFederationExecutionResponseMessage;
      response->setCreateFederationExecutionResponseType(CreateFederationExecutionResponseFederationExecutionAlreadyExists);
      send(connectHandle, response);
    } else {
      // Successful create
      FederationServer* federationServer;
      federationServer = new FederationServer(*this);
      federationServer->setName(message->getFederationExecution());
      federationServer->setLogicalTimeFactoryName(message->getLogicalTimeFactoryName());
      try {
        federationServer->insert(message->getFOMStringModuleList());

        // register this one
        insert(*federationServer);

        Log(ServerFederation, Info) << getServerPath() << ": Create federation execution \""
                                    << message->getFederationExecution() << "\"." << std::endl;

        // ... and respond with Success
        SharedPtr<CreateFederationExecutionResponseMessage> response;
        response = new CreateFederationExecutionResponseMessage;
        response->setCreateFederationExecutionResponseType(CreateFederationExecutionResponseSuccess);
        send(connectHandle, response);

      } catch (const InconsistentFDD& e) {
        delete federationServer;

        Log(ServerFederation, Info) << getServerPath() << ": Caught Exception creating federation execution \""
                                    << e.getReason() << "\"." << std::endl;
        SharedPtr<CreateFederationExecutionResponseMessage> response;
        response = new CreateFederationExecutionResponseMessage;
        response->setCreateFederationExecutionResponseType(CreateFederationExecutionResponseInconsistentFDD);
        response->setExceptionString(e.getReason());
        send(connectHandle, response);

      } catch (const Exception& e) {
        Log(ServerFederation, Info) << getServerPath() << ": Caught Exception creating federation execution \""
                                    << e.getReason() << "\"." << std::endl;
        SharedPtr<CreateFederationExecutionResponseMessage> response;
        response = new CreateFederationExecutionResponseMessage;
        response->setCreateFederationExecutionResponseType(CreateFederationExecutionResponseRTIinternalError);
        response->setExceptionString(e.getReason());
        send(connectHandle, response);

      } catch (...) {
        delete federationServer;

        Log(ServerFederation, Info) << getServerPath() << ": Caught unknown Exception creating federation execution." << std::endl;
        SharedPtr<CreateFederationExecutionResponseMessage> response;
        response = new CreateFederationExecutionResponseMessage;
        response->setCreateFederationExecutionResponseType(CreateFederationExecutionResponseRTIinternalError);
        send(connectHandle, response);
      }
    }
  }
  void accept(const ConnectHandle& connectHandle, const CreateFederationExecutionRequestMessage* message)
  { forwardUpstreamMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const CreateFederationExecutionResponseMessage* message)
  { forwardDownstreamMessage(connectHandle, message); }

  // Destroy messages
  void acceptAsRoot(const ConnectHandle& connectHandle, const DestroyFederationExecutionRequestMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    OpenRTIAssert(isRootServer());

    // Check if it is there?
    FederationServer* federationServer = getFederation(message->getFederationExecution());
    if (!federationServer) {
      Log(ServerFederation, Debug) << getServerPath()
                                   << ": DestroyFederationExecutionRequestMessage faild for unknown federation named \""
                                   << message->getFederationExecution() << "\"!" << std::endl;
      // Ok, have an inactive federation, destroy needs to fail
      SharedPtr<DestroyFederationExecutionResponseMessage> response;
      response = new DestroyFederationExecutionResponseMessage;
      response->setDestroyFederationExecutionResponseType(DestroyFederationExecutionResponseFederationExecutionDoesNotExist);
      send(connectHandle, response);
    } else {

      // Federates currently joined?
      if (federationServer->hasJoinedFederates()) {
        Log(ServerFederation, Debug) << getServerPath() << ": DestroyFederationExecutionRequestMessage faild for \""
                                     << message->getFederationExecution() << "\", federates joined!" << std::endl;
        // federates there, so, no
        SharedPtr<DestroyFederationExecutionResponseMessage> response;
        response = new DestroyFederationExecutionResponseMessage;
        response->setDestroyFederationExecutionResponseType(DestroyFederationExecutionResponseFederatesCurrentlyJoined);
        send(connectHandle, response);
      } else {
        // Successful destroy
        Log(ServerFederation, Info) << getServerPath() << ": DestroyFederationExecutionRequestMessage succeeded for \""
                                    << message->getFederationExecution() << "\"!" << std::endl;

        if (federationServer->hasChildConnects()) {
          // ... we are not the lower most server node that still knows that federation,
          // so forward this to all children.
          federationServer->broadcastEraseFederationExecution();
          eraseName(*federationServer);
        } else {
          // ... we are the lower most server node that still knows about that federation,
          // so just throw away the federation handle.
          erase(*federationServer);
        }

        // ... and just respond with Success
        SharedPtr<DestroyFederationExecutionResponseMessage> response;
        response = new DestroyFederationExecutionResponseMessage;
        response->setDestroyFederationExecutionResponseType(DestroyFederationExecutionResponseSuccess);
        send(connectHandle, response);
      }
    }
  }
  void accept(const ConnectHandle& connectHandle, const DestroyFederationExecutionRequestMessage* message)
  { forwardUpstreamMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const DestroyFederationExecutionResponseMessage* message)
  { forwardDownstreamMessage(connectHandle, message); }


  // Enumerate federations
  void acceptAsRoot(const ConnectHandle& connectHandle, const EnumerateFederationExecutionsRequestMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    OpenRTIAssert(isRootServer());

    SharedPtr<EnumerateFederationExecutionsResponseMessage> response;
    response = new EnumerateFederationExecutionsResponseMessage;
    response->getFederationExecutionInformationVector().reserve(getFederationHandleFederationMap().size());
    for (ServerModel::Federation::HandleMap::iterator i = getFederationHandleFederationMap().begin();
         i != getFederationHandleFederationMap().end(); ++i) {
      FederationExecutionInformation federationExecutionInformation;
      federationExecutionInformation.setFederationExecutionName(i->getName());
      federationExecutionInformation.setLogicalTimeFactoryName(i->getLogicalTimeFactoryName());
      response->getFederationExecutionInformationVector().push_back(federationExecutionInformation);
    }
    send(connectHandle, response);
  }
  void accept(const ConnectHandle& connectHandle, const EnumerateFederationExecutionsRequestMessage* message)
  { forwardUpstreamMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const EnumerateFederationExecutionsResponseMessage* message)
  { forwardDownstreamMessage(connectHandle, message); }


  // Insert a new federation into the server node
  void accept(const ConnectHandle& connectHandle, const InsertFederationExecutionMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    if (!isParentConnect(connectHandle))
      throw MessageError("Received InsertFederationExecutionMessage through a child connect!");
    if (getFederation(message->getFederationName()))
      throw MessageError("Received InsertFederationExecutionMessage for an already existing federation!");
    FederationHandle federationHandle = message->getFederationHandle();
    if (!federationHandle.valid())
      throw MessageError("Received InsertFederationExecutionMessage with invalid federation handle!");
    FederationServer* federationServer = getFederation(federationHandle);
    // FIXME: revisit, how to handle this
    // if (federationServer)
    //   throw MessageError("Received InsertFederationExecutionMessage for an already existing federation!");
    if (federationServer) {
      // reinsert the already existing datastructure to get the index by name back
      insertName(*federationServer);
      return;
    }

    federationServer = insertFederation(message->getFederationName(), federationHandle);
    federationServer->setParentConfigurationParameterMap(message->getConfigurationParameterMap());
    federationServer->setLogicalTimeFactoryName(message->getLogicalTimeFactoryName());
    // FIXME add the server options
  }
  // A child server or an ambassador sends this request to be removed from the federation execution.
  void accept(const ConnectHandle& connectHandle, const ShutdownFederationExecutionMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    // Such a response must originate from the parent.
    if (isParentConnect(connectHandle))
      throw MessageError("Received ShutdownFederationExecutionMessage through a parent connect!");

    FederationHandle federationHandle = message->getFederationHandle();
    if (!federationHandle.valid())
      throw MessageError("Received ShutdownFederationExecutionMessage with invalid federation handle!");
    FederationServer* federationServer = getFederation(federationHandle);
    if (!federationServer)
      throw MessageError("Received ShutdownFederationExecutionMessage for a non existing federation!");
    ServerModel::FederationConnect* federationConnect = federationServer->getFederationConnect(connectHandle);
    if (!federationConnect)
      throw MessageError("Received ShutdownFederationExecutionMessage for a federation not knowing this connect!");

    // Check if the federation has again gained a new federate in between
    if (federationConnect->getHasFederates())
      return;
    // Can happen that we ask for shutdown that is already underway
    if (!federationConnect->getActive())
      return;

    federationServer->eraseFederationExecutionAtConnect(connectHandle);
  }
  // Erase a new federation from the server node
  void accept(const ConnectHandle& connectHandle, const EraseFederationExecutionMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    // Such a response must originate from the parent.
    if (!isParentConnect(connectHandle))
      throw MessageError("Received EraseFederationExecutionMessage through a child connect!");

    FederationHandle federationHandle = message->getFederationHandle();
    if (!federationHandle.valid())
      throw MessageError("Received EraseFederationExecutionMessage with invalid federation handle!");
    // FIXME go back to iterators erase below is more efficient
    FederationServer* federationServer = getFederation(federationHandle);
    if (!federationServer)
      throw MessageError("Received EraseFederationExecutionMessage for a non existing federation!");

    /// FIXME we should not hit these two cases at all.
    /// We should only send this if a connect gets idle by having no Federates in there.
    /// IF so, send the EraseFederationExecutionMessage which flushes the federation.
    /// This nicely serializes with pushing a new one.
    /// This must not be confused with resign!!!
    /// Do the same for modules and other stuff which we can just judge about at the parent.

    // Two cases ...
    if (federationServer->hasChildConnects()) {
      // ... we are not the lower most server node that still knows that federation,
      // so forward this to all children.
      federationServer->broadcastEraseFederationExecution();
      eraseName(*federationServer);
    } else {
      // ... we are the lower most server node that still knows about that federation,
      // so respond with releasing the federation handle.
      SharedPtr<ReleaseFederationHandleMessage> response;
      response = new ReleaseFederationHandleMessage;
      response->setFederationHandle(federationHandle);
      sendToParent(response);
      erase(*federationServer);
    }
  }
  // Erase a federation handle from the server node, this is part of the two way shutdown with a child
  void accept(const ConnectHandle& connectHandle, const ReleaseFederationHandleMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    // Such a response must originate from the parent.
    if (isParentConnect(connectHandle))
      throw MessageError("Received ReleaseFederationHandleMessage through a parent connect!");

    FederationHandle federationHandle = message->getFederationHandle();
    if (!federationHandle.valid())
      throw MessageError("Received ReleaseFederationHandleMessage with invalid federation handle!");
    FederationServer* federationServer = getFederation(federationHandle);
    if (!federationServer)
      throw MessageError("Received ReleaseFederationHandleMessage for a non existing federation!");

    Log(ServerFederation, Info) << getServerPath() << ": ReleaseFederationHandleMessage for federation named \""
                                << federationServer->getName() << "\"!" << std::endl;

    // If we are still not the last one, wait until this happens
    // FIXME, move that into the erase connect in some way - may be with a new function
    // FIXME See if this is a real problem
    ServerModel::FederationConnect* federationConnect = federationServer->getFederationConnect(connectHandle);
    if (!federationConnect)
      return;
    if (federationConnect->getActive())
      return;

    federationServer->eraseConnect(connectHandle);
    if (federationServer->hasChildConnects())
      return;
    // Only release those federations that we were asked to release
    // FIXME better test for that ...
    if (getFederation(federationServer->getName()))
      return;

    if (isRootServer())
      return;
    sendToParent(message);
    erase(*federationServer);
  }

  // The Join messages
  void acceptAsRoot(const ConnectHandle& connectHandle, const JoinFederationExecutionRequestMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    OpenRTIAssert(isRootServer());

    // The ambassador already needs to care for that. So, if we get that here, drop the connection.
    if (message->getFederateName().compare(0, 3, "HLA") == 0)
      throw MessageError("Got JoinFederationExecutionRequestMessage with name starting with HLA.");

    FederationServer* federationServer = getFederation(message->getFederationExecution());
    if (!federationServer) {
      // FederationExecutionDoesNotExist ...
      Log(ServerFederate, Info) << getServerPath()
                                << ": JoinFederationExecutionRequestMessage faild for unknown federation named \""
                                << message->getFederationExecution() << "\"!" << std::endl;
      SharedPtr<JoinFederationExecutionResponseMessage> response;
      response = new JoinFederationExecutionResponseMessage;
      response->setJoinFederationExecutionResponseType(JoinFederationExecutionResponseFederationExecutionDoesNotExist);
      send(connectHandle, response);

    } else {
      // Process the join request - which still might fail
      federationServer->accept(connectHandle, message);
    }
  }
  void accept(const ConnectHandle& connectHandle, const JoinFederationExecutionRequestMessage* message)
  { forwardUpstreamMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const JoinFederationExecutionResponseMessage* message)
  {
    OpenRTIAssert(connectHandle.valid());
    // Such a response must originate from the parent.
    if (!isParentConnect(connectHandle))
      throw MessageError("Received JoinFederationExecutionResponseMessage through a child connect!");
    // We need to have something left here
    if (_pendingMessageList.empty())
      throw MessageError("No pending JoinFederationExecutionResponseMessage!");

    ConnectHandle requestConnectHandle = _pendingMessageList.front().first;
    if (message->getJoinFederationExecutionResponseType() == JoinFederationExecutionResponseSuccess) {
      // Check the messages content somehow
      if (!message->getFederateHandle().valid())
        throw MessageError("Received successful JoinFederationExecutionResponseMessage with an invalid federate handle!");
      if (!message->getFederationHandle().valid())
        throw MessageError("Received successful JoinFederationExecutionResponseMessage with an invalid federation handle!");
      FederationServer* federationServer = getFederation(message->getFederationHandle());
      if (!federationServer)
        throw MessageError("Received successful JoinFederationExecutionResponseMessage with an unknown federation handle!");

      if (requestConnectHandle.valid()) {
        ServerModel::NodeConnect* connect = getNodeConnect(requestConnectHandle);
        OpenRTIAssert(connect);
        federationServer->getOrInsertConnect(*connect);
        federationServer->pushFederation(connect->getConnectHandle());
      }
      federationServer->accept(connectHandle, requestConnectHandle, message);
    }

    forwardDownstreamMessage(connectHandle, message);

    if (message->getJoinFederationExecutionResponseType() == JoinFederationExecutionResponseSuccess
        && !requestConnectHandle.valid()) {
      // if the connect has died in between, resign now again
      SharedPtr<ResignFederationExecutionLeafRequestMessage> resign = new ResignFederationExecutionLeafRequestMessage;
      resign->setFederationHandle(message->getFederationHandle());
      resign->setFederateHandle(message->getFederateHandle());
      resign->setResignAction(CANCEL_THEN_DELETE_THEN_DIVEST);
      accept(connectHandle, resign.get());
    }
  }


  /// Messages that get handled in the Federation.
  template<typename M>
  void acceptFederationMessage(const ConnectHandle& connectHandle, const M* message)
  {
    OpenRTIAssert(connectHandle.valid());
    FederationServer* federationServer = getFederation(message->getFederationHandle());
    if (!federationServer)
      throw MessageError(getServerPath() + std::string(" received ") + message->getTypeName()
                        + " for unknown federation id: " + message->getFederationHandle().toString() + "!");
    federationServer->acceptFederationMessage(connectHandle, message);
  }
  template<typename M>
  void acceptUpstreamFederationMessage(const ConnectHandle& connectHandle, const M* message)
  {
    OpenRTIAssert(connectHandle.valid());
    if (isParentConnect(connectHandle))
      throw MessageError(std::string("Received ") + message->getTypeName() + " through the parent connect!");
    acceptFederationMessage(connectHandle, message);
  }
  template<typename M>
  void acceptDownstreamFederationMessage(const ConnectHandle& connectHandle, const M* message)
  {
    OpenRTIAssert(connectHandle.valid());
    if (!isParentConnect(connectHandle))
      throw MessageError(std::string("Received ") + message->getTypeName() + " through a child connect!");
    acceptFederationMessage(connectHandle, message);
  }

  void accept(const ConnectHandle& connectHandle, const ResignFederationExecutionRequestMessage* message)
  { acceptUpstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const ResignFederationExecutionLeafRequestMessage* message)
  {
    acceptUpstreamFederationMessage(connectHandle, message);

    SharedPtr<ShutdownFederationExecutionMessage> request = new ShutdownFederationExecutionMessage;
    request->setFederationHandle(message->getFederationHandle());
    accept(connectHandle, request.get());
  }

  // Insert a set of object modules into the federation
  void accept(const ConnectHandle& connectHandle, const InsertModulesMessage* message)
  { acceptDownstreamFederationMessage(connectHandle, message); }

  // Message to inform federates about newly joined federates
  void accept(const ConnectHandle& connectHandle, const JoinFederateNotifyMessage* message)
  { acceptDownstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const ResignFederateNotifyMessage* message)
  { acceptDownstreamFederationMessage(connectHandle, message); }
  // How to proceed on implicit resign
  void accept(const ConnectHandle& connectHandle, const ChangeAutomaticResignDirectiveMessage* message)
  { acceptFederationMessage(connectHandle, message); }

  // Synchronization labels
  void accept(const ConnectHandle& connectHandle, const RegisterFederationSynchronizationPointMessage* message)
  { acceptUpstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const RegisterFederationSynchronizationPointResponseMessage* message)
  { acceptDownstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const AnnounceSynchronizationPointMessage* message)
  { acceptDownstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const SynchronizationPointAchievedMessage* message)
  { acceptUpstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const FederationSynchronizedMessage* message)
  { acceptDownstreamFederationMessage(connectHandle, message); }

  // Time Management
  void accept(const ConnectHandle& connectHandle, const EnableTimeRegulationRequestMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const EnableTimeRegulationResponseMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const DisableTimeRegulationRequestMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const CommitLowerBoundTimeStampMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const CommitLowerBoundTimeStampResponseMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const LockedByNextMessageRequestMessage* message)
  { acceptFederationMessage(connectHandle, message); }

  // Regions
  void accept(const ConnectHandle& connectHandle, const InsertRegionMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const CommitRegionMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const EraseRegionMessage* message)
  { acceptFederationMessage(connectHandle, message); }

  // Publications
  void accept(const ConnectHandle& connectHandle, const ChangeInteractionClassPublicationMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const ChangeObjectClassPublicationMessage* message)
  { acceptFederationMessage(connectHandle, message); }

  // Subscriptions
  void accept(const ConnectHandle& connectHandle, const ChangeInteractionClassSubscriptionMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const ChangeObjectClassSubscriptionMessage* message)
  { acceptFederationMessage(connectHandle, message); }

  // ObjectInstance handle management
  void accept(const ConnectHandle& connectHandle, const ObjectInstanceHandlesRequestMessage* message)
  { acceptUpstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const ObjectInstanceHandlesResponseMessage* message)
  { acceptDownstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const ReleaseMultipleObjectInstanceNameHandlePairsMessage* message)
  { acceptUpstreamFederationMessage(connectHandle, message); }

  // ObjectInstance name management
  void accept(const ConnectHandle& connectHandle, const ReserveObjectInstanceNameRequestMessage* message)
  { acceptUpstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const ReserveObjectInstanceNameResponseMessage* message)
  { acceptDownstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const ReserveMultipleObjectInstanceNameRequestMessage* message)
  { acceptUpstreamFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const ReserveMultipleObjectInstanceNameResponseMessage* message)
  { acceptDownstreamFederationMessage(connectHandle, message); }

  // Object instance messages
  void accept(const ConnectHandle& connectHandle, const InsertObjectInstanceMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const DeleteObjectInstanceMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const TimeStampedDeleteObjectInstanceMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const AttributeUpdateMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const TimeStampedAttributeUpdateMessage* message)
  { acceptFederationMessage(connectHandle, message); }

  // InteractionMessages
  void accept(const ConnectHandle& connectHandle, const InteractionMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const TimeStampedInteractionMessage* message)
  { acceptFederationMessage(connectHandle, message); }

  // AttributeUpdateRequest messages
  void accept(const ConnectHandle& connectHandle, const RequestAttributeUpdateMessage* message)
  { acceptFederationMessage(connectHandle, message); }
  void accept(const ConnectHandle& connectHandle, const RequestClassAttributeUpdateMessage* message)
  { acceptFederationMessage(connectHandle, message); }

  void accept(const ConnectHandle&, const AbstractMessage* message)
  { throw MessageError("Received unexpected message???"); }

  class OPENRTI_LOCAL DispatchFunctor {
  public:
    DispatchFunctor(ServerMessageDispatcher& serverMessageDispatcher, const ConnectHandle& connectHandle) :
      _serverMessageDispatcher(serverMessageDispatcher), _connectHandle(connectHandle)
    { }
    template<typename M>
    void operator()(const M& message) const
    { _serverMessageDispatcher.accept(_connectHandle, &message); }
  private:
    ServerMessageDispatcher& _serverMessageDispatcher;
    ConnectHandle _connectHandle;
  };

  void dispatch(const AbstractMessage& message, const ConnectHandle& connectHandle)
  {
    Log(ServerMessage, Debug3) << getServerPath() << ": Received " << message << "!" << std::endl;
    try {
      message.dispatchFunctor(DispatchFunctor(*this, connectHandle));
    } catch (MessageError& messageError) {
      Log(ServerFederation, Warning) << getServerPath() << ": " << messageError.what() << std::endl;
      throw;
    }
  }

  ConnectHandle insertConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& options)
  {
    ServerModel::NodeConnect* nodeConnect = Node::insertNodeConnect(messageSender, options);
    return nodeConnect->getConnectHandle();
  }
  ConnectHandle insertParentConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& options)
  {
    ServerModel::NodeConnect* nodeConnect = Node::insertParentNodeConnect(messageSender, options);
    getServerOptions().setParentOptionMap(options);
    return nodeConnect->getConnectHandle();
  }
  void removeConnect(const ConnectHandle& connectHandle)
  {
    bool isParent = isParentConnect(connectHandle);

    // Hmm, we need a server node shutdown mechanism instead of asking for isIdle here
    if (isParent && !isIdle())
      Log(ServerConnect, Error) << getServerPath() << ": Removing parent connect!" << std::endl;

    // Remove that from the federations.
    for (ServerModel::Federation::HandleMap::iterator i = getFederationHandleFederationMap().begin();
         i != getFederationHandleFederationMap().end(); ++i)
      static_cast<FederationServer*>(i.get())->removeConnect(connectHandle);

    // And remove it here
    Node::erase(connectHandle);

    if (isParent) {
      // Replay messages that we forwarded to the parent server to ourself.
      // We are the parent now ...
      for (ConnectHandleMessagePairList::iterator i = _pendingMessageList.begin();
           i != _pendingMessageList.end();) {
        if (i->first.valid())
          dispatch(*i->second, i->first);
        i = _pendingMessageList.erase(i);
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

private:
  FederationServer* getFederation(const std::string& federationName)
  {
    ServerModel::Federation* federation = ServerModel::Node::getFederation(federationName);
    if (!federation)
      return 0;
    return static_cast<FederationServer*>(federation);
  }
  FederationServer* getFederation(const FederationHandle& federationHandle)
  {
    ServerModel::Federation* federation = ServerModel::Node::getFederation(federationHandle);
    if (!federation)
      return 0;
    return static_cast<FederationServer*>(federation);
  }


  FederationServer* insertFederation(const std::string& name, const FederationHandle& federationHandle)
  {
    OpenRTIAssert(!isRootServer());
    OpenRTIAssert(federationHandle.valid());
    OpenRTIAssert(_federationNameFederationMap.find(name) == _federationNameFederationMap.end());
    OpenRTIAssert(_federationHandleFederationMap.find(federationHandle) == _federationHandleFederationMap.end());

    ServerModel::NodeConnect* connect = getNodeConnect(getParentConnectHandle());
    OpenRTIAssert(connect);

    FederationServer* federationServer = new FederationServer(*this);
    federationServer->setName(name);
    federationServer->setFederationHandle(federationHandle);
    federationServer->getOrInsertConnect(*connect);

    insert(*federationServer);

    return federationServer;
  }

  /// Messages that require a response from the root server.
  /// We store a list of these pending messages to
  /// 1. know where to respond.
  /// 2. to be able to replay these messages if the parent dies
  /// Note that the connect handles are invalidated once a connect dies while a response
  /// is pending. This way we can keep track of the responses even when some reqests get
  /// invalid before they completed.
  /// FIXME have also a list per connect so that only those are invaliated ...
  typedef std::pair<ConnectHandle, SharedPtr<const AbstractMessage> > ConnectHandleMessagePair;
  typedef std::list<ConnectHandleMessagePair> ConnectHandleMessagePairList;
  ConnectHandleMessagePairList _pendingMessageList;
};

ServerNode::ServerNode() :
  _serverMessageDispatcher(new ServerMessageDispatcher)
{
}

ServerNode::~ServerNode()
{
  delete _serverMessageDispatcher;
  _serverMessageDispatcher = 0;
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
ServerNode::isIdle() const
{
  return _serverMessageDispatcher->isIdle();
}

ServerOptions&
ServerNode::getServerOptions()
{
  return _serverMessageDispatcher->getServerOptions();
}

const ServerOptions&
ServerNode::getServerOptions() const
{
  return _serverMessageDispatcher->getServerOptions();
}

ConnectHandle
ServerNode::_insertConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& options)
{
  return _serverMessageDispatcher->insertConnect(messageSender, options);
}

ConnectHandle
ServerNode::_insertParentConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& options)
{
  return _serverMessageDispatcher->insertParentConnect(messageSender, options);
}

void
ServerNode::_eraseConnect(const ConnectHandle& connectHandle)
{
  _serverMessageDispatcher->removeConnect(connectHandle);
}

void
ServerNode::_dispatchMessage(const AbstractMessage* message, const ConnectHandle& connectHandle)
{
  _serverMessageDispatcher->dispatch(*message, connectHandle);
}

} // namespace OpenRTI
