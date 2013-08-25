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
#include "InternalAmbassador.h"
#include "Clock.h"
#include "Federate.h"
#include "LogStream.h"
#include "Message.h"
#include "MessageList.h"
#include "StringUtils.h"
#include "TimeManagement.h"

namespace OpenRTI {

template<typename T>
class OPENRTI_LOCAL Ambassador : public InternalAmbassador {
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

  const FederateHandle& getFederateHandle() const
  { return _federate->getFederateHandle(); }
  const FederationHandle& getFederationHandle() const
  { return _federate->getFederationHandle(); }

  void connect(const StringMap& parameterMap, const Clock& abstime)
    throw (ConnectionFailed,
           AlreadyConnected,
           CallNotAllowedFromWithinCallback,
           RTIinternalError)
  {
    try {
      if (isConnected())
        Traits::throwAlreadyConnected("Ambassador is already connected!");

      InternalAmbassador::connect(parameterMap, abstime);

      if (!isConnected())
        Traits::throwConnectionFailed("Connection failed!");

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

      if (!isConnected())
        return;
      InternalAmbassador::disconnect();

    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

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
      if (!isConnected())
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
      send(request);

      std::pair<CreateFederationExecutionResponseType, std::string> responseTypeStringPair;
      responseTypeStringPair = dispatchWaitCreateFederationExecutionResponse(abstime);
      if (responseTypeStringPair.first == CreateFederationExecutionResponseFederationExecutionAlreadyExists)
        Traits::throwFederationExecutionAlreadyExists(federationExecutionName);
      if (responseTypeStringPair.first == CreateFederationExecutionResponseCouldNotCreateLogicalTimeFactory)
        Traits::throwCouldNotCreateLogicalTimeFactory(logicalTimeFactoryName);
      if (responseTypeStringPair.first != CreateFederationExecutionResponseSuccess)
        Traits::throwRTIinternalError(responseTypeStringPair.second);

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
      if (!isConnected())
        Traits::throwNotConnected(std::string("Could not get connect RTI of federation execution \"") +
                                  federationExecutionName + std::string("\"."));

      // The destroy request message
      SharedPtr<DestroyFederationExecutionRequestMessage> request = new DestroyFederationExecutionRequestMessage;
      request->setFederationExecution(federationExecutionName);

      // The maximum abstime to try to connect
      Clock abstime = Clock::now() + Clock::fromSeconds(70);

      // Send this message and wait for the response
      send(request);

      DestroyFederationExecutionResponseType responseType;
      responseType = dispatchWaitDestroyFederationExecutionResponse(abstime);
      if (responseType == DestroyFederationExecutionResponseFederatesCurrentlyJoined)
        Traits::throwFederatesCurrentlyJoined(federationExecutionName);
      if (responseType == DestroyFederationExecutionResponseFederationExecutionDoesNotExist)
        Traits::throwFederationExecutionDoesNotExist(federationExecutionName);
      if (responseType != DestroyFederationExecutionResponseSuccess)
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
      if (!isConnected())
        Traits::throwNotConnected();

      SharedPtr<EnumerateFederationExecutionsRequestMessage> request;
      request = new EnumerateFederationExecutionsRequestMessage;
      send(request);

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
      if (!isConnected())
        Traits::throwNotConnected(std::string("Could not get connect RTI of federation execution \"") +
                                  federationExecutionName + std::string("\"."));
      if (_federate.valid())
        Traits::throwFederateAlreadyExecutionMember();

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
      send(request);
      JoinFederationExecutionResponseType responseType;
      responseType = dispatchWaitJoinFederationExecutionResponse(abstime);
      switch (responseType) {
      case JoinFederationExecutionResponseFederateNameAlreadyInUse:
        _federate = 0;
        Traits::throwFederateNameAlreadyInUse(federateName);
        break;
      case JoinFederationExecutionResponseFederationExecutionDoesNotExist:
        _federate = 0;
        Traits::throwFederationExecutionDoesNotExist(federationExecutionName);
        break;
      case JoinFederationExecutionResponseSaveInProgress:
        _federate = 0;
        Traits::throwSaveInProgress();
        break;
      case JoinFederationExecutionResponseRestoreInProgress:
        _federate = 0;
        Traits::throwRestoreInProgress();
        break;
      default:
        break;
      }
      if (!_federate.valid())
        Traits::throwRTIinternalError("Federate is not valid!");

      if (!_timeManagement.valid()) {
        std::string logicalTimeFactoryName = _federate->getLogicalTimeFactoryName();

        _resignFederationExecution(CANCEL_THEN_DELETE_THEN_DIVEST);

        if (!logicalTimeFactoryName.empty())
          Traits::throwCouldNotCreateLogicalTimeFactory(logicalTimeFactoryName);
        else
          Traits::throwRTIinternalError();
      }

      // Request new object instance handles, once we are here ...
      _requestObjectInstanceHandles(16);

      return getFederateHandle();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      _resignFederationExecution(resignAction);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void _resignFederationExecution(ResignAction resignAction)
  {
    if (!_federate.valid())
      return;

    Clock clock = Clock::now() + Clock::fromSeconds(70);

    // Puh FIXME: have a concept for this in the server nodes instead of relying on that here
    for (size_t i = 0; i < _federate->getNumObjectClasses(); ++i) {
      try {
        unsubscribeObjectClass(i);
      } catch (...) {
      }
    }
    for (size_t i = 0; i < _federate->getNumInteractionClasses(); ++i) {
      try {
        unsubscribeInteractionClass(i);
      } catch (...) {
      }
    }

    // disable time regulation, for now from here
    if (_timeManagement.valid() && _timeManagement->getTimeRegulationEnabledOrPending())
      _timeManagement->disableTimeRegulation(*this);
    // We should no longer respond to time regulation requests.
    _timeManagement = 0;

    // delete object instances if requested
    bool deleteObjects = resignAction == DELETE_OBJECTS ||
      resignAction == DELETE_OBJECTS_THEN_DIVEST || resignAction == CANCEL_THEN_DELETE_THEN_DIVEST;
    // FIXME: currently we do not have ownership management - so, if the owner dies the object needs to die too
    deleteObjects = true;

    if (deleteObjects) {
      ObjectInstanceHandleVector objectInstanceHandleVector = _federate->getOwnedObjectInstanceHandles();
      for (ObjectInstanceHandleVector::iterator i = objectInstanceHandleVector.begin();
           i != objectInstanceHandleVector.end(); ++i) {
        try {
          deleteObjectInstance(*i, VariableLengthData("Delete on resign FIXME!"));
        } catch (...) {
        }
      }
    }

    for (size_t i = 0; i < _federate->getNumObjectClasses(); ++i) {
      try {
        unpublishObjectClass(i);
      } catch (...) {
      }
    }
    for (size_t i = 0; i < _federate->getNumInteractionClasses(); ++i) {
      try {
        unpublishInteractionClass(i);
      } catch (...) {
      }
    }

    // Now release the resources this federate owns
    ObjectInstanceHandleVector objectInstanceHandleVector = _federate->getReferencedObjectInstanceHandles();
    if (!objectInstanceHandleVector.empty()) {
      SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> message;
      message = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
      message->setFederationHandle(getFederationHandle());
      message->getObjectInstanceHandleVector().swap(objectInstanceHandleVector);
      send(message);
    }

    RegionHandleVector regionHandleVector = _federate->getLocalRegionHandles();
    if (!regionHandleVector.empty()) {
      SharedPtr<EraseRegionMessage> eraseRegionRequest;
      eraseRegionRequest = new EraseRegionMessage;
      eraseRegionRequest->setFederationHandle(getFederationHandle());
      eraseRegionRequest->getRegionHandleVector().swap(regionHandleVector);
      send(eraseRegionRequest);
    }

    SharedPtr<ResignFederationExecutionRequestMessage> request = new ResignFederationExecutionRequestMessage;
    request->setFederationHandle(getFederationHandle());
    request->setFederateHandle(getFederateHandle());
    send(request);

    SharedPtr<ShutdownFederationExecutionMessage> request2 = new ShutdownFederationExecutionMessage;
    request2->setFederationHandle(getFederationHandle());
    send(request2);

    if (!dispatchWaitEraseFederationExecutionResponse(clock))
      Traits::throwRTIinternalError("resignFederationExecution hit timeout!");

    SharedPtr<ReleaseFederationHandleMessage> request3 = new ReleaseFederationHandleMessage;
    request3->setFederationHandle(getFederationHandle());
    send(request3);

    _federate = 0;
  }


  void registerFederationSynchronizationPoint(const std::string& label, const VariableLengthData& tag, const FederateHandleSet& syncSet)
    throw (FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();

      // Tell all federates about that label
      SharedPtr<RegisterFederationSynchronizationPointMessage> message;
      message = new RegisterFederationSynchronizationPointMessage;
      message->setFederationHandle(getFederationHandle());
      message->setFederateHandle(getFederateHandle());
      message->setLabel(label);
      message->setTag(tag);
      message->setFederateHandleSet(syncSet);
      send(message);

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      if (!_federate->synchronizationLabelAnnounced(label))
        Traits::throwSynchronizationPointLabelNotAnnounced();

      // tell all federates about that label
      SharedPtr<SynchronizationPointAchievedMessage> message;
      message = new SynchronizationPointAchievedMessage;
      message->setFederationHandle(getFederationHandle());
      message->getFederateHandleSet().insert(getFederateHandle());
      message->setLabel(label);
      send(message);

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError("Save/Restore not implemented!");
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError("Save/Restore not implemented!");
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError("Save/Restore not implemented!");
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError("Save/Restore not implemented!");
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError("Save/Restore not implemented!");
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError("Save/Restore not implemented!");
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError("Save/Restore not implemented!");
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError("Save/Restore not implemented!");
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError("Save/Restore not implemented!");
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError("Save/Restore not implemented!");
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
      // At first the complete error checks
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::ObjectClass* objectClass = _federate->getObjectClass(objectClassHandle);
      if (!objectClass)
        Traits::throwObjectClassNotDefined(objectClassHandle.toString());
      for (AttributeHandleSet::const_iterator i = attributeList.begin(); i != attributeList.end(); ++i)
        if (!objectClass->getAttribute(*i))
          Traits::throwAttributeNotDefined(i->toString());

      SharedPtr<ChangeObjectClassPublicationMessage> request = new ChangeObjectClassPublicationMessage;
      request->setFederationHandle(getFederationHandle());
      request->setObjectClassHandle(objectClassHandle);

      // Alway one more because of the implicit privilege to delete
      request->getAttributeHandles().reserve(1 + attributeList.size());
      request->setPublicationType(Published);

      // Mark the objectclass itself as published.
      // Append this to the request if this publication has changed
      if (objectClass->setPublicationType(Published))
        request->getAttributeHandles().push_back(AttributeHandle(0));
      for (AttributeHandleSet::const_iterator i = attributeList.begin(); i != attributeList.end(); ++i) {
        // returns true if there is a change in the publication state
        if (!objectClass->setAttributePublicationType(*i, Published))
          continue;
        request->getAttributeHandles().push_back(*i);
      }
      // If there has nothing changed, don't send anything.
      if (request->getAttributeHandles().empty())
        return;

      send(request);

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
      // At first the complete error checks
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::ObjectClass* objectClass = _federate->getObjectClass(objectClassHandle);
      if (!objectClass)
        Traits::throwObjectClassNotDefined(objectClassHandle.toString());

      // now that we know not to throw, handle the request
      SharedPtr<ChangeObjectClassPublicationMessage> request = new ChangeObjectClassPublicationMessage;
      request->setFederationHandle(getFederationHandle());
      request->setObjectClassHandle(objectClassHandle);

      // Alway one more because of the implicit privilege to delete
      request->getAttributeHandles().reserve(objectClass->getNumAttributes());
      request->setPublicationType(Unpublished);

      // Mark the objectclass itself as unpublished.
      // Append this to the request if this publication has changed
      if (objectClass->setPublicationType(Unpublished))
        request->getAttributeHandles().push_back(AttributeHandle(0));
      for (size_t i = 0; i < objectClass->getNumAttributes(); ++i) {
        // returns true if there is a change in the publication state
        if (!objectClass->setAttributePublicationType(AttributeHandle(i), Unpublished))
          continue;
        request->getAttributeHandles().push_back(AttributeHandle(i));
      }
      // If there has nothing changed, don't send anything.
      if (request->getAttributeHandles().empty())
        return;

      send(request);

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
      // At first the complete error checks
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::ObjectClass* objectClass = _federate->getObjectClass(objectClassHandle);
      if (!objectClass)
        Traits::throwObjectClassNotDefined(objectClassHandle.toString());
      for (AttributeHandleSet::const_iterator i = attributeList.begin(); i != attributeList.end(); ++i)
        if (!objectClass->getAttribute(*i))
          Traits::throwAttributeNotDefined(i->toString());

      // now that we know not to throw, handle the request
      SharedPtr<ChangeObjectClassPublicationMessage> request = new ChangeObjectClassPublicationMessage;
      request->setFederationHandle(getFederationHandle());
      request->setObjectClassHandle(objectClassHandle);

      // Alway one more because of the implicit privilege to delete
      request->getAttributeHandles().reserve(attributeList.size());
      request->setPublicationType(Unpublished);

      for (AttributeHandleSet::const_iterator i = attributeList.begin(); i != attributeList.end(); ++i) {
        // returns true if there is a change in the publication state
        if (!objectClass->setAttributePublicationType(*i, Unpublished))
          continue;
        request->getAttributeHandles().push_back(*i);
      }
      // If there has nothing changed, don't send anything.
      if (request->getAttributeHandles().empty())
        return;

      send(request);

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::InteractionClass* interactionClass = _federate->getInteractionClass(interactionClassHandle);
      if (!interactionClass)
        Traits::throwInteractionClassNotDefined(interactionClassHandle.toString());
      if (!interactionClass->setPublicationType(Published))
        return;

      SharedPtr<ChangeInteractionClassPublicationMessage> request = new ChangeInteractionClassPublicationMessage;
      request->setFederationHandle(getFederationHandle());
      request->setInteractionClassHandle(interactionClassHandle);
      request->setPublicationType(Published);

      send(request);
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();

      Federate::InteractionClass* interactionClass = _federate->getInteractionClass(interactionClassHandle);
      if (!interactionClass)
        Traits::throwInteractionClassNotDefined(interactionClassHandle.toString());
      if (!interactionClass->setPublicationType(Unpublished))
        return;

      SharedPtr<ChangeInteractionClassPublicationMessage> request = new ChangeInteractionClassPublicationMessage;
      request->setFederationHandle(getFederationHandle());
      request->setInteractionClassHandle(interactionClassHandle);
      request->setPublicationType(Unpublished);

      send(request);

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
      // At first the complete error checks
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::ObjectClass* objectClass = _federate->getObjectClass(objectClassHandle);
      if (!objectClass)
        Traits::throwObjectClassNotDefined(objectClassHandle.toString());
      for (AttributeHandleSet::const_iterator i = attributeList.begin(); i != attributeList.end(); ++i)
        if (!objectClass->getAttribute(*i))
          Traits::throwAttributeNotDefined(i->toString());

      // now that we know not to throw, handle the request
      SharedPtr<ChangeObjectClassSubscriptionMessage> request = new ChangeObjectClassSubscriptionMessage;
      request->setFederationHandle(getFederationHandle());
      request->setObjectClassHandle(objectClassHandle);

      // Alway one more because of the implicit privilege to delete
      request->getAttributeHandles().reserve(1 + attributeList.size());

      SubscriptionType subscriptionType;
      if (active) {
        subscriptionType = SubscribedActive;
      } else {
        subscriptionType = SubscribedPassive;
      }
      request->setSubscriptionType(subscriptionType);

      // Mark the objectclass itself as subscribed.
      // Append this to the request if this subscription has changed
      if (objectClass->setSubscriptionType(subscriptionType))
        request->getAttributeHandles().push_back(AttributeHandle(0));
      for (AttributeHandleSet::const_iterator i = attributeList.begin(); i != attributeList.end(); ++i) {
        // returns true if there is a change in the subscription state
        if (!objectClass->setAttributeSubscriptionType(*i, subscriptionType))
          continue;
        request->getAttributeHandles().push_back(*i);
      }
      // If there has nothing changed, don't send anything.
      if (request->getAttributeHandles().empty())
        return;

      send(request);

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
      // At first the complete error checks
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::ObjectClass* objectClass = _federate->getObjectClass(objectClassHandle);
      if (!objectClass)
        Traits::throwObjectClassNotDefined(objectClassHandle.toString());

      // now that we know not to throw, handle the request
      SharedPtr<ChangeObjectClassSubscriptionMessage> request = new ChangeObjectClassSubscriptionMessage;
      request->setFederationHandle(getFederationHandle());
      request->setObjectClassHandle(objectClassHandle);
      request->setSubscriptionType(Unsubscribed);
      request->getAttributeHandles().reserve(objectClass->getNumAttributes());
      if (objectClass->setSubscriptionType(Unsubscribed))
        request->getAttributeHandles().push_back(AttributeHandle(0));
      for (size_t i = 0; i < objectClass->getNumAttributes(); ++i) {
        // returns true if there is a change in the subscription state
        if (!objectClass->setAttributeSubscriptionType(AttributeHandle(i), Unsubscribed))
          continue;
        request->getAttributeHandles().push_back(AttributeHandle(i));
      }
      // If there has nothing changed, don't send anything.
      if (request->getAttributeHandles().empty())
        return;

      send(request);

      OpenRTIAssert(objectClass->getEffectiveSubscriptionType() == Unsubscribed);

      for (Federate::ObjectInstanceHandleMap::const_iterator i = _federate->getObjectInstanceHandleMap().begin();
           i != _federate->getObjectInstanceHandleMap().end();) {
        if (i->second->getObjectClassHandle() != objectClassHandle) {
          ++i;
        } else if (i->second->isOwnedByFederate()) {
          ++i;
        } else {
          _releaseObjectInstance(ObjectInstanceHandle((i++)->first));
        }
      }

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
      // At first the complete error checks
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::ObjectClass* objectClass = _federate->getObjectClass(objectClassHandle);
      if (!objectClass)
        Traits::throwObjectClassNotDefined(objectClassHandle.toString());
      for (AttributeHandleSet::const_iterator i = attributeList.begin(); i != attributeList.end(); ++i)
        if (!objectClass->getAttribute(*i))
          Traits::throwAttributeNotDefined(i->toString());

      // now that we know not to throw, handle the request
      SharedPtr<ChangeObjectClassSubscriptionMessage> request = new ChangeObjectClassSubscriptionMessage;
      request->setFederationHandle(getFederationHandle());
      request->setObjectClassHandle(objectClassHandle);
      request->setSubscriptionType(Unsubscribed);

      request->getAttributeHandles().reserve(attributeList.size());
      for (AttributeHandleSet::const_iterator i = attributeList.begin(); i != attributeList.end(); ++i) {
        // returns true if there is a change in the subscription state
        if (!objectClass->setAttributeSubscriptionType(*i, Unsubscribed))
          continue;
        request->getAttributeHandles().push_back(*i);
      }
      // If there has nothing changed, don't send anything.
      if (request->getAttributeHandles().empty())
        return;

      send(request);

      if (objectClass->getEffectiveSubscriptionType() == Unsubscribed) {
        for (Federate::ObjectInstanceHandleMap::const_iterator i = _federate->getObjectInstanceHandleMap().begin();
             i != _federate->getObjectInstanceHandleMap().end();) {
          if (i->second->getObjectClassHandle() != objectClassHandle) {
            ++i;
          } else if (i->second->isOwnedByFederate()) {
            ++i;
          } else {
            _releaseObjectInstance(ObjectInstanceHandle((i++)->first));
          }
        }
      }
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::InteractionClass* interactionClass = _federate->getInteractionClass(interactionClassHandle);
      if (!interactionClass)
        Traits::throwInteractionClassNotDefined(interactionClassHandle.toString());

      SubscriptionType subscriptionType;
      if (active) {
        subscriptionType = SubscribedActive;
      } else {
        subscriptionType = SubscribedPassive;
      }
      if (!interactionClass->setSubscriptionType(subscriptionType))
        return;

      SharedPtr<ChangeInteractionClassSubscriptionMessage> request = new ChangeInteractionClassSubscriptionMessage;
      request->setFederationHandle(getFederationHandle());
      request->setInteractionClassHandle(interactionClassHandle);
      request->setSubscriptionType(subscriptionType);

      send(request);

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::InteractionClass* interactionClass = _federate->getInteractionClass(interactionClassHandle);
      if (!interactionClass)
        Traits::throwInteractionClassNotDefined(interactionClassHandle.toString());

      if (!interactionClass->setSubscriptionType(Unsubscribed))
        return;

      SharedPtr<ChangeInteractionClassSubscriptionMessage> request = new ChangeInteractionClassSubscriptionMessage;
      request->setFederationHandle(getFederationHandle());
      request->setInteractionClassHandle(interactionClassHandle);
      request->setSubscriptionType(Unsubscribed);

      send(request);

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      if (objectInstanceName.empty())
        Traits::throwIllegalName("Empty object names are not allowed!");
      if (objectInstanceName.compare(0, 3, "HLA") == 0)
        Traits::throwIllegalName("Object instance names starting with \"HLA\" are reserved for the RTI.");

      SharedPtr<ReserveObjectInstanceNameRequestMessage> request;
      request = new ReserveObjectInstanceNameRequestMessage;
      request->setFederationHandle(getFederationHandle());
      request->setFederateHandle(getFederateHandle());
      request->setName(objectInstanceName);

      send(request);

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      ObjectInstanceHandle objectInstanceHandle = _federate->takeReservedObjectInstanceName(objectInstanceName);
      if (!objectInstanceHandle.valid())
        Traits::throwObjectInstanceNameNotReserved(objectInstanceName);

      // ... and send the release message to the rti
      SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> message;
      message = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
      message->setFederationHandle(getFederationHandle());
      message->getObjectInstanceHandleVector().push_back(objectInstanceHandle);

      send(message);

    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void reserveMultipleObjectInstanceName(const std::set<std::string>& objectInstanceNameSet)
    throw (IllegalName,
           NameSetWasEmpty,
           FederateNotExecutionMember,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    try {
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      if (objectInstanceNameSet.empty())
        Traits::throwNameSetWasEmpty("Empty object name set is not allowed!");

      SharedPtr<ReserveMultipleObjectInstanceNameRequestMessage> request;
      request = new ReserveMultipleObjectInstanceNameRequestMessage;
      request->getNameList().reserve(objectInstanceNameSet.size());
      for (std::set<std::string>::const_iterator i = objectInstanceNameSet.begin(); i != objectInstanceNameSet.end(); ++i) {
        if (i->empty())
          Traits::throwIllegalName("Empty object hames are not allowed!");
        if (i->compare(0, 3, "HLA") == 0)
          Traits::throwIllegalName("Object instance names starting with \"HLA\" are reserved for the RTI.");
        request->getNameList().push_back(*i);
      }
      request->setFederationHandle(getFederationHandle());
      request->setFederateHandle(getFederateHandle());

      send(request);

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      for (std::set<std::string>::const_iterator i = objectInstanceNameSet.begin(); i != objectInstanceNameSet.end(); ++i) {
        if (!_federate->objectInstanceNameReserved(*i))
          Traits::throwObjectInstanceNameNotReserved(*i);
      }

      SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> message;
      message = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
      message->setFederationHandle(getFederationHandle());
      message->getObjectInstanceHandleVector().reserve(objectInstanceNameSet.size());
      for (std::set<std::string>::const_iterator i = objectInstanceNameSet.begin(); i != objectInstanceNameSet.end(); ++i) {
        ObjectInstanceHandle objectInstanceHandle = _federate->takeReservedObjectInstanceName(*i);
        OpenRTIAssert(objectInstanceHandle.valid());
        message->getObjectInstanceHandleVector().push_back(objectInstanceHandle);
      }
      send(message);

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
      // At first the complete error checks
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::ObjectClass* objectClass = _federate->getObjectClass(objectClassHandle);
      if (!objectClass)
        Traits::throwObjectClassNotDefined(objectClassHandle.toString());
      if (Published != objectClass->getEffectivePublicationType())
        Traits::throwObjectClassNotPublished(objectClass->getName());

      ObjectInstanceHandleNamePair handleNamePair = _getFreeObjectInstanceHandleNamePair();

      _federate->insertObjectInstance(handleNamePair.first, handleNamePair.second, objectClassHandle, true);

      SharedPtr<InsertObjectInstanceMessage> request;
      request = new InsertObjectInstanceMessage;
      request->setFederationHandle(getFederationHandle());
      request->setObjectClassHandle(objectClassHandle);
      request->setObjectInstanceHandle(handleNamePair.first);
      request->setName(handleNamePair.second);
      request->getAttributeStateVector().reserve(objectClass->getNumAttributes());
      for (size_t i = 0; i < objectClass->getNumAttributes(); ++i) {
        if (!objectClass->isAttributePublished(AttributeHandle(i)))
          continue;
        AttributeState attributeState;
        attributeState.setAttributeHandle(AttributeHandle(i));
        request->getAttributeStateVector().push_back(attributeState);
      }
      send(request);

      return handleNamePair.first;

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
    // Short circuit to invent an own name
    if (objectInstanceName.empty())
      return registerObjectInstance(objectClassHandle);

    try {
      // At first the complete error checks
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::ObjectClass* objectClass = _federate->getObjectClass(objectClassHandle);
      if (!objectClass)
        Traits::throwObjectClassNotDefined(objectClassHandle.toString());
      if (Published != objectClass->getEffectivePublicationType())
        Traits::throwObjectClassNotPublished(objectClass->getName());

      // The already available objectInstanceHandle should be stored here.
      ObjectInstanceHandle objectInstanceHandle;
      objectInstanceHandle = _federate->takeReservedObjectInstanceName(objectInstanceName);

      // Check if we already have the object instance name reserved
      if (!objectInstanceHandle.valid()) {
        // If not, either policy tells us to just bail out - the standard rti1516 behaviour ...
        if (!allowUnreservedObjectNames) {
          Traits::throwObjectInstanceNameNotReserved(objectInstanceName);
        } else {
          // Or, we try to reserve this object name behind the scenes.
          // Ok, if this is allowed, like for a rti13 federate or for the option
          // of allowing that to emulate certi behavior, we need to do the reservation
          // of the name now. This is the only syncronous operation in the rti.
          Clock timeout = Clock::now() + Clock::fromSeconds(60); // FIXME???
          objectInstanceHandle = dispatchWaitReserveObjectInstanceName(timeout, objectInstanceName);
          if (!objectInstanceHandle.valid())
            Traits::throwObjectInstanceNameInUse(objectInstanceName);
        }
      }

      // Once we have survived, we know that the objectInstanceName given in the argument is unique and ours.
      // Also the object instance handle in this local scope must be valid and ours.

      _federate->insertObjectInstance(objectInstanceHandle, objectInstanceName, objectClassHandle, true);

      SharedPtr<InsertObjectInstanceMessage> request;
      request = new InsertObjectInstanceMessage;
      request->setFederationHandle(getFederationHandle());
      request->setObjectClassHandle(objectClassHandle);
      request->setObjectInstanceHandle(objectInstanceHandle);
      request->setName(objectInstanceName);
      request->getAttributeStateVector().reserve(objectClass->getNumAttributes());
      for (size_t i = 0; i < objectClass->getNumAttributes(); ++i) {
        if (!objectClass->isAttributePublished(AttributeHandle(i)))
          continue;
        AttributeState attributeState;
        attributeState.setAttributeHandle(AttributeHandle(i));
        request->getAttributeStateVector().push_back(attributeState);
      }
      send(request);

      return objectInstanceHandle;

    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void updateAttributeValues(ObjectInstanceHandle objectInstanceHandle,
                             AttributeValueVector& attributeValues,
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::ObjectInstance* objectInstance = _federate->getObjectInstance(objectInstanceHandle);
      if (!objectInstance)
        Traits::throwObjectInstanceNotKnown(objectInstanceHandle.toString());
      // passels
      AttributeValueVector passels[2];
      for (std::vector<OpenRTI::AttributeValue>::iterator i = attributeValues.begin(); i != attributeValues.end(); ++i) {
        const Federate::InstanceAttribute* instanceAttribute = objectInstance->getInstanceAttribute(i->getAttributeHandle());
        if (!instanceAttribute)
          Traits::throwAttributeNotDefined(i->getAttributeHandle().toString());
        if (!instanceAttribute->getIsOwnedByFederate())
          Traits::throwAttributeNotOwned(i->getAttributeHandle().toString());
        unsigned index = instanceAttribute->getTransportationType();
        passels[index].reserve(attributeValues.size());
        passels[index].push_back(AttributeValue());
        passels[index].back().setAttributeHandle(i->getAttributeHandle());
        passels[index].back().getValue().swap(i->getValue());
      }

      for (unsigned i = 0; i < 2; ++i) {
        if (passels[i].empty())
          continue;
        SharedPtr<AttributeUpdateMessage> request;
        request = new AttributeUpdateMessage;
        request->setFederationHandle(getFederationHandle());
        request->setObjectInstanceHandle(objectInstanceHandle);
        request->getAttributeValues().swap(passels[i]);
        request->setTransportationType(TransportationType(i));
        request->setTag(tag);
        send(request);
      }

    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  MessageRetractionHandle updateAttributeValues(ObjectInstanceHandle objectInstanceHandle,
                                                AttributeValueVector& attributeValues,
                                                const VariableLengthData& tag,
                                                const NativeLogicalTime& nativeLogicalTime)
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::ObjectInstance* objectInstance = _federate->getObjectInstance(objectInstanceHandle);
      if (!objectInstance)
        Traits::throwObjectInstanceNotKnown(objectInstanceHandle.toString());
      bool timeRegulationEnabled = getTimeManagement()->getTimeRegulationEnabled();
      // passels
      AttributeValueVector passels[2][2];
      for (std::vector<OpenRTI::AttributeValue>::iterator i = attributeValues.begin(); i != attributeValues.end(); ++i) {
        const Federate::InstanceAttribute* instanceAttribute = objectInstance->getInstanceAttribute(i->getAttributeHandle());
        if (!instanceAttribute)
          Traits::throwAttributeNotDefined(i->getAttributeHandle().toString());
        if (!instanceAttribute->getIsOwnedByFederate())
          Traits::throwAttributeNotOwned(i->getAttributeHandle().toString());
        unsigned index0 = instanceAttribute->getTransportationType();
        unsigned index1 = RECEIVE;
        if (timeRegulationEnabled)
          index1 = instanceAttribute->getOrderType();
        passels[index0][index1].reserve(attributeValues.size());
        passels[index0][index1].push_back(AttributeValue());
        passels[index0][index1].back().setAttributeHandle(i->getAttributeHandle());
        passels[index0][index1].back().getValue().swap(i->getValue());
      }
      if (timeRegulationEnabled && getTimeManagement()->logicalTimeAlreadyPassed(nativeLogicalTime))
        Traits::throwInvalidLogicalTime(getTimeManagement()->logicalTimeToString(nativeLogicalTime));

      MessageRetractionHandle messageRetractionHandle = getNextMessageRetractionHandle();
      VariableLengthData timeStamp = getTimeManagement()->encodeLogicalTime(nativeLogicalTime);

      for (unsigned i = 0; i < 2; ++i) {
        for (unsigned j = 0; j < 2; ++j) {
          if (passels[i][j].empty())
            continue;
          SharedPtr<TimeStampedAttributeUpdateMessage> request;
          request = new TimeStampedAttributeUpdateMessage;
          request->setFederationHandle(getFederationHandle());
          request->setObjectInstanceHandle(objectInstanceHandle);
          request->getAttributeValues().swap(passels[i][j]);
          request->setTimeStamp(timeStamp);
          request->setTransportationType(TransportationType(i));
          request->setOrderType(OrderType(j));
          request->setTag(tag);
          request->setMessageRetractionHandle(messageRetractionHandle);
          send(request);
        }
      }

      return messageRetractionHandle;
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      const Federate::InteractionClass* interactionClass = _federate->getInteractionClass(interactionClassHandle);
      if (!interactionClass)
        Traits::throwInteractionClassNotDefined(interactionClassHandle.toString());
      if (!interactionClass->isPublished())
        Traits::throwInteractionClassNotPublished(interactionClassHandle.toString());
      for (std::vector<ParameterValue>::const_iterator i = parameterValues.begin(); i != parameterValues.end(); ++i)
        if (!interactionClass->getParameter(i->getParameterHandle()))
          Traits::throwInteractionParameterNotDefined(i->getParameterHandle().toString());

      SharedPtr<InteractionMessage> request;
      request = new InteractionMessage;
      request->setFederationHandle(getFederationHandle());
      request->setInteractionClassHandle(interactionClassHandle);
      request->setTransportationType(interactionClass->getTransportationType());
      request->setTag(tag);
      request->getParameterValues().swap(parameterValues);
      send(request);

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();

      const Federate::InteractionClass* interactionClass = _federate->getInteractionClass(interactionClassHandle);
      if (!interactionClass)
        Traits::throwInteractionClassNotDefined(interactionClassHandle.toString());
      if (!interactionClass->isPublished())
        Traits::throwInteractionClassNotPublished(interactionClassHandle.toString());
      for (std::vector<ParameterValue>::const_iterator i = parameterValues.begin(); i != parameterValues.end(); ++i)
        if (!interactionClass->getParameter(i->getParameterHandle()))
          Traits::throwInteractionParameterNotDefined(i->getParameterHandle().toString());
      bool timeRegulationEnabled = getTimeManagement()->getTimeRegulationEnabled();
      if (timeRegulationEnabled && getTimeManagement()->logicalTimeAlreadyPassed(logicalTime))
        Traits::throwInvalidLogicalTime(getTimeManagement()->logicalTimeToString(logicalTime));

      MessageRetractionHandle messageRetractionHandle = getNextMessageRetractionHandle();

      SharedPtr<TimeStampedInteractionMessage> request;
      request = new TimeStampedInteractionMessage;
      request->setFederationHandle(getFederationHandle());
      request->setInteractionClassHandle(interactionClassHandle);
      if (timeRegulationEnabled)
        request->setOrderType(TIMESTAMP);
      else
        request->setOrderType(RECEIVE);
      request->setTransportationType(interactionClass->getTransportationType());
      request->setTag(tag);
      request->setTimeStamp(getTimeManagement()->encodeLogicalTime(logicalTime));
      request->setMessageRetractionHandle(messageRetractionHandle);
      request->getParameterValues().swap(parameterValues);
      send(request);

      return messageRetractionHandle;
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::ObjectInstance* objectInstance = _federate->getObjectInstance(objectInstanceHandle);
      if (!objectInstance)
        Traits::throwObjectInstanceNotKnown(objectInstanceHandle.toString());
      if (!objectInstance->isOwnedByFederate())
        Traits::throwDeletePrivilegeNotHeld(objectInstanceHandle.toString());

      SharedPtr<DeleteObjectInstanceMessage> request;
      request = new DeleteObjectInstanceMessage;
      request->setFederationHandle(getFederationHandle());
      request->setObjectInstanceHandle(objectInstanceHandle);
      request->setTag(tag);

      send(request);

      // Note that this also sends the unreference message just past the delete
      _releaseObjectInstance(objectInstanceHandle);
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::ObjectInstance* objectInstance = _federate->getObjectInstance(objectInstanceHandle);
      if (!objectInstance)
        Traits::throwObjectInstanceNotKnown(objectInstanceHandle.toString());
      if (!objectInstance->isOwnedByFederate())
        Traits::throwDeletePrivilegeNotHeld(objectInstanceHandle.toString());
      bool timeRegulationEnabled = getTimeManagement()->getTimeRegulationEnabled();
      if (timeRegulationEnabled && getTimeManagement()->logicalTimeAlreadyPassed(logicalTime))
        Traits::throwInvalidLogicalTime(getTimeManagement()->logicalTimeToString(logicalTime));

      MessageRetractionHandle messageRetractionHandle = getNextMessageRetractionHandle();

      SharedPtr<TimeStampedDeleteObjectInstanceMessage> request;
      request = new TimeStampedDeleteObjectInstanceMessage;
      request->setFederationHandle(getFederationHandle());
      request->setObjectInstanceHandle(objectInstanceHandle);
      if (timeRegulationEnabled)
        request->setOrderType(TIMESTAMP);
      else
        request->setOrderType(RECEIVE);
      request->setTag(tag);
      request->setTimeStamp(getTimeManagement()->encodeLogicalTime(logicalTime));
      request->setMessageRetractionHandle(messageRetractionHandle);

      send(request);

      // FIXME do this once the logical time has passed
      // When implementing message retraction this needs to be delayed probably ...
      // Note that this also sends the unreference message just past the delete
      // _releaseObjectInstance(objectInstanceHandle);

      return messageRetractionHandle;
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::ObjectInstance* objectInstance = _federate->getObjectInstance(objectInstanceHandle);
      if (!objectInstance)
        Traits::throwObjectInstanceNotKnown(objectInstanceHandle.toString());
      if (objectInstance->ownsAnyAttribute())
        Traits::throwFederateOwnsAttributes(objectInstanceHandle.toString());

      Traits::throwRTIinternalError("Not implemented!");

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::ObjectInstance* objectInstance = _federate->getObjectInstance(objectInstanceHandle);
      if (!objectInstance)
        Traits::throwObjectInstanceNotKnown(objectInstanceHandle.toString());
      for (AttributeHandleSet::const_iterator j = attributeHandleSet.begin(); j != attributeHandleSet.end(); ++j) {
        const Federate::InstanceAttribute* attribute = objectInstance->getInstanceAttribute(j->getHandle());
        if (!attribute)
          Traits::throwAttributeNotDefined(j->toString());
        if (!attribute->getIsOwnedByFederate())
          Traits::throwAttributeNotOwned(j->toString());
      }
      for (AttributeHandleSet::const_iterator j = attributeHandleSet.begin(); j != attributeHandleSet.end(); ++j) {
        objectInstance->getInstanceAttribute(j->getHandle())->setTransportationType(transportationType);
      }
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::InteractionClass* interactionClass = _federate->getInteractionClass(interactionClassHandle);
      if (!interactionClass)
        Traits::throwInteractionClassNotDefined(interactionClassHandle.toString());
      if (!interactionClass->isPublished())
        Traits::throwInteractionClassNotPublished(interactionClass->getName());
      interactionClass->setTransportationType(transportationType);
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::ObjectInstance* objectInstance = _federate->getObjectInstance(objectInstanceHandle);
      if (!objectInstance)
        Traits::throwObjectInstanceNotKnown(objectInstanceHandle.toString());
      for (AttributeHandleSet::const_iterator j = attributeHandleSet.begin(); j != attributeHandleSet.end(); ++j) {
        if (!objectInstance->getInstanceAttribute(j->getHandle()))
          Traits::throwAttributeNotDefined(j->toString());
      }

      AttributeHandleVector attributeHandleVector;
      attributeHandleVector.reserve(attributeHandleSet.size());
      for (AttributeHandleSet::const_iterator j = attributeHandleSet.begin(); j != attributeHandleSet.end(); ++j) {
        attributeHandleVector.push_back(*j);
      }

      SharedPtr<RequestAttributeUpdateMessage> request;
      request = new RequestAttributeUpdateMessage;
      request->setFederationHandle(getFederationHandle());
      request->setObjectInstanceHandle(objectInstanceHandle);
      request->getAttributeHandles().swap(attributeHandleVector);
      request->setTag(tag);

      send(request);

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::ObjectClass* objectClass = _federate->getObjectClass(objectClassHandle);
      if (!objectClass)
        Traits::throwObjectClassNotDefined(objectClassHandle.toString());
      for (AttributeHandleSet::const_iterator j = attributeHandleSet.begin(); j != attributeHandleSet.end(); ++j) {
        if (!objectClass->getAttribute(*j))
          Traits::throwAttributeNotDefined(j->toString());
      }

      AttributeHandleVector attributeHandleVector;
      attributeHandleVector.reserve(attributeHandleSet.size());
      for (AttributeHandleSet::const_iterator j = attributeHandleSet.begin(); j != attributeHandleSet.end(); ++j) {
        attributeHandleVector.push_back(*j);
      }

      SharedPtr<RequestClassAttributeUpdateMessage> request;
      request = new RequestClassAttributeUpdateMessage;
      request->setFederationHandle(getFederationHandle());
      request->setObjectClassHandle(objectClassHandle);
      request->getAttributeHandles().swap(attributeHandleVector);
      request->setTag(tag);

      send(request);

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      const Federate::ObjectInstance* objectInstance = _federate->getObjectInstance(objectInstanceHandle);
      if (!objectInstance)
        Traits::throwObjectInstanceNotKnown(objectInstanceHandle.toString());
      const Federate::InstanceAttribute* attribute = objectInstance->getInstanceAttribute(attributeHandle);
      if (!attribute)
        Traits::throwAttributeNotDefined(attributeHandle.toString());
      return attribute->getIsOwnedByFederate();
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
      if (!_timeManagement.valid())
        Traits::throwFederateNotExecutionMember();
      if (_timeManagement->getTimeRegulationEnabled())
        Traits::throwTimeRegulationAlreadyEnabled();
      if (_timeManagement->getTimeRegulationEnablePending())
        Traits::throwRequestForTimeRegulationPending();
      if (_timeManagement->getTimeAdvancePending())
        Traits::throwInTimeAdvancingState();
      if (!_timeManagement->isPositiveLogicalTimeInterval(lookahead))
        Traits::throwInvalidLookahead(_timeManagement->logicalTimeIntervalToString(lookahead));
      if (!_federate->getPermitTimeRegulation())
        Traits::throwRTIinternalError("Enable time regulation not permitted due to server policy!");
      _timeManagement->enableTimeRegulation(*this, lookahead);
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  // the RTI13 variant
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
      if (!_timeManagement.valid())
        Traits::throwFederateNotExecutionMember();
      if (_timeManagement->getTimeRegulationEnabled())
        Traits::throwTimeRegulationAlreadyEnabled();
      if (_timeManagement->getTimeRegulationEnablePending())
        Traits::throwRequestForTimeRegulationPending();
      if (_timeManagement->getTimeAdvancePending())
        Traits::throwInTimeAdvancingState();
      if (_timeManagement->isLogicalTimeStrictlyInThePast(logicalTime))
        Traits::throwInvalidLogicalTime(_timeManagement->logicalTimeToString(logicalTime));
      if (!_timeManagement->isPositiveLogicalTimeInterval(lookahead))
        Traits::throwInvalidLookahead(_timeManagement->logicalTimeIntervalToString(lookahead));
      if (!_federate->getPermitTimeRegulation())
        Traits::throwRTIinternalError("Enable time regulation not permitted due to server policy!");
      _timeManagement->enableTimeRegulation(*this, logicalTime, lookahead);
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
      if (!_timeManagement.valid())
        Traits::throwFederateNotExecutionMember();
      if (!_timeManagement->getTimeRegulationEnabled())
        Traits::throwTimeRegulationIsNotEnabled();
      _timeManagement->disableTimeRegulation(*this);
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
      if (!_timeManagement.valid())
        Traits::throwFederateNotExecutionMember();
      if (_timeManagement->getTimeConstrainedEnabled())
        Traits::throwTimeConstrainedAlreadyEnabled();
      if (_timeManagement->getTimeConstrainedEnablePending())
        Traits::throwRequestForTimeConstrainedPending();
      if (_timeManagement->getTimeAdvancePending())
        Traits::throwInTimeAdvancingState();
      _timeManagement->enableTimeConstrained(*this);
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
      if (!_timeManagement.valid())
        Traits::throwFederateNotExecutionMember();
      if (!_timeManagement->getTimeConstrainedEnabled())
        Traits::throwTimeConstrainedIsNotEnabled();
      _timeManagement->disableTimeConstrained(*this);
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
      if (!_timeManagement.valid())
        Traits::throwFederateNotExecutionMember();
      if (_timeManagement->isLogicalTimeInThePast(logicalTime))
        Traits::throwLogicalTimeAlreadyPassed(_timeManagement->logicalTimeToString(logicalTime));
      if (_timeManagement->getTimeAdvancePending())
        Traits::throwInTimeAdvancingState();
      if (_timeManagement->getTimeRegulationEnablePending())
        Traits::throwRequestForTimeRegulationPending();
      if (_timeManagement->getTimeConstrainedEnablePending())
        Traits::throwRequestForTimeConstrainedPending();
      _timeManagement->timeAdvanceRequest(*this, logicalTime, false, false, false);
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
      if (!_timeManagement.valid())
        Traits::throwFederateNotExecutionMember();
      if (_timeManagement->isLogicalTimeInThePast(logicalTime))
        Traits::throwLogicalTimeAlreadyPassed(_timeManagement->logicalTimeToString(logicalTime));
      if (_timeManagement->getTimeAdvancePending())
        Traits::throwInTimeAdvancingState();
      if (_timeManagement->getTimeRegulationEnablePending())
        Traits::throwRequestForTimeRegulationPending();
      if (_timeManagement->getTimeConstrainedEnablePending())
        Traits::throwRequestForTimeConstrainedPending();
      _timeManagement->timeAdvanceRequest(*this, logicalTime, true, false, false);
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
      if (!_timeManagement.valid())
        Traits::throwFederateNotExecutionMember();
      if (_timeManagement->isLogicalTimeInThePast(logicalTime))
        Traits::throwLogicalTimeAlreadyPassed(_timeManagement->logicalTimeToString(logicalTime));
      if (_timeManagement->getTimeAdvancePending())
        Traits::throwInTimeAdvancingState();
      if (_timeManagement->getTimeRegulationEnablePending())
        Traits::throwRequestForTimeRegulationPending();
      if (_timeManagement->getTimeConstrainedEnablePending())
        Traits::throwRequestForTimeConstrainedPending();
      // FIXME
      Traits::throwRTIinternalError("nextMessageRequest is not implemented!");
      _timeManagement->timeAdvanceRequest(*this, logicalTime, false, true, false);
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
      if (!_timeManagement.valid())
        Traits::throwFederateNotExecutionMember();
      if (_timeManagement->isLogicalTimeInThePast(logicalTime))
        Traits::throwLogicalTimeAlreadyPassed(_timeManagement->logicalTimeToString(logicalTime));
      if (_timeManagement->getTimeAdvancePending())
        Traits::throwInTimeAdvancingState();
      if (_timeManagement->getTimeRegulationEnablePending())
        Traits::throwRequestForTimeRegulationPending();
      if (_timeManagement->getTimeConstrainedEnablePending())
        Traits::throwRequestForTimeConstrainedPending();
      // FIXME
      Traits::throwRTIinternalError("nextMessageRequest is not implemented!");
      _timeManagement->timeAdvanceRequest(*this, logicalTime, true, true, false);
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
      if (!_timeManagement.valid())
        Traits::throwFederateNotExecutionMember();
      if (_timeManagement->isLogicalTimeInThePast(logicalTime))
        Traits::throwLogicalTimeAlreadyPassed(_timeManagement->logicalTimeToString(logicalTime));
      if (_timeManagement->getTimeAdvancePending())
        Traits::throwInTimeAdvancingState();
      if (_timeManagement->getTimeRegulationEnablePending())
        Traits::throwRequestForTimeRegulationPending();
      if (_timeManagement->getTimeConstrainedEnablePending())
        Traits::throwRequestForTimeConstrainedPending();
      _timeManagement->timeAdvanceRequest(*this, logicalTime, false, false, true);
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
      if (!_timeManagement.valid())
        Traits::throwFederateNotExecutionMember();
      if (_timeManagement->getAsynchronousDeliveryEnabled())
        Traits::throwAsynchronousDeliveryAlreadyEnabled();
      _timeManagement->setAsynchronousDeliveryEnabled(true);
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
      if (!_timeManagement.valid())
        Traits::throwFederateNotExecutionMember();
      if (!_timeManagement->getAsynchronousDeliveryEnabled())
        Traits::throwAsynchronousDeliveryAlreadyDisabled();
      _timeManagement->setAsynchronousDeliveryEnabled(false);
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
      if (!_timeManagement.valid())
        Traits::throwFederateNotExecutionMember();
      return _timeManagement->queryGALT(*this, logicalTime);
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
      if (!_timeManagement.valid())
        Traits::throwFederateNotExecutionMember();
      _timeManagement->queryLogicalTime(*this, logicalTime);
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
      if (!_timeManagement.valid())
        Traits::throwFederateNotExecutionMember();
      return _timeManagement->queryLITS(*this, logicalTime);
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
      if (!_timeManagement.valid())
        Traits::throwFederateNotExecutionMember();
      if (!_timeManagement->getTimeRegulationEnabled())
        Traits::throwTimeRegulationIsNotEnabled();
      if (!_timeManagement->isPositiveLogicalTimeInterval(lookahead))
        Traits::throwInvalidLookahead(_timeManagement->logicalTimeIntervalToString(lookahead));
      if (_timeManagement->getTimeAdvancePending())
        Traits::throwInTimeAdvancingState();
      _timeManagement->modifyLookahead(*this, lookahead);
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
      if (!_timeManagement.valid())
        Traits::throwFederateNotExecutionMember();
      if (!_timeManagement->getTimeRegulationEnabled())
        Traits::throwTimeRegulationIsNotEnabled();
      _timeManagement->queryLookahead(*this, logicalTimeInterval);
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      if (!messageRetractionHandle.valid())
        Traits::throwInvalidRetractionHandle(messageRetractionHandle.toString());
      if (!messageRetractionHandle.getFederateHandle() != getFederateHandle())
        Traits::throwInvalidRetractionHandle(messageRetractionHandle.toString());
      if (!getTimeManagement()->getTimeRegulationEnabled())
        Traits::throwTimeRegulationIsNotEnabled();

      Traits::throwRTIinternalError("Message retraction is not implemented!");

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::ObjectInstance* objectInstance = _federate->getObjectInstance(objectInstanceHandle);
      if (!objectInstance)
        Traits::throwObjectInstanceNotKnown(objectInstanceHandle.toString());
      for (AttributeHandleSet::const_iterator j = attributeHandleSet.begin(); j != attributeHandleSet.end(); ++j) {
        const Federate::InstanceAttribute* instanceAttribute = objectInstance->getInstanceAttribute(*j);
        if (!instanceAttribute)
          Traits::throwAttributeNotDefined(j->toString());
        if (!instanceAttribute->getIsOwnedByFederate())
          Traits::throwAttributeNotOwned(j->toString());
      }
      for (AttributeHandleSet::const_iterator j = attributeHandleSet.begin(); j != attributeHandleSet.end(); ++j) {
        objectInstance->getInstanceAttribute(*j)->setOrderType(orderType);
      }
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::InteractionClass* interactionClass = _federate->getInteractionClass(interactionClassHandle);
      if (!interactionClass)
        Traits::throwInteractionClassNotDefined(interactionClassHandle.toString());
      if (!interactionClass->isPublished())
        Traits::throwInteractionClassNotPublished(interactionClass->getName());
      interactionClass->setOrderType(orderType);
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      for (DimensionHandleSet::const_iterator i = dimensionHandleSet.begin(); i != dimensionHandleSet.end(); ++i) {
        if (!_federate->getDimension(*i))
          Traits::throwInvalidDimensionHandle(i->toString());
      }
      RegionHandle regionHandle = _federate->insertLocalRegion(dimensionHandleSet);

      SharedPtr<InsertRegionMessage> request = new InsertRegionMessage;
      request->setFederationHandle(getFederationHandle());
      RegionHandleDimensionHandleSetPairVector value(1);
      value[0].first = regionHandle;
      value[0].second = dimensionHandleSet;
      request->getRegionHandleDimensionHandleSetPairVector().swap(value);
      send(request);

      return regionHandle;

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      for (RegionHandleSet::const_iterator i = regionHandleSet.begin(); i != regionHandleSet.end(); ++i) {
        if (!i->valid())
          Traits::throwInvalidRegion(i->toString());
        if (!_federate->getRegion(*i))
          Traits::throwInvalidRegion(i->toString());
        if (!i->getFederateHandle() != getFederateHandle())
          Traits::throwRegionNotCreatedByThisFederate(i->toString());
      }

      Traits::throwRTIinternalError();

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      if (!regionHandle.valid())
        Traits::throwInvalidRegion(regionHandle.toString());
      Federate::RegionData* region = _federate->getRegion(regionHandle);
      if (!region)
        Traits::throwInvalidRegion(regionHandle.toString());
      if (!regionHandle.getFederateHandle() != getFederateHandle())
        Traits::throwRegionNotCreatedByThisFederate(regionHandle.toString());

      // FIXME check for in use
      _federate->eraseLocalRegion(regionHandle);

      SharedPtr<EraseRegionMessage> request = new EraseRegionMessage;
      request->setFederationHandle(getFederationHandle());
      RegionHandleVector value(1);
      value[0] = regionHandle;
      request->getRegionHandleVector().swap(value);

      send(request);

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError();
      return ObjectInstanceHandle();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError();
      return ObjectInstanceHandle();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError();
      return MessageRetractionHandle();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      return _federate->getAutomaticResignDirective();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      switch (resignAction) {
      case UNCONDITIONALLY_DIVEST_ATTRIBUTES:
      case DELETE_OBJECTS:
      case CANCEL_PENDING_OWNERSHIP_ACQUISITIONS:
      case DELETE_OBJECTS_THEN_DIVEST:
      case CANCEL_THEN_DELETE_THEN_DIVEST:
      case NO_ACTION:
        break;
      default:
        Traits::throwInvalidResignAction();
      }
      _federate->setAutomaticResignDirective(resignAction);

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      ObjectClassHandle objectClassHandle = _federate->getObjectClassHandle(name);
      if (!objectClassHandle.valid())
        Traits::throwNameNotFound(name);
      return objectClassHandle;
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      const Federate::ObjectClass* objectClass = _federate->getObjectClass(objectClassHandle);
      if (!objectClass)
        Traits::throwInvalidObjectClassHandle(objectClassHandle.toString());
      return objectClass->getName();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      const Federate::ObjectClass* objectClass = _federate->getObjectClass(objectClassHandle);
      if (!objectClass)
        Traits::throwInvalidObjectClassHandle(objectClassHandle.toString());
      AttributeHandle attributeHandle = objectClass->getAttributeHandle(name);
      if (!attributeHandle.valid())
        Traits::throwNameNotFound(name);
      return attributeHandle;
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      const Federate::ObjectClass* objectClass = _federate->getObjectClass(objectClassHandle);
      if (!objectClass)
        Traits::throwInvalidObjectClassHandle(objectClassHandle.toString());
      const Federate::Attribute* attribute = objectClass->getAttribute(attributeHandle);
      if (!attribute)
        Traits::throwInvalidAttributeHandle(attributeHandle.toString());
      return attribute->getName();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      InteractionClassHandle interactionClassHandle;
      interactionClassHandle = _federate->getInteractionClassHandle(name);
      if (!interactionClassHandle.valid())
        Traits::throwNameNotFound(name);
      return interactionClassHandle;
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      const Federate::InteractionClass* interactionClass = _federate->getInteractionClass(interactionClassHandle);
      if (!interactionClass)
        Traits::throwInvalidInteractionClassHandle(interactionClassHandle.toString());
      return interactionClass->getName();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      const Federate::InteractionClass* interactionClass = _federate->getInteractionClass(interactionClassHandle);
      if (!interactionClass)
        Traits::throwInvalidInteractionClassHandle(interactionClassHandle.toString());
      ParameterHandle parameterHandle;
      parameterHandle = interactionClass->getParameterHandle(name);
      if (!parameterHandle.valid())
        Traits::throwNameNotFound(name);
      return parameterHandle;
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      const Federate::InteractionClass* interactionClass = _federate->getInteractionClass(interactionClassHandle);
      if (!interactionClass)
        Traits::throwInvalidInteractionClassHandle(interactionClassHandle.toString());
      const Federate::Parameter* parameter = interactionClass->getParameter(parameterHandle);
      if (!parameter)
        Traits::throwInvalidParameterHandle(parameterHandle.toString());
      return parameter->getName();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      ObjectInstanceHandle objectInstanceHandle = _federate->getObjectInstanceHandle(name);
      if (!objectInstanceHandle.valid())
        Traits::throwObjectInstanceNotKnown(name);
      return objectInstanceHandle;
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      const Federate::ObjectInstance* objectInstance = _federate->getObjectInstance(objectInstanceHandle);
      if (!objectInstance)
        Traits::throwObjectInstanceNotKnown(objectInstanceHandle.toString());
      return objectInstance->getName();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      DimensionHandle dimensionHandle = _federate->getDimensionHandle(name);
      if (!dimensionHandle.valid())
        Traits::throwNameNotFound(name);
      return dimensionHandle;
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      const Federate::Dimension* dimension = _federate->getDimension(dimensionHandle);
      if (!dimension)
        Traits::throwInvalidDimensionHandle(dimensionHandle.toString());
      return dimension->getName();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      const Federate::Dimension* dimension = _federate->getDimension(dimensionHandle);
      if (!dimension)
        Traits::throwInvalidDimensionHandle(dimensionHandle.toString());
      return dimension->getUpperBound();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      const Federate::ObjectClass* objectClass = _federate->getObjectClass(objectClassHandle);
      if (!objectClass)
        Traits::throwInvalidObjectClassHandle(objectClassHandle.toString());
      const Federate::Attribute* attribute = objectClass->getAttribute(attributeHandle);
      if (!attribute)
        Traits::throwInvalidAttributeHandle(attributeHandle.toString());
      return attribute->getDimensionHandleSet();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      const Federate::ObjectInstance* objectInstance = _federate->getObjectInstance(objectInstanceHandle);
      if (!objectInstance)
        Traits::throwObjectInstanceNotKnown(objectInstanceHandle.toString());
      return objectInstance->getObjectClassHandle();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      const Federate::InteractionClass* interactionClass = _federate->getInteractionClass(interactionClassHandle);
      if (!interactionClass)
        Traits::throwInvalidInteractionClassHandle(interactionClassHandle.toString());
      return interactionClass->getDimensionHandleSet();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      const TransportationType* transportationType = _federate->getTransportationType(name);
      if (!transportationType)
        Traits::throwInvalidTransportationName(name);
      return *transportationType;
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      const std::string* name = _federate->getTransportationName(transportationType);
      if (!name)
        Traits::throwInvalidTransportationType();
      return *name;
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      const OrderType* orderType = _federate->getOrderType(name);
      if (!orderType)
        Traits::throwInvalidOrderName(name);
      return *orderType;
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      const std::string* name = _federate->getOrderName(orderType);
      if (!name)
        Traits::throwInvalidOrderType();
      return *name;
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      if (_federate->getObjectClassRelevanceAdvisorySwitchEnabled())
        Traits::throwObjectClassRelevanceAdvisorySwitchIsOn();
      _federate->setObjectClassRelevanceAdvisorySwitchEnabled(true);

      Traits::throwRTIinternalError();

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      if (!_federate->getObjectClassRelevanceAdvisorySwitchEnabled())
        Traits::throwObjectClassRelevanceAdvisorySwitchIsOff();
      _federate->setObjectClassRelevanceAdvisorySwitchEnabled(false);

      Traits::throwRTIinternalError();

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      if (_federate->getAttributeRelevanceAdvisorySwitchEnabled())
        Traits::throwAttributeRelevanceAdvisorySwitchIsOn();
      _federate->setAttributeRelevanceAdvisorySwitchEnabled(true);

      Traits::throwRTIinternalError();

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      if (!_federate->getAttributeRelevanceAdvisorySwitchEnabled())
        Traits::throwAttributeRelevanceAdvisorySwitchIsOff();
      _federate->setAttributeRelevanceAdvisorySwitchEnabled(false);

      Traits::throwRTIinternalError();

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      if (_federate->getAttributeScopeAdvisorySwitchEnabled())
        Traits::throwAttributeScopeAdvisorySwitchIsOn();
      _federate->setAttributeScopeAdvisorySwitchEnabled(true);

      Traits::throwRTIinternalError();

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      if (!_federate->getAttributeScopeAdvisorySwitchEnabled())
        Traits::throwAttributeScopeAdvisorySwitchIsOff();
      _federate->setAttributeScopeAdvisorySwitchEnabled(false);

      Traits::throwRTIinternalError();

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      if (_federate->getInteractionRelevanceAdvisorySwitchEnabled())
        Traits::throwInteractionRelevanceAdvisorySwitchIsOn();
      _federate->setInteractionRelevanceAdvisorySwitchEnabled(true);

      Traits::throwRTIinternalError();

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      if (!_federate->getInteractionRelevanceAdvisorySwitchEnabled())
        Traits::throwInteractionRelevanceAdvisorySwitchIsOff();
      _federate->setInteractionRelevanceAdvisorySwitchEnabled(false);

      Traits::throwRTIinternalError();

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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      const Federate::RegionData* region = _federate->getRegion(regionHandle);
      if (!region)
        Traits::throwInvalidRegion(regionHandle.toString());
      return region->getDimensionHandleSet();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      const Federate::RegionData* region = _federate->getRegion(regionHandle);
      if (!region)
        Traits::throwInvalidRegion(regionHandle.toString());
      if (!region->containsDimensionHandle(dimensionHandle))
        Traits::throwRegionDoesNotContainSpecifiedDimension();
      return region->getRegion().getRangeBounds(dimensionHandle);
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Federate::RegionData* region = _federate->getRegion(regionHandle);
      if (!region)
        Traits::throwInvalidRegion(regionHandle.toString());
      if (regionHandle.getFederateHandle() != getFederateHandle())
        Traits::throwRegionNotCreatedByThisFederate(regionHandle.toString());
      if (!region->containsDimensionHandle(dimensionHandle))
        Traits::throwRegionDoesNotContainSpecifiedDimension();
      const Federate::Dimension* dimension = _federate->getDimension(dimensionHandle);
      OpenRTIAssert(dimension);
      if (dimension->getUpperBound() < rangeBounds.getUpperBound())
        Traits::throwInvalidRangeBound();
      region->getRegion().setRangeBounds(dimensionHandle, rangeBounds);
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      return federateHandle.getHandle();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      return serviceGroup;
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();

      Clock clock = Clock::now();
      clock += Clock::fromSeconds(approximateMinimumTimeInSeconds);
      return dispatchCallback(clock);
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();

      Clock clock = Clock::now();
      clock += Clock::fromSeconds(approximateMinimumTimeInSeconds);
      if (!dispatchCallback(clock))
        return false;

      clock = Clock::now() + Clock::fromSeconds(approximateMaximumTimeInSeconds);
      do {
        if (!dispatchCallback(Clock::initial()))
          return false;
      } while (Clock::now() <= clock);

      return true;
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError();
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
      if (!_federate.valid())
        Traits::throwFederateNotExecutionMember();
      Traits::throwRTIinternalError();
    } catch (const typename Traits::Exception&) {
      throw;
    } catch (const OpenRTI::IgnoredError&) {
    } catch (const OpenRTI::Exception& e) {
      Traits::throwRTIinternalError(e.getReason());
    }
  }

  void _requestObjectInstanceHandles(unsigned count)
  {
    SharedPtr<ObjectInstanceHandlesRequestMessage> request;
    request = new ObjectInstanceHandlesRequestMessage;
    request->setFederationHandle(getFederationHandle());
    request->setFederateHandle(getFederateHandle());
    request->setCount(count);
    send(request);
  }

  ObjectInstanceHandleNamePair
  _getFreeObjectInstanceHandleNamePair()
  {
    // Usually we already have some handles locally available, but for each new one,
    // start requesting the next from the server, this way we should stay asyncronous for ever.
    // May be the initial amount of object handles should be configuration option ...
    _requestObjectInstanceHandles(1);

    if (!_federate->haveFreeObjectInstanceHandleNamePair()) {
      Clock timeout = Clock::now() + Clock::fromSeconds(60);
      while (!_federate->haveFreeObjectInstanceHandleNamePair()) {
        if (receiveAndDispatchInternalMessage(timeout))
          continue;
        if (timeout < Clock::now())
          Traits::throwRTIinternalError("Timeout while waiting for free object handles.");
      }
    }

    return _federate->takeFreeObjectInstanceHandleNamePair();
  }

  void _releaseObjectInstance(const ObjectInstanceHandle& objectInstanceHandle)
  {
    // Remove the instance from the object model
    _federate->eraseObjectInstance(objectInstanceHandle);

    // Unreference the object instance handle resource
    SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> message;
    message = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
    message->setFederationHandle(getFederationHandle());
    message->getObjectInstanceHandleVector().push_back(objectInstanceHandle);
    send(message);
  }

  // Get a next message retraction handle
  MessageRetractionHandle getNextMessageRetractionHandle()
  { return MessageRetractionHandle(getFederateHandle(), getTimeManagement()->getNextMessageRetractionSerial()); }

  class OPENRTI_LOCAL _CallbackDispatchFunctor {
  public:
    _CallbackDispatchFunctor(Ambassador& ambassador) :
      _ambassador(ambassador)
    { }
    template<typename M>
    void operator()(const M& message) const
    { _ambassador.acceptCallbackMessage(message); }
  private:
    Ambassador& _ambassador;
  };

  bool _dispatchCallbackMessage()
  {
    _CallbackDispatchFunctor callbackDispatchfunctor(*this);
    FunctorMessageDispatcher<_CallbackDispatchFunctor> dispatcher(callbackDispatchfunctor);
    return InternalAmbassador::_dispatchCallbackMessage(dispatcher);
  }

  // Here we just should see messages which do callbacks in the ambassador
  bool dispatchCallback(const Clock& clock)
  {
    while (!_dispatchCallbackMessage()) {
      if (!receiveAndDispatchInternalMessage(clock))
        return false;
    }
    while (!_callbackMessageAvailable()) {
      if (!receiveAndDispatchInternalMessage(Clock::initial()))
        return false;
    }
    return true;
  }

  /// Default internal message processing method
  void acceptCallbackMessage(const AbstractMessage& message)
  { Traits::throwRTIinternalError("Unexpected message in callback message processing!"); }

  void acceptCallbackMessage(const ConnectionLostMessage& message)
  { connectionLost(message.getFaultDescription()); }
  void acceptCallbackMessage(const EnumerateFederationExecutionsResponseMessage& message)
  { reportFederationExecutions(message.getFederationExecutionInformationVector()); }

  void acceptCallbackMessage(const RegisterFederationSynchronizationPointResponseMessage& message)
  { synchronizationPointRegistrationResponse(message.getLabel(), message.getRegisterFederationSynchronizationPointResponseType()); }
  void acceptCallbackMessage(const AnnounceSynchronizationPointMessage& message)
  {
    if (!_federate.valid())
      return;
    _federate->insertAnnouncedFederationSynchonizationLabel(message.getLabel());
    announceSynchronizationPoint(message.getLabel(), message.getTag());
  }
  void acceptCallbackMessage(const FederationSynchronizedMessage& message)
  {
    if (!_federate.valid())
      return;
    federationSynchronized(message.getLabel());
    _federate->eraseAnnouncedFederationSynchonizationLabel(message.getLabel());
  }
  void acceptCallbackMessage(const RegistrationForObjectClassMessage& message)
  { registrationForObjectClass(message.getObjectClassHandle(), message.getStart()); }
  void acceptCallbackMessage(const TurnInteractionsOnMessage& message)
  { turnInteractionsOn(message.getInteractionClassHandle(), message.getOn()); }

  void acceptCallbackMessage(const ReserveObjectInstanceNameResponseMessage& message)
  {
    if (!_federate.valid())
      return;
    if (message.getSuccess())
      _federate->insertReservedNameObjectInstanceHandlePair(message.getObjectInstanceHandleNamePair().second, message.getObjectInstanceHandleNamePair().first);
    objectInstanceNameReservation(message);
  }
  void acceptCallbackMessage(const ReserveMultipleObjectInstanceNameResponseMessage& message)
  {
    if (!_federate.valid())
      return;
    if (message.getSuccess()) {
      for (ObjectInstanceHandleNamePairVector::const_iterator i = message.getObjectInstanceHandleNamePairVector().begin();
           i != message.getObjectInstanceHandleNamePairVector().end(); ++i) {
        _federate->insertReservedNameObjectInstanceHandlePair(i->second, i->first);
      }
    }
    objectInstanceNameReservation(message);
  }
  void acceptCallbackMessage(const InsertObjectInstanceMessage& message)
  {
    if (!_federate.valid())
      return;
    ObjectClassHandle objectClassHandle = message.getObjectClassHandle();
    Federate::ObjectClass* objectClass = _federate->getObjectClass(objectClassHandle);
    if (!objectClass)
      return;
    while (Unsubscribed == objectClass->getEffectiveSubscriptionType()) {
      objectClassHandle = objectClass->getParentObjectClassHandle();
      objectClass = _federate->getObjectClass(objectClassHandle);
      if (!objectClass)
        return;
    }
    // Ok we get duplicate inserts. FIXME investigate this
    if (_federate->getObjectInstance(message.getObjectInstanceHandle()))
      return;
    _federate->insertObjectInstance(message.getObjectInstanceHandle(), message.getName(), objectClassHandle, false);
    discoverObjectInstance(message.getObjectInstanceHandle(), objectClassHandle, message.getName());
  }
  void acceptCallbackMessage(const DeleteObjectInstanceMessage& message)
  {
    if (!_federate.valid())
      return;
    ObjectInstanceHandle objectInstanceHandle = message.getObjectInstanceHandle();
    Federate::ObjectInstance* objectInstance = _federate->getObjectInstance(objectInstanceHandle);
    if (!objectInstance)
      return;
    Federate::ObjectClass* objectClass = _federate->getObjectClass(objectInstance->getObjectClassHandle());
    if (objectClass) {
      if (Unsubscribed != objectClass->getEffectiveSubscriptionType()) {
        removeObjectInstance(message);
      }
    }
    _releaseObjectInstance(message.getObjectInstanceHandle());
  }
  void acceptCallbackMessage(const TimeStampedDeleteObjectInstanceMessage& message)
  {
    if (!_federate.valid())
      return;
    if (!_timeManagement.valid())
      return;
    ObjectInstanceHandle objectInstanceHandle = message.getObjectInstanceHandle();
    Federate::ObjectInstance* objectInstance = _federate->getObjectInstance(objectInstanceHandle);
    if (!objectInstance)
      return;
    if (Federate::ObjectClass* objectClass = _federate->getObjectClass(objectInstance->getObjectClassHandle())) {
      if (Unsubscribed != objectClass->getEffectiveSubscriptionType()) {
        _timeManagement->removeObjectInstance(*this, message);
      }
    }
    _releaseObjectInstance(message.getObjectInstanceHandle());
  }
  void acceptCallbackMessage(const AttributeUpdateMessage& message)
  {
    if (!_federate.valid())
      return;
    // Look for the known object class of this object instance.
    // Is required for the right subset of attributes that are reflected
    ObjectInstanceHandle objectInstanceHandle = message.getObjectInstanceHandle();
    Federate::ObjectInstance* objectInstance = _federate->getObjectInstance(objectInstanceHandle);
    if (!objectInstance)
      return;
    Federate::ObjectClass* objectClass = _federate->getObjectClass(objectInstance->getObjectClassHandle());
    if (!objectClass)
      return;
    reflectAttributeValues(*objectClass, message);
  }
  void acceptCallbackMessage(const TimeStampedAttributeUpdateMessage& message)
  {
    if (!_federate.valid())
      return;
    if (!_timeManagement.valid())
      return;
    // Look for the known object class of this object instance.
    // Is required for the right subset of attributes that are reflected
    ObjectInstanceHandle objectInstanceHandle = message.getObjectInstanceHandle();
    Federate::ObjectInstance* objectInstance = _federate->getObjectInstance(objectInstanceHandle);
    if (!objectInstance)
      return;
    Federate::ObjectClass* objectClass = _federate->getObjectClass(objectInstance->getObjectClassHandle());
    if (!objectClass)
      return;
    _timeManagement->reflectAttributeValues(*this, *objectClass, message);
  }
  void acceptCallbackMessage(const InteractionMessage& message)
  {
    if (!_federate.valid())
      return;
    InteractionClassHandle interactionClassHandle = message.getInteractionClassHandle();
    Federate::InteractionClass* interactionClass = _federate->getInteractionClass(interactionClassHandle);
    if (!interactionClass)
      return;
    // Subscriptions can race against sending them to this federate
    // FIXME: store the effective next interaction class that is subscribed for all interaction classes.
    // This would avoid this loop
    while (Unsubscribed == interactionClass->getSubscriptionType()) {
      interactionClassHandle = interactionClass->getParentInteractionClassHandle();
      interactionClass = _federate->getInteractionClass(interactionClassHandle);
      if (!interactionClass)
        return;
    }
    receiveInteraction(interactionClassHandle, *interactionClass, message);
  }
  void acceptCallbackMessage(const TimeStampedInteractionMessage& message)
  {
    if (!_federate.valid())
      return;
    if (!_timeManagement.valid())
      return;
    InteractionClassHandle interactionClassHandle = message.getInteractionClassHandle();
    Federate::InteractionClass* interactionClass = _federate->getInteractionClass(interactionClassHandle);
    if (!interactionClass)
      return;
    // Subscriptions can race against sending them to this federate
    // FIXME: store the effective next interaction class that is subscribed for all interaction classes.
    // This would avoid this loop
    while (Unsubscribed == interactionClass->getSubscriptionType()) {
      interactionClassHandle = interactionClass->getParentInteractionClassHandle();
      interactionClass = _federate->getInteractionClass(interactionClassHandle);
      if (!interactionClass)
        return;
    }
    _timeManagement->receiveInteraction(*this, interactionClassHandle, *interactionClass, message);
  }
  void acceptCallbackMessage(const TimeConstrainedEnabledMessage& message)
  {
    if (!_timeManagement.valid())
      return;
    _timeManagement->acceptCallbackMessage(*this, message);
  }
  void acceptCallbackMessage(const TimeRegulationEnabledMessage& message)
  {
    if (!_timeManagement.valid())
      return;
    _timeManagement->acceptCallbackMessage(*this, message);
  }
  void acceptCallbackMessage(const TimeAdvanceGrantedMessage& message)
  {
    if (!_timeManagement.valid())
      return;
    _timeManagement->acceptCallbackMessage(*this, message);
  }
  void acceptCallbackMessage(const RequestAttributeUpdateMessage& message)
  {
    if (!_federate.valid())
      return;
    // FIXME: can fail. Check that already in the internal processing
    ObjectInstanceHandle objectInstanceHandle = message.getObjectInstanceHandle();
    Federate::ObjectInstance* objectInstance = _federate->getObjectInstance(objectInstanceHandle);
    if (!objectInstance)
      return;
    AttributeHandleVector attributeHandleSet;
    for (AttributeHandleVector::const_iterator j = message.getAttributeHandles().begin(); j != message.getAttributeHandles().end(); ++j) {
      Federate::InstanceAttribute* attribute = objectInstance->getInstanceAttribute(j->getHandle());
      if (attribute)
        continue;
      if (!attribute->getIsOwnedByFederate())
        continue;
      attributeHandleSet.reserve(message.getAttributeHandles().size());
      attributeHandleSet.push_back(*j);
    }
    if (attributeHandleSet.empty())
      return;
    provideAttributeValueUpdate(objectInstanceHandle, attributeHandleSet, message.getTag());
  }



  // The callback into the binding concrete implementation.
  virtual void connectionLost(const std::string& faultDescription)
    throw ()
  { }

  virtual void reportFederationExecutions(const FederationExecutionInformationVector& theFederationExecutionInformationList)
    throw ()
  { }

  virtual void synchronizationPointRegistrationResponse(const std::string& label, RegisterFederationSynchronizationPointResponseType reason)
    throw () = 0;
  virtual void announceSynchronizationPoint(const std::string& label, const VariableLengthData& tag)
    throw () = 0;
  virtual void federationSynchronized(const std::string& label)
    throw () = 0;

  virtual void registrationForObjectClass(ObjectClassHandle objectClassHandle, bool start)
    throw () = 0;

  virtual void turnInteractionsOn(InteractionClassHandle interactionClassHandle, bool on)
    throw () = 0;

  virtual void objectInstanceNameReservation(const ReserveObjectInstanceNameResponseMessage&)
    throw () = 0;
  virtual void objectInstanceNameReservation(const ReserveMultipleObjectInstanceNameResponseMessage&)
    throw () = 0;

  // 6.5
  virtual void discoverObjectInstance(ObjectInstanceHandle objectInstanceHandle, ObjectClassHandle objectClassHandle,
                                      const std::string& objectInstanceName)
    throw () = 0;


  virtual void reflectAttributeValues(const Federate::ObjectClass& objectClass, const AttributeUpdateMessage& message)
    throw () = 0;

  virtual void reflectAttributeValues(const Federate::ObjectClass& objectClass, bool flushQueueMode,
                                      bool timeConstrainedEnabled, const TimeStampedAttributeUpdateMessage& message,
                                      const NativeLogicalTime& logicalTime)
    throw () = 0;

  // 6.11
  virtual void removeObjectInstance(const DeleteObjectInstanceMessage& message)
    throw () = 0;

  virtual void removeObjectInstance(bool flushQueueMode, bool timeConstrainedEnabled, const TimeStampedDeleteObjectInstanceMessage& message, const NativeLogicalTime& logicalTime)
    throw () = 0;

  // 6.9
  virtual void receiveInteraction(const InteractionClassHandle& interactionClassHandle, const Federate::InteractionClass& interactionClass, const InteractionMessage& message)
    throw () = 0;

  virtual void receiveInteraction(const InteractionClassHandle& interactionClassHandle, const Federate::InteractionClass& interactionClass, bool flushQueueMode,
                                  bool timeConstrainedEnabled, const TimeStampedInteractionMessage& message,
                                  const NativeLogicalTime& logicalTime)
    throw () = 0;

  // 6.15
  virtual void attributesInScope(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw () = 0;

  // 6.16
  virtual void attributesOutOfScope(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw () = 0;

  // 6.18
  virtual void provideAttributeValueUpdate(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector,
                                           const VariableLengthData& tag)
    throw () = 0;

  // 6.19
  virtual void turnUpdatesOnForObjectInstance(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw () = 0;

  // 6.20
  virtual void turnUpdatesOffForObjectInstance(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw () = 0;

  // 7.4
  virtual void requestAttributeOwnershipAssumption(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector,
                                                   const VariableLengthData& tag)
    throw () = 0;

  // 7.5
  virtual void requestDivestitureConfirmation(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw () = 0;

  // 7.7
  virtual void attributeOwnershipAcquisitionNotification(ObjectInstanceHandle objectInstanceHandle,
                                                         const AttributeHandleVector& attributeHandleVector,
                                                         const VariableLengthData& tag)
    throw () = 0;

  // 7.10
  virtual void attributeOwnershipUnavailable(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw () = 0;

  // 7.11
  virtual void requestAttributeOwnershipRelease(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector,
                                                const VariableLengthData& tag)
    throw () = 0;

  // 7.15
  virtual void confirmAttributeOwnershipAcquisitionCancellation(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw () = 0;

  // 7.17
  virtual void informAttributeOwnership(ObjectInstanceHandle objectInstanceHandle, AttributeHandle attributeHandle, FederateHandle theOwner)
    throw () = 0;

  virtual void attributeIsNotOwned(ObjectInstanceHandle objectInstanceHandle, AttributeHandle attributeHandle)
    throw () = 0;

  virtual void attributeIsOwnedByRTI(ObjectInstanceHandle objectInstanceHandle, AttributeHandle attributeHandle)
    throw () = 0;

  virtual void timeRegulationEnabled(const NativeLogicalTime& logicalTime)
    throw () = 0;

  virtual void timeConstrainedEnabled(const NativeLogicalTime& logicalTime)
    throw () = 0;

  virtual void timeAdvanceGrant(const NativeLogicalTime& logicalTime)
    throw () = 0;

  virtual void requestRetraction(MessageRetractionHandle messageRetractionHandle)
    throw () = 0;

  virtual TimeManagement<Traits>* createTimeManagement(Federate& federate) = 0;

  virtual Federate* getFederate()
  {
    return _federate.get();
  }
  virtual TimeManagement<Traits>* getTimeManagement()
  {
    return _timeManagement.get();
  }
  virtual void acceptInternalMessage(const InsertFederationExecutionMessage& message)
  {
    _federate = new Federate;
    _federate->setFederationHandle(message.getFederationHandle());
    _federate->setLogicalTimeFactoryName(message.getLogicalTimeFactoryName());
    _federate->insertFOMModuleList(message.getFOMModuleList());

    ConfigurationParameterMap::const_iterator i;
    // time regulation is by default permitted, but may be denied due to parent server policy
    i = message.getConfigurationParameterMap().find("permitTimeRegulation");
    if (i != message.getConfigurationParameterMap().end() && !i->second.empty() && i->second.front() != "true")
      _federate->setPermitTimeRegulation(false);

    _timeManagement = createTimeManagement(*_federate);
  }

private:
  // The federate if available
  SharedPtr<Federate> _federate;
  // The timestamped queues
  SharedPtr<TimeManagement<Traits> > _timeManagement;
};

} // namespace OpenRTI

#endif
