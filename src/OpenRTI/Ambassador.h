/* -*-c++-*- OpenRTI - Copyright (C) 2009-2012 Mathias Froehlich
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

#ifndef OpenRTI_Ambassador_h
#define OpenRTI_Ambassador_h

#include <algorithm>
#include <sstream>
#include "AbstractConnect.h"
#include "AbstractFederate.h"
#include "Clock.h"
#include "Federate.h"
#include "LogStream.h"
#include "Message.h"
#include "MessageList.h"
#include "ProtocolRegistry.h"
#include "StringUtils.h"

namespace OpenRTI {

template<typename T>
class OPENRTI_LOCAL Ambassador {
public:
  typedef T Traits;

  // Times
  typedef typename Traits::NativeLogicalTime NativeLogicalTime;
  typedef typename Traits::NativeLogicalTimeInterval NativeLogicalTimeInterval;

  // The federate ambassador
  typedef typename Traits::FederateAmbassador FederateAmbassador;

  // The exceptions
  typedef typename Traits::AlreadyConnected AlreadyConnected;
  typedef typename Traits::AsynchronousDeliveryAlreadyDisabled AsynchronousDeliveryAlreadyDisabled;
  typedef typename Traits::AsynchronousDeliveryAlreadyEnabled AsynchronousDeliveryAlreadyEnabled;
  typedef typename Traits::AttributeAcquisitionWasNotRequested AttributeAcquisitionWasNotRequested;
  typedef typename Traits::AttributeAlreadyBeingAcquired AttributeAlreadyBeingAcquired;
  typedef typename Traits::AttributeAlreadyBeingDivested AttributeAlreadyBeingDivested;
  typedef typename Traits::AttributeAlreadyOwned AttributeAlreadyOwned;
  typedef typename Traits::AttributeDivestitureWasNotRequested AttributeDivestitureWasNotRequested;
  typedef typename Traits::AttributeNotDefined AttributeNotDefined;
  typedef typename Traits::AttributeNotOwned AttributeNotOwned;
  typedef typename Traits::AttributeNotPublished AttributeNotPublished;
  typedef typename Traits::AttributeRelevanceAdvisorySwitchIsOff AttributeRelevanceAdvisorySwitchIsOff;
  typedef typename Traits::AttributeRelevanceAdvisorySwitchIsOn AttributeRelevanceAdvisorySwitchIsOn;
  typedef typename Traits::AttributeScopeAdvisorySwitchIsOff AttributeScopeAdvisorySwitchIsOff;
  typedef typename Traits::AttributeScopeAdvisorySwitchIsOn AttributeScopeAdvisorySwitchIsOn;
  typedef typename Traits::CallNotAllowedFromWithinCallback CallNotAllowedFromWithinCallback;
  typedef typename Traits::ConnectionFailed ConnectionFailed;
  typedef typename Traits::CouldNotCreateLogicalTimeFactory CouldNotCreateLogicalTimeFactory;
  typedef typename Traits::CouldNotOpenFDD CouldNotOpenFDD;
  typedef typename Traits::CouldNotOpenMIM CouldNotOpenMIM;
  typedef typename Traits::DeletePrivilegeNotHeld DeletePrivilegeNotHeld;
  typedef typename Traits::ErrorReadingFDD ErrorReadingFDD;
  typedef typename Traits::ErrorReadingMIM ErrorReadingMIM;
  typedef typename Traits::FederateAlreadyExecutionMember FederateAlreadyExecutionMember;
  typedef typename Traits::FederateNameAlreadyInUse FederateNameAlreadyInUse;
  typedef typename Traits::FederateHasNotBegunSave FederateHasNotBegunSave;
  typedef typename Traits::FederateIsExecutionMember FederateIsExecutionMember;
  typedef typename Traits::FederateNotExecutionMember FederateNotExecutionMember;
  typedef typename Traits::FederateOwnsAttributes FederateOwnsAttributes;
  typedef typename Traits::FederatesCurrentlyJoined FederatesCurrentlyJoined;
  typedef typename Traits::FederateServiceInvocationsAreBeingReportedViaMOM FederateServiceInvocationsAreBeingReportedViaMOM;
  typedef typename Traits::FederateUnableToUseTime FederateUnableToUseTime;
  typedef typename Traits::FederationExecutionAlreadyExists FederationExecutionAlreadyExists;
  typedef typename Traits::FederationExecutionDoesNotExist FederationExecutionDoesNotExist;
  typedef typename Traits::IllegalName IllegalName;
  typedef typename Traits::InconsistentFDD InconsistentFDD;
  typedef typename Traits::InteractionClassNotDefined InteractionClassNotDefined;
  typedef typename Traits::InteractionClassNotPublished InteractionClassNotPublished;
  typedef typename Traits::InteractionParameterNotDefined InteractionParameterNotDefined;
  typedef typename Traits::InteractionRelevanceAdvisorySwitchIsOff InteractionRelevanceAdvisorySwitchIsOff;
  typedef typename Traits::InteractionRelevanceAdvisorySwitchIsOn InteractionRelevanceAdvisorySwitchIsOn;
  typedef typename Traits::InTimeAdvancingState InTimeAdvancingState;
  typedef typename Traits::InvalidAttributeHandle InvalidAttributeHandle;
  typedef typename Traits::InvalidDimensionHandle InvalidDimensionHandle;
  typedef typename Traits::InvalidFederateHandle InvalidFederateHandle;
  typedef typename Traits::InvalidInteractionClassHandle InvalidInteractionClassHandle;
  typedef typename Traits::InvalidLocalSettingsDesignator InvalidLocalSettingsDesignator;
  typedef typename Traits::InvalidLogicalTime InvalidLogicalTime;
  typedef typename Traits::InvalidLookahead InvalidLookahead;
  typedef typename Traits::InvalidObjectClassHandle InvalidObjectClassHandle;
  typedef typename Traits::InvalidOrderName InvalidOrderName;
  typedef typename Traits::InvalidOrderType InvalidOrderType;
  typedef typename Traits::InvalidParameterHandle InvalidParameterHandle;
  typedef typename Traits::InvalidRangeBound InvalidRangeBound;
  typedef typename Traits::InvalidRegion InvalidRegion;
  typedef typename Traits::InvalidRegionContext InvalidRegionContext;
  typedef typename Traits::InvalidResignAction InvalidResignAction;
  typedef typename Traits::InvalidRetractionHandle InvalidRetractionHandle;
  typedef typename Traits::InvalidServiceGroup InvalidServiceGroup;
  typedef typename Traits::InvalidTransportationName InvalidTransportationName;
  typedef typename Traits::InvalidTransportationType InvalidTransportationType;
  typedef typename Traits::LogicalTimeAlreadyPassed LogicalTimeAlreadyPassed;
  typedef typename Traits::MessageCanNoLongerBeRetracted MessageCanNoLongerBeRetracted;
  typedef typename Traits::NameNotFound NameNotFound;
  typedef typename Traits::NameSetWasEmpty NameSetWasEmpty;
  typedef typename Traits::NoAcquisitionPending NoAcquisitionPending;
  typedef typename Traits::NotConnected NotConnected;
  typedef typename Traits::ObjectClassNotDefined ObjectClassNotDefined;
  typedef typename Traits::ObjectClassNotPublished ObjectClassNotPublished;
  typedef typename Traits::ObjectClassRelevanceAdvisorySwitchIsOff ObjectClassRelevanceAdvisorySwitchIsOff;
  typedef typename Traits::ObjectClassRelevanceAdvisorySwitchIsOn ObjectClassRelevanceAdvisorySwitchIsOn;
  typedef typename Traits::ObjectInstanceNameInUse ObjectInstanceNameInUse;
  typedef typename Traits::ObjectInstanceNameNotReserved ObjectInstanceNameNotReserved;
  typedef typename Traits::ObjectInstanceNotKnown ObjectInstanceNotKnown;
  typedef typename Traits::OwnershipAcquisitionPending OwnershipAcquisitionPending;
  typedef typename Traits::RegionDoesNotContainSpecifiedDimension RegionDoesNotContainSpecifiedDimension;
  typedef typename Traits::RegionInUseForUpdateOrSubscription RegionInUseForUpdateOrSubscription;
  typedef typename Traits::RegionNotCreatedByThisFederate RegionNotCreatedByThisFederate;
  typedef typename Traits::RequestForTimeConstrainedPending RequestForTimeConstrainedPending;
  typedef typename Traits::RequestForTimeRegulationPending RequestForTimeRegulationPending;
  typedef typename Traits::RestoreInProgress RestoreInProgress;
  typedef typename Traits::RestoreNotRequested RestoreNotRequested;
  typedef typename Traits::RTIinternalError RTIinternalError;
  typedef typename Traits::SaveInProgress SaveInProgress;
  typedef typename Traits::SaveNotInitiated SaveNotInitiated;
  typedef typename Traits::SynchronizationPointLabelNotAnnounced SynchronizationPointLabelNotAnnounced;
  typedef typename Traits::TimeConstrainedAlreadyEnabled TimeConstrainedAlreadyEnabled;
  typedef typename Traits::TimeConstrainedIsNotEnabled TimeConstrainedIsNotEnabled;
  typedef typename Traits::TimeRegulationAlreadyEnabled TimeRegulationAlreadyEnabled;
  typedef typename Traits::TimeRegulationIsNotEnabled TimeRegulationIsNotEnabled;
  typedef typename Traits::UnsupportedCallbackModel UnsupportedCallbackModel;

  ~Ambassador()
  {
    // FIXME: rethink
    if (_federate.valid()) {
      try {
        resignFederationExecution(getAutomaticResignDirective());
      } catch (...) {
      }
    }
  }

  void connect(const StringMap& parameterMap, const Clock& abstime)
    throw (ConnectionFailed,
           AlreadyConnected,
           CallNotAllowedFromWithinCallback,
           RTIinternalError)
  {
    try {
      if (_connect.valid())
        Traits::throwAlreadyConnected();

      StringMap::const_iterator i = parameterMap.find("protocol");
      if (i == parameterMap.end())
        Traits::throwConnectionFailed("Cannot get protocol.");
      SharedPtr<const AbstractProtocol> protocol = ProtocolRegistry::instance()->getProtocol(i->second);
      if (!protocol.valid())
        Traits::throwConnectionFailed(std::string("Unknown or unsupported protocol \"") + i->second + std::string("\"."));

      // a send receive pair to create a federation
      // FIXME
      StringStringListMap stringStringListMap;
      for (StringMap::const_iterator j = parameterMap.begin(); j != parameterMap.end(); ++j)
        stringStringListMap[j->first].push_back(j->second);
      stringStringListMap["serverName"].push_back("ambassadorConnect");
      _connect = protocol->connect(stringStringListMap, abstime);
      if (!_connect.valid())
        Traits::throwConnectionFailed();

    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void disconnect()
    throw (FederateIsExecutionMember,
           CallNotAllowedFromWithinCallback,
           RTIinternalError)
  {
    try {
      if (_federate.valid())
        Traits::throwFederateIsExecutionMember();

      if (!_connect.valid())
        return;
      _connect = 0;

    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  bool isConnected() const
  { return _connect.valid(); }

  void createFederationExecution(const std::string& federationExecutionName,
                                 const FOMStringModuleList& fomModules,
                                 const std::string& logicalTimeFactoryName)
    throw (FederationExecutionAlreadyExists,
           // InconsistentFDD,
           CouldNotOpenFDD,
           ErrorReadingFDD,
           // DesignatorIsHLAstandardMIM,
           // ErrorReadingMIM,
           // CouldNotOpenMIM,
           CouldNotCreateLogicalTimeFactory,
           NotConnected,
           RTIinternalError)
  {
    try {
      if (!_connect.valid())
        Traits::throwNotConnected(std::string("Could not get connect RTI of federation execution \"") +
                                      federationExecutionName + std::string("\"."));

      // The create request message
      SharedPtr<CreateFederationExecutionRequestMessage> request = new CreateFederationExecutionRequestMessage;
      request->setFederationExecution(federationExecutionName);
      request->setLogicalTimeFactoryName(logicalTimeFactoryName);
      request->setFOMStringModuleList(fomModules);

      // The maximum abstime to try to connect
      Clock abstime = Clock::now() + Clock::fromSeconds(70);

      // Send this message and wait for the response
      _connect->send(request);
      SharedPtr<const AbstractMessage> response = _connect->receive(abstime);

      // Read the response and interpret that one
      if (!response.valid())
        Traits::throwRTIinternalError(std::string("Received no response message while talking to RTI of federation execution \"") +
                                      federationExecutionName + std::string("\"."));

      const CreateFederationExecutionResponseMessage* create = dynamic_cast<const CreateFederationExecutionResponseMessage*>(response.get());
      if (!create)
        Traits::throwRTIinternalError(std::string("Received unexpected message type \"") + response->getTypeName() + std::string("\" while talking to RTI of federation execution \"") +
                                      federationExecutionName + std::string("\"."));

      if (create->getCreateFederationExecutionResponseType() == CreateFederationExecutionResponseFederationExecutionAlreadyExists)
        Traits::throwFederationExecutionAlreadyExists(federationExecutionName);
      if (create->getCreateFederationExecutionResponseType() == CreateFederationExecutionResponseCouldNotCreateLogicalTimeFactory)
        Traits::throwCouldNotCreateLogicalTimeFactory(logicalTimeFactoryName);
      if (create->getCreateFederationExecutionResponseType() != CreateFederationExecutionResponseSuccess)
        Traits::throwRTIinternalError(create->getExceptionString());
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void destroyFederationExecution(const std::string& federationExecutionName)
    throw (FederatesCurrentlyJoined,
           FederationExecutionDoesNotExist,
           NotConnected,
           RTIinternalError)
  {
    try {
      if (!_connect.valid())
        Traits::throwNotConnected(std::string("Could not get connect RTI of federation execution \"") +
                                  federationExecutionName + std::string("\"."));

      // The destroy request message
      SharedPtr<DestroyFederationExecutionRequestMessage> request = new DestroyFederationExecutionRequestMessage;
      request->setFederationExecution(federationExecutionName);

      // The maximum abstime to try to connect
      Clock abstime = Clock::now() + Clock::fromSeconds(70);

      // Send this message and wait for the response
      _connect->send(request);

      SharedPtr<const DestroyFederationExecutionResponseMessage> destroy;
      while (!destroy.valid()) {
        SharedPtr<const AbstractMessage> response = _connect->receive(abstime);
        // Read the response and interpret that one
        if (!response.valid())
          Traits::throwRTIinternalError(std::string("Received no response message while talking to RTI of federation execution \"") +
                                        federationExecutionName + std::string("\"."));
        destroy = dynamic_cast<const DestroyFederationExecutionResponseMessage*>(response.get());
        if (_federate.valid()) {
          if (!destroy.valid()) {
            _federate->dispatchInternalMessage(*response);
          }
        } else {
          if (!destroy.valid())
            Traits::throwRTIinternalError(std::string("Received unexpected message type \"") + response->getTypeName() + std::string("\" while talking to RTI of federation execution \"") +
                                          federationExecutionName + std::string("\".") + response->getTypeName());
        }
      }

      if (destroy->getDestroyFederationExecutionResponseType() == DestroyFederationExecutionResponseFederatesCurrentlyJoined)
        Traits::throwFederatesCurrentlyJoined(federationExecutionName);
      if (destroy->getDestroyFederationExecutionResponseType() == DestroyFederationExecutionResponseFederationExecutionDoesNotExist)
        Traits::throwFederationExecutionDoesNotExist(federationExecutionName);
      if (destroy->getDestroyFederationExecutionResponseType() != DestroyFederationExecutionResponseSuccess)
        Traits::throwRTIinternalError("Unspecified internal error FIXME");
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void listFederationExecutions()
    throw (NotConnected,
           RTIinternalError)
  {
    try {
      if (!_connect.valid())
        Traits::throwNotConnected();

      SharedPtr<EnumerateFederationExecutionsRequestMessage> request;
      request = new EnumerateFederationExecutionsRequestMessage;
      _connect->send(request);

    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  FederateHandle joinFederationExecution(const std::string& federateName, const std::string& federateType,
                                         const std::string& federationExecutionName,
                                         const FOMStringModuleList& fomModules,
                                         FederateAmbassador* federateAmbassador)
    throw (CouldNotCreateLogicalTimeFactory,
           FederationExecutionDoesNotExist,
           FederateNameAlreadyInUse,
           // InconsistentFDD,
           // ErrorReadingFDD,
           // CouldNotOpenFDD,
           FederateAlreadyExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           NotConnected,
           CallNotAllowedFromWithinCallback,
           RTIinternalError)
  {
    try {
      if (_federate.valid())
        Traits::throwFederateAlreadyExecutionMember();

      if (!_connect.valid())
        Traits::throwNotConnected(std::string("Could not get connect RTI of federation execution \"") +
                                  federationExecutionName + std::string("\"."));

      if (!fomModules.empty())
        Traits::throwRTIinternalError("Additionaly FOM modules are not implemented yet!");

      // The maximum abstime to try to connect
      Clock abstime = Clock::now() + Clock::fromSeconds(70);

      // The destroy request message
      SharedPtr<JoinFederationExecutionRequestMessage> request;
      request = new JoinFederationExecutionRequestMessage;
      request->setFederationExecution(federationExecutionName);
      request->setFederateType(federateType);
      request->setFederateName(federateName);

      // Send this message and wait for the response
      _connect->send(request);
      SharedPtr<const InsertFederationExecutionMessage> insertFederationMessage;
      SharedPtr<const JoinFederationExecutionResponseMessage> joinResponse;
      do {
        SharedPtr<const AbstractMessage> response = _connect->receive(abstime);

        // Read the response and interpret that one
        if (!response.valid())
          Traits::throwRTIinternalError(std::string("Received no response message while talking to RTI of federation execution \"") +
                                        federationExecutionName + std::string("\"."));

        if (dynamic_cast<const InsertFederationExecutionMessage*>(response.get())) {
          insertFederationMessage = static_cast<const InsertFederationExecutionMessage*>(response.get());
          // Put that into a factory into the ambassador FIXME
          //// FIXME: instead of a connect we need to cache message filters???
          std::string federateName; // FIXME
          FederateHandle federateHandle; // FIXME
          _federate = createFederate(federateType, federateName, federateHandle, federationExecutionName,
                                     *insertFederationMessage, _connect, federateAmbassador);
        } else if (_federate.valid()) {

          _federate->dispatchInternalMessage(*response);
        }

        if (dynamic_cast<const JoinFederationExecutionResponseMessage*>(response.get())) {
          joinResponse = new JoinFederationExecutionResponseMessage(*static_cast<const JoinFederationExecutionResponseMessage*>(response.get()));

          switch (joinResponse->getJoinFederationExecutionResponseType()) {
          case JoinFederationExecutionResponseFederateNameAlreadyInUse:
            _federate = 0;
            Traits::throwFederateNameAlreadyInUse(federateName);
          case JoinFederationExecutionResponseFederationExecutionDoesNotExist:
            _federate = 0;
            Traits::throwFederationExecutionDoesNotExist(federationExecutionName);
          case JoinFederationExecutionResponseSaveInProgress:
            _federate = 0;
            Traits::throwSaveInProgress();
          case JoinFederationExecutionResponseRestoreInProgress:
            _federate = 0;
            Traits::throwRestoreInProgress();
          default:
            break;
          }
        }
      } while (!joinResponse.valid());

      // Put that into a factory into the ambassador FIXME
      if (!_federate.valid()) {
        SharedPtr<ResignFederationExecutionRequestMessage> request;
        request = new ResignFederationExecutionRequestMessage;
        request->setFederationHandle(joinResponse->getFederationHandle());
        request->setFederateHandle(joinResponse->getFederateHandle());
        _connect->send(request);
        SharedPtr<ShutdownFederationExecutionMessage> request2 = new ShutdownFederationExecutionMessage;
        request2->setFederationHandle(joinResponse->getFederationHandle());
        _connect->send(request2);

        Clock abstime = Clock::now() + Clock::fromSeconds(70);
        for (;;) {
          // Skip everything that is not the resign response - don't need that anymore
          SharedPtr<const AbstractMessage> message = _connect->receive(abstime);
          if (!message.valid())
            break;
          if (dynamic_cast<const EraseFederationExecutionMessage*>(message.get()))
            break;
        }

        SharedPtr<ReleaseFederationHandleMessage> request3 = new ReleaseFederationHandleMessage;
        request3->setFederationHandle(joinResponse->getFederationHandle());
        _connect->send(request3);

        if (insertFederationMessage.valid())
          Traits::throwCouldNotCreateLogicalTimeFactory(insertFederationMessage->getLogicalTimeFactoryName());
        else
          Traits::throwRTIinternalError();
      }

      // Request new object instance handles, once we are here ...
      _federate->requestObjectInstanceHandles(16);

      return _federate->getFederateHandle();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void resignFederationExecution(ResignAction resignAction)
    throw (OwnershipAcquisitionPending,
           FederateOwnsAttributes,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      Clock abstime = Clock::now() + Clock::fromSeconds(70);
      getFederate().resignFederationExecution(resignAction, abstime);
      _federate = 0;
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void registerFederationSynchronizationPoint(const std::string& label, const VariableLengthData& tag)
    throw (FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      // According to the standard, an empty set also means all federates currently joined.
      getFederate().registerFederationSynchronizationPoint(label, tag, FederateHandleSet());
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void registerFederationSynchronizationPoint(const std::string& label, const VariableLengthData& tag, const FederateHandleSet& syncSet)
    throw (FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().registerFederationSynchronizationPoint(label, tag, syncSet);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void synchronizationPointAchieved(const std::string& label)
    throw (SynchronizationPointLabelNotAnnounced,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().synchronizationPointAchieved(label);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void requestFederationSave(const std::string& label)
    throw (FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().requestFederationSave(label);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void requestFederationSave(const std::string& label, const NativeLogicalTime& locicalTime)
    throw (LogicalTimeAlreadyPassed,
           InvalidLogicalTime,
           FederateUnableToUseTime,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().requestFederationSave(label, locicalTime);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void federateSaveBegun()
    throw (SaveNotInitiated,
           FederateNotExecutionMember,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().federateSaveBegun();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void federateSaveComplete()
    throw (FederateHasNotBegunSave,
           FederateNotExecutionMember,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().federateSaveComplete();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void federateSaveNotComplete()
    throw (FederateHasNotBegunSave,
           FederateNotExecutionMember,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().federateSaveNotComplete();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void queryFederationSaveStatus()
    throw (FederateNotExecutionMember,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().queryFederationSaveStatus();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void requestFederationRestore(const std::string& label)
    throw (FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().requestFederationRestore(label);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void federateRestoreComplete()
    throw (RestoreNotRequested,
           FederateNotExecutionMember,
           SaveInProgress,
           RTIinternalError)
  {
    try {
      getFederate().federateRestoreComplete();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void federateRestoreNotComplete()
    throw (RestoreNotRequested,
           FederateNotExecutionMember,
           SaveInProgress,
           RTIinternalError)
  {
    try {
      getFederate().federateRestoreNotComplete();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void queryFederationRestoreStatus()
    throw (FederateNotExecutionMember,
           SaveInProgress,
           RTIinternalError)
  {
    try {
      getFederate().queryFederationRestoreStatus();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void publishObjectClassAttributes(ObjectClassHandle objectClassHandle, const AttributeHandleSet& attributeList)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().publishObjectClassAttributes(objectClassHandle, attributeList);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void unpublishObjectClass(ObjectClassHandle objectClassHandle)
    throw (ObjectClassNotDefined,
           OwnershipAcquisitionPending,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().unpublishObjectClass(objectClassHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void unpublishObjectClassAttributes(ObjectClassHandle objectClassHandle, const AttributeHandleSet& attributeList)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           OwnershipAcquisitionPending,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().unpublishObjectClassAttributes(objectClassHandle, attributeList);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void publishInteractionClass(InteractionClassHandle interactionClassHandle)
    throw (InteractionClassNotDefined,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().publishInteractionClass(interactionClassHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void unpublishInteractionClass(InteractionClassHandle interactionClassHandle)
    throw (InteractionClassNotDefined,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().unpublishInteractionClass(interactionClassHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void subscribeObjectClassAttributes(ObjectClassHandle objectClassHandle, const AttributeHandleSet& attributeList, bool active)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().subscribeObjectClassAttributes(objectClassHandle, attributeList, active);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void unsubscribeObjectClass(ObjectClassHandle objectClassHandle)
    throw (ObjectClassNotDefined,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().unsubscribeObjectClass(objectClassHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void unsubscribeObjectClassAttributes(ObjectClassHandle objectClassHandle, const AttributeHandleSet& attributeList)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().unsubscribeObjectClassAttributes(objectClassHandle, attributeList);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void subscribeInteractionClass(InteractionClassHandle interactionClassHandle, bool active)
    throw (InteractionClassNotDefined,
           FederateServiceInvocationsAreBeingReportedViaMOM,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().subscribeInteractionClass(interactionClassHandle, active);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void unsubscribeInteractionClass(InteractionClassHandle interactionClassHandle)
    throw (InteractionClassNotDefined,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().unsubscribeInteractionClass(interactionClassHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void reserveObjectInstanceName(const std::string& objectInstanceName)
    throw (IllegalName,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().reserveObjectInstanceName(objectInstanceName);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void releaseObjectInstanceName(const std::string& objectInstanceName)
    throw (ObjectInstanceNameNotReserved,
           SaveInProgress,
           RestoreInProgress,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      getFederate().releaseObjectInstanceName(objectInstanceName);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void reserveMultipleObjectInstanceName(const std::set<std::string>& objectInstanceName)
    throw (IllegalName,
           NameSetWasEmpty,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().reserveMultipleObjectInstanceName(objectInstanceName);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void releaseMultipleObjectInstanceName(const std::set<std::string>& objectInstanceNameSet)
    throw (ObjectInstanceNameNotReserved,
           SaveInProgress,
           RestoreInProgress,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      getFederate().releaseMultipleObjectInstanceName(objectInstanceNameSet);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  ObjectInstanceHandle registerObjectInstance(ObjectClassHandle objectClassHandle)
    throw (ObjectClassNotDefined,
           ObjectClassNotPublished,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      return getFederate().registerObjectInstance(objectClassHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  ObjectInstanceHandle registerObjectInstance(ObjectClassHandle objectClassHandle, const std::string& objectInstanceName,
                                              bool allowUnreservedObjectNames)
    throw (ObjectClassNotDefined,
           ObjectClassNotPublished,
           ObjectInstanceNameNotReserved,
           ObjectInstanceNameInUse,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      return getFederate().registerObjectInstance(objectClassHandle, objectInstanceName, allowUnreservedObjectNames);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void updateAttributeValues(ObjectInstanceHandle objectClassHandle,
                             std::vector<OpenRTI::AttributeValue>& attributeValues,
                             const VariableLengthData& tag)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().updateAttributeValues(objectClassHandle, attributeValues, tag);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  MessageRetractionHandle updateAttributeValues(ObjectInstanceHandle objectClassHandle,
                                                std::vector<OpenRTI::AttributeValue>& attributeValues,
                                                const VariableLengthData& tag,
                                                const NativeLogicalTime& logicalTime)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           InvalidLogicalTime,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      return getFederate().updateAttributeValues(objectClassHandle, attributeValues, tag, logicalTime);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void sendInteraction(InteractionClassHandle interactionClassHandle, std::vector<ParameterValue>& parameterValues,
                       const VariableLengthData& tag)
    throw (InteractionClassNotPublished,
           InteractionClassNotDefined,
           InteractionParameterNotDefined,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().sendInteraction(interactionClassHandle, parameterValues, tag);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  MessageRetractionHandle sendInteraction(InteractionClassHandle interactionClassHandle,
                                          std::vector<ParameterValue>& parameterValues,
                                          const VariableLengthData& tag,
                                          const NativeLogicalTime& logicalTime)
    throw (InteractionClassNotPublished,
           InteractionClassNotDefined,
           InteractionParameterNotDefined,
           InvalidLogicalTime,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      return getFederate().sendInteraction(interactionClassHandle, parameterValues, tag, logicalTime);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void deleteObjectInstance(ObjectInstanceHandle objectInstanceHandle, const VariableLengthData& tag)
    throw (DeletePrivilegeNotHeld,
           ObjectInstanceNotKnown,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().deleteObjectInstance(objectInstanceHandle, tag);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  MessageRetractionHandle deleteObjectInstance(ObjectInstanceHandle objectInstanceHandle, const VariableLengthData& tag, const NativeLogicalTime& logicalTime)
    throw (DeletePrivilegeNotHeld,
           ObjectInstanceNotKnown,
           InvalidLogicalTime,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      return getFederate().deleteObjectInstance(objectInstanceHandle, tag, logicalTime);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void localDeleteObjectInstance(ObjectInstanceHandle objectInstanceHandle)
    throw (ObjectInstanceNotKnown,
           FederateOwnsAttributes,
           OwnershipAcquisitionPending,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().localDeleteObjectInstance(objectInstanceHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void changeAttributeTransportationType(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet, TransportationType transportationType)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().changeAttributeTransportationType(objectInstanceHandle, attributeHandleSet, transportationType);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void changeInteractionTransportationType(InteractionClassHandle interactionClassHandle, TransportationType transportationType)
    throw (InteractionClassNotDefined,
           InteractionClassNotPublished,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().changeInteractionTransportationType(interactionClassHandle, transportationType);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void requestAttributeValueUpdate(ObjectInstanceHandle objectInstanceHandle,
                                   const AttributeHandleSet& attributeHandleSet,
                                   const VariableLengthData& tag)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().requestAttributeValueUpdate(objectInstanceHandle, attributeHandleSet, tag);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void requestAttributeValueUpdate(ObjectClassHandle objectClassHandle,
                                   const AttributeHandleSet& attributeHandleSet,
                                   const VariableLengthData& tag)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().requestAttributeValueUpdate(objectClassHandle, attributeHandleSet, tag);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void unconditionalAttributeOwnershipDivestiture(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().unconditionalAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleSet);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void negotiatedAttributeOwnershipDivestiture(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet, const VariableLengthData& tag)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           AttributeAlreadyBeingDivested,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().negotiatedAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleSet, tag);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void confirmDivestiture(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet, const VariableLengthData& tag)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           AttributeDivestitureWasNotRequested,
           NoAcquisitionPending,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().confirmDivestiture(objectInstanceHandle, attributeHandleSet, tag);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void attributeOwnershipAcquisition(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet, const VariableLengthData& tag)
    throw (ObjectInstanceNotKnown,
           ObjectClassNotPublished,
           AttributeNotDefined,
           AttributeNotPublished,
           FederateOwnsAttributes,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().attributeOwnershipAcquisition(objectInstanceHandle, attributeHandleSet, tag);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void attributeOwnershipAcquisitionIfAvailable(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet)
    throw (ObjectInstanceNotKnown,
           ObjectClassNotPublished,
           AttributeNotDefined,
           AttributeNotPublished,
           FederateOwnsAttributes,
           AttributeAlreadyBeingAcquired,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().attributeOwnershipAcquisitionIfAvailable(objectInstanceHandle, attributeHandleSet);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void attributeOwnershipDivestitureIfWanted(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet,
                                             AttributeHandleSet& divestedAttributes)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().attributeOwnershipDivestitureIfWanted(objectInstanceHandle, attributeHandleSet, divestedAttributes);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void cancelNegotiatedAttributeOwnershipDivestiture(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           AttributeDivestitureWasNotRequested,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().cancelNegotiatedAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleSet);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void cancelAttributeOwnershipAcquisition(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeAlreadyOwned,
           AttributeAcquisitionWasNotRequested,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().cancelAttributeOwnershipAcquisition(objectInstanceHandle, attributeHandleSet);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void queryAttributeOwnership(ObjectInstanceHandle objectInstanceHandle, AttributeHandle attributeHandle)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().queryAttributeOwnership(objectInstanceHandle, attributeHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  bool isAttributeOwnedByFederate(ObjectInstanceHandle objectInstanceHandle, AttributeHandle attributeHandle)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      return getFederate().isAttributeOwnedByFederate(objectInstanceHandle, attributeHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void enableTimeRegulation(const NativeLogicalTimeInterval& lookahead)
    throw (TimeRegulationAlreadyEnabled,
           InvalidLookahead,
           InTimeAdvancingState,
           RequestForTimeRegulationPending,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().enableTimeRegulation(lookahead);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void enableTimeRegulation(const NativeLogicalTime& logicalTime, const NativeLogicalTimeInterval& lookahead)
    throw (TimeRegulationAlreadyEnabled,
           InvalidLogicalTime,
           InvalidLookahead,
           InTimeAdvancingState,
           RequestForTimeRegulationPending,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().enableTimeRegulation(logicalTime, lookahead);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void disableTimeRegulation()
    throw (TimeRegulationIsNotEnabled,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().disableTimeRegulation();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void enableTimeConstrained()
    throw (TimeConstrainedAlreadyEnabled,
           InTimeAdvancingState,
           RequestForTimeConstrainedPending,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().enableTimeConstrained();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void disableTimeConstrained()
    throw (TimeConstrainedIsNotEnabled,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().disableTimeConstrained();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void timeAdvanceRequest(const NativeLogicalTime& logicalTime)
    throw (InvalidLogicalTime,
           LogicalTimeAlreadyPassed,
           InTimeAdvancingState,
           RequestForTimeRegulationPending,
           RequestForTimeConstrainedPending,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().timeAdvanceRequest(logicalTime);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void timeAdvanceRequestAvailable(const NativeLogicalTime& logicalTime)
    throw (InvalidLogicalTime,
           LogicalTimeAlreadyPassed,
           InTimeAdvancingState,
           RequestForTimeRegulationPending,
           RequestForTimeConstrainedPending,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().timeAdvanceRequestAvailable(logicalTime);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void nextMessageRequest(const NativeLogicalTime& logicalTime)
    throw (InvalidLogicalTime,
           LogicalTimeAlreadyPassed,
           InTimeAdvancingState,
           RequestForTimeRegulationPending,
           RequestForTimeConstrainedPending,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().nextMessageRequest(logicalTime);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void nextMessageRequestAvailable(const NativeLogicalTime& logicalTime)
    throw (InvalidLogicalTime,
           LogicalTimeAlreadyPassed,
           InTimeAdvancingState,
           RequestForTimeRegulationPending,
           RequestForTimeConstrainedPending,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().nextMessageRequestAvailable(logicalTime);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void flushQueueRequest(const NativeLogicalTime& logicalTime)
    throw (InvalidLogicalTime,
           LogicalTimeAlreadyPassed,
           InTimeAdvancingState,
           RequestForTimeRegulationPending,
           RequestForTimeConstrainedPending,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().flushQueueRequest(logicalTime);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void enableAsynchronousDelivery()
    throw (AsynchronousDeliveryAlreadyEnabled,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().enableAsynchronousDelivery();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void disableAsynchronousDelivery()
    throw (AsynchronousDeliveryAlreadyDisabled,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().disableAsynchronousDelivery();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  bool queryGALT(NativeLogicalTime& logicalTime)
    throw (FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      return getFederate().queryGALT(logicalTime);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void queryLogicalTime(NativeLogicalTime& logicalTime)
    throw (FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().queryLogicalTime(logicalTime);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  bool queryLITS(NativeLogicalTime& logicalTime)
    throw (FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      return getFederate().queryLITS(logicalTime);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void modifyLookahead(const NativeLogicalTimeInterval& lookahead)
    throw (TimeRegulationIsNotEnabled,
           InvalidLookahead,
           InTimeAdvancingState,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().modifyLookahead(lookahead);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void queryLookahead(NativeLogicalTimeInterval& logicalTimeInterval)
    throw (TimeRegulationIsNotEnabled,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().queryLookahead(logicalTimeInterval);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void retract(MessageRetractionHandle messageRetractionHandle)
    throw (InvalidRetractionHandle,
           TimeRegulationIsNotEnabled,
           MessageCanNoLongerBeRetracted,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().retract(messageRetractionHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void changeAttributeOrderType(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet, OrderType orderType)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress)
  {
    try {
      getFederate().changeAttributeOrderType(objectInstanceHandle, attributeHandleSet, orderType);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void changeInteractionOrderType(InteractionClassHandle interactionClassHandle, OrderType orderType)
    throw (InteractionClassNotDefined,
           InteractionClassNotPublished,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().changeInteractionOrderType(interactionClassHandle, orderType);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  RegionHandle createRegion(const DimensionHandleSet& dimensionHandleSet)
    throw (InvalidDimensionHandle,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      return getFederate().createRegion(dimensionHandleSet);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void commitRegionModifications(const RegionHandleSet& regionHandleSet)
    throw (InvalidRegion,
           RegionNotCreatedByThisFederate,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().commitRegionModifications(regionHandleSet);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void deleteRegion(RegionHandle regionHandle)
    throw (InvalidRegion,
           RegionNotCreatedByThisFederate,
           RegionInUseForUpdateOrSubscription,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().deleteRegion(regionHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  ObjectInstanceHandle registerObjectInstanceWithRegions(ObjectClassHandle objectClassHandle,
                                                         const AttributeHandleSetRegionHandleSetPairVector& attributeHandleSetRegionHandleSetPairVector)
    throw (ObjectClassNotDefined,
           ObjectClassNotPublished,
           AttributeNotDefined,
           AttributeNotPublished,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      return getFederate().registerObjectInstanceWithRegions(objectClassHandle, attributeHandleSetRegionHandleSetPairVector);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  ObjectInstanceHandle
  registerObjectInstanceWithRegions(ObjectClassHandle objectClassHandle,
                                    const AttributeHandleSetRegionHandleSetPairVector& attributeHandleSetRegionHandleSetPairVector,
                                    const std::string& objectInstanceName)
    throw (ObjectClassNotDefined,
           ObjectClassNotPublished,
           AttributeNotDefined,
           AttributeNotPublished,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           ObjectInstanceNameNotReserved,
           ObjectInstanceNameInUse,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      return getFederate().registerObjectInstanceWithRegions(objectClassHandle, attributeHandleSetRegionHandleSetPairVector, objectInstanceName);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void associateRegionsForUpdates(ObjectInstanceHandle objectInstanceHandle,
                                  const AttributeHandleSetRegionHandleSetPairVector& attributeHandleHandleSetRegionHandleSetPairVector)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().associateRegionsForUpdates(objectInstanceHandle, attributeHandleHandleSetRegionHandleSetPairVector);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void unassociateRegionsForUpdates(ObjectInstanceHandle objectInstanceHandle,
                                    const AttributeHandleSetRegionHandleSetPairVector& attributeHandleHandleSetRegionHandleSetPairVector)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().unassociateRegionsForUpdates(objectInstanceHandle, attributeHandleHandleSetRegionHandleSetPairVector);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void subscribeObjectClassAttributesWithRegions(ObjectClassHandle objectClassHandle,
                                                 const AttributeHandleSetRegionHandleSetPairVector& attributeHandleHandleSetRegionHandleSetPairVector,
                                                 bool active)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().subscribeObjectClassAttributesWithRegions(objectClassHandle, attributeHandleHandleSetRegionHandleSetPairVector, active);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void unsubscribeObjectClassAttributesWithRegions(ObjectClassHandle objectClassHandle,
                                                   const AttributeHandleSetRegionHandleSetPairVector& attributeHandleHandleSetRegionHandleSetPairVector)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().unsubscribeObjectClassAttributesWithRegions(objectClassHandle, attributeHandleHandleSetRegionHandleSetPairVector);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void subscribeInteractionClassWithRegions(InteractionClassHandle objectClassHandle, const RegionHandleSet& regionHandleSet, bool active)
    throw (InteractionClassNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           FederateServiceInvocationsAreBeingReportedViaMOM,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().subscribeInteractionClassWithRegions(objectClassHandle, regionHandleSet, active);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void unsubscribeInteractionClassWithRegions(InteractionClassHandle objectClassHandle, const RegionHandleSet& regionHandleSet)
    throw (InteractionClassNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().unsubscribeInteractionClassWithRegions(objectClassHandle, regionHandleSet);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void sendInteractionWithRegions(InteractionClassHandle interactionClassHandle,
                                  std::vector<ParameterValue>& parameterValues,
                                  const RegionHandleSet& regionHandleSet,
                                  const VariableLengthData& tag)
    throw (InteractionClassNotDefined,
           InteractionClassNotPublished,
           InteractionParameterNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().sendInteractionWithRegions(interactionClassHandle, parameterValues, regionHandleSet, tag);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  MessageRetractionHandle
  sendInteractionWithRegions(InteractionClassHandle interactionClassHandle,
                             std::vector<ParameterValue>& parameterValues,
                             const RegionHandleSet& regionHandleSet,
                             const VariableLengthData& tag,
                             const NativeLogicalTime& logicalTime)
    throw (InteractionClassNotDefined,
           InteractionClassNotPublished,
           InteractionParameterNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           InvalidLogicalTime,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      return getFederate().sendInteractionWithRegions(interactionClassHandle, parameterValues, regionHandleSet, tag, logicalTime);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void requestAttributeValueUpdateWithRegions(ObjectClassHandle objectClassHandle, const AttributeHandleSetRegionHandleSetPairVector& theSet,
                                              const VariableLengthData& tag)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().requestAttributeValueUpdateWithRegions(objectClassHandle, theSet, tag);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  ResignAction getAutomaticResignDirective()
    throw (FederateNotExecutionMember,
           NotConnected,
           RTIinternalError)
  {
    try {
      return getFederate().getAutomaticResignDirective();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void setAutomaticResignDirective(ResignAction resignAction)
    throw (InvalidResignAction,
           FederateNotExecutionMember,
           NotConnected,
           RTIinternalError)
  {
    try {
      getFederate().setAutomaticResignDirective(resignAction);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  ObjectClassHandle getObjectClassHandle(const std::string& name)
    throw (NameNotFound,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      return getFederate().getObjectClassHandle(name);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  const std::string& getObjectClassName(ObjectClassHandle objectClassHandle)
    throw (InvalidObjectClassHandle,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      return getFederate().getObjectClassName(objectClassHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  AttributeHandle getAttributeHandle(ObjectClassHandle objectClassHandle, const std::string& name)
    throw (InvalidObjectClassHandle,
           NameNotFound,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      return getFederate().getAttributeHandle(objectClassHandle, name);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  const std::string& getAttributeName(ObjectClassHandle objectClassHandle, AttributeHandle attributeHandle)
    throw (InvalidObjectClassHandle,
           InvalidAttributeHandle,
           AttributeNotDefined,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      return getFederate().getAttributeName(objectClassHandle, attributeHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  InteractionClassHandle getInteractionClassHandle(const std::string& name)
    throw (NameNotFound,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      return getFederate().getInteractionClassHandle(name);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  const std::string& getInteractionClassName(InteractionClassHandle interactionClassHandle)
    throw (InvalidInteractionClassHandle,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      return getFederate().getInteractionClassName(interactionClassHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  ParameterHandle getParameterHandle(InteractionClassHandle interactionClassHandle, const std::string& name)
    throw (InvalidInteractionClassHandle,
           NameNotFound,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      return getFederate().getParameterHandle(interactionClassHandle, name);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  const std::string& getParameterName(InteractionClassHandle interactionClassHandle, ParameterHandle parameterHandle)
    throw (InvalidInteractionClassHandle,
           InvalidParameterHandle,
           InteractionParameterNotDefined,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      return getFederate().getParameterName(interactionClassHandle, parameterHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  ObjectInstanceHandle getObjectInstanceHandle(const std::string& name)
    throw (ObjectInstanceNotKnown,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      return getFederate().getObjectInstanceHandle(name);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  const std::string& getObjectInstanceName(ObjectInstanceHandle objectInstanceHandle)
    throw (ObjectInstanceNotKnown,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      return getFederate().getObjectInstanceName(objectInstanceHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  DimensionHandle getDimensionHandle(const std::string& name)
    throw (NameNotFound,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      return getFederate().getDimensionHandle(name);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  const std::string& getDimensionName(DimensionHandle dimensionHandle)
    throw (InvalidDimensionHandle,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      return getFederate().getDimensionName(dimensionHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  unsigned long getDimensionUpperBound(DimensionHandle dimensionHandle)
    throw (InvalidDimensionHandle,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      return getFederate().getDimensionUpperBound(dimensionHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  DimensionHandleSet getAvailableDimensionsForClassAttribute(ObjectClassHandle objectClassHandle,
                                                             AttributeHandle attributeHandle)
    throw (InvalidObjectClassHandle,
           InvalidAttributeHandle,
           AttributeNotDefined,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      return getFederate().getAvailableDimensionsForClassAttribute(objectClassHandle, attributeHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  ObjectClassHandle getKnownObjectClassHandle(ObjectInstanceHandle objectInstanceHandle)
    throw (ObjectInstanceNotKnown,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      return getFederate().getKnownObjectClassHandle(objectInstanceHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  DimensionHandleSet getAvailableDimensionsForInteractionClass(InteractionClassHandle interactionClassHandle)
    throw (InvalidInteractionClassHandle,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      return getFederate().getAvailableDimensionsForInteractionClass(interactionClassHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  TransportationType getTransportationType(const std::string& name)
    throw (InvalidTransportationName,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      return getFederate().getTransportationType(name);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  const std::string& getTransportationName(TransportationType transportationType)
    throw (InvalidTransportationType,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      return getFederate().getTransportationName(transportationType);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  OrderType getOrderType(const std::string& name)
    throw (InvalidOrderName,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      return getFederate().getOrderType(name);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  std::string getOrderName(OrderType orderType)
    throw (InvalidOrderType,
           FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      return getFederate().getOrderName(orderType);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void enableObjectClassRelevanceAdvisorySwitch()
    throw (ObjectClassRelevanceAdvisorySwitchIsOn,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().enableObjectClassRelevanceAdvisorySwitch();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void disableObjectClassRelevanceAdvisorySwitch()
    throw (ObjectClassRelevanceAdvisorySwitchIsOff,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().disableObjectClassRelevanceAdvisorySwitch();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void enableAttributeRelevanceAdvisorySwitch()
    throw (AttributeRelevanceAdvisorySwitchIsOn,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().enableAttributeRelevanceAdvisorySwitch();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void disableAttributeRelevanceAdvisorySwitch()
    throw (AttributeRelevanceAdvisorySwitchIsOff,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().disableAttributeRelevanceAdvisorySwitch();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void enableAttributeScopeAdvisorySwitch()
    throw (AttributeScopeAdvisorySwitchIsOn,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().enableAttributeScopeAdvisorySwitch();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void disableAttributeScopeAdvisorySwitch()
    throw (AttributeScopeAdvisorySwitchIsOff,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().disableAttributeScopeAdvisorySwitch();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void enableInteractionRelevanceAdvisorySwitch()
    throw (InteractionRelevanceAdvisorySwitchIsOn,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().enableInteractionRelevanceAdvisorySwitch();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void disableInteractionRelevanceAdvisorySwitch()
    throw (InteractionRelevanceAdvisorySwitchIsOff,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().disableInteractionRelevanceAdvisorySwitch();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  DimensionHandleSet getDimensionHandleSet(RegionHandle regionHandle)
    throw (InvalidRegion,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      return getFederate().getDimensionHandleSet(regionHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  RangeBounds getRangeBounds(RegionHandle regionHandle, DimensionHandle dimensionHandle)
    throw (InvalidRegion,
           RegionDoesNotContainSpecifiedDimension,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      return getFederate().getRangeBounds(regionHandle, dimensionHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void setRangeBounds(RegionHandle regionHandle, DimensionHandle dimensionHandle, const RangeBounds& rangeBounds)
    throw (InvalidRegion,
           RegionNotCreatedByThisFederate,
           RegionDoesNotContainSpecifiedDimension,
           InvalidRangeBound,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().setRangeBounds(regionHandle, dimensionHandle, rangeBounds);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  unsigned long normalizeFederateHandle(FederateHandle federateHandle)
    throw (FederateNotExecutionMember,
           InvalidFederateHandle,
           RTIinternalError)
  {
    try {
      return getFederate().normalizeFederateHandle(federateHandle);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  unsigned long normalizeServiceGroup(ServiceGroupIndicator serviceGroup)
    throw (FederateNotExecutionMember,
           InvalidServiceGroup,
           RTIinternalError)
  {
    try {
      return getFederate().normalizeServiceGroup(serviceGroup);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  bool evokeCallback(double approximateMinimumTimeInSeconds)
    throw (FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      return getFederate().evokeCallback(approximateMinimumTimeInSeconds);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  bool evokeMultipleCallbacks(double approximateMinimumTimeInSeconds, double approximateMaximumTimeInSeconds)
    throw (FederateNotExecutionMember,
           RTIinternalError)
  {
    try {
      return getFederate().evokeMultipleCallbacks(approximateMinimumTimeInSeconds, approximateMaximumTimeInSeconds);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void enableCallbacks()
    throw (FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().enableCallbacks();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void disableCallbacks()
    throw (FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      getFederate().disableCallbacks();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  virtual AbstractFederate<Traits>* createFederate(const std::string& federateType, const std::string& federateName,
                                                   const FederateHandle& federateHandle, const std::string& federationName,
                                                   const InsertFederationExecutionMessage& insertFederationExecution,
                                                   SharedPtr<AbstractConnect> connect, FederateAmbassador* federateAmbassador) = 0;

  virtual void connectionLost(const std::string& faultDescription)
    throw ()
  { }

  virtual void reportFederationExecutions(const FederationExecutionInformationVector& theFederationExecutionInformationList)
    throw ()
  { }

  AbstractFederate<Traits>& getFederate()
    throw (FederateNotExecutionMember)
  {
    if (!_federate.valid())
      Traits::throwFederateNotExecutionMember();
    return *_federate;
  }

  void send(const SharedPtr<AbstractMessage>& message)
  {
    if (!_connect.valid())
      Traits::throwNotConnected();
    _connect->send(message);
  }

  SharedPtr<const AbstractMessage> receive(const Clock& abstime)
  {
    if (!_connect.valid())
      Traits::throwNotConnected();
    return _connect->receive(abstime);
  }
  SharedPtr<const AbstractMessage> receive()
  {
    if (!_connect.valid())
      Traits::throwNotConnected();
    return _connect->receive();
  }

private:
  // The connect once we are connected
  SharedPtr<AbstractConnect> _connect;

  // The federate if available
  SharedPtr<AbstractFederate<Traits> > _federate;
};

} // namespace OpenRTI

#endif
