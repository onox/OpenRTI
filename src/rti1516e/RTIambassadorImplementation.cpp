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

// This time, the first include is above the api include.
// the rti1516/Exception header misses that.
#include <iosfwd>
#include <memory>

#include "RTIambassadorImplementation.h"

#include <algorithm>

#include <RTI/RTIambassador.h>
#include <RTI/FederateAmbassador.h>
#include <RTI/LogicalTime.h>
#include <RTI/LogicalTimeInterval.h>
#include <RTI/LogicalTimeFactory.h>
#include <RTI/RangeBounds.h>

#include "Ambassador.h"
#include "LogStream.h"

#include "HandleImplementation.h"
#include "RTI1516ELogicalTimeFactory.h"
#include "RTI1516Einteger64TimeFactory.h"
#include "RTI1516Efloat64TimeFactory.h"
#include "VariableLengthDataImplementation.h"

namespace OpenRTI {

static OpenRTI::CallbackModel translate(rti1516e::CallbackModel callbackModel)
{
  switch (callbackModel) {
  case rti1516e::HLA_IMMEDIATE:
    return OpenRTI::HLA_IMMEDIATE;
  case rti1516e::HLA_EVOKED:
  default:
    return OpenRTI::HLA_EVOKED;
  }
}

static rti1516e::OrderType translate(OpenRTI::OrderType orderType)
{
  switch (orderType) {
  case OpenRTI::TIMESTAMP:
    return rti1516e::TIMESTAMP;
  case OpenRTI::RECEIVE:
  default:
    return rti1516e::RECEIVE;
  }
}

static OpenRTI::OrderType translate(rti1516e::OrderType orderType)
{
  switch (orderType) {
  case rti1516e::TIMESTAMP:
    return OpenRTI::TIMESTAMP;
  case rti1516e::RECEIVE:
  default:
    return OpenRTI::RECEIVE;
  }
}

static rti1516e::TransportationType translate(OpenRTI::TransportationType transportationType)
{
  switch (transportationType) {
  case OpenRTI::BEST_EFFORT:
    return rti1516e::BEST_EFFORT;
  case OpenRTI::RELIABLE:
  default:
    return rti1516e::RELIABLE;
  }
}

static OpenRTI::TransportationType translate(rti1516e::TransportationType transportationType)
{
  switch (transportationType) {
  case rti1516e::BEST_EFFORT:
    return OpenRTI::BEST_EFFORT;
  case rti1516e::RELIABLE:
  default:
    return OpenRTI::RELIABLE;
  }
}

static OpenRTI::ResignAction translate(rti1516e::ResignAction resignAction)
{
  switch (resignAction) {
  case rti1516e::UNCONDITIONALLY_DIVEST_ATTRIBUTES:
    return OpenRTI::UNCONDITIONALLY_DIVEST_ATTRIBUTES;
  case rti1516e::DELETE_OBJECTS:
    return OpenRTI::DELETE_OBJECTS;
  case rti1516e::CANCEL_PENDING_OWNERSHIP_ACQUISITIONS:
    return OpenRTI::CANCEL_PENDING_OWNERSHIP_ACQUISITIONS;
  case rti1516e::DELETE_OBJECTS_THEN_DIVEST:
    return OpenRTI::DELETE_OBJECTS_THEN_DIVEST;
  case rti1516e::CANCEL_THEN_DELETE_THEN_DIVEST:
    return OpenRTI::CANCEL_THEN_DELETE_THEN_DIVEST;
  case rti1516e::NO_ACTION:
    return OpenRTI::NO_ACTION;
  default:
    return OpenRTI::CANCEL_THEN_DELETE_THEN_DIVEST;
  }
}

static rti1516e::ResignAction translate(OpenRTI::ResignAction resignAction)
{
  switch (resignAction) {
  case OpenRTI::UNCONDITIONALLY_DIVEST_ATTRIBUTES:
    return rti1516e::UNCONDITIONALLY_DIVEST_ATTRIBUTES;
  case OpenRTI::DELETE_OBJECTS:
    return rti1516e::DELETE_OBJECTS;
  case OpenRTI::CANCEL_PENDING_OWNERSHIP_ACQUISITIONS:
    return rti1516e::CANCEL_PENDING_OWNERSHIP_ACQUISITIONS;
  case OpenRTI::DELETE_OBJECTS_THEN_DIVEST:
    return rti1516e::DELETE_OBJECTS_THEN_DIVEST;
  case OpenRTI::CANCEL_THEN_DELETE_THEN_DIVEST:
    return rti1516e::CANCEL_THEN_DELETE_THEN_DIVEST;
  case OpenRTI::NO_ACTION:
    return rti1516e::NO_ACTION;
  default:
    return rti1516e::CANCEL_THEN_DELETE_THEN_DIVEST;
  }
}

class OPENRTI_LOCAL RTI1516ETraits {
public:
  // The bindings have different logical times
  typedef rti1516e::LogicalTime NativeLogicalTime;
  typedef rti1516e::LogicalTimeInterval NativeLogicalTimeInterval;

  typedef rti1516e::FederateAmbassador FederateAmbassador;

  // static OpenRTI::FederateHandle fromNative(const rti1516e::FederateHandle& federateHandle)
  // { return rti1516e::FederateHandleFriend::getOpenRTIHandle(federateHandle); }
  // static rti1516e::FederateHandle toNative(const OpenRTI::FederateHandle& federateHandle)
  // { return rti1516e::FederateHandleFriend::createHandle(federateHandle); }

  // static OpenRTI::ObjectClassHandle fromNative(const rti1516e::ObjectClassHandle& objectClassHandle)
  // { return rti1516e::ObjectClassHandleFriend::getOpenRTIHandle(objectClassHandle); }
  // static rti1516e::ObjectClassHandle toNative(const OpenRTI::ObjectClassHandle& objectClassHandle)
  // { return rti1516e::ObjectClassHandleFriend::createHandle(objectClassHandle); }

  // static OpenRTI::InteractionClassHandle fromNative(const rti1516e::InteractionClassHandle& interactionClassHandle)
  // { return rti1516e::InteractionClassHandleFriend::getOpenRTIHandle(interactionClassHandle); }
  // static rti1516e::InsteractionClassHandle toNative(const OpenRTI::InsteractionClassHandle& insteractionClassHandle)
  // { return rti1516e::InsteractionClassHandleFriend::createHandle(insteractionClassHandle); }

  // static OpenRTI::ObjectInstanceHandle fromNative(const rti1516e::ObjectInstanceHandle& objectInstanceHandle)
  // { return rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(objectInstanceHandle); }
  // static rti1516e::ObjectInstanceHandle toNative(const OpenRTI::ObjectInstanceHandle& objectInstanceHandle)
  // { return rti1516e::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle); }

  // static OpenRTI::AttributeHandle fromNative(const rti1516e::AttributeHandle& attributeHandle)
  // { return rti1516e::AttributeHandleFriend::getOpenRTIHandle(attributeHandle); }
  // static rti1516e::AttributeHandle toNative(const OpenRTI::AttributeHandle& attributeHandle)
  // { return rti1516e::AttributeHandleFriend::createHandle(attributeHandle); }

  // static OpenRTI::ParameterHandle fromNative(const rti1516e::ParameterHandle& parameterHandle)
  // { return rti1516e::ParameterHandleFriend::getOpenRTIHandle(parameterHandle); }
  // static rti1516e::ParameterHandle toNative(const OpenRTI::ParameterHandle& parameterHandle)
  // { return rti1516e::ParameterHandleFriend::createHandle(parameterHandle); }

  // static OpenRTI::DimensionHandle fromNative(const rti1516e::DimensionHandle& dimensionHandle)
  // { return rti1516e::DimensionHandleFriend::getOpenRTIHandle(dimensionHandle); }
  // static rti1516e::DimensionHandle toNative(const OpenRTI::DimensionHandle& dimensionHandle)
  // { return rti1516e::DimensionHandleFriend::createHandle(dimensionHandle); }

  // static OpenRTI::RegionHandle fromNative(const rti1516e::RegionHandle& regionHandle)
  // { return rti1516e::RegionHandleFriend::getOpenRTIHandle(regionHandle); }
  // static rti1516e::RegionHandle toNative(const OpenRTI::RegionHandle& regionHandle)
  // { return rti1516e::RegionHandleFriend::createHandle(regionHandle); }

  // static OpenRTI::MessageRetractionHandle fromNative(const rti1516e::MessageRetractionHandle& messageRetractionHandle)
  // { return rti1516e::MessageRetractionHandleFriend::getOpenRTIHandle(messageRetractionHandle); }
  // static rti1516e::MessageRetractionHandle toNative(const OpenRTI::MessageRetractionHandle& messageRetractionHandle)
  // { return rti1516e::MessageRetractionHandleFriend::createHandle(messageRetractionHandle); }


  // // Ok, have here also methods to convert handle sets to handle sets!!!!
  // static void copy(OpenRTI::AttributeHandleSet& dst, const rti1516e::AttributeHandleSet& src)
  // {
  //   for (rti1516e::AttributeHandleSet::const_iterator i = src.begin(); i != src.end(); ++i)
  //     dst.insert(rti1516e::AttributeHandleFriend::getOpenRTIHandle(*i));
  // }
  // static void copy(rti1516e::AttributeHandleSet& dst, const OpenRTI::AttributeHandleSet& src)
  // {
  //   for (OpenRTI::AttributeHandleSet::const_iterator i = src.begin(); i != src.end(); ++i)
  //     dst.insert(rti1516e::AttributeHandleFriend::createHandle(*i));
  // }
  // static void copy(OpenRTI::AttributeHandleVector& dst, const rti1516e::AttributeHandleSet& src)
  // {
  //   dst.reserve(src.size());
  //   for (rti1516e::AttributeHandleSet::const_iterator i = src.begin(); i != src.end(); ++i)
  //     dst.push_back(rti1516e::AttributeHandleFriend::getOpenRTIHandle(*i));
  // }
  // static void copy(rti1516e::AttributeHandleSet& dst, const OpenRTI::AttributeHandleVector& src)
  // {
  //   for (OpenRTI::AttributeHandleVector::const_iterator i = src.begin(); i != src.end(); ++i)
  //     dst.insert(rti1516e::AttributeHandleFriend::createHandle(*i));
  // }

  // The exceptions
  typedef rti1516e::Exception Exception;

#define MAP_EXCEPTION(Exception, MappedException) \
  typedef MappedException Exception; \
  static void throw ## Exception(const std::string& reason) { throw MappedException(OpenRTI::localeToUcs(reason)); } \
  static void throw ## Exception(const std::wstring& reason) { throw MappedException(reason); } \
  static void throw ## Exception() { throw MappedException(std::wstring(L"/* NO COMMENT */")); }

  MAP_EXCEPTION(AlreadyConnected, rti1516e::AlreadyConnected)
  MAP_EXCEPTION(AsynchronousDeliveryAlreadyDisabled, rti1516e::AsynchronousDeliveryAlreadyDisabled)
  MAP_EXCEPTION(AsynchronousDeliveryAlreadyEnabled, rti1516e::AsynchronousDeliveryAlreadyEnabled)
  MAP_EXCEPTION(AttributeAcquisitionWasNotRequested, rti1516e::AttributeAcquisitionWasNotRequested)
  MAP_EXCEPTION(AttributeAlreadyBeingAcquired, rti1516e::AttributeAlreadyBeingAcquired)
  MAP_EXCEPTION(AttributeAlreadyBeingDivested, rti1516e::AttributeAlreadyBeingDivested)
  MAP_EXCEPTION(AttributeAlreadyOwned, rti1516e::AttributeAlreadyOwned)
  MAP_EXCEPTION(AttributeDivestitureWasNotRequested, rti1516e::AttributeDivestitureWasNotRequested)
  MAP_EXCEPTION(AttributeNotDefined, rti1516e::AttributeNotDefined)
  MAP_EXCEPTION(AttributeNotOwned, rti1516e::AttributeNotOwned)
  MAP_EXCEPTION(AttributeNotPublished, rti1516e::AttributeNotPublished)
  MAP_EXCEPTION(AttributeRelevanceAdvisorySwitchIsOff, rti1516e::AttributeRelevanceAdvisorySwitchIsOff)
  MAP_EXCEPTION(AttributeRelevanceAdvisorySwitchIsOn, rti1516e::AttributeRelevanceAdvisorySwitchIsOn)
  MAP_EXCEPTION(AttributeScopeAdvisorySwitchIsOff, rti1516e::AttributeScopeAdvisorySwitchIsOff)
  MAP_EXCEPTION(AttributeScopeAdvisorySwitchIsOn, rti1516e::AttributeScopeAdvisorySwitchIsOn)
  MAP_EXCEPTION(CallNotAllowedFromWithinCallback, rti1516e::CallNotAllowedFromWithinCallback)
  MAP_EXCEPTION(ConnectionFailed, rti1516e::ConnectionFailed)
  MAP_EXCEPTION(CouldNotCreateLogicalTimeFactory, rti1516e::CouldNotCreateLogicalTimeFactory)
  MAP_EXCEPTION(CouldNotOpenFDD, rti1516e::CouldNotOpenFDD)
  MAP_EXCEPTION(CouldNotOpenMIM, rti1516e::CouldNotOpenMIM)
  MAP_EXCEPTION(DeletePrivilegeNotHeld, rti1516e::DeletePrivilegeNotHeld)
  MAP_EXCEPTION(ErrorReadingFDD, rti1516e::ErrorReadingFDD)
  MAP_EXCEPTION(ErrorReadingMIM, rti1516e::ErrorReadingMIM)
  MAP_EXCEPTION(FederateAlreadyExecutionMember, rti1516e::FederateAlreadyExecutionMember)
  MAP_EXCEPTION(FederateNameAlreadyInUse, rti1516e::FederateNameAlreadyInUse)
  MAP_EXCEPTION(FederateHasNotBegunSave, rti1516e::FederateHasNotBegunSave)
  MAP_EXCEPTION(FederateIsExecutionMember, rti1516e::FederateIsExecutionMember)
  MAP_EXCEPTION(FederateNotExecutionMember, rti1516e::FederateNotExecutionMember)
  MAP_EXCEPTION(FederateOwnsAttributes, rti1516e::FederateOwnsAttributes)
  MAP_EXCEPTION(FederatesCurrentlyJoined, rti1516e::FederatesCurrentlyJoined)
  MAP_EXCEPTION(FederateServiceInvocationsAreBeingReportedViaMOM, rti1516e::FederateServiceInvocationsAreBeingReportedViaMOM)
  MAP_EXCEPTION(FederateUnableToUseTime, rti1516e::FederateUnableToUseTime)
  MAP_EXCEPTION(FederationExecutionAlreadyExists, rti1516e::FederationExecutionAlreadyExists)
  MAP_EXCEPTION(FederationExecutionDoesNotExist, rti1516e::FederationExecutionDoesNotExist)
  MAP_EXCEPTION(IllegalName, rti1516e::IllegalName)
  MAP_EXCEPTION(InconsistentFDD, rti1516e::InconsistentFDD)
  MAP_EXCEPTION(InteractionClassNotDefined, rti1516e::InteractionClassNotDefined)
  MAP_EXCEPTION(InteractionClassNotPublished, rti1516e::InteractionClassNotPublished)
  MAP_EXCEPTION(InteractionParameterNotDefined, rti1516e::InteractionParameterNotDefined)
  MAP_EXCEPTION(InteractionRelevanceAdvisorySwitchIsOff, rti1516e::InteractionRelevanceAdvisorySwitchIsOff)
  MAP_EXCEPTION(InteractionRelevanceAdvisorySwitchIsOn, rti1516e::InteractionRelevanceAdvisorySwitchIsOn)
  MAP_EXCEPTION(InTimeAdvancingState, rti1516e::InTimeAdvancingState)
  MAP_EXCEPTION(InvalidAttributeHandle, rti1516e::InvalidAttributeHandle)
  MAP_EXCEPTION(InvalidDimensionHandle, rti1516e::InvalidDimensionHandle)
  MAP_EXCEPTION(InvalidFederateHandle, rti1516e::InvalidFederateHandle)
  MAP_EXCEPTION(InvalidInteractionClassHandle, rti1516e::InvalidInteractionClassHandle)
  MAP_EXCEPTION(InvalidLocalSettingsDesignator, rti1516e::InvalidLocalSettingsDesignator)
  MAP_EXCEPTION(InvalidLogicalTime, rti1516e::InvalidLogicalTime)
  MAP_EXCEPTION(InvalidLookahead, rti1516e::InvalidLookahead)
  MAP_EXCEPTION(InvalidObjectClassHandle, rti1516e::InvalidObjectClassHandle)
  MAP_EXCEPTION(InvalidOrderName, rti1516e::InvalidOrderName)
  MAP_EXCEPTION(InvalidOrderType, rti1516e::InvalidOrderType)
  MAP_EXCEPTION(InvalidParameterHandle, rti1516e::InvalidParameterHandle)
  MAP_EXCEPTION(InvalidRangeBound, rti1516e::InvalidRangeBound)
  MAP_EXCEPTION(InvalidRegion, rti1516e::InvalidRegion)
  MAP_EXCEPTION(InvalidRegionContext, rti1516e::InvalidRegionContext)
  MAP_EXCEPTION(InvalidResignAction, rti1516e::InvalidResignAction)
  MAP_EXCEPTION(InvalidRetractionHandle, rti1516e::InvalidMessageRetractionHandle)
  MAP_EXCEPTION(InvalidServiceGroup, rti1516e::InvalidServiceGroup)
  MAP_EXCEPTION(InvalidTransportationName, rti1516e::InvalidTransportationName)
  MAP_EXCEPTION(InvalidTransportationType, rti1516e::InvalidTransportationType)
  MAP_EXCEPTION(LogicalTimeAlreadyPassed, rti1516e::LogicalTimeAlreadyPassed)
  MAP_EXCEPTION(MessageCanNoLongerBeRetracted, rti1516e::MessageCanNoLongerBeRetracted)
  MAP_EXCEPTION(NameNotFound, rti1516e::NameNotFound)
  MAP_EXCEPTION(NameSetWasEmpty, rti1516e::NameSetWasEmpty)
  MAP_EXCEPTION(NoAcquisitionPending, rti1516e::NoAcquisitionPending)
  MAP_EXCEPTION(NotConnected, rti1516e::NotConnected)
  MAP_EXCEPTION(ObjectClassNotDefined, rti1516e::ObjectClassNotDefined)
  MAP_EXCEPTION(ObjectClassNotPublished, rti1516e::ObjectClassNotPublished)
  MAP_EXCEPTION(ObjectClassRelevanceAdvisorySwitchIsOff, rti1516e::ObjectClassRelevanceAdvisorySwitchIsOff)
  MAP_EXCEPTION(ObjectClassRelevanceAdvisorySwitchIsOn, rti1516e::ObjectClassRelevanceAdvisorySwitchIsOn)
  MAP_EXCEPTION(ObjectInstanceNameInUse, rti1516e::ObjectInstanceNameInUse)
  MAP_EXCEPTION(ObjectInstanceNameNotReserved, rti1516e::ObjectInstanceNameNotReserved)
  MAP_EXCEPTION(ObjectInstanceNotKnown, rti1516e::ObjectInstanceNotKnown)
  MAP_EXCEPTION(OwnershipAcquisitionPending, rti1516e::OwnershipAcquisitionPending)
  MAP_EXCEPTION(RegionDoesNotContainSpecifiedDimension, rti1516e::RegionDoesNotContainSpecifiedDimension)
  MAP_EXCEPTION(RegionInUseForUpdateOrSubscription, rti1516e::RegionInUseForUpdateOrSubscription)
  MAP_EXCEPTION(RegionNotCreatedByThisFederate, rti1516e::RegionNotCreatedByThisFederate)
  MAP_EXCEPTION(RequestForTimeConstrainedPending, rti1516e::RequestForTimeConstrainedPending)
  MAP_EXCEPTION(RequestForTimeRegulationPending, rti1516e::RequestForTimeRegulationPending)
  MAP_EXCEPTION(RestoreInProgress, rti1516e::RestoreInProgress)
  MAP_EXCEPTION(RestoreNotRequested, rti1516e::RestoreNotRequested)
  MAP_EXCEPTION(RTIinternalError, rti1516e::RTIinternalError)
  MAP_EXCEPTION(SaveInProgress, rti1516e::SaveInProgress)
  MAP_EXCEPTION(SaveNotInitiated, rti1516e::SaveNotInitiated)
  MAP_EXCEPTION(SynchronizationPointLabelNotAnnounced, rti1516e::SynchronizationPointLabelNotAnnounced)
  MAP_EXCEPTION(TimeConstrainedAlreadyEnabled, rti1516e::TimeConstrainedAlreadyEnabled)
  MAP_EXCEPTION(TimeConstrainedIsNotEnabled, rti1516e::TimeConstrainedIsNotEnabled)
  MAP_EXCEPTION(TimeRegulationAlreadyEnabled, rti1516e::TimeRegulationAlreadyEnabled)
  MAP_EXCEPTION(TimeRegulationIsNotEnabled, rti1516e::TimeRegulationIsNotEnabled)
  MAP_EXCEPTION(UnsupportedCallbackModel, rti1516e::UnsupportedCallbackModel)
#undef MAP_EXCEPTION
};

template<typename LogicalTimeFactory>
class OPENRTI_LOCAL RTI1516EFederate : public Federate<RTI1516ETraits, LogicalTimeFactory> {
public:
  using Federate<RTI1516ETraits, LogicalTimeFactory>::getTimeConstrainedEnabled;
  using Federate<RTI1516ETraits, LogicalTimeFactory>::getFlushQueueMode;

  RTI1516EFederate(const std::wstring& federateType, const std::wstring& federateName,
                   const FederateHandle& federateHandle, SharedPtr<AbstractConnect> connect,
                   const InsertFederationExecutionMessage& insertFederationExecution,
                   const LogicalTimeFactory& logicalTimeFactory,
                   rti1516e::FederateAmbassador* federateAmbassador) :
    Federate<RTI1516ETraits, LogicalTimeFactory>(federateType, federateName, federateHandle, connect, insertFederationExecution, logicalTimeFactory),
    _federateAmbassador(federateAmbassador)
  { }

  virtual void synchronizationPointRegistrationResponse(const std::wstring& label, RegisterFederationSynchronizationPointResponseType reason)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      switch (reason) {
      case OpenRTI::RegisterFederationSynchronizationPointResponseSuccess:
        _federateAmbassador->synchronizationPointRegistrationSucceeded(label);
        break;
      case OpenRTI::RegisterFederationSynchronizationPointResponseLabelNotUnique:
        _federateAmbassador->synchronizationPointRegistrationFailed(label, rti1516e::SYNCHRONIZATION_POINT_LABEL_NOT_UNIQUE);
        break;
      case OpenRTI::RegisterFederationSynchronizationPointResponseMemberNotJoined:
        _federateAmbassador->synchronizationPointRegistrationFailed(label, rti1516e::SYNCHRONIZATION_SET_MEMBER_NOT_JOINED);
        break;
      }
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void announceSynchronizationPoint(const std::wstring& label, const OpenRTI::VariableLengthData& tag)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::VariableLengthData rti1516Tag = rti1516e::VariableLengthDataFriend::create(tag);
      _federateAmbassador->announceSynchronizationPoint(label, rti1516Tag);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void federationSynchronized(const std::wstring& label)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->federationSynchronized(label, rti1516e::FederateHandleSet() /* FIXME */);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // // 4.12
  // virtual void
  // initiateFederateSave(std::wstring const& label)
  //   throw (OpenRTI::UnableToPerformSave,
  //          OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->initiateFederateSave(label);
  //   } catch (const rti1516e::UnableToPerformSave& e) {
  //     throw OpenRTI::UnableToPerformSave(e.what());
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // virtual void
  // initiateFederateSave(const std::wstring& label,
  //                      const LogicalTime& logicalTime)
  //   throw (OpenRTI::UnableToPerformSave,
  //          OpenRTI::InvalidLogicalTime,
  //          OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->initiateFederateSave(label, RTI1516ELogicalTimeImplementation::getLogicalTime(logicalTime));
  //   } catch (const rti1516e::UnableToPerformSave& e) {
  //     throw OpenRTI::UnableToPerformSave(e.what());
  //   } catch (const rti1516e::InvalidLogicalTime& e) {
  //     throw OpenRTI::InvalidLogicalTime(e.what());
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // // 4.15
  // virtual void
  // federationSaved()
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->federationSaved();
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // virtual void
  // federationNotSaved(OpenRTI::SaveFailureReason reason)
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();

  //   rti1516e::SaveFailureReason rti1516Reason;
  //   switch (reason) {
  //   case OpenRTI::RTI_UNABLE_TO_SAVE:
  //     rti1516Reason = rti1516e::RTI_UNABLE_TO_SAVE;
  //     break;
  //   case OpenRTI::FEDERATE_REPORTED_FAILURE_DURING_SAVE:
  //     rti1516Reason = rti1516e::FEDERATE_REPORTED_FAILURE_DURING_SAVE;
  //     break;
  //   case OpenRTI::FEDERATE_RESIGNED_DURING_SAVE:
  //     rti1516Reason = rti1516e::FEDERATE_RESIGNED_DURING_SAVE;
  //     break;
  //   case OpenRTI::RTI_DETECTED_FAILURE_DURING_SAVE:
  //     rti1516Reason = rti1516e::RTI_DETECTED_FAILURE_DURING_SAVE;
  //     break;
  //   case OpenRTI::SAVE_TIME_CANNOT_BE_HONORED:
  //     rti1516Reason = rti1516e::SAVE_TIME_CANNOT_BE_HONORED;
  //     break;
  //   default:
  //     throw OpenRTI::FederateInternalError();
  //   }

  //   try {
  //     _federateAmbassador->federationNotSaved(rti1516Reason);
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }


  // // 4.17
  // virtual void
  // federationSaveStatusResponse(OpenRTI::FederateHandleSaveStatusPairVector const& federateStatusVector)
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //  /* enum SaveStatus */
  //  /* { */
  //  /*    NO_SAVE_IN_PROGRESS, */
  //  /*    FEDERATE_INSTRUCTED_TO_SAVE, */
  //  /*    FEDERATE_SAVING, */
  //  /*    FEDERATE_WAITING_FOR_FEDERATION_TO_SAVE */
  //  /* }; */

  //   try {
  //     throw rti1516e::FederateInternalError(L"Not implemented");
  //     // _federateAmbassador
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // // 4.19
  // virtual void
  // requestFederationRestoreSucceeded(std::wstring const& label)
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->requestFederationRestoreSucceeded(label);
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // virtual void
  // requestFederationRestoreFailed(std::wstring const& label)
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->requestFederationRestoreFailed(label);
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }


  // // 4.20
  // virtual void
  // federationRestoreBegun()
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->federationRestoreBegun();
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // // 4.21
  // virtual void
  // initiateFederateRestore(std::wstring const & label,
  //                         FederateHandle handle)
  //   throw (OpenRTI::SpecifiedSaveLabelDoesNotExist,
  //          OpenRTI::CouldNotInitiateRestore,
  //          OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     rti1516e::FederateHandle rti1516Handle = rti1516e::FederateHandleFriend::createHandle(handle);
  //     _federateAmbassador->initiateFederateRestore(label, rti1516Handle);
  //   } catch (const rti1516e::SpecifiedSaveLabelDoesNotExist& e) {
  //     throw OpenRTI::SpecifiedSaveLabelDoesNotExist(e.what());
  //   } catch (const rti1516e::CouldNotInitiateRestore& e) {
  //     throw OpenRTI::CouldNotInitiateRestore(e.what());
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // // 4.23
  // virtual
  // void federationRestored()
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->federationRestored();
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // virtual void
  // federationNotRestored(RestoreFailureReason reason)
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();

  //   rti1516e::RestoreFailureReason rti1516Reason;
  //   switch (reason) {
  //   case OpenRTI::RTI_UNABLE_TO_RESTORE:
  //     rti1516Reason = rti1516e::RTI_UNABLE_TO_RESTORE;
  //     break;
  //   case OpenRTI::FEDERATE_REPORTED_FAILURE_DURING_RESTORE:
  //     rti1516Reason = rti1516e::FEDERATE_REPORTED_FAILURE_DURING_RESTORE;
  //     break;
  //   case OpenRTI::FEDERATE_RESIGNED_DURING_RESTORE:
  //     rti1516Reason = rti1516e::FEDERATE_RESIGNED_DURING_RESTORE;
  //     break;
  //   case OpenRTI::RTI_DETECTED_FAILURE_DURING_RESTORE:
  //     rti1516Reason = rti1516e::RTI_DETECTED_FAILURE_DURING_RESTORE;
  //     break;
  //   default:
  //     throw OpenRTI::FederateInternalError();
  //   }

  //   try {
  //     _federateAmbassador->federationNotRestored(rti1516Reason);
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // // 4.25
  // virtual void
  // federationRestoreStatusResponse(FederateHandleRestoreStatusPairVector const& theFederateStatusVector)
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();


  //  /* enum RestoreStatus */
  //  /* { */
  //  /*    NO_RESTORE_IN_PROGRESS, */
  //  /*    FEDERATE_RESTORE_REQUEST_PENDING, */
  //  /*    FEDERATE_WAITING_FOR_RESTORE_TO_BEGIN, */
  //  /*    FEDERATE_PREPARED_TO_RESTORE, */
  //  /*    FEDERATE_RESTORING, */
  //  /*    FEDERATE_WAITING_FOR_FEDERATION_TO_RESTORE */
  //  /* }; */


  //   try {
  //     throw rti1516e::FederateInternalError(L"Not implemented");
  //     // _federateAmbassador
  //   } catch (const rti1516e::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  /////////////////////////////////////
  // Declaration Management Services //
  /////////////////////////////////////

  virtual void registrationForObjectClass(OpenRTI::ObjectClassHandle objectClassHandle, bool start)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::ObjectClassHandle rti1516Handle = rti1516e::ObjectClassHandleFriend::createHandle(objectClassHandle);
      if (start)
        _federateAmbassador->startRegistrationForObjectClass(rti1516Handle);
      else
        _federateAmbassador->stopRegistrationForObjectClass(rti1516Handle);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void turnInteractionsOn(InteractionClassHandle interactionClassHandle, bool on)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::InteractionClassHandle rti1516Handle = rti1516e::InteractionClassHandleFriend::createHandle(interactionClassHandle);
      if (on)
        _federateAmbassador->turnInteractionsOn(rti1516Handle);
      else
        _federateAmbassador->turnInteractionsOff(rti1516Handle);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  ////////////////////////////////
  // Object Management Services //
  ////////////////////////////////

  virtual void objectInstanceNameReservation(const OpenRTI::ReserveObjectInstanceNameResponseMessage& message)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      if (message.getSuccess())
        _federateAmbassador->objectInstanceNameReservationSucceeded(message.getObjectInstanceHandleNamePair().second);
      else
        _federateAmbassador->objectInstanceNameReservationFailed(message.getObjectInstanceHandleNamePair().second);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }
  virtual void objectInstanceNameReservation(const OpenRTI::ReserveMultipleObjectInstanceNameResponseMessage& message)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      std::set<std::wstring> stringSet;
      for (OpenRTI::ObjectInstanceHandleNamePairVector::const_iterator i = message.getObjectInstanceHandleNamePairVector().begin();
           i != message.getObjectInstanceHandleNamePairVector().end(); ++i)
        stringSet.insert(i->second);
      if (message.getSuccess())
        _federateAmbassador->multipleObjectInstanceNameReservationSucceeded(stringSet);
      else
        _federateAmbassador->multipleObjectInstanceNameReservationFailed(stringSet);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.5
  virtual
  void
  discoverObjectInstance(ObjectInstanceHandle objectInstanceHandle,
                         ObjectClassHandle objectClassHandle,
                         std::wstring const& name)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle
        = rti1516e::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);
      rti1516e::ObjectClassHandle rti1516ObjectClassHandle
        = rti1516e::ObjectClassHandleFriend::createHandle(objectClassHandle);

      _federateAmbassador->discoverObjectInstance(rti1516ObjectInstanceHandle,
                                                  rti1516ObjectClassHandle,
                                                  name);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void reflectAttributeValues(const typename RTI1516EFederate::ObjectClass& objectClass, const AttributeUpdateMessage& message)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::AttributeHandleValueMap rti1516AttributeValues;
      for (std::vector<OpenRTI::AttributeValue>::const_iterator i = message.getAttributeValues().begin();
           i != message.getAttributeValues().end(); ++i) {
        if (objectClass.getAttributeSubscriptionType(i->getAttributeHandle()) == Unsubscribed)
          continue;
        rti1516AttributeValues[rti1516e::AttributeHandleFriend::createHandle(i->getAttributeHandle())]
          = rti1516e::VariableLengthDataFriend::create(i->getValue());
      }

      if (!rti1516AttributeValues.empty()) {
        rti1516e::VariableLengthData rti1516Tag = rti1516e::VariableLengthDataFriend::create(message.getTag());

        rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle;
        rti1516ObjectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::createHandle(message.getObjectInstanceHandle());

        rti1516e::TransportationType rti1516TransportationType = translate(message.getTransportationType());

        rti1516e::SupplementalReflectInfo rti1516SupplementalReflectInfo; // FIXME

        _federateAmbassador->reflectAttributeValues(rti1516ObjectInstanceHandle, rti1516AttributeValues, rti1516Tag, rti1516e::RECEIVE,
                                                    rti1516TransportationType, rti1516SupplementalReflectInfo);
      }
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void reflectAttributeValues(const typename RTI1516EFederate::ObjectClass& objectClass,
                                      const TimeStampedAttributeUpdateMessage& message, const rti1516e::LogicalTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::AttributeHandleValueMap rti1516AttributeValues;
      for (std::vector<OpenRTI::AttributeValue>::const_iterator i = message.getAttributeValues().begin();
           i != message.getAttributeValues().end(); ++i) {
        if (objectClass.getAttributeSubscriptionType(i->getAttributeHandle()) == Unsubscribed)
          continue;
        rti1516AttributeValues[rti1516e::AttributeHandleFriend::createHandle(i->getAttributeHandle())]
          = rti1516e::VariableLengthDataFriend::create(i->getValue());
      }

      if (!rti1516AttributeValues.empty()) {
        rti1516e::VariableLengthData rti1516Tag = rti1516e::VariableLengthDataFriend::create(message.getTag());

        rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle;
        rti1516ObjectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::createHandle(message.getObjectInstanceHandle());

        rti1516e::TransportationType rti1516TransportationType = translate(message.getTransportationType());

        rti1516e::SupplementalReflectInfo rti1516SupplementalReflectInfo; // FIXME

        if (getFlushQueueMode()) {
          rti1516e::MessageRetractionHandle rti1516MessageRetractionHandle;
          rti1516MessageRetractionHandle = rti1516e::MessageRetractionHandleFriend::createHandle(message.getMessageRetractionHandle());
          _federateAmbassador->reflectAttributeValues(rti1516ObjectInstanceHandle, rti1516AttributeValues, rti1516Tag, rti1516e::TIMESTAMP,
                                                      rti1516TransportationType, logicalTime, rti1516e::TIMESTAMP,
                                                      rti1516MessageRetractionHandle, rti1516SupplementalReflectInfo);
        } else {
          if (getTimeConstrainedEnabled()) {
            _federateAmbassador->reflectAttributeValues(rti1516ObjectInstanceHandle, rti1516AttributeValues, rti1516Tag, rti1516e::TIMESTAMP,
                                                        rti1516TransportationType, logicalTime, rti1516e::TIMESTAMP,
                                                        rti1516SupplementalReflectInfo);
          } else {
            _federateAmbassador->reflectAttributeValues(rti1516ObjectInstanceHandle, rti1516AttributeValues, rti1516Tag, rti1516e::TIMESTAMP,
                                                        rti1516TransportationType, rti1516SupplementalReflectInfo);
          }
        }
      }
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.11
  virtual void removeObjectInstance(const DeleteObjectInstanceMessage& message)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle;
      rti1516ObjectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::createHandle(message.getObjectInstanceHandle());
      rti1516e::VariableLengthData rti1516Tag = rti1516e::VariableLengthDataFriend::create(message.getTag());
      rti1516e::SupplementalRemoveInfo rti1516SupplementalRemoveInfo; // FIXME
      _federateAmbassador->removeObjectInstance(rti1516ObjectInstanceHandle, rti1516Tag, rti1516e::RECEIVE, rti1516SupplementalRemoveInfo);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void removeObjectInstance(const TimeStampedDeleteObjectInstanceMessage& message, const rti1516e::LogicalTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle;
      rti1516ObjectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::createHandle(message.getObjectInstanceHandle());
      rti1516e::VariableLengthData rti1516Tag = rti1516e::VariableLengthDataFriend::create(message.getTag());
      rti1516e::SupplementalRemoveInfo rti1516SupplementalRemoveInfo; // FIXME
      if (getFlushQueueMode()) {
        rti1516e::MessageRetractionHandle rti1516MessageRetractionHandle;
        rti1516MessageRetractionHandle = rti1516e::MessageRetractionHandleFriend::createHandle(message.getMessageRetractionHandle());
        _federateAmbassador->removeObjectInstance(rti1516ObjectInstanceHandle, rti1516Tag, rti1516e::TIMESTAMP,
                                                  logicalTime, rti1516e::TIMESTAMP, rti1516MessageRetractionHandle,
                                                  rti1516SupplementalRemoveInfo);
      } else {
        if (getTimeConstrainedEnabled()) {
          _federateAmbassador->removeObjectInstance(rti1516ObjectInstanceHandle, rti1516Tag, rti1516e::TIMESTAMP,
                                                    logicalTime, rti1516e::TIMESTAMP, rti1516SupplementalRemoveInfo);
        } else {
          _federateAmbassador->removeObjectInstance(rti1516ObjectInstanceHandle, rti1516Tag, rti1516e::TIMESTAMP,
                                                    rti1516SupplementalRemoveInfo);
        }
      }
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }


  // 6.9
  virtual void
  receiveInteraction(const OpenRTI::InteractionMessage& message)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::InteractionClassHandle rti1516InteractionClassHandle;
      rti1516InteractionClassHandle = rti1516e::InteractionClassHandleFriend::createHandle(message.getInteractionClassHandle());

      rti1516e::ParameterHandleValueMap rti1516ParameterValues;
      for (std::vector<OpenRTI::ParameterValue>::const_iterator i = message.getParameterValues().begin();
           i != message.getParameterValues().end(); ++i) {
        rti1516ParameterValues[rti1516e::ParameterHandleFriend::createHandle(i->getParameterHandle())]
          = rti1516e::VariableLengthDataFriend::create(i->getValue());
      }

      rti1516e::VariableLengthData rti1516Tag;
      rti1516Tag = rti1516e::VariableLengthDataFriend::create(message.getTag());
      rti1516e::TransportationType rti1516TransportationType;
      rti1516TransportationType = translate(message.getTransportationType());
      rti1516e::SupplementalReceiveInfo rti1516eSupplementalReceiveInfo; // FIXME
      _federateAmbassador->receiveInteraction(rti1516InteractionClassHandle, rti1516ParameterValues, rti1516Tag, rti1516e::RECEIVE,
                                              rti1516TransportationType, rti1516eSupplementalReceiveInfo);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void
  receiveInteraction(const OpenRTI::TimeStampedInteractionMessage& message, const rti1516e::LogicalTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::InteractionClassHandle rti1516InteractionClassHandle;
      rti1516InteractionClassHandle = rti1516e::InteractionClassHandleFriend::createHandle(message.getInteractionClassHandle());

      rti1516e::ParameterHandleValueMap rti1516ParameterValues;
      for (std::vector<OpenRTI::ParameterValue>::const_iterator i = message.getParameterValues().begin();
           i != message.getParameterValues().end(); ++i) {
        rti1516ParameterValues[rti1516e::ParameterHandleFriend::createHandle(i->getParameterHandle())]
          = rti1516e::VariableLengthDataFriend::create(i->getValue());
      }

      rti1516e::VariableLengthData rti1516Tag;
      rti1516Tag = rti1516e::VariableLengthDataFriend::create(message.getTag());
      rti1516e::OrderType rti1516OrderType;
      rti1516OrderType = translate(OpenRTI::TIMESTAMP);
      rti1516e::TransportationType rti1516TransportationType;
      rti1516TransportationType = translate(message.getTransportationType());
      rti1516e::SupplementalReceiveInfo rti1516eSupplementalReceiveInfo; // FIXME

      if (getFlushQueueMode()) {
        rti1516e::MessageRetractionHandle rti1516MessageRetractionHandle;
        rti1516MessageRetractionHandle = rti1516e::MessageRetractionHandleFriend::createHandle(message.getMessageRetractionHandle());
        _federateAmbassador->receiveInteraction(rti1516InteractionClassHandle, rti1516ParameterValues, rti1516Tag, rti1516e::TIMESTAMP,
                                                rti1516TransportationType, logicalTime, rti1516e::TIMESTAMP,
                                                rti1516MessageRetractionHandle, rti1516eSupplementalReceiveInfo);
      } else {
        if (getTimeConstrainedEnabled()) {
          _federateAmbassador->receiveInteraction(rti1516InteractionClassHandle, rti1516ParameterValues, rti1516Tag, rti1516e::TIMESTAMP,
                                                  rti1516TransportationType, logicalTime, rti1516e::TIMESTAMP,
                                                  rti1516eSupplementalReceiveInfo);
        } else {
          _federateAmbassador->receiveInteraction(rti1516InteractionClassHandle, rti1516ParameterValues, rti1516Tag, rti1516e::TIMESTAMP,
                                                  rti1516TransportationType, rti1516eSupplementalReceiveInfo);
        }
      }
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.15
  virtual
  void
  attributesInScope(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);

      rti1516e::AttributeHandleSet rti1516AttributeHandleSet;
      for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
        rti1516AttributeHandleSet.insert(rti1516e::AttributeHandleFriend::createHandle(*i));

      _federateAmbassador->attributesInScope(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.16
  virtual
  void
  attributesOutOfScope(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);

      rti1516e::AttributeHandleSet rti1516AttributeHandleSet;
      for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
        rti1516AttributeHandleSet.insert(rti1516e::AttributeHandleFriend::createHandle(*i));

      _federateAmbassador->attributesOutOfScope(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.18
  virtual
  void
  provideAttributeValueUpdate(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector,
                              const VariableLengthData& tag)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);

      rti1516e::AttributeHandleSet rti1516AttributeHandleSet;
      for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
        rti1516AttributeHandleSet.insert(rti1516e::AttributeHandleFriend::createHandle(*i));

      rti1516e::VariableLengthData rti1516Tag = rti1516e::VariableLengthDataFriend::create(tag);

      _federateAmbassador->provideAttributeValueUpdate(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet, rti1516Tag);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.19
  virtual
  void
  turnUpdatesOnForObjectInstance(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);

      rti1516e::AttributeHandleSet rti1516AttributeHandleSet;
      for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
        rti1516AttributeHandleSet.insert(rti1516e::AttributeHandleFriend::createHandle(*i));

      _federateAmbassador->turnUpdatesOnForObjectInstance(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 6.20
  virtual
  void
  turnUpdatesOffForObjectInstance(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);

      rti1516e::AttributeHandleSet rti1516AttributeHandleSet;
      for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
        rti1516AttributeHandleSet.insert(rti1516e::AttributeHandleFriend::createHandle(*i));

      _federateAmbassador->turnUpdatesOffForObjectInstance(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  ///////////////////////////////////
  // Ownership Management Services //
  ///////////////////////////////////

  // 7.4
  virtual
  void
  requestAttributeOwnershipAssumption(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector,
                                      const VariableLengthData& tag)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);

      rti1516e::AttributeHandleSet rti1516AttributeHandleSet;
      for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
        rti1516AttributeHandleSet.insert(rti1516e::AttributeHandleFriend::createHandle(*i));

      rti1516e::VariableLengthData rti1516Tag = rti1516e::VariableLengthDataFriend::create(tag);

      _federateAmbassador->requestAttributeOwnershipAssumption(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet, rti1516Tag);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.5
  virtual
  void
  requestDivestitureConfirmation(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);

      rti1516e::AttributeHandleSet rti1516AttributeHandleSet;
      for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
        rti1516AttributeHandleSet.insert(rti1516e::AttributeHandleFriend::createHandle(*i));

      _federateAmbassador->requestDivestitureConfirmation(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.7
  virtual
  void
  attributeOwnershipAcquisitionNotification(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector,
                                            const VariableLengthData& tag)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);

      rti1516e::AttributeHandleSet rti1516AttributeHandleSet;
      for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
        rti1516AttributeHandleSet.insert(rti1516e::AttributeHandleFriend::createHandle(*i));

      rti1516e::VariableLengthData rti1516Tag = rti1516e::VariableLengthDataFriend::create(tag);

      _federateAmbassador->attributeOwnershipAcquisitionNotification(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet, rti1516Tag);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.10
  virtual
  void
  attributeOwnershipUnavailable(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);

      rti1516e::AttributeHandleSet rti1516AttributeHandleSet;
      for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
        rti1516AttributeHandleSet.insert(rti1516e::AttributeHandleFriend::createHandle(*i));

      _federateAmbassador->attributeOwnershipUnavailable(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.11
  virtual
  void
  requestAttributeOwnershipRelease(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector,
                                   const VariableLengthData& tag)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);

      rti1516e::AttributeHandleSet rti1516AttributeHandleSet;
      for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
        rti1516AttributeHandleSet.insert(rti1516e::AttributeHandleFriend::createHandle(*i));

      rti1516e::VariableLengthData rti1516Tag = rti1516e::VariableLengthDataFriend::create(tag);

      _federateAmbassador->requestAttributeOwnershipRelease(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet, rti1516Tag);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.15
  virtual
  void
  confirmAttributeOwnershipAcquisitionCancellation(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);

      rti1516e::AttributeHandleSet rti1516AttributeHandleSet;
      for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
        rti1516AttributeHandleSet.insert(rti1516e::AttributeHandleFriend::createHandle(*i));

      _federateAmbassador->confirmAttributeOwnershipAcquisitionCancellation(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 7.17
  virtual
  void
  informAttributeOwnership(ObjectInstanceHandle objectInstanceHandle,
                           AttributeHandle attributeHandle,
                           FederateHandle federateHandle)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle
        = rti1516e::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);
      rti1516e::AttributeHandle rti1516AttributeHandle
        = rti1516e::AttributeHandleFriend::createHandle(attributeHandle);
      rti1516e::FederateHandle rti1516FederateHandle
        = rti1516e::FederateHandleFriend::createHandle(federateHandle);

      _federateAmbassador->informAttributeOwnership(rti1516ObjectInstanceHandle,
                                                    rti1516AttributeHandle,
                                                    rti1516FederateHandle);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual
  void
  attributeIsNotOwned(ObjectInstanceHandle objectInstanceHandle, AttributeHandle attributeHandle)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle
        = rti1516e::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);
      rti1516e::AttributeHandle rti1516AttributeHandle
        = rti1516e::AttributeHandleFriend::createHandle(attributeHandle);

      _federateAmbassador->attributeIsNotOwned(rti1516ObjectInstanceHandle,
                                               rti1516AttributeHandle);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual
  void
  attributeIsOwnedByRTI(ObjectInstanceHandle objectInstanceHandle, AttributeHandle attributeHandle)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle
        = rti1516e::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);
      rti1516e::AttributeHandle rti1516AttributeHandle
        = rti1516e::AttributeHandleFriend::createHandle(attributeHandle);

      _federateAmbassador->attributeIsOwnedByRTI(rti1516ObjectInstanceHandle,
                                                 rti1516AttributeHandle);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  //////////////////////////////
  // Time Management Services //
  //////////////////////////////

  // 8.3
  virtual void
  timeRegulationEnabled(const rti1516e::LogicalTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->timeRegulationEnabled(logicalTime);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 8.6
  virtual void
  timeConstrainedEnabled(const rti1516e::LogicalTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->timeConstrainedEnabled(logicalTime);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 8.13
  virtual void
  timeAdvanceGrant(const rti1516e::LogicalTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->timeAdvanceGrant(logicalTime);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 8.22
  virtual
  void
  requestRetraction(MessageRetractionHandle messageRetractionHandle)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::MessageRetractionHandle rti1516MessageRetractionHandle;
      rti1516MessageRetractionHandle = rti1516e::MessageRetractionHandleFriend::createHandle(messageRetractionHandle);
      _federateAmbassador->requestRetraction(rti1516MessageRetractionHandle);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  rti1516e::FederateAmbassador* _federateAmbassador;
};

class OPENRTI_LOCAL RTIambassadorImplementation::RTI1516EAmbassadorInterface : public OpenRTI::Ambassador<RTI1516ETraits> {
public:
  RTI1516EAmbassadorInterface() :
    Ambassador<RTI1516ETraits>(),
    _federateAmbassador(0)
  { }

  std::auto_ptr<rti1516e::LogicalTimeFactory> getTimeFactory()
  {
    return rti1516e::LogicalTimeFactoryFactory::makeLogicalTimeFactory(getFederate().getLogicalTimeFactoryName());
  }

  virtual OpenRTI::AbstractFederate<Traits>*
  createFederate(const std::wstring& federateType, const std::wstring& federateName,
                 const FederateHandle& federateHandle, const std::wstring& federationName,
                 const InsertFederationExecutionMessage& insertFederationExecution,
                 SharedPtr<AbstractConnect> connect, rti1516e::FederateAmbassador* federateAmbassador)
  {
    std::wstring logicalTimeFactoryName = insertFederationExecution.getLogicalTimeFactoryName();
    std::auto_ptr<rti1516e::LogicalTimeFactory> logicalTimeFactory;
    logicalTimeFactory = rti1516e::LogicalTimeFactoryFactory::makeLogicalTimeFactory(logicalTimeFactoryName);
    if (!logicalTimeFactory.get())
      return 0;

    // Get logical time and logical time interval. If they are the well known ones,
    // try to use the optimized implementation using the native time data types directly.
    // An implementation is considered equal if the implementation name is the same and they are assignable in each direction,
    // Also add a flag that forces the to use the opaque factory

    // FIXME: make that again configurable
    bool forceOpaqueTime = false;
    if (!forceOpaqueTime) {
      std::auto_ptr<rti1516e::LogicalTime> logicalTime = logicalTimeFactory->makeInitial();
      std::auto_ptr<rti1516e::LogicalTimeInterval> logicalTimeInterval = logicalTimeFactory->makeZero();
      try {
        rti1516e::HLAinteger64Time time;
        rti1516e::HLAinteger64Interval interval;
        if (time.implementationName() == logicalTime->implementationName() &&
            interval.implementationName() == logicalTimeInterval->implementationName()) {
          time = *logicalTime;
          interval = *logicalTimeInterval;
          *logicalTime = time;
          *logicalTimeInterval = interval;
          if (*logicalTime == time && *logicalTimeInterval == interval) {
            return new RTI1516EFederate<RTI1516Einteger64TimeFactory>(federateType, federateName, federateHandle, connect, insertFederationExecution, RTI1516Einteger64TimeFactory(time.implementationName()),
                                       federateAmbassador);
          }
        }
      } catch (...) {
      }

      try {
        rti1516e::HLAfloat64Time time;
        rti1516e::HLAfloat64Interval interval;
        if (time.implementationName() == logicalTime->implementationName() &&
            interval.implementationName() == logicalTimeInterval->implementationName()) {
          time = *logicalTime;
          interval = *logicalTimeInterval;
          *logicalTime = time;
          *logicalTimeInterval = interval;
          if (*logicalTime == time && *logicalTimeInterval == interval) {
            return new RTI1516EFederate<RTI1516Efloat64TimeFactory>(federateType, federateName, federateHandle, connect, insertFederationExecution, RTI1516Efloat64TimeFactory(time.implementationName()),
                                       federateAmbassador);
          }
        }
      } catch (...) {
      }
    }

    // Ok, we will just need to use the opaque logical time factory
    return new RTI1516EFederate<RTI1516ELogicalTimeFactory>(federateType, federateName, federateHandle, connect, insertFederationExecution, RTI1516ELogicalTimeFactory(logicalTimeFactory), federateAmbassador);
  }

  virtual void connectionLost(const std::wstring& faultDescription)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->connectionLost(faultDescription);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void reportFederationExecutions(const OpenRTI::FederationExecutionInformationVector& federationExecutionInformationVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516e::FederationExecutionInformationVector federationInformations;

      federationInformations.reserve(federationExecutionInformationVector.size());
      for (OpenRTI::FederationExecutionInformationVector::const_iterator i = federationExecutionInformationVector.begin();
           i != federationExecutionInformationVector.end(); ++i) {
        federationInformations.push_back(rti1516e::FederationExecutionInformation(i->getFederationExecutionName(),
                                                                                  i->getLogicalTimeFactoryName()));
      }

      _federateAmbassador->reportFederationExecutions(federationInformations);
    } catch (const rti1516e::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  rti1516e::FederateAmbassador* _federateAmbassador;
};

RTIambassadorImplementation::RTIambassadorImplementation() throw () :
  _ambassadorInterface(new RTI1516EAmbassadorInterface)
{
}

RTIambassadorImplementation::~RTIambassadorImplementation()
{
  delete _ambassadorInterface;
  _ambassadorInterface = 0;
}

void
RTIambassadorImplementation::connect(rti1516e::FederateAmbassador & federateAmbassador, rti1516e::CallbackModel rti1516CallbackModel,
                                     std::wstring const & localSettingsDesignator)
  throw (rti1516e::ConnectionFailed,
         rti1516e::InvalidLocalSettingsDesignator,
         rti1516e::UnsupportedCallbackModel,
         rti1516e::AlreadyConnected,
         rti1516e::CallNotAllowedFromWithinCallback,
         rti1516e::RTIinternalError)
{
  if (rti1516CallbackModel != rti1516e::HLA_EVOKED)
    throw rti1516e::UnsupportedCallbackModel(L"Only HLA_EVOKED supported!");

  // FIXME make that again configurable
  Clock abstime = Clock::now() + Clock::fromSeconds(70);

  // FIXME allow the local settings stuff to be a configuration file and others
  StringMap defaults;
  defaults[L"protocol"] = L"thread";
  StringMap stringMap = getStringMapFromUrl(defaults, localSettingsDesignator);

  _ambassadorInterface->connect(stringMap, abstime);
  _ambassadorInterface->_federateAmbassador = &federateAmbassador;
}

void
RTIambassadorImplementation::disconnect()
  throw (rti1516e::FederateIsExecutionMember,
         rti1516e::CallNotAllowedFromWithinCallback,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->disconnect();
  _ambassadorInterface->_federateAmbassador = 0;
}

void
RTIambassadorImplementation::createFederationExecution(std::wstring const & federationExecutionName,
                                                       std::wstring const & fomModule,
                                                       std::wstring const & logicalTimeImplementationName)
  throw (rti1516e::CouldNotCreateLogicalTimeFactory,
         rti1516e::InconsistentFDD,
         rti1516e::CouldNotOpenFDD,
         rti1516e::ErrorReadingFDD,
         rti1516e::FederationExecutionAlreadyExists,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  std::vector<std::wstring> fomModules;
  fomModules.push_back(fomModule);
  _ambassadorInterface->createFederationExecution(federationExecutionName, fomModules, logicalTimeImplementationName);
}

void
RTIambassadorImplementation::createFederationExecution(std::wstring const & federationExecutionName,
                                                       std::vector<std::wstring> const & fomModules,
                                                       std::wstring const & logicalTimeImplementationName)
  throw (rti1516e::CouldNotCreateLogicalTimeFactory,
         rti1516e::InconsistentFDD,
         rti1516e::CouldNotOpenFDD,
         rti1516e::ErrorReadingFDD,
         rti1516e::FederationExecutionAlreadyExists,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->createFederationExecution(federationExecutionName, fomModules, logicalTimeImplementationName);
}

void
RTIambassadorImplementation::createFederationExecution (std::wstring const & federationExecutionName,
                                                        std::vector<std::wstring> const & fomModules,
                                                        std::wstring const & mimModule,
                                                        std::wstring const & logicalTimeImplementationName)
  throw (rti1516e::CouldNotCreateLogicalTimeFactory,
         rti1516e::InconsistentFDD,
         rti1516e::CouldNotOpenFDD,
         rti1516e::ErrorReadingFDD,
         rti1516e::DesignatorIsHLAstandardMIM,
         rti1516e::ErrorReadingMIM,
         rti1516e::CouldNotOpenMIM,
         rti1516e::FederationExecutionAlreadyExists,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  std::vector<std::wstring> fomModules2 = fomModules;
  fomModules2.insert(fomModules2.begin(), mimModule);
  _ambassadorInterface->createFederationExecution(federationExecutionName, fomModules2, logicalTimeImplementationName);
}

void
RTIambassadorImplementation::destroyFederationExecution(std::wstring const & federationExecutionName)
  throw (rti1516e::FederatesCurrentlyJoined,
         rti1516e::FederationExecutionDoesNotExist,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->destroyFederationExecution(federationExecutionName);
}

void
RTIambassadorImplementation::listFederationExecutions()
  throw (rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->listFederationExecutions();
}

rti1516e::FederateHandle
RTIambassadorImplementation::joinFederationExecution(std::wstring const & federateType,
                                                     std::wstring const & federationExecutionName,
                                                     std::vector<std::wstring> const & additionalFomModules)
  throw (rti1516e::CouldNotCreateLogicalTimeFactory,
         rti1516e::FederationExecutionDoesNotExist,
         rti1516e::InconsistentFDD,
         rti1516e::ErrorReadingFDD,
         rti1516e::CouldNotOpenFDD,
         rti1516e::FederateAlreadyExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::CallNotAllowedFromWithinCallback,
         rti1516e::RTIinternalError)
{
  FederateHandle federateHandle = _ambassadorInterface->joinFederationExecution(std::wstring(), federateType, federationExecutionName,
                                                                                additionalFomModules, _ambassadorInterface->_federateAmbassador);
  return rti1516e::FederateHandleFriend::createHandle(federateHandle);
}

rti1516e::FederateHandle
RTIambassadorImplementation::joinFederationExecution(std::wstring const & federateName,
                                                     std::wstring const & federateType,
                                                     std::wstring const & federationExecutionName,
                                                     std::vector<std::wstring> const & additionalFomModules)
  throw (rti1516e::CouldNotCreateLogicalTimeFactory,
         rti1516e::FederateNameAlreadyInUse,
         rti1516e::FederationExecutionDoesNotExist,
         rti1516e::InconsistentFDD,
         rti1516e::ErrorReadingFDD,
         rti1516e::CouldNotOpenFDD,
         rti1516e::FederateAlreadyExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::CallNotAllowedFromWithinCallback,
         rti1516e::RTIinternalError)
{
  FederateHandle federateHandle = _ambassadorInterface->joinFederationExecution(federateName, federateType, federationExecutionName,
                                                                                additionalFomModules, _ambassadorInterface->_federateAmbassador);
  return rti1516e::FederateHandleFriend::createHandle(federateHandle);
}

void
RTIambassadorImplementation::resignFederationExecution(rti1516e::ResignAction rti1516ResignAction)
  throw (rti1516e::OwnershipAcquisitionPending,
         rti1516e::FederateOwnsAttributes,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->resignFederationExecution(translate(rti1516ResignAction));
}

// 4.6
void
RTIambassadorImplementation::registerFederationSynchronizationPoint(std::wstring const & label,
                                                                    rti1516e::VariableLengthData const & rti1516Tag)
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::VariableLengthData tag = rti1516e::VariableLengthDataFriend::readPointer(rti1516Tag);
  _ambassadorInterface->registerFederationSynchronizationPoint(label, tag);
}

void
RTIambassadorImplementation::registerFederationSynchronizationPoint(std::wstring const & label,
                                                                    rti1516e::VariableLengthData const & rti1516Tag,
                                                                    rti1516e::FederateHandleSet const & rti1516FederateHandleSet)
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::VariableLengthData tag = rti1516e::VariableLengthDataFriend::readPointer(rti1516Tag);

  OpenRTI::FederateHandleSet federateHandleSet;
  for (rti1516e::FederateHandleSet::const_iterator i = rti1516FederateHandleSet.begin(); i != rti1516FederateHandleSet.end(); ++i)
    federateHandleSet.insert(rti1516e::FederateHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->registerFederationSynchronizationPoint(label, tag, federateHandleSet);
}

// 4.9
void
RTIambassadorImplementation::synchronizationPointAchieved(std::wstring const & label, bool)
  throw (rti1516e::SynchronizationPointLabelNotAnnounced,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  throw rti1516e::RTIinternalError(L"Not implemented yet!"); // bool argumnent missing
  // _ambassadorInterface->synchronizationPointAchieved(label);
}

// 4.11
void
RTIambassadorImplementation::requestFederationSave(std::wstring const & label)
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->requestFederationSave(label);
}

void
RTIambassadorImplementation::requestFederationSave(const std::wstring& label,
                                                   const rti1516e::LogicalTime& rti1516LogicalTime)
  throw (rti1516e::LogicalTimeAlreadyPassed,
         rti1516e::InvalidLogicalTime,
         rti1516e::FederateUnableToUseTime,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->requestFederationSave(label, rti1516LogicalTime);
}

// 4.13
void
RTIambassadorImplementation::federateSaveBegun()
  throw (rti1516e::SaveNotInitiated,
         rti1516e::FederateNotExecutionMember,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->federateSaveBegun();
}

// 4.14
void
RTIambassadorImplementation::federateSaveComplete()
  throw (rti1516e::FederateHasNotBegunSave,
         rti1516e::FederateNotExecutionMember,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->federateSaveComplete();
}

void
RTIambassadorImplementation::federateSaveNotComplete()
  throw (rti1516e::FederateHasNotBegunSave,
         rti1516e::FederateNotExecutionMember,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->federateSaveNotComplete();
}

void
RTIambassadorImplementation::abortFederationSave()
  throw (rti1516e::SaveNotInProgress,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  throw rti1516e::RTIinternalError(L"Not implemented yet!");
}

// 4.16
void
RTIambassadorImplementation::queryFederationSaveStatus ()
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->queryFederationSaveStatus();
}

// 4.18
void
RTIambassadorImplementation::requestFederationRestore(std::wstring const & label)
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->requestFederationRestore(label);
}

// 4.22
void
RTIambassadorImplementation::federateRestoreComplete()
  throw (rti1516e::RestoreNotRequested,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->federateRestoreComplete();
}

void
RTIambassadorImplementation::federateRestoreNotComplete()
  throw (rti1516e::RestoreNotRequested,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->federateRestoreNotComplete();
}

void
RTIambassadorImplementation::abortFederationRestore()
  throw (rti1516e::RestoreNotInProgress,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  throw rti1516e::RTIinternalError(L"Not implemented yet!");
}

// 4.24
void
RTIambassadorImplementation::queryFederationRestoreStatus()
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->queryFederationRestoreStatus();
}

/////////////////////////////////////
// Declaration Management Services //
/////////////////////////////////////

// 5.2
void
RTIambassadorImplementation::publishObjectClassAttributes(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                                          rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::AttributeNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516e::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516e::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516e::AttributeHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->publishObjectClassAttributes(objectClassHandle, attributeHandleSet);
}

// 5.3
void
RTIambassadorImplementation::unpublishObjectClass(rti1516e::ObjectClassHandle rti1516ObjectClassHandle)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::OwnershipAcquisitionPending,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516e::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);
  _ambassadorInterface->unpublishObjectClass(objectClassHandle);
}

void
RTIambassadorImplementation::unpublishObjectClassAttributes(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                                            rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::AttributeNotDefined,
         rti1516e::OwnershipAcquisitionPending,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516e::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516e::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516e::AttributeHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->unpublishObjectClassAttributes(objectClassHandle, attributeHandleSet);
}

// 5.4
void
RTIambassadorImplementation::publishInteractionClass(rti1516e::InteractionClassHandle rti1516InteractionClassHandle)
  throw (rti1516e::InteractionClassNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516e::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);
  _ambassadorInterface->publishInteractionClass(interactionClassHandle);
}

// 5.5
void
RTIambassadorImplementation::unpublishInteractionClass(rti1516e::InteractionClassHandle rti1516InteractionClassHandle)
  throw (rti1516e::InteractionClassNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516e::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);
  _ambassadorInterface->unpublishInteractionClass(interactionClassHandle);
}

// 5.6
void
RTIambassadorImplementation::subscribeObjectClassAttributes(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                                            rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                            bool active, std::wstring const & updateRateDesignator)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::AttributeNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::InvalidUpdateRateDesignator,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  throw rti1516e::RTIinternalError(L"Not implemented yet!"); // update rate

  OpenRTI::ObjectClassHandle objectClassHandle = rti1516e::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516e::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516e::AttributeHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->subscribeObjectClassAttributes(objectClassHandle, attributeHandleSet, active);
}

// 5.7
void
RTIambassadorImplementation::unsubscribeObjectClass(rti1516e::ObjectClassHandle rti1516ObjectClassHandle)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516e::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);
  _ambassadorInterface->unsubscribeObjectClass(objectClassHandle);
}

void
RTIambassadorImplementation::unsubscribeObjectClassAttributes(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                                              rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::AttributeNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516e::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516e::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516e::AttributeHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->unsubscribeObjectClassAttributes(objectClassHandle, attributeHandleSet);
}

// 5.8
void
RTIambassadorImplementation::subscribeInteractionClass(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                                       bool active)
  throw (rti1516e::InteractionClassNotDefined,
         rti1516e::FederateServiceInvocationsAreBeingReportedViaMOM,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516e::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);
  _ambassadorInterface->subscribeInteractionClass(interactionClassHandle, active);
}

// 5.9
void
RTIambassadorImplementation::unsubscribeInteractionClass(rti1516e::InteractionClassHandle rti1516InteractionClassHandle)
  throw (rti1516e::InteractionClassNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516e::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);
  _ambassadorInterface->unsubscribeInteractionClass(interactionClassHandle);
}

////////////////////////////////
// Object Management Services //
////////////////////////////////

// 6.2
void
RTIambassadorImplementation::reserveObjectInstanceName(std::wstring const & objectInstanceName)
  throw (rti1516e::IllegalName,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->reserveObjectInstanceName(objectInstanceName);
}

void
RTIambassadorImplementation::releaseObjectInstanceName(std::wstring const & objectInstanceName)
  throw (rti1516e::ObjectInstanceNameNotReserved,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->releaseObjectInstanceName(objectInstanceName);
}

// 6.2
void
RTIambassadorImplementation::reserveMultipleObjectInstanceName(std::set<std::wstring> const & objectInstanceName)
  throw (rti1516e::IllegalName,
         rti1516e::NameSetWasEmpty,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->reserveMultipleObjectInstanceName(objectInstanceName);
}

void
RTIambassadorImplementation::releaseMultipleObjectInstanceName(std::set<std::wstring> const & objectInstanceNameSet)
  throw (rti1516e::ObjectInstanceNameNotReserved,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->releaseMultipleObjectInstanceName(objectInstanceNameSet);
}


// 6.4
rti1516e::ObjectInstanceHandle
RTIambassadorImplementation::registerObjectInstance(rti1516e::ObjectClassHandle rti1516ObjectClassHandle)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::ObjectClassNotPublished,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516e::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = _ambassadorInterface->registerObjectInstance(objectClassHandle);
  return rti1516e::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);
}

rti1516e::ObjectInstanceHandle
RTIambassadorImplementation::registerObjectInstance(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                                    std::wstring const & objectInstanceName)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::ObjectClassNotPublished,
         rti1516e::ObjectInstanceNameNotReserved,
         rti1516e::ObjectInstanceNameInUse,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516e::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = _ambassadorInterface->registerObjectInstance(objectClassHandle, objectInstanceName, false);
  return rti1516e::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);
}

// 6.6
void
RTIambassadorImplementation::updateAttributeValues(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                   const rti1516e::AttributeHandleValueMap& rti1516AttributeValues,
                                                   const rti1516e::VariableLengthData& rti1516Tag)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotOwned,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::VariableLengthData tag = rti1516e::VariableLengthDataFriend::readPointer(rti1516Tag);

  std::vector<OpenRTI::AttributeValue> attributeValueVector;
  attributeValueVector.reserve(rti1516AttributeValues.size());
  for (rti1516e::AttributeHandleValueMap::const_iterator i = rti1516AttributeValues.begin(); i != rti1516AttributeValues.end(); ++i) {
    attributeValueVector.push_back(OpenRTI::AttributeValue());
    OpenRTI::AttributeHandle attributeHandle = rti1516e::AttributeHandleFriend::getOpenRTIHandle(i->first);
    attributeValueVector.back().setAttributeHandle(attributeHandle);
    attributeValueVector.back().getValue() = rti1516e::VariableLengthDataFriend::readPointer(i->second);
  }

  _ambassadorInterface->updateAttributeValues(objectInstanceHandle, attributeValueVector, tag);
}

rti1516e::MessageRetractionHandle
RTIambassadorImplementation::updateAttributeValues(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                   const rti1516e::AttributeHandleValueMap& rti1516AttributeValues,
                                                   const rti1516e::VariableLengthData& rti1516Tag,
                                                   const rti1516e::LogicalTime& rti1516LogicalTime)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotOwned,
         rti1516e::InvalidLogicalTime,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);
  OpenRTI::VariableLengthData tag = rti1516e::VariableLengthDataFriend::readPointer(rti1516Tag);

  std::vector<OpenRTI::AttributeValue> attributeValueVector;
  attributeValueVector.reserve(rti1516AttributeValues.size());
  for (rti1516e::AttributeHandleValueMap::const_iterator i = rti1516AttributeValues.begin(); i != rti1516AttributeValues.end(); ++i) {
    attributeValueVector.push_back(OpenRTI::AttributeValue());
    OpenRTI::AttributeHandle attributeHandle = rti1516e::AttributeHandleFriend::getOpenRTIHandle(i->first);
    attributeValueVector.back().setAttributeHandle(attributeHandle);
    attributeValueVector.back().getValue() = rti1516e::VariableLengthDataFriend::readPointer(i->second);
  }

  OpenRTI::MessageRetractionHandle messageRetractionHandle;
  messageRetractionHandle = _ambassadorInterface->updateAttributeValues(objectInstanceHandle, attributeValueVector, tag, rti1516LogicalTime);
  return rti1516e::MessageRetractionHandleFriend::createHandle(messageRetractionHandle);
}

// 6.8
void
RTIambassadorImplementation::sendInteraction(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                             const rti1516e::ParameterHandleValueMap& rti1516ParameterValues,
                                             const rti1516e::VariableLengthData& rti1516Tag)
  throw (rti1516e::InteractionClassNotPublished,
         rti1516e::InteractionClassNotDefined,
         rti1516e::InteractionParameterNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle;
  interactionClassHandle = rti1516e::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);

  std::vector<OpenRTI::ParameterValue> parameterValueVector;
  parameterValueVector.reserve(rti1516ParameterValues.size());
  for (rti1516e::ParameterHandleValueMap::const_iterator i = rti1516ParameterValues.begin(); i != rti1516ParameterValues.end(); ++i) {
    parameterValueVector.push_back(OpenRTI::ParameterValue());
    OpenRTI::ParameterHandle parameterHandle = rti1516e::ParameterHandleFriend::getOpenRTIHandle(i->first);
    parameterValueVector.back().setParameterHandle(parameterHandle);
    parameterValueVector.back().getValue() = rti1516e::VariableLengthDataFriend::readPointer(i->second);
  }

  OpenRTI::VariableLengthData tag = rti1516e::VariableLengthDataFriend::readPointer(rti1516Tag);
  _ambassadorInterface->sendInteraction(interactionClassHandle, parameterValueVector, tag);
}

rti1516e::MessageRetractionHandle
RTIambassadorImplementation::sendInteraction(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                             const rti1516e::ParameterHandleValueMap& rti1516ParameterValues,
                                             const rti1516e::VariableLengthData& rti1516Tag,
                                             const rti1516e::LogicalTime& rti1516LogicalTime)
  throw (rti1516e::InteractionClassNotPublished,
         rti1516e::InteractionClassNotDefined,
         rti1516e::InteractionParameterNotDefined,
         rti1516e::InvalidLogicalTime,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle;
  interactionClassHandle = rti1516e::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);

  std::vector<OpenRTI::ParameterValue> parameterValueVector;
  parameterValueVector.reserve(rti1516ParameterValues.size());
  for (rti1516e::ParameterHandleValueMap::const_iterator i = rti1516ParameterValues.begin(); i != rti1516ParameterValues.end(); ++i) {
    parameterValueVector.push_back(OpenRTI::ParameterValue());
    OpenRTI::ParameterHandle parameterHandle = rti1516e::ParameterHandleFriend::getOpenRTIHandle(i->first);
    parameterValueVector.back().setParameterHandle(parameterHandle);
    parameterValueVector.back().getValue() = rti1516e::VariableLengthDataFriend::readPointer(i->second);
  }

  OpenRTI::VariableLengthData tag = rti1516e::VariableLengthDataFriend::readPointer(rti1516Tag);
  OpenRTI::MessageRetractionHandle messageRetractionHandle;
  messageRetractionHandle = _ambassadorInterface->sendInteraction(interactionClassHandle, parameterValueVector, tag, rti1516LogicalTime);
  return rti1516e::MessageRetractionHandleFriend::createHandle(messageRetractionHandle);
}

// 6.10
void
RTIambassadorImplementation::deleteObjectInstance(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                  const rti1516e::VariableLengthData& rti1516Tag)
  throw (rti1516e::DeletePrivilegeNotHeld,
         rti1516e::ObjectInstanceNotKnown,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);
  OpenRTI::VariableLengthData tag = rti1516e::VariableLengthDataFriend::readPointer(rti1516Tag);
  _ambassadorInterface->deleteObjectInstance(objectInstanceHandle, tag);
}

rti1516e::MessageRetractionHandle
RTIambassadorImplementation::deleteObjectInstance(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                  const rti1516e::VariableLengthData& rti1516Tag,
                                                  const rti1516e::LogicalTime& rti1516LogicalTime)
  throw (rti1516e::DeletePrivilegeNotHeld,
         rti1516e::ObjectInstanceNotKnown,
         rti1516e::InvalidLogicalTime,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);
  OpenRTI::VariableLengthData tag = rti1516e::VariableLengthDataFriend::readPointer(rti1516Tag);
  OpenRTI::MessageRetractionHandle messageRetractionHandle = _ambassadorInterface->deleteObjectInstance(objectInstanceHandle, tag, rti1516LogicalTime);
  return rti1516e::MessageRetractionHandleFriend::createHandle(messageRetractionHandle);
}

// 6.12
void
RTIambassadorImplementation::localDeleteObjectInstance(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::FederateOwnsAttributes,
         rti1516e::OwnershipAcquisitionPending,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);
  _ambassadorInterface->localDeleteObjectInstance(objectInstanceHandle);
}

// 6.13
void
RTIambassadorImplementation::changeAttributeTransportationType(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                               rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                               rti1516e::TransportationType rti1516TransportationType)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotOwned,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516e::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516e::AttributeHandleFriend::getOpenRTIHandle(*i));

  OpenRTI::TransportationType transportationType = translate(rti1516TransportationType);

  _ambassadorInterface->changeAttributeTransportationType(objectInstanceHandle, attributeHandleSet, transportationType);
}

// 6.14
void
RTIambassadorImplementation::changeInteractionTransportationType(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                                                 rti1516e::TransportationType rti1516TransportationType)
  throw (rti1516e::InteractionClassNotDefined,
         rti1516e::InteractionClassNotPublished,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516e::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);
  OpenRTI::TransportationType transportationType = translate(rti1516TransportationType);
  _ambassadorInterface->changeInteractionTransportationType(interactionClassHandle, transportationType);
}

// 6.17
void
RTIambassadorImplementation::requestAttributeValueUpdate(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                         rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                         rti1516e::VariableLengthData const & rti1516Tag)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516e::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516e::AttributeHandleFriend::getOpenRTIHandle(*i));

  OpenRTI::VariableLengthData tag = rti1516e::VariableLengthDataFriend::readPointer(rti1516Tag);

  _ambassadorInterface->requestAttributeValueUpdate(objectInstanceHandle, attributeHandleSet, tag);
}

void
RTIambassadorImplementation::requestAttributeValueUpdate(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                                         rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                         rti1516e::VariableLengthData const & rti1516Tag)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::AttributeNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516e::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516e::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516e::AttributeHandleFriend::getOpenRTIHandle(*i));

  OpenRTI::VariableLengthData tag = rti1516e::VariableLengthDataFriend::readPointer(rti1516Tag);

  _ambassadorInterface->requestAttributeValueUpdate(objectClassHandle, attributeHandleSet, tag);
}

void
RTIambassadorImplementation::requestAttributeTransportationTypeChange(rti1516e::ObjectInstanceHandle theObject,
                                                                      rti1516e::AttributeHandleSet const & theAttributes,
                                                                      rti1516e::TransportationType theType)
  throw (rti1516e::AttributeAlreadyBeingChanged,
         rti1516e::AttributeNotOwned,
         rti1516e::AttributeNotDefined,
         rti1516e::ObjectInstanceNotKnown,
         rti1516e::InvalidTransportationType,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  throw rti1516e::RTIinternalError(L"Not implemented yet!");
}

void
RTIambassadorImplementation::queryAttributeTransportationType(rti1516e::ObjectInstanceHandle theObject, rti1516e::AttributeHandle theAttribute)
  throw (rti1516e::AttributeNotDefined,
         rti1516e::ObjectInstanceNotKnown,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  throw rti1516e::RTIinternalError(L"Not implemented yet!");
}

void
RTIambassadorImplementation::requestInteractionTransportationTypeChange(rti1516e::InteractionClassHandle theClass, rti1516e::TransportationType theType)
  throw (rti1516e::InteractionClassAlreadyBeingChanged,
         rti1516e::InteractionClassNotPublished,
         rti1516e::InteractionClassNotDefined,
         rti1516e::InvalidTransportationType,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  throw rti1516e::RTIinternalError(L"Not implemented yet!");
}

void
RTIambassadorImplementation::queryInteractionTransportationType(rti1516e::FederateHandle theFederate, rti1516e::InteractionClassHandle theInteraction)
  throw (rti1516e::InteractionClassNotDefined,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  throw rti1516e::RTIinternalError(L"Not implemented yet!");
}


///////////////////////////////////
// Ownership Management Services //
///////////////////////////////////
// 7.2
void
RTIambassadorImplementation::unconditionalAttributeOwnershipDivestiture(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                        rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotOwned,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516e::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516e::AttributeHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->unconditionalAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleSet);
}

// 7.3
void
RTIambassadorImplementation::negotiatedAttributeOwnershipDivestiture(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                     rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                                     rti1516e::VariableLengthData const & rti1516Tag)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotOwned,
         rti1516e::AttributeAlreadyBeingDivested,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516e::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516e::AttributeHandleFriend::getOpenRTIHandle(*i));

  OpenRTI::VariableLengthData tag = rti1516e::VariableLengthDataFriend::readPointer(rti1516Tag);

  _ambassadorInterface->negotiatedAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleSet, tag);
}

// 7.6
void
RTIambassadorImplementation::confirmDivestiture(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                rti1516e::AttributeHandleSet const& rti1516AttributeHandleSet,
                                                rti1516e::VariableLengthData const& rti1516Tag)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotOwned,
         rti1516e::AttributeDivestitureWasNotRequested,
         rti1516e::NoAcquisitionPending,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516e::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516e::AttributeHandleFriend::getOpenRTIHandle(*i));

  OpenRTI::VariableLengthData tag = rti1516e::VariableLengthDataFriend::readPointer(rti1516Tag);

  _ambassadorInterface->confirmDivestiture(objectInstanceHandle, attributeHandleSet, tag);
}

// 7.8
void
RTIambassadorImplementation::attributeOwnershipAcquisition(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                           rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                           rti1516e::VariableLengthData const & rti1516Tag)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::ObjectClassNotPublished,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotPublished,
         rti1516e::FederateOwnsAttributes,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516e::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516e::AttributeHandleFriend::getOpenRTIHandle(*i));

  OpenRTI::VariableLengthData tag = rti1516e::VariableLengthDataFriend::readPointer(rti1516Tag);

  _ambassadorInterface->attributeOwnershipAcquisition(objectInstanceHandle, attributeHandleSet, tag);
}

// 7.9
void
RTIambassadorImplementation::attributeOwnershipAcquisitionIfAvailable(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                      rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::ObjectClassNotPublished,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotPublished,
         rti1516e::FederateOwnsAttributes,
         rti1516e::AttributeAlreadyBeingAcquired,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516e::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516e::AttributeHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->attributeOwnershipAcquisitionIfAvailable(objectInstanceHandle, attributeHandleSet);
}

void
RTIambassadorImplementation::attributeOwnershipReleaseDenied(rti1516e::ObjectInstanceHandle theObject, rti1516e::AttributeHandleSet const & theAttributes)
  throw (rti1516e::AttributeNotOwned,
         rti1516e::AttributeNotDefined,
         rti1516e::ObjectInstanceNotKnown,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  throw rti1516e::RTIinternalError(L"Not implemented yet!");
}

// 7.12
void
RTIambassadorImplementation::attributeOwnershipDivestitureIfWanted(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                   rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                                   rti1516e::AttributeHandleSet & rti1516DivestedAttributeSet)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotOwned,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516e::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516e::AttributeHandleFriend::getOpenRTIHandle(*i));

  OpenRTI::AttributeHandleSet divestedAttributeHandleSet;
  _ambassadorInterface->attributeOwnershipDivestitureIfWanted(objectInstanceHandle, attributeHandleSet, divestedAttributeHandleSet);

  rti1516DivestedAttributeSet.clear();
  for (OpenRTI::AttributeHandleSet::const_iterator i = divestedAttributeHandleSet.begin(); i != divestedAttributeHandleSet.end(); ++i)
    rti1516DivestedAttributeSet.insert(rti1516e::AttributeHandleFriend::createHandle(*i));
}

// 7.13
void
RTIambassadorImplementation::cancelNegotiatedAttributeOwnershipDivestiture(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                           rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotOwned,
         rti1516e::AttributeDivestitureWasNotRequested,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516e::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516e::AttributeHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->cancelNegotiatedAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleSet);
}

// 7.14
void
RTIambassadorImplementation::cancelAttributeOwnershipAcquisition(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                 rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeAlreadyOwned,
         rti1516e::AttributeAcquisitionWasNotRequested,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516e::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516e::AttributeHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->cancelAttributeOwnershipAcquisition(objectInstanceHandle, attributeHandleSet);
}

// 7.16
void
RTIambassadorImplementation::queryAttributeOwnership(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                     rti1516e::AttributeHandle rti1516AttributeHandle)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);
  OpenRTI::AttributeHandle attributeHandle = rti1516e::AttributeHandleFriend::getOpenRTIHandle(rti1516AttributeHandle);
  _ambassadorInterface->queryAttributeOwnership(objectInstanceHandle, attributeHandle);
}

// 7.18
bool
RTIambassadorImplementation::isAttributeOwnedByFederate(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                        rti1516e::AttributeHandle rti1516AttributeHandle)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);
  OpenRTI::AttributeHandle attributeHandle = rti1516e::AttributeHandleFriend::getOpenRTIHandle(rti1516AttributeHandle);
  return _ambassadorInterface->isAttributeOwnedByFederate(objectInstanceHandle, attributeHandle);
}

//////////////////////////////
// Time Management Services //
//////////////////////////////

// 8.2
void
RTIambassadorImplementation::enableTimeRegulation(const rti1516e::LogicalTimeInterval& rti1516Lookahead)
  throw (rti1516e::TimeRegulationAlreadyEnabled,
         rti1516e::InvalidLookahead,
         rti1516e::InTimeAdvancingState,
         rti1516e::RequestForTimeRegulationPending,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->enableTimeRegulation(rti1516Lookahead);
}

// 8.4
void
RTIambassadorImplementation::disableTimeRegulation()
  throw (rti1516e::TimeRegulationIsNotEnabled,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->disableTimeRegulation();
}

// 8.5
void
RTIambassadorImplementation::enableTimeConstrained()
  throw (rti1516e::TimeConstrainedAlreadyEnabled,
         rti1516e::InTimeAdvancingState,
         rti1516e::RequestForTimeConstrainedPending,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->enableTimeConstrained();
}

// 8.7
void
RTIambassadorImplementation::disableTimeConstrained()
  throw (rti1516e::TimeConstrainedIsNotEnabled,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->disableTimeConstrained();
}

// 8.8
void
RTIambassadorImplementation::timeAdvanceRequest(const rti1516e::LogicalTime& logicalTime)
  throw (rti1516e::InvalidLogicalTime,
         rti1516e::LogicalTimeAlreadyPassed,
         rti1516e::InTimeAdvancingState,
         rti1516e::RequestForTimeRegulationPending,
         rti1516e::RequestForTimeConstrainedPending,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->timeAdvanceRequest(logicalTime);
}

// 8.9
void
RTIambassadorImplementation::timeAdvanceRequestAvailable(const rti1516e::LogicalTime& logicalTime)
  throw (rti1516e::InvalidLogicalTime,
         rti1516e::LogicalTimeAlreadyPassed,
         rti1516e::InTimeAdvancingState,
         rti1516e::RequestForTimeRegulationPending,
         rti1516e::RequestForTimeConstrainedPending,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->timeAdvanceRequestAvailable(logicalTime);
}

// 8.10
void
RTIambassadorImplementation::nextMessageRequest(const rti1516e::LogicalTime& logicalTime)
  throw (rti1516e::InvalidLogicalTime,
         rti1516e::LogicalTimeAlreadyPassed,
         rti1516e::InTimeAdvancingState,
         rti1516e::RequestForTimeRegulationPending,
         rti1516e::RequestForTimeConstrainedPending,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->nextMessageRequest(logicalTime);
}

// 8.11
void
RTIambassadorImplementation::nextMessageRequestAvailable(const rti1516e::LogicalTime& logicalTime)
  throw (rti1516e::InvalidLogicalTime,
         rti1516e::LogicalTimeAlreadyPassed,
         rti1516e::InTimeAdvancingState,
         rti1516e::RequestForTimeRegulationPending,
         rti1516e::RequestForTimeConstrainedPending,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->nextMessageRequestAvailable(logicalTime);
}

// 8.12
void
RTIambassadorImplementation::flushQueueRequest(const rti1516e::LogicalTime& logicalTime)
  throw (rti1516e::InvalidLogicalTime,
         rti1516e::LogicalTimeAlreadyPassed,
         rti1516e::InTimeAdvancingState,
         rti1516e::RequestForTimeRegulationPending,
         rti1516e::RequestForTimeConstrainedPending,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->flushQueueRequest(logicalTime);
}

// 8.14
void
RTIambassadorImplementation::enableAsynchronousDelivery()
  throw (rti1516e::AsynchronousDeliveryAlreadyEnabled,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->enableAsynchronousDelivery();
}

// 8.15
void
RTIambassadorImplementation::disableAsynchronousDelivery()
  throw (rti1516e::AsynchronousDeliveryAlreadyDisabled,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->disableAsynchronousDelivery();
}

// 8.16
bool
RTIambassadorImplementation::queryGALT(rti1516e::LogicalTime& logicalTime)
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  return _ambassadorInterface->queryGALT(logicalTime);
}

// 8.17
void
RTIambassadorImplementation::queryLogicalTime(rti1516e::LogicalTime& logicalTime)
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->queryLogicalTime(logicalTime);
}

// 8.18
bool
RTIambassadorImplementation::queryLITS(rti1516e::LogicalTime& logicalTime)
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  return _ambassadorInterface->queryLITS(logicalTime);
}

// 8.19
void
RTIambassadorImplementation::modifyLookahead(const rti1516e::LogicalTimeInterval& lookahead)
  throw (rti1516e::TimeRegulationIsNotEnabled,
         rti1516e::InvalidLookahead,
         rti1516e::InTimeAdvancingState,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->modifyLookahead(lookahead);
}

// 8.20
void
RTIambassadorImplementation::queryLookahead(rti1516e::LogicalTimeInterval& lookahead)
  throw (rti1516e::TimeRegulationIsNotEnabled,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->queryLookahead(lookahead);
}

// 8.21
void
RTIambassadorImplementation::retract(rti1516e::MessageRetractionHandle rti1516MessageRetractionHandle)
  throw (rti1516e::InvalidMessageRetractionHandle,
         rti1516e::TimeRegulationIsNotEnabled,
         rti1516e::MessageCanNoLongerBeRetracted,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::MessageRetractionHandle messageRetractionHandle = rti1516e::MessageRetractionHandleFriend::getOpenRTIHandle(rti1516MessageRetractionHandle);
  _ambassadorInterface->retract(messageRetractionHandle);
}

// 8.23
void
RTIambassadorImplementation::changeAttributeOrderType(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                      rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                      rti1516e::OrderType rti1516OrderType)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotOwned,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516e::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516e::AttributeHandleFriend::getOpenRTIHandle(*i));

  OpenRTI::OrderType orderType = translate(rti1516OrderType);

  _ambassadorInterface->changeAttributeOrderType(objectInstanceHandle, attributeHandleSet, orderType);
}

// 8.24
void
RTIambassadorImplementation::changeInteractionOrderType(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                                        rti1516e::OrderType rti1516OrderType)
  throw (rti1516e::InteractionClassNotDefined,
         rti1516e::InteractionClassNotPublished,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516e::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);

  OpenRTI::OrderType orderType = translate(rti1516OrderType);

  _ambassadorInterface->changeInteractionOrderType(interactionClassHandle, orderType);
}

//////////////////////////////////
// Data Distribution Management //
//////////////////////////////////

// 9.2
rti1516e::RegionHandle
RTIambassadorImplementation::createRegion(rti1516e::DimensionHandleSet const & rti1516DimensionHandleSet)
  throw (rti1516e::InvalidDimensionHandle,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::DimensionHandleSet dimensionHandleSet;
  for (rti1516e::DimensionHandleSet::const_iterator i = rti1516DimensionHandleSet.begin(); i != rti1516DimensionHandleSet.end(); ++i)
    dimensionHandleSet.insert(rti1516e::DimensionHandleFriend::getOpenRTIHandle(*i));

  return rti1516e::RegionHandleFriend::createHandle(_ambassadorInterface->createRegion(dimensionHandleSet));
}

// 9.3
void
RTIambassadorImplementation::commitRegionModifications(rti1516e::RegionHandleSet const & rti1516RegionHandleSet)
  throw (rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::RegionHandleSet regionHandleSet;
  for (rti1516e::RegionHandleSet::const_iterator i = rti1516RegionHandleSet.begin(); i != rti1516RegionHandleSet.end(); ++i)
    regionHandleSet.insert(rti1516e::RegionHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->commitRegionModifications(regionHandleSet);
}

void
RTIambassadorImplementation::deleteRegion(const rti1516e::RegionHandle& rti1516RegionHandle)
  throw (rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::RegionInUseForUpdateOrSubscription,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::RegionHandle regionHandle = rti1516e::RegionHandleFriend::getOpenRTIHandle(rti1516RegionHandle);
  _ambassadorInterface->deleteRegion(regionHandle);
}

// 9.5
rti1516e::ObjectInstanceHandle
RTIambassadorImplementation::registerObjectInstanceWithRegions(rti1516e::ObjectClassHandle theClass,
                                                               rti1516e::AttributeHandleSetRegionHandleSetPairVector const &
                                                               theAttributeHandleSetRegionHandleSetPairVector)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::ObjectClassNotPublished,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotPublished,
         rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::InvalidRegionContext,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  throw rti1516e::RTIinternalError(L"Not implemented");
  // FIXME
  /* _ambassadorInterface->registerObjectInstanceWithRegions(); */
}

rti1516e::ObjectInstanceHandle
RTIambassadorImplementation::registerObjectInstanceWithRegions(rti1516e::ObjectClassHandle theClass,
                                                               rti1516e::AttributeHandleSetRegionHandleSetPairVector const &
                                                               theAttributeHandleSetRegionHandleSetPairVector,
                                                               std::wstring const & theObjectInstanceName)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::ObjectClassNotPublished,
         rti1516e::AttributeNotDefined,
         rti1516e::AttributeNotPublished,
         rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::InvalidRegionContext,
         rti1516e::ObjectInstanceNameNotReserved,
         rti1516e::ObjectInstanceNameInUse,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  // FIXME
  /* _ambassadorInterface->registerObjectInstanceWithRegions(); */
  throw rti1516e::RTIinternalError(L"not implemented");
}

// 9.6
void
RTIambassadorImplementation::associateRegionsForUpdates(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                        rti1516e::AttributeHandleSetRegionHandleSetPairVector const &
                                                        theAttributeHandleSetRegionHandleSetPairVector)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::InvalidRegionContext,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);
  throw rti1516e::RTIinternalError(L"Not implemented");
}

// 9.7
void
RTIambassadorImplementation::unassociateRegionsForUpdates(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                          rti1516e::AttributeHandleSetRegionHandleSetPairVector const &
                                                          theAttributeHandleSetRegionHandleSetPairVector)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::AttributeNotDefined,
         rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);
  // FIXME
  /* _ambassadorInterface->registerObjectInstanceWithRegions(); */
}

// 9.8
void
RTIambassadorImplementation::subscribeObjectClassAttributesWithRegions(rti1516e::ObjectClassHandle theClass,
                                                                       rti1516e::AttributeHandleSetRegionHandleSetPairVector const &
                                                                       theAttributeHandleSetRegionHandleSetPairVector,
                                                                       bool active, std::wstring const & updateRateDesignator)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::AttributeNotDefined,
         rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::InvalidRegionContext,
         rti1516e::FederateNotExecutionMember,
         rti1516e::InvalidUpdateRateDesignator,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  // FIXME
  /* _ambassadorInterface->registerObjectInstanceWithRegions(); */
}

// 9.9
void
RTIambassadorImplementation::unsubscribeObjectClassAttributesWithRegions(rti1516e::ObjectClassHandle theClass,
                                                                         rti1516e::AttributeHandleSetRegionHandleSetPairVector const &
                                                                         theAttributeHandleSetRegionHandleSetPairVector)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::AttributeNotDefined,
         rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  // FIXME
  /* _ambassadorInterface->registerObjectInstanceWithRegions(); */
}

// 9.10
void
RTIambassadorImplementation::subscribeInteractionClassWithRegions(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                                                  rti1516e::RegionHandleSet const & rti1516RegionHandleSet,
                                                                  bool active)
  throw (rti1516e::InteractionClassNotDefined,
         rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::InvalidRegionContext,
         rti1516e::FederateServiceInvocationsAreBeingReportedViaMOM,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516e::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);

  OpenRTI::RegionHandleSet regionHandleSet;
  for (rti1516e::RegionHandleSet::const_iterator i = rti1516RegionHandleSet.begin(); i != rti1516RegionHandleSet.end(); ++i)
    regionHandleSet.insert(rti1516e::RegionHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->subscribeInteractionClassWithRegions(interactionClassHandle, regionHandleSet, active);
}

// 9.11
void
RTIambassadorImplementation::unsubscribeInteractionClassWithRegions(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                                                    rti1516e::RegionHandleSet const & rti1516RegionHandleSet)
  throw (rti1516e::InteractionClassNotDefined,
         rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516e::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);

  OpenRTI::RegionHandleSet regionHandleSet;
  for (rti1516e::RegionHandleSet::const_iterator i = rti1516RegionHandleSet.begin(); i != rti1516RegionHandleSet.end(); ++i)
    regionHandleSet.insert(rti1516e::RegionHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->unsubscribeInteractionClassWithRegions(interactionClassHandle, regionHandleSet);
}

// 9.12
void
RTIambassadorImplementation::sendInteractionWithRegions(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                                        rti1516e::ParameterHandleValueMap const & rti1516ParameterHandleValueMap,
                                                        rti1516e::RegionHandleSet const & rti1516RegionHandleSet,
                                                        rti1516e::VariableLengthData const & rti1516Tag)
  throw (rti1516e::InteractionClassNotDefined,
         rti1516e::InteractionClassNotPublished,
         rti1516e::InteractionParameterNotDefined,
         rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::InvalidRegionContext,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516e::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);

  OpenRTI::RegionHandleSet regionHandleSet;
  for (rti1516e::RegionHandleSet::const_iterator i = rti1516RegionHandleSet.begin(); i != rti1516RegionHandleSet.end(); ++i)
    regionHandleSet.insert(rti1516e::RegionHandleFriend::getOpenRTIHandle(*i));

  OpenRTI::VariableLengthData tag = rti1516e::VariableLengthDataFriend::readPointer(rti1516Tag);

  // FIXME
  /* _ambassadorInterface->sendInteractionWithRegions(); */
}

rti1516e::MessageRetractionHandle
RTIambassadorImplementation::sendInteractionWithRegions(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                                        rti1516e::ParameterHandleValueMap const & rti1516ParameterHandleValueMap,
                                                        rti1516e::RegionHandleSet const & rti1516RegionHandleSet,
                                                        rti1516e::VariableLengthData const & rti1516Tag,
                                                        rti1516e::LogicalTime const & rti1516LogicalTime)
  throw (rti1516e::InteractionClassNotDefined,
         rti1516e::InteractionClassNotPublished,
         rti1516e::InteractionParameterNotDefined,
         rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::InvalidRegionContext,
         rti1516e::InvalidLogicalTime,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516e::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);

  OpenRTI::RegionHandleSet regionHandleSet;
  for (rti1516e::RegionHandleSet::const_iterator i = rti1516RegionHandleSet.begin(); i != rti1516RegionHandleSet.end(); ++i)
    regionHandleSet.insert(rti1516e::RegionHandleFriend::getOpenRTIHandle(*i));

  OpenRTI::VariableLengthData tag = rti1516e::VariableLengthDataFriend::readPointer(rti1516Tag);
  OpenRTI::MessageRetractionHandle messageRetractionHandle;
  // messageRetractionHandle = _ambassadorInterface->sendInteractionWithRegions(interactionClassHandle, parameterValueVector, tag, logicalTime);
  return rti1516e::MessageRetractionHandleFriend::createHandle(messageRetractionHandle);
}

// 9.13
void
RTIambassadorImplementation::requestAttributeValueUpdateWithRegions(rti1516e::ObjectClassHandle theClass,
                                                                    rti1516e::AttributeHandleSetRegionHandleSetPairVector const & theSet,
                                                                    rti1516e::VariableLengthData const & rti1516Tag)
  throw (rti1516e::ObjectClassNotDefined,
         rti1516e::AttributeNotDefined,
         rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::InvalidRegionContext,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::VariableLengthData tag = rti1516e::VariableLengthDataFriend::readPointer(rti1516Tag);

  // FIXME
  /* _ambassadorInterface->registerObjectInstanceWithRegions(); */
}

//////////////////////////
// RTI Support Services //
//////////////////////////

rti1516e::ResignAction
RTIambassadorImplementation::getAutomaticResignDirective()
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  return translate(_ambassadorInterface->getAutomaticResignDirective());
}

void
RTIambassadorImplementation::setAutomaticResignDirective(rti1516e::ResignAction resignAction)
  throw (rti1516e::InvalidResignAction,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->setAutomaticResignDirective(translate(resignAction));
}

rti1516e::FederateHandle
RTIambassadorImplementation::getFederateHandle(std::wstring const & theName)
  throw (rti1516e::NameNotFound,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  throw rti1516e::RTIinternalError(L"Not implemented yet!");
}

std::wstring
RTIambassadorImplementation::getFederateName(rti1516e::FederateHandle theHandle)
  throw (rti1516e::InvalidFederateHandle,
         rti1516e::FederateHandleNotKnown,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  throw rti1516e::RTIinternalError(L"Not implemented yet!");
}

// 10.2
rti1516e::ObjectClassHandle
RTIambassadorImplementation::getObjectClassHandle(std::wstring const & name)
  throw (rti1516e::NameNotFound,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectClassHandle handle = _ambassadorInterface->getObjectClassHandle(name);
  return rti1516e::ObjectClassHandleFriend::createHandle(handle);
}

// 10.3
std::wstring
RTIambassadorImplementation::getObjectClassName(rti1516e::ObjectClassHandle rti1516ObjectClassHandle)
  throw (rti1516e::InvalidObjectClassHandle,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516e::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);
  return _ambassadorInterface->getObjectClassName(objectClassHandle);
}

// 10.4
rti1516e::AttributeHandle
RTIambassadorImplementation::getAttributeHandle(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                                std::wstring const & attributeName)
  throw (rti1516e::InvalidObjectClassHandle,
         rti1516e::NameNotFound,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516e::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);
  OpenRTI::AttributeHandle handle = _ambassadorInterface->getAttributeHandle(objectClassHandle, attributeName);
  return rti1516e::AttributeHandleFriend::createHandle(handle);
}

// 10.5
std::wstring
RTIambassadorImplementation::getAttributeName(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                              rti1516e::AttributeHandle rti1516AttributeHandle)
  throw (rti1516e::InvalidObjectClassHandle,
         rti1516e::InvalidAttributeHandle,
         rti1516e::AttributeNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516e::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);
  OpenRTI::AttributeHandle attributeHandle = rti1516e::AttributeHandleFriend::getOpenRTIHandle(rti1516AttributeHandle);
  return _ambassadorInterface->getAttributeName(objectClassHandle, attributeHandle);
}

double
RTIambassadorImplementation::getUpdateRateValue(std::wstring const & updateRateDesignator)
    throw (rti1516e::InvalidUpdateRateDesignator,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError)
{
  throw rti1516e::RTIinternalError(L"Not implemented yet!");
}

double
RTIambassadorImplementation::getUpdateRateValueForAttribute(rti1516e::ObjectInstanceHandle theObject, rti1516e::AttributeHandle theAttribute)
    throw (rti1516e::ObjectInstanceNotKnown,
           rti1516e::AttributeNotDefined,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError)
{
  throw rti1516e::RTIinternalError(L"Not implemented yet!");
}

// 10.6
rti1516e::InteractionClassHandle
RTIambassadorImplementation::getInteractionClassHandle(std::wstring const & name)
  throw (rti1516e::NameNotFound,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::InteractionClassHandle handle = _ambassadorInterface->getInteractionClassHandle(name);
  return rti1516e::InteractionClassHandleFriend::createHandle(handle);
}

// 10.7
std::wstring
RTIambassadorImplementation::getInteractionClassName(rti1516e::InteractionClassHandle rti1516InteractionClassHandle)
  throw (rti1516e::InvalidInteractionClassHandle,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516e::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);
  return _ambassadorInterface->getInteractionClassName(interactionClassHandle);
}

// 10.8
rti1516e::ParameterHandle
RTIambassadorImplementation::getParameterHandle(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                                std::wstring const & parameterName)
  throw (rti1516e::InvalidInteractionClassHandle,
         rti1516e::NameNotFound,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516e::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);
  OpenRTI::ParameterHandle handle = _ambassadorInterface->getParameterHandle(interactionClassHandle, parameterName);
  return rti1516e::ParameterHandleFriend::createHandle(handle);
}

// 10.9
std::wstring
RTIambassadorImplementation::getParameterName(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                              rti1516e::ParameterHandle rti1516ParameterHandle)
  throw (rti1516e::InvalidInteractionClassHandle,
         rti1516e::InvalidParameterHandle,
         rti1516e::InteractionParameterNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516e::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);
  OpenRTI::ParameterHandle parameterHandle = rti1516e::ParameterHandleFriend::getOpenRTIHandle(rti1516ParameterHandle);
  return _ambassadorInterface->getParameterName(interactionClassHandle, parameterHandle);
}

// 10.10
rti1516e::ObjectInstanceHandle
RTIambassadorImplementation::getObjectInstanceHandle(std::wstring const & name)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle handle = _ambassadorInterface->getObjectInstanceHandle(name);
  return rti1516e::ObjectInstanceHandleFriend::createHandle(handle);
}

// 10.11
std::wstring
RTIambassadorImplementation::getObjectInstanceName(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);
  return _ambassadorInterface->getObjectInstanceName(objectInstanceHandle);
}

// 10.12
rti1516e::DimensionHandle
RTIambassadorImplementation::getDimensionHandle(std::wstring const & name)
  throw (rti1516e::NameNotFound,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::DimensionHandle handle = _ambassadorInterface->getDimensionHandle(name);
  return rti1516e::DimensionHandleFriend::createHandle(handle);
}

// 10.13
std::wstring
RTIambassadorImplementation::getDimensionName(rti1516e::DimensionHandle rti1516DimensionHandle)
  throw (rti1516e::InvalidDimensionHandle,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::DimensionHandle dimensionHandle = rti1516e::DimensionHandleFriend::getOpenRTIHandle(rti1516DimensionHandle);
  return _ambassadorInterface->getDimensionName(dimensionHandle);
}

// 10.14
unsigned long
RTIambassadorImplementation::getDimensionUpperBound(rti1516e::DimensionHandle rti1516DimensionHandle)
  throw (rti1516e::InvalidDimensionHandle,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::DimensionHandle dimensionHandle = rti1516e::DimensionHandleFriend::getOpenRTIHandle(rti1516DimensionHandle);
  return _ambassadorInterface->getDimensionUpperBound(dimensionHandle);
}

// 10.15
rti1516e::DimensionHandleSet
RTIambassadorImplementation::getAvailableDimensionsForClassAttribute(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                                                     rti1516e::AttributeHandle rti1516AttributeHandle)
  throw (rti1516e::InvalidObjectClassHandle,
         rti1516e::InvalidAttributeHandle,
         rti1516e::AttributeNotDefined,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516e::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);

  OpenRTI::AttributeHandle attributeHandle = rti1516e::AttributeHandleFriend::getOpenRTIHandle(rti1516AttributeHandle);

  OpenRTI::DimensionHandleSet dimensionHandleSet = _ambassadorInterface->getAvailableDimensionsForClassAttribute(objectClassHandle, attributeHandle);

  rti1516e::DimensionHandleSet rti1516DimensionHandleSet;
  for (OpenRTI::DimensionHandleSet::const_iterator i = dimensionHandleSet.begin(); i != dimensionHandleSet.end(); ++i)
    rti1516DimensionHandleSet.insert(rti1516e::DimensionHandleFriend::createHandle(*i));

  return rti1516DimensionHandleSet;
}

// 10.16
rti1516e::ObjectClassHandle
RTIambassadorImplementation::getKnownObjectClassHandle(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle)
  throw (rti1516e::ObjectInstanceNotKnown,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516e::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);
  OpenRTI::ObjectClassHandle objectClassHandle = _ambassadorInterface->getKnownObjectClassHandle(objectInstanceHandle);
  return rti1516e::ObjectClassHandleFriend::createHandle(objectClassHandle);
}

// 10.17
rti1516e::DimensionHandleSet
RTIambassadorImplementation::getAvailableDimensionsForInteractionClass(rti1516e::InteractionClassHandle rti1516InteractionClassHandle)
  throw (rti1516e::InvalidInteractionClassHandle,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516e::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);

  OpenRTI::DimensionHandleSet dimensionHandleSet = _ambassadorInterface->getAvailableDimensionsForInteractionClass(interactionClassHandle);

  rti1516e::DimensionHandleSet rti1516DimensionHandleSet;
  for (OpenRTI::DimensionHandleSet::const_iterator i = dimensionHandleSet.begin(); i != dimensionHandleSet.end(); ++i)
    rti1516DimensionHandleSet.insert(rti1516e::DimensionHandleFriend::createHandle(*i));

  return rti1516DimensionHandleSet;
}

// 10.18
rti1516e::TransportationType
RTIambassadorImplementation::getTransportationType(std::wstring const & transportationName)
  throw (rti1516e::InvalidTransportationName,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  return translate(_ambassadorInterface->getTransportationType(transportationName));
}

// 10.19
std::wstring
RTIambassadorImplementation::getTransportationName(rti1516e::TransportationType transportationType)
  throw (rti1516e::InvalidTransportationType,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  return _ambassadorInterface->getTransportationName(translate(transportationType));
}

// 10.20
rti1516e::OrderType
RTIambassadorImplementation::getOrderType(std::wstring const & orderName)
  throw (rti1516e::InvalidOrderName,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  return translate(_ambassadorInterface->getOrderType(orderName));
}

// 10.21
std::wstring
RTIambassadorImplementation::getOrderName(rti1516e::OrderType orderType)
  throw (rti1516e::InvalidOrderType,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  return _ambassadorInterface->getOrderName(translate(orderType));
}

// 10.22
void
RTIambassadorImplementation::enableObjectClassRelevanceAdvisorySwitch()
  throw (rti1516e::ObjectClassRelevanceAdvisorySwitchIsOn,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->enableObjectClassRelevanceAdvisorySwitch();
}

// 10.23
void
RTIambassadorImplementation::disableObjectClassRelevanceAdvisorySwitch()
  throw (rti1516e::ObjectClassRelevanceAdvisorySwitchIsOff,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->disableObjectClassRelevanceAdvisorySwitch();
}

// 10.24
void
RTIambassadorImplementation::enableAttributeRelevanceAdvisorySwitch ()
  throw (rti1516e::AttributeRelevanceAdvisorySwitchIsOn,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->enableAttributeRelevanceAdvisorySwitch();
}

// 10.25
void
RTIambassadorImplementation::disableAttributeRelevanceAdvisorySwitch ()
  throw (rti1516e::AttributeRelevanceAdvisorySwitchIsOff,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->disableAttributeRelevanceAdvisorySwitch();
}

// 10.26
void
RTIambassadorImplementation::enableAttributeScopeAdvisorySwitch ()
  throw (rti1516e::AttributeScopeAdvisorySwitchIsOn,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->enableAttributeScopeAdvisorySwitch();
}

// 10.27
void
RTIambassadorImplementation::disableAttributeScopeAdvisorySwitch ()
  throw (rti1516e::AttributeScopeAdvisorySwitchIsOff,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->disableAttributeScopeAdvisorySwitch();
}

// 10.28
void
RTIambassadorImplementation::enableInteractionRelevanceAdvisorySwitch ()
  throw (rti1516e::InteractionRelevanceAdvisorySwitchIsOn,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->enableInteractionRelevanceAdvisorySwitch();
}

// 10.29
void
RTIambassadorImplementation::disableInteractionRelevanceAdvisorySwitch ()
  throw (rti1516e::InteractionRelevanceAdvisorySwitchIsOff,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->disableInteractionRelevanceAdvisorySwitch();
}

// 10.30
rti1516e::DimensionHandleSet
RTIambassadorImplementation::getDimensionHandleSet(rti1516e::RegionHandle rti1516RegionHandle)
  throw (rti1516e::InvalidRegion,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::RegionHandle regionHandle = rti1516e::RegionHandleFriend::getOpenRTIHandle(rti1516RegionHandle);

  OpenRTI::DimensionHandleSet dimensionHandleSet = _ambassadorInterface->getDimensionHandleSet(regionHandle);

  rti1516e::DimensionHandleSet rti1516DimensionHandleSet;
  for (OpenRTI::DimensionHandleSet::const_iterator i = dimensionHandleSet.begin(); i != dimensionHandleSet.end(); ++i)
    rti1516DimensionHandleSet.insert(rti1516e::DimensionHandleFriend::createHandle(*i));

  return rti1516DimensionHandleSet;
}

// 10.31
rti1516e::RangeBounds
RTIambassadorImplementation::getRangeBounds(rti1516e::RegionHandle theRegionHandle,
                                            rti1516e::DimensionHandle theDimensionHandle)
  throw (rti1516e::InvalidRegion,
         rti1516e::RegionDoesNotContainSpecifiedDimension,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  throw rti1516e::RTIinternalError(L"Not implemented");
  // FIXME
  /* _ambassadorInterface->setRangeBounds(); */
}

// 10.32
void
RTIambassadorImplementation::setRangeBounds(rti1516e::RegionHandle theRegionHandle,
                                            rti1516e::DimensionHandle theDimensionHandle,
                                            rti1516e::RangeBounds const & theRangeBounds)
  throw (rti1516e::InvalidRegion,
         rti1516e::RegionNotCreatedByThisFederate,
         rti1516e::RegionDoesNotContainSpecifiedDimension,
         rti1516e::InvalidRangeBound,
         rti1516e::FederateNotExecutionMember,
         rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  // FIXME
  /* _ambassadorInterface->setRangeBounds(); */
}

// 10.33
unsigned long
RTIambassadorImplementation::normalizeFederateHandle(rti1516e::FederateHandle rti1516FederateHandle)
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::InvalidFederateHandle,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::FederateHandle federateHandle = rti1516e::FederateHandleFriend::getOpenRTIHandle(rti1516FederateHandle);
  return _ambassadorInterface->normalizeFederateHandle(federateHandle);
}

// 10.34
unsigned long
RTIambassadorImplementation::normalizeServiceGroup(rti1516e::ServiceGroup rti1516ServiceGroup)
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::InvalidServiceGroup,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  OpenRTI::ServiceGroupIndicator serviceGroup;
  switch (rti1516ServiceGroup) {
  case rti1516e::FEDERATION_MANAGEMENT:
    serviceGroup = OpenRTI::FEDERATION_MANAGEMENT;
    break;
  case rti1516e::DECLARATION_MANAGEMENT:
    serviceGroup = OpenRTI::DECLARATION_MANAGEMENT;
    break;
  case rti1516e::OBJECT_MANAGEMENT:
    serviceGroup = OpenRTI::OBJECT_MANAGEMENT;
    break;
  case rti1516e::OWNERSHIP_MANAGEMENT:
    serviceGroup = OpenRTI::OWNERSHIP_MANAGEMENT;
    break;
  case rti1516e::TIME_MANAGEMENT:
    serviceGroup = OpenRTI::TIME_MANAGEMENT;
    break;
  case rti1516e::DATA_DISTRIBUTION_MANAGEMENT:
    serviceGroup = OpenRTI::DATA_DISTRIBUTION_MANAGEMENT;
    break;
  case rti1516e::SUPPORT_SERVICES:
    serviceGroup = OpenRTI::SUPPORT_SERVICES;
    break;
  default:
    throw rti1516e::InvalidServiceGroup(L"");
  }

  return _ambassadorInterface->normalizeServiceGroup(serviceGroup);
}

// 10.37
bool
RTIambassadorImplementation::evokeCallback(double approximateMinimumTimeInSeconds)
  throw (rti1516e::CallNotAllowedFromWithinCallback,
         rti1516e::RTIinternalError)
{
  return _ambassadorInterface->evokeCallback(approximateMinimumTimeInSeconds);
}

// 10.38
bool
RTIambassadorImplementation::evokeMultipleCallbacks(double approximateMinimumTimeInSeconds,
                                                    double approximateMaximumTimeInSeconds)
  throw (rti1516e::CallNotAllowedFromWithinCallback,
         rti1516e::RTIinternalError)
{
  return _ambassadorInterface->evokeMultipleCallbacks(approximateMinimumTimeInSeconds,
                                                      approximateMaximumTimeInSeconds);
}

// 10.39
void
RTIambassadorImplementation::enableCallbacks ()
  throw (rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->enableCallbacks();
}

// 10.40
void
RTIambassadorImplementation::disableCallbacks ()
  throw (rti1516e::SaveInProgress,
         rti1516e::RestoreInProgress,
         rti1516e::RTIinternalError)
{
  _ambassadorInterface->disableCallbacks();
}

std::auto_ptr<rti1516e::LogicalTimeFactory>
RTIambassadorImplementation::getTimeFactory() const
  throw (rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  return _ambassadorInterface->getTimeFactory();
}

rti1516e::FederateHandle
RTIambassadorImplementation::decodeFederateHandle(rti1516e::VariableLengthData const & encodedValue) const
  throw (rti1516e::CouldNotDecode,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  return rti1516e::FederateHandleFriend::createHandle(encodedValue);
}

rti1516e::ObjectClassHandle
RTIambassadorImplementation::decodeObjectClassHandle(rti1516e::VariableLengthData const & encodedValue) const
  throw (rti1516e::CouldNotDecode,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  return rti1516e::ObjectClassHandleFriend::createHandle(encodedValue);
}

rti1516e::InteractionClassHandle
RTIambassadorImplementation::decodeInteractionClassHandle(rti1516e::VariableLengthData const & encodedValue) const
  throw (rti1516e::CouldNotDecode,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  return rti1516e::InteractionClassHandleFriend::createHandle(encodedValue);
}

rti1516e::ObjectInstanceHandle
RTIambassadorImplementation::decodeObjectInstanceHandle(rti1516e::VariableLengthData const & encodedValue) const
  throw (rti1516e::CouldNotDecode,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  return rti1516e::ObjectInstanceHandleFriend::createHandle(encodedValue);
}

rti1516e::AttributeHandle
RTIambassadorImplementation::decodeAttributeHandle(rti1516e::VariableLengthData const & encodedValue) const
  throw (rti1516e::CouldNotDecode,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  return rti1516e::AttributeHandleFriend::createHandle(encodedValue);
}

rti1516e::ParameterHandle
RTIambassadorImplementation::decodeParameterHandle(rti1516e::VariableLengthData const & encodedValue) const
  throw (rti1516e::CouldNotDecode,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  return rti1516e::ParameterHandleFriend::createHandle(encodedValue);
}

rti1516e::DimensionHandle
RTIambassadorImplementation::decodeDimensionHandle(rti1516e::VariableLengthData const & encodedValue) const
  throw (rti1516e::CouldNotDecode,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  return rti1516e::DimensionHandleFriend::createHandle(encodedValue);
}

rti1516e::MessageRetractionHandle
RTIambassadorImplementation::decodeMessageRetractionHandle(rti1516e::VariableLengthData const & encodedValue) const
  throw (rti1516e::CouldNotDecode,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  return rti1516e::MessageRetractionHandleFriend::createHandle(encodedValue);
}

rti1516e::RegionHandle
RTIambassadorImplementation::decodeRegionHandle(rti1516e::VariableLengthData const & encodedValue) const
  throw (rti1516e::CouldNotDecode,
         rti1516e::FederateNotExecutionMember,
         rti1516e::NotConnected,
         rti1516e::RTIinternalError)
{
  return rti1516e::RegionHandleFriend::createHandle(encodedValue);
}

}
