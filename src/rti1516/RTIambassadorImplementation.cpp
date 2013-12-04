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

// This time, the first include is above the api include.
// the rti1516/Exception header misses that.
#include <iosfwd>

#include "RTIambassadorImplementation.h"

#include <algorithm>
#include <fstream>
#include <memory>

#include <RTI/RTIambassador.h>
#include <RTI/FederateAmbassador.h>
#include <RTI/LogicalTime.h>
#include <RTI/LogicalTimeInterval.h>
#include <RTI/LogicalTimeFactory.h>
#include <RTI/RangeBounds.h>

#include "Ambassador.h"
#include "FDD1516FileReader.h"
#include "LogStream.h"
#include "TemplateTimeManagement.h"

#include "HandleImplementation.h"
#include "RTI1516LogicalTimeFactory.h"
#include "RTI1516integer64TimeFactory.h"
#include "RTI1516float64TimeFactory.h"
#include "VariableLengthDataImplementation.h"

namespace OpenRTI {

static rti1516::OrderType translate(OpenRTI::OrderType orderType)
{
  switch (orderType) {
  case OpenRTI::TIMESTAMP:
    return rti1516::TIMESTAMP;
  case OpenRTI::RECEIVE:
  default:
    return rti1516::RECEIVE;
  }
}

static OpenRTI::OrderType translate(rti1516::OrderType orderType)
{
  switch (orderType) {
  case rti1516::TIMESTAMP:
    return OpenRTI::TIMESTAMP;
  case rti1516::RECEIVE:
  default:
    return OpenRTI::RECEIVE;
  }
}

static rti1516::TransportationType translate(OpenRTI::TransportationType transportationType)
{
  switch (transportationType) {
  case OpenRTI::BEST_EFFORT:
    return rti1516::BEST_EFFORT;
  case OpenRTI::RELIABLE:
  default:
    return rti1516::RELIABLE;
  }
}

static OpenRTI::TransportationType translate(rti1516::TransportationType transportationType)
{
  switch (transportationType) {
  case rti1516::BEST_EFFORT:
    return OpenRTI::BEST_EFFORT;
  case rti1516::RELIABLE:
  default:
    return OpenRTI::RELIABLE;
  }
}

class OPENRTI_LOCAL RTI1516Traits {
public:
  // The bindings have different logical times
  typedef rti1516::LogicalTime NativeLogicalTime;
  typedef rti1516::LogicalTimeInterval NativeLogicalTimeInterval;

  typedef rti1516::FederateAmbassador FederateAmbassador;

  // static OpenRTI::FederateHandle fromNative(const rti1516::FederateHandle& federateHandle)
  // { return rti1516::FederateHandleFriend::getOpenRTIHandle(federateHandle); }
  // static rti1516::FederateHandle toNative(const OpenRTI::FederateHandle& federateHandle)
  // { return rti1516::FederateHandleFriend::createHandle(federateHandle); }

  // static OpenRTI::ObjectClassHandle fromNative(const rti1516::ObjectClassHandle& objectClassHandle)
  // { return rti1516::ObjectClassHandleFriend::getOpenRTIHandle(objectClassHandle); }
  // static rti1516::ObjectClassHandle toNative(const OpenRTI::ObjectClassHandle& objectClassHandle)
  // { return rti1516::ObjectClassHandleFriend::createHandle(objectClassHandle); }

  // static OpenRTI::InteractionClassHandle fromNative(const rti1516::InteractionClassHandle& interactionClassHandle)
  // { return rti1516::InteractionClassHandleFriend::getOpenRTIHandle(interactionClassHandle); }
  // static rti1516::InsteractionClassHandle toNative(const OpenRTI::InsteractionClassHandle& insteractionClassHandle)
  // { return rti1516::InsteractionClassHandleFriend::createHandle(insteractionClassHandle); }

  // static OpenRTI::ObjectInstanceHandle fromNative(const rti1516::ObjectInstanceHandle& objectInstanceHandle)
  // { return rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(objectInstanceHandle); }
  // static rti1516::ObjectInstanceHandle toNative(const OpenRTI::ObjectInstanceHandle& objectInstanceHandle)
  // { return rti1516::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle); }

  // static OpenRTI::AttributeHandle fromNative(const rti1516::AttributeHandle& attributeHandle)
  // { return rti1516::AttributeHandleFriend::getOpenRTIHandle(attributeHandle); }
  // static rti1516::AttributeHandle toNative(const OpenRTI::AttributeHandle& attributeHandle)
  // { return rti1516::AttributeHandleFriend::createHandle(attributeHandle); }

  // static OpenRTI::ParameterHandle fromNative(const rti1516::ParameterHandle& parameterHandle)
  // { return rti1516::ParameterHandleFriend::getOpenRTIHandle(parameterHandle); }
  // static rti1516::ParameterHandle toNative(const OpenRTI::ParameterHandle& parameterHandle)
  // { return rti1516::ParameterHandleFriend::createHandle(parameterHandle); }

  // static OpenRTI::DimensionHandle fromNative(const rti1516::DimensionHandle& dimensionHandle)
  // { return rti1516::DimensionHandleFriend::getOpenRTIHandle(dimensionHandle); }
  // static rti1516::DimensionHandle toNative(const OpenRTI::DimensionHandle& dimensionHandle)
  // { return rti1516::DimensionHandleFriend::createHandle(dimensionHandle); }

  // static OpenRTI::RegionHandle fromNative(const rti1516::RegionHandle& regionHandle)
  // { return rti1516::RegionHandleFriend::getOpenRTIHandle(regionHandle); }
  // static rti1516::RegionHandle toNative(const OpenRTI::RegionHandle& regionHandle)
  // { return rti1516::RegionHandleFriend::createHandle(regionHandle); }

  // static OpenRTI::MessageRetractionHandle fromNative(const rti1516::MessageRetractionHandle& messageRetractionHandle)
  // { return rti1516::MessageRetractionHandleFriend::getOpenRTIHandle(messageRetractionHandle); }
  // static rti1516::MessageRetractionHandle toNative(const OpenRTI::MessageRetractionHandle& messageRetractionHandle)
  // { return rti1516::MessageRetractionHandleFriend::createHandle(messageRetractionHandle); }


  // // Ok, have here also methods to convert handle sets to handle sets!!!!
  // static void copy(OpenRTI::AttributeHandleSet& dst, const rti1516::AttributeHandleSet& src)
  // {
  //   for (rti1516::AttributeHandleSet::const_iterator i = src.begin(); i != src.end(); ++i)
  //     dst.insert(rti1516::AttributeHandleFriend::getOpenRTIHandle(*i));
  // }
  // static void copy(rti1516::AttributeHandleSet& dst, const OpenRTI::AttributeHandleSet& src)
  // {
  //   for (OpenRTI::AttributeHandleSet::const_iterator i = src.begin(); i != src.end(); ++i)
  //     dst.insert(rti1516::AttributeHandleFriend::createHandle(*i));
  // }
  // static void copy(OpenRTI::AttributeHandleVector& dst, const rti1516::AttributeHandleSet& src)
  // {
  //   dst.reserve(src.size());
  //   for (rti1516::AttributeHandleSet::const_iterator i = src.begin(); i != src.end(); ++i)
  //     dst.push_back(rti1516::AttributeHandleFriend::getOpenRTIHandle(*i));
  // }
  // static void copy(rti1516::AttributeHandleSet& dst, const OpenRTI::AttributeHandleVector& src)
  // {
  //   for (OpenRTI::AttributeHandleVector::const_iterator i = src.begin(); i != src.end(); ++i)
  //     dst.insert(rti1516::AttributeHandleFriend::createHandle(*i));
  // }

  // The exceptions
  typedef rti1516::Exception Exception;

#define MAP_EXCEPTION(Exception, MappedException) \
  typedef MappedException Exception; \
  static void throw ## Exception(const std::string& reason) { throw MappedException(OpenRTI::utf8ToUcs(reason)); } \
  static void throw ## Exception(const std::wstring& reason) { throw MappedException(reason); } \
  static void throw ## Exception() { throw MappedException(std::wstring(L"/* NO COMMENT */")); }

  MAP_EXCEPTION(AlreadyConnected, OpenRTI::IgnoredError)
  MAP_EXCEPTION(AsynchronousDeliveryAlreadyDisabled, rti1516::AsynchronousDeliveryAlreadyDisabled)
  MAP_EXCEPTION(AsynchronousDeliveryAlreadyEnabled, rti1516::AsynchronousDeliveryAlreadyEnabled)
  MAP_EXCEPTION(AttributeAcquisitionWasNotRequested, rti1516::AttributeAcquisitionWasNotRequested)
  MAP_EXCEPTION(AttributeAlreadyBeingAcquired, rti1516::AttributeAlreadyBeingAcquired)
  MAP_EXCEPTION(AttributeAlreadyBeingDivested, rti1516::AttributeAlreadyBeingDivested)
  MAP_EXCEPTION(AttributeAlreadyOwned, rti1516::AttributeAlreadyOwned)
  MAP_EXCEPTION(AttributeDivestitureWasNotRequested, rti1516::AttributeDivestitureWasNotRequested)
  MAP_EXCEPTION(AttributeNotDefined, rti1516::AttributeNotDefined)
  MAP_EXCEPTION(AttributeNotOwned, rti1516::AttributeNotOwned)
  MAP_EXCEPTION(AttributeNotPublished, rti1516::AttributeNotPublished)
  MAP_EXCEPTION(AttributeRelevanceAdvisorySwitchIsOff, rti1516::AttributeRelevanceAdvisorySwitchIsOff)
  MAP_EXCEPTION(AttributeRelevanceAdvisorySwitchIsOn, rti1516::AttributeRelevanceAdvisorySwitchIsOn)
  MAP_EXCEPTION(AttributeScopeAdvisorySwitchIsOff, rti1516::AttributeScopeAdvisorySwitchIsOff)
  MAP_EXCEPTION(AttributeScopeAdvisorySwitchIsOn, rti1516::AttributeScopeAdvisorySwitchIsOn)
  MAP_EXCEPTION(CallNotAllowedFromWithinCallback, rti1516::RTIinternalError)
  MAP_EXCEPTION(ConnectionFailed, rti1516::RTIinternalError)
  MAP_EXCEPTION(CouldNotCreateLogicalTimeFactory, rti1516::CouldNotCreateLogicalTimeFactory)
  MAP_EXCEPTION(CouldNotOpenFDD, rti1516::CouldNotOpenFDD)
  MAP_EXCEPTION(CouldNotOpenMIM, rti1516::CouldNotOpenFDD)
  MAP_EXCEPTION(DeletePrivilegeNotHeld, rti1516::DeletePrivilegeNotHeld)
  MAP_EXCEPTION(ErrorReadingFDD, rti1516::ErrorReadingFDD)
  MAP_EXCEPTION(ErrorReadingMIM, rti1516::ErrorReadingFDD)
  MAP_EXCEPTION(FederateAlreadyExecutionMember, rti1516::FederateAlreadyExecutionMember)
  MAP_EXCEPTION(FederateNameAlreadyInUse, rti1516::RTIinternalError)
  MAP_EXCEPTION(FederateHasNotBegunSave, rti1516::FederateHasNotBegunSave)
  MAP_EXCEPTION(FederateIsExecutionMember, rti1516::FederateAlreadyExecutionMember)
  MAP_EXCEPTION(FederateNotExecutionMember, rti1516::FederateNotExecutionMember)
  MAP_EXCEPTION(FederateOwnsAttributes, rti1516::FederateOwnsAttributes)
  MAP_EXCEPTION(FederatesCurrentlyJoined, rti1516::FederatesCurrentlyJoined)
  MAP_EXCEPTION(FederateServiceInvocationsAreBeingReportedViaMOM, rti1516::FederateServiceInvocationsAreBeingReportedViaMOM)
  MAP_EXCEPTION(FederateUnableToUseTime, rti1516::FederateUnableToUseTime)
  MAP_EXCEPTION(FederationExecutionAlreadyExists, rti1516::FederationExecutionAlreadyExists)
  MAP_EXCEPTION(FederationExecutionDoesNotExist, rti1516::FederationExecutionDoesNotExist)
  MAP_EXCEPTION(IllegalName, rti1516::IllegalName)
  MAP_EXCEPTION(NameSetWasEmpty, rti1516::RTIinternalError) // does not happen
  MAP_EXCEPTION(InconsistentFDD, rti1516::ErrorReadingFDD)
  MAP_EXCEPTION(InteractionClassNotDefined, rti1516::InteractionClassNotDefined)
  MAP_EXCEPTION(InteractionClassNotPublished, rti1516::InteractionClassNotPublished)
  MAP_EXCEPTION(InteractionParameterNotDefined, rti1516::InteractionParameterNotDefined)
  MAP_EXCEPTION(InteractionRelevanceAdvisorySwitchIsOff, rti1516::InteractionRelevanceAdvisorySwitchIsOff)
  MAP_EXCEPTION(InteractionRelevanceAdvisorySwitchIsOn, rti1516::InteractionRelevanceAdvisorySwitchIsOn)
  MAP_EXCEPTION(InTimeAdvancingState, rti1516::InTimeAdvancingState)
  MAP_EXCEPTION(InvalidAttributeHandle, rti1516::InvalidAttributeHandle)
  MAP_EXCEPTION(InvalidDimensionHandle, rti1516::InvalidDimensionHandle)
  MAP_EXCEPTION(InvalidFederateHandle, rti1516::InvalidFederateHandle)
  MAP_EXCEPTION(InvalidInteractionClassHandle, rti1516::InvalidInteractionClassHandle)
  MAP_EXCEPTION(InvalidLocalSettingsDesignator, rti1516::RTIinternalError)
  MAP_EXCEPTION(InvalidLogicalTime, rti1516::InvalidLogicalTime)
  MAP_EXCEPTION(InvalidLookahead, rti1516::InvalidLookahead)
  MAP_EXCEPTION(InvalidObjectClassHandle, rti1516::InvalidObjectClassHandle)
  MAP_EXCEPTION(InvalidOrderName, rti1516::InvalidOrderName)
  MAP_EXCEPTION(InvalidOrderType, rti1516::InvalidOrderType)
  MAP_EXCEPTION(InvalidParameterHandle, rti1516::InvalidParameterHandle)
  MAP_EXCEPTION(InvalidRangeBound, rti1516::InvalidRangeBound)
  MAP_EXCEPTION(InvalidRegion, rti1516::InvalidRegion)
  MAP_EXCEPTION(InvalidRegionContext, rti1516::InvalidRegionContext)
  MAP_EXCEPTION(InvalidResignAction, rti1516::RTIinternalError)
  MAP_EXCEPTION(InvalidRetractionHandle, rti1516::InvalidRetractionHandle)
  MAP_EXCEPTION(InvalidServiceGroup, rti1516::InvalidServiceGroup)
  MAP_EXCEPTION(InvalidTransportationName, rti1516::InvalidTransportationName)
  MAP_EXCEPTION(InvalidTransportationType, rti1516::InvalidTransportationType)
  MAP_EXCEPTION(LogicalTimeAlreadyPassed, rti1516::LogicalTimeAlreadyPassed)
  MAP_EXCEPTION(MessageCanNoLongerBeRetracted, rti1516::MessageCanNoLongerBeRetracted)
  MAP_EXCEPTION(NameNotFound, rti1516::NameNotFound)
  MAP_EXCEPTION(NoAcquisitionPending, rti1516::NoAcquisitionPending)
  MAP_EXCEPTION(NotConnected, rti1516::FederateNotExecutionMember)
  MAP_EXCEPTION(ObjectClassNotDefined, rti1516::ObjectClassNotDefined)
  MAP_EXCEPTION(ObjectClassNotPublished, rti1516::ObjectClassNotPublished)
  MAP_EXCEPTION(ObjectClassRelevanceAdvisorySwitchIsOff, rti1516::ObjectClassRelevanceAdvisorySwitchIsOff)
  MAP_EXCEPTION(ObjectClassRelevanceAdvisorySwitchIsOn, rti1516::ObjectClassRelevanceAdvisorySwitchIsOn)
  MAP_EXCEPTION(ObjectInstanceNameInUse, rti1516::ObjectInstanceNameInUse)
  MAP_EXCEPTION(ObjectInstanceNameNotReserved, rti1516::ObjectInstanceNameNotReserved)
  MAP_EXCEPTION(ObjectInstanceNotKnown, rti1516::ObjectInstanceNotKnown)
  MAP_EXCEPTION(OwnershipAcquisitionPending, rti1516::OwnershipAcquisitionPending)
  MAP_EXCEPTION(RegionDoesNotContainSpecifiedDimension, rti1516::RegionDoesNotContainSpecifiedDimension)
  MAP_EXCEPTION(RegionInUseForUpdateOrSubscription, rti1516::RegionInUseForUpdateOrSubscription)
  MAP_EXCEPTION(RegionNotCreatedByThisFederate, rti1516::RegionNotCreatedByThisFederate)
  MAP_EXCEPTION(RequestForTimeConstrainedPending, rti1516::RequestForTimeConstrainedPending)
  MAP_EXCEPTION(RequestForTimeRegulationPending, rti1516::RequestForTimeRegulationPending)
  MAP_EXCEPTION(RestoreInProgress, rti1516::RestoreInProgress)
  MAP_EXCEPTION(RestoreNotRequested, rti1516::RestoreNotRequested)
  MAP_EXCEPTION(RTIinternalError, rti1516::RTIinternalError)
  MAP_EXCEPTION(SaveInProgress, rti1516::SaveInProgress)
  MAP_EXCEPTION(SaveNotInitiated, rti1516::SaveNotInitiated)
  MAP_EXCEPTION(SynchronizationPointLabelNotAnnounced, rti1516::SynchronizationPointLabelNotAnnounced)
  MAP_EXCEPTION(TimeConstrainedAlreadyEnabled, rti1516::TimeConstrainedAlreadyEnabled)
  MAP_EXCEPTION(TimeConstrainedIsNotEnabled, rti1516::TimeConstrainedIsNotEnabled)
  MAP_EXCEPTION(TimeRegulationAlreadyEnabled, rti1516::TimeRegulationAlreadyEnabled)
  MAP_EXCEPTION(TimeRegulationIsNotEnabled, rti1516::TimeRegulationIsNotEnabled)
  MAP_EXCEPTION(UnsupportedCallbackModel, rti1516::RTIinternalError)
#undef MAP_EXCEPTION
};

class OPENRTI_LOCAL RTIambassadorImplementation::RTI1516AmbassadorInterface : public OpenRTI::Ambassador<RTI1516Traits> {
public:
  RTI1516AmbassadorInterface(const std::vector<std::wstring>& args) :
    Ambassador<RTI1516Traits>(),
    _federateAmbassador(0),
    _forceOpaqueLogicalTime(false)
  {
    for (unsigned i = 0; i < args.size(); ++i) {
      if (args[i] == L"force-opaque-time") {
        _forceOpaqueLogicalTime = true;
      } else {
        std::wstring::size_type pos = args[i].find('=');
        if (pos == std::wstring::npos)
          continue;
        if (args[i].size() <= pos + 1)
          continue;
        std::string key = ucsToUtf8(args[i].substr(0, pos));
        std::string value = ucsToUtf8(args[i].substr(pos + 1));
        if (key == "address") {
          URL url = URL::fromProtocolAddress(_defaultUrl.getProtocol(), value);
          _defaultUrl.setHost(url.getHost());
          _defaultUrl.setService(url.getService());
        } else if (key == "protocol") {
          _defaultUrl.setProtocol(value);
        } else {
          _stringStringListMap[key].push_back(value);
        }
      }
    }
  }

  virtual void connectionLost(const std::string& faultDescription)
  {
    throw rti1516::RTIinternalError(utf8ToUcs(faultDescription));
  }

  virtual void reportFederationExecutions(const FederationExecutionInformationVector& theFederationExecutionInformationList)
    throw ()
  { }

  virtual void synchronizationPointRegistrationResponse(const std::string& label, RegisterFederationSynchronizationPointResponseType reason)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      switch (reason) {
      case OpenRTI::RegisterFederationSynchronizationPointResponseSuccess:
        _federateAmbassador->synchronizationPointRegistrationSucceeded(utf8ToUcs(label));
        break;
      case OpenRTI::RegisterFederationSynchronizationPointResponseLabelNotUnique:
        _federateAmbassador->synchronizationPointRegistrationFailed(utf8ToUcs(label), rti1516::SYNCHRONIZATION_POINT_LABEL_NOT_UNIQUE);
        break;
      case OpenRTI::RegisterFederationSynchronizationPointResponseMemberNotJoined:
        _federateAmbassador->synchronizationPointRegistrationFailed(utf8ToUcs(label), rti1516::SYNCHRONIZATION_SET_MEMBER_NOT_JOINED);
        break;
      }
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void announceSynchronizationPoint(const std::string& label, const OpenRTI::VariableLengthData& tag)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516::VariableLengthData rti1516Tag = rti1516::VariableLengthDataFriend::create(tag);
      _federateAmbassador->announceSynchronizationPoint(utf8ToUcs(label), rti1516Tag);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void federationSynchronized(const std::string& label)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->federationSynchronized(utf8ToUcs(label));
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // // 4.12
  // virtual void
  // initiateFederateSave(std::string const& label)
  //   throw (OpenRTI::UnableToPerformSave,
  //          OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->initiateFederateSave(label);
  //   } catch (const rti1516::UnableToPerformSave& e) {
  //     throw OpenRTI::UnableToPerformSave(e.what());
  //   } catch (const rti1516::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // virtual void
  // initiateFederateSave(const std::string& label,
  //                      const LogicalTime& logicalTime)
  //   throw (OpenRTI::UnableToPerformSave,
  //          OpenRTI::InvalidLogicalTime,
  //          OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->initiateFederateSave(label, RTI1516LogicalTimeImplementation::getLogicalTime(logicalTime));
  //   } catch (const rti1516::UnableToPerformSave& e) {
  //     throw OpenRTI::UnableToPerformSave(e.what());
  //   } catch (const rti1516::InvalidLogicalTime& e) {
  //     throw OpenRTI::InvalidLogicalTime(e.what());
  //   } catch (const rti1516::Exception& e) {
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
  //   } catch (const rti1516::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // virtual void
  // federationNotSaved(OpenRTI::SaveFailureReason reason)
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();

  //   rti1516::SaveFailureReason rti1516Reason;
  //   switch (reason) {
  //   case OpenRTI::RTI_UNABLE_TO_SAVE:
  //     rti1516Reason = rti1516::RTI_UNABLE_TO_SAVE;
  //     break;
  //   case OpenRTI::FEDERATE_REPORTED_FAILURE_DURING_SAVE:
  //     rti1516Reason = rti1516::FEDERATE_REPORTED_FAILURE_DURING_SAVE;
  //     break;
  //   case OpenRTI::FEDERATE_RESIGNED_DURING_SAVE:
  //     rti1516Reason = rti1516::FEDERATE_RESIGNED_DURING_SAVE;
  //     break;
  //   case OpenRTI::RTI_DETECTED_FAILURE_DURING_SAVE:
  //     rti1516Reason = rti1516::RTI_DETECTED_FAILURE_DURING_SAVE;
  //     break;
  //   case OpenRTI::SAVE_TIME_CANNOT_BE_HONORED:
  //     rti1516Reason = rti1516::SAVE_TIME_CANNOT_BE_HONORED;
  //     break;
  //   default:
  //     throw OpenRTI::FederateInternalError();
  //   }

  //   try {
  //     _federateAmbassador->federationNotSaved(rti1516Reason);
  //   } catch (const rti1516::Exception& e) {
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
  //     throw rti1516::FederateInternalError(L"Not implemented");
  //     // _federateAmbassador
  //   } catch (const rti1516::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // // 4.19
  // virtual void
  // requestFederationRestoreSucceeded(std::string const& label)
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->requestFederationRestoreSucceeded(label);
  //   } catch (const rti1516::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // virtual void
  // requestFederationRestoreFailed(std::string const& label)
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->requestFederationRestoreFailed(label);
  //   } catch (const rti1516::Exception& e) {
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
  //   } catch (const rti1516::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // // 4.21
  // virtual void
  // initiateFederateRestore(std::string const & label,
  //                         FederateHandle handle)
  //   throw (OpenRTI::SpecifiedSaveLabelDoesNotExist,
  //          OpenRTI::CouldNotInitiateRestore,
  //          OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     rti1516::FederateHandle rti1516Handle = rti1516::FederateHandleFriend::createHandle(handle);
  //     _federateAmbassador->initiateFederateRestore(label, rti1516Handle);
  //   } catch (const rti1516::SpecifiedSaveLabelDoesNotExist& e) {
  //     throw OpenRTI::SpecifiedSaveLabelDoesNotExist(e.what());
  //   } catch (const rti1516::CouldNotInitiateRestore& e) {
  //     throw OpenRTI::CouldNotInitiateRestore(e.what());
  //   } catch (const rti1516::Exception& e) {
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
  //   } catch (const rti1516::Exception& e) {
  //     throw OpenRTI::FederateInternalError(e.what());
  //   }
  // }

  // virtual void
  // federationNotRestored(RestoreFailureReason reason)
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();

  //   rti1516::RestoreFailureReason rti1516Reason;
  //   switch (reason) {
  //   case OpenRTI::RTI_UNABLE_TO_RESTORE:
  //     rti1516Reason = rti1516::RTI_UNABLE_TO_RESTORE;
  //     break;
  //   case OpenRTI::FEDERATE_REPORTED_FAILURE_DURING_RESTORE:
  //     rti1516Reason = rti1516::FEDERATE_REPORTED_FAILURE_DURING_RESTORE;
  //     break;
  //   case OpenRTI::FEDERATE_RESIGNED_DURING_RESTORE:
  //     rti1516Reason = rti1516::FEDERATE_RESIGNED_DURING_RESTORE;
  //     break;
  //   case OpenRTI::RTI_DETECTED_FAILURE_DURING_RESTORE:
  //     rti1516Reason = rti1516::RTI_DETECTED_FAILURE_DURING_RESTORE;
  //     break;
  //   default:
  //     throw OpenRTI::FederateInternalError();
  //   }

  //   try {
  //     _federateAmbassador->federationNotRestored(rti1516Reason);
  //   } catch (const rti1516::Exception& e) {
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
  //     throw rti1516::FederateInternalError(L"Not implemented");
  //     // _federateAmbassador
  //   } catch (const rti1516::Exception& e) {
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
      rti1516::ObjectClassHandle rti1516Handle = rti1516::ObjectClassHandleFriend::createHandle(objectClassHandle);
      if (start)
        _federateAmbassador->startRegistrationForObjectClass(rti1516Handle);
      else
        _federateAmbassador->stopRegistrationForObjectClass(rti1516Handle);
    } catch (const rti1516::Exception& e) {
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
      rti1516::InteractionClassHandle rti1516Handle = rti1516::InteractionClassHandleFriend::createHandle(interactionClassHandle);
      if (on)
        _federateAmbassador->turnInteractionsOn(rti1516Handle);
      else
        _federateAmbassador->turnInteractionsOff(rti1516Handle);
    } catch (const rti1516::Exception& e) {
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
        _federateAmbassador->objectInstanceNameReservationSucceeded(utf8ToUcs(message.getObjectInstanceHandleNamePair().second));
      else
        _federateAmbassador->objectInstanceNameReservationFailed(utf8ToUcs(message.getObjectInstanceHandleNamePair().second));
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }
  virtual void objectInstanceNameReservation(const OpenRTI::ReserveMultipleObjectInstanceNameResponseMessage& message)
    throw ()
  {
    Log(FederateAmbassador, Warning) << "Ignoring unexpected multiple name reservation response!" << std::endl;
  }

  // 6.5
  virtual
  void
  discoverObjectInstance(ObjectInstanceHandle objectInstanceHandle,
                         ObjectClassHandle objectClassHandle,
                         std::string const& name)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle
        = rti1516::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);
      rti1516::ObjectClassHandle rti1516ObjectClassHandle
        = rti1516::ObjectClassHandleFriend::createHandle(objectClassHandle);

      _federateAmbassador->discoverObjectInstance(rti1516ObjectInstanceHandle,
                                                  rti1516ObjectClassHandle,
                                                  utf8ToUcs(name));
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void reflectAttributeValues(const Federate::ObjectClass& objectClass, const AttributeUpdateMessage& message)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516::AttributeHandleValueMap rti1516AttributeValues;
      for (std::vector<OpenRTI::AttributeValue>::const_iterator i = message.getAttributeValues().begin();
           i != message.getAttributeValues().end(); ++i) {
        if (objectClass.getAttributeSubscriptionType(i->getAttributeHandle()) == Unsubscribed)
          continue;
        rti1516AttributeValues[rti1516::AttributeHandleFriend::createHandle(i->getAttributeHandle())]
          = rti1516::VariableLengthDataFriend::create(i->getValue());
      }

      if (!rti1516AttributeValues.empty()) {
        rti1516::VariableLengthData rti1516Tag = rti1516::VariableLengthDataFriend::create(message.getTag());

        rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle;
        rti1516ObjectInstanceHandle = rti1516::ObjectInstanceHandleFriend::createHandle(message.getObjectInstanceHandle());

        rti1516::TransportationType rti1516TransportationType = translate(message.getTransportationType());

        _federateAmbassador->reflectAttributeValues(rti1516ObjectInstanceHandle, rti1516AttributeValues, rti1516Tag, rti1516::RECEIVE,
                                                    rti1516TransportationType);
      }
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void reflectAttributeValues(const Federate::ObjectClass& objectClass, bool flushQueueMode, OrderType orderType,
                                      const TimeStampedAttributeUpdateMessage& message, const rti1516::LogicalTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516::AttributeHandleValueMap rti1516AttributeValues;
      for (std::vector<OpenRTI::AttributeValue>::const_iterator i = message.getAttributeValues().begin();
           i != message.getAttributeValues().end(); ++i) {
        if (objectClass.getAttributeSubscriptionType(i->getAttributeHandle()) == Unsubscribed)
          continue;
        rti1516AttributeValues[rti1516::AttributeHandleFriend::createHandle(i->getAttributeHandle())]
          = rti1516::VariableLengthDataFriend::create(i->getValue());
      }

      if (!rti1516AttributeValues.empty()) {
        rti1516::VariableLengthData rti1516Tag = rti1516::VariableLengthDataFriend::create(message.getTag());

        rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle;
        rti1516ObjectInstanceHandle = rti1516::ObjectInstanceHandleFriend::createHandle(message.getObjectInstanceHandle());

        rti1516::OrderType rti1516SentOrderType = translate(message.getOrderType());
        rti1516::OrderType rti1516ReceivedOrderType = translate(orderType);
        rti1516::TransportationType rti1516TransportationType = translate(message.getTransportationType());

        if (flushQueueMode) {
          rti1516::MessageRetractionHandle rti1516MessageRetractionHandle;
          rti1516MessageRetractionHandle = rti1516::MessageRetractionHandleFriend::createHandle(message.getMessageRetractionHandle());
          _federateAmbassador->reflectAttributeValues(rti1516ObjectInstanceHandle, rti1516AttributeValues, rti1516Tag, rti1516SentOrderType,
                                                      rti1516TransportationType, logicalTime, rti1516ReceivedOrderType, rti1516MessageRetractionHandle);
        } else {
          _federateAmbassador->reflectAttributeValues(rti1516ObjectInstanceHandle, rti1516AttributeValues, rti1516Tag, rti1516SentOrderType,
                                                      rti1516TransportationType, logicalTime, rti1516ReceivedOrderType);
        }
      }
    } catch (const rti1516::Exception& e) {
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
      rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle;
      rti1516ObjectInstanceHandle = rti1516::ObjectInstanceHandleFriend::createHandle(message.getObjectInstanceHandle());
      rti1516::VariableLengthData rti1516Tag = rti1516::VariableLengthDataFriend::create(message.getTag());
      _federateAmbassador->removeObjectInstance(rti1516ObjectInstanceHandle, rti1516Tag, rti1516::RECEIVE);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void removeObjectInstance(bool flushQueueMode, OrderType orderType,
                                    const TimeStampedDeleteObjectInstanceMessage& message, const rti1516::LogicalTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle;
      rti1516ObjectInstanceHandle = rti1516::ObjectInstanceHandleFriend::createHandle(message.getObjectInstanceHandle());
      rti1516::VariableLengthData rti1516Tag = rti1516::VariableLengthDataFriend::create(message.getTag());
      rti1516::OrderType rti1516SentOrderType = translate(message.getOrderType());
      rti1516::OrderType rti1516ReceivedOrderType = translate(orderType);
      if (flushQueueMode) {
        rti1516::MessageRetractionHandle rti1516MessageRetractionHandle;
        rti1516MessageRetractionHandle = rti1516::MessageRetractionHandleFriend::createHandle(message.getMessageRetractionHandle());
        _federateAmbassador->removeObjectInstance(rti1516ObjectInstanceHandle, rti1516Tag, rti1516SentOrderType,
                                                  logicalTime, rti1516ReceivedOrderType, rti1516MessageRetractionHandle);
      } else {
        _federateAmbassador->removeObjectInstance(rti1516ObjectInstanceHandle, rti1516Tag, rti1516SentOrderType,
                                                  logicalTime, rti1516ReceivedOrderType);
      }
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }


  // 6.9
  virtual void
  receiveInteraction(const InteractionClassHandle& interactionClassHandle, const Federate::InteractionClass& interactionClass, const InteractionMessage& message)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516::InteractionClassHandle rti1516InteractionClassHandle;
      rti1516InteractionClassHandle = rti1516::InteractionClassHandleFriend::createHandle(interactionClassHandle);

      rti1516::ParameterHandleValueMap rti1516ParameterValues;
      for (std::vector<OpenRTI::ParameterValue>::const_iterator i = message.getParameterValues().begin();
           i != message.getParameterValues().end(); ++i) {
        if (!interactionClass.getParameter(i->getParameterHandle()))
          continue;
        rti1516ParameterValues[rti1516::ParameterHandleFriend::createHandle(i->getParameterHandle())]
          = rti1516::VariableLengthDataFriend::create(i->getValue());
      }

      rti1516::VariableLengthData rti1516Tag;
      rti1516Tag = rti1516::VariableLengthDataFriend::create(message.getTag());
      rti1516::TransportationType rti1516TransportationType;
      rti1516TransportationType = translate(message.getTransportationType());
      _federateAmbassador->receiveInteraction(rti1516InteractionClassHandle, rti1516ParameterValues, rti1516Tag, rti1516::RECEIVE,
                                              rti1516TransportationType);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual void
  receiveInteraction(const InteractionClassHandle& interactionClassHandle, const Federate::InteractionClass& interactionClass, bool flushQueueMode, OrderType orderType,
                     const TimeStampedInteractionMessage& message, const rti1516::LogicalTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      rti1516::InteractionClassHandle rti1516InteractionClassHandle;
      rti1516InteractionClassHandle = rti1516::InteractionClassHandleFriend::createHandle(interactionClassHandle);

      rti1516::ParameterHandleValueMap rti1516ParameterValues;
      for (std::vector<OpenRTI::ParameterValue>::const_iterator i = message.getParameterValues().begin();
           i != message.getParameterValues().end(); ++i) {
        if (!interactionClass.getParameter(i->getParameterHandle()))
          continue;
        rti1516ParameterValues[rti1516::ParameterHandleFriend::createHandle(i->getParameterHandle())]
          = rti1516::VariableLengthDataFriend::create(i->getValue());
      }

      rti1516::VariableLengthData rti1516Tag;
      rti1516Tag = rti1516::VariableLengthDataFriend::create(message.getTag());
      rti1516::OrderType rti1516SentOrderType = translate(message.getOrderType());
      rti1516::OrderType rti1516ReceivedOrderType = translate(orderType);
      rti1516::TransportationType rti1516TransportationType;
      rti1516TransportationType = translate(message.getTransportationType());

      if (flushQueueMode) {
        rti1516::MessageRetractionHandle rti1516MessageRetractionHandle;
        rti1516MessageRetractionHandle = rti1516::MessageRetractionHandleFriend::createHandle(message.getMessageRetractionHandle());
        _federateAmbassador->receiveInteraction(rti1516InteractionClassHandle, rti1516ParameterValues, rti1516Tag, rti1516SentOrderType,
                                                rti1516TransportationType, logicalTime, rti1516ReceivedOrderType, rti1516MessageRetractionHandle);
      } else {
        _federateAmbassador->receiveInteraction(rti1516InteractionClassHandle, rti1516ParameterValues, rti1516Tag, rti1516SentOrderType,
                                                rti1516TransportationType, logicalTime, rti1516ReceivedOrderType);
      }
    } catch (const rti1516::Exception& e) {
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
      rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle = rti1516::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);

      rti1516::AttributeHandleSet rti1516AttributeHandleSet;
      for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
        rti1516AttributeHandleSet.insert(rti1516::AttributeHandleFriend::createHandle(*i));

      _federateAmbassador->attributesInScope(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516::Exception& e) {
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
      rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle = rti1516::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);

      rti1516::AttributeHandleSet rti1516AttributeHandleSet;
      for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
        rti1516AttributeHandleSet.insert(rti1516::AttributeHandleFriend::createHandle(*i));

      _federateAmbassador->attributesOutOfScope(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516::Exception& e) {
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
      rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle = rti1516::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);

      rti1516::AttributeHandleSet rti1516AttributeHandleSet;
      for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
        rti1516AttributeHandleSet.insert(rti1516::AttributeHandleFriend::createHandle(*i));

      rti1516::VariableLengthData rti1516Tag = rti1516::VariableLengthDataFriend::create(tag);

      _federateAmbassador->provideAttributeValueUpdate(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet, rti1516Tag);
    } catch (const rti1516::Exception& e) {
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
      rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle = rti1516::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);

      rti1516::AttributeHandleSet rti1516AttributeHandleSet;
      for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
        rti1516AttributeHandleSet.insert(rti1516::AttributeHandleFriend::createHandle(*i));

      _federateAmbassador->turnUpdatesOnForObjectInstance(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516::Exception& e) {
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
      rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle = rti1516::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);

      rti1516::AttributeHandleSet rti1516AttributeHandleSet;
      for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
        rti1516AttributeHandleSet.insert(rti1516::AttributeHandleFriend::createHandle(*i));

      _federateAmbassador->turnUpdatesOffForObjectInstance(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516::Exception& e) {
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
      rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle = rti1516::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);

      rti1516::AttributeHandleSet rti1516AttributeHandleSet;
      for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
        rti1516AttributeHandleSet.insert(rti1516::AttributeHandleFriend::createHandle(*i));

      rti1516::VariableLengthData rti1516Tag = rti1516::VariableLengthDataFriend::create(tag);

      _federateAmbassador->requestAttributeOwnershipAssumption(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet, rti1516Tag);
    } catch (const rti1516::Exception& e) {
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
      rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle = rti1516::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);

      rti1516::AttributeHandleSet rti1516AttributeHandleSet;
      for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
        rti1516AttributeHandleSet.insert(rti1516::AttributeHandleFriend::createHandle(*i));

      _federateAmbassador->requestDivestitureConfirmation(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516::Exception& e) {
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
      rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle = rti1516::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);

      rti1516::AttributeHandleSet rti1516AttributeHandleSet;
      for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
        rti1516AttributeHandleSet.insert(rti1516::AttributeHandleFriend::createHandle(*i));

      rti1516::VariableLengthData rti1516Tag = rti1516::VariableLengthDataFriend::create(tag);

      _federateAmbassador->attributeOwnershipAcquisitionNotification(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet, rti1516Tag);
    } catch (const rti1516::Exception& e) {
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
      rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle = rti1516::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);

      rti1516::AttributeHandleSet rti1516AttributeHandleSet;
      for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
        rti1516AttributeHandleSet.insert(rti1516::AttributeHandleFriend::createHandle(*i));

      _federateAmbassador->attributeOwnershipUnavailable(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516::Exception& e) {
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
      rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle = rti1516::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);

      rti1516::AttributeHandleSet rti1516AttributeHandleSet;
      for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
        rti1516AttributeHandleSet.insert(rti1516::AttributeHandleFriend::createHandle(*i));

      rti1516::VariableLengthData rti1516Tag = rti1516::VariableLengthDataFriend::create(tag);

      _federateAmbassador->requestAttributeOwnershipRelease(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet, rti1516Tag);
    } catch (const rti1516::Exception& e) {
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
      rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle = rti1516::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);

      rti1516::AttributeHandleSet rti1516AttributeHandleSet;
      for (OpenRTI::AttributeHandleVector::const_iterator i = attributeHandleVector.begin(); i != attributeHandleVector.end(); ++i)
        rti1516AttributeHandleSet.insert(rti1516::AttributeHandleFriend::createHandle(*i));

      _federateAmbassador->confirmAttributeOwnershipAcquisitionCancellation(rti1516ObjectInstanceHandle, rti1516AttributeHandleSet);
    } catch (const rti1516::Exception& e) {
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
      rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle
        = rti1516::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);
      rti1516::AttributeHandle rti1516AttributeHandle
        = rti1516::AttributeHandleFriend::createHandle(attributeHandle);
      rti1516::FederateHandle rti1516FederateHandle
        = rti1516::FederateHandleFriend::createHandle(federateHandle);

      _federateAmbassador->informAttributeOwnership(rti1516ObjectInstanceHandle,
                                                    rti1516AttributeHandle,
                                                    rti1516FederateHandle);
    } catch (const rti1516::Exception& e) {
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
      rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle
        = rti1516::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);
      rti1516::AttributeHandle rti1516AttributeHandle
        = rti1516::AttributeHandleFriend::createHandle(attributeHandle);

      _federateAmbassador->attributeIsNotOwned(rti1516ObjectInstanceHandle,
                                               rti1516AttributeHandle);
    } catch (const rti1516::Exception& e) {
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
      rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle
        = rti1516::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);
      rti1516::AttributeHandle rti1516AttributeHandle
        = rti1516::AttributeHandleFriend::createHandle(attributeHandle);

      _federateAmbassador->attributeIsOwnedByRTI(rti1516ObjectInstanceHandle,
                                                 rti1516AttributeHandle);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  //////////////////////////////
  // Time Management Services //
  //////////////////////////////

  // 8.3
  virtual void
  timeRegulationEnabled(const rti1516::LogicalTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->timeRegulationEnabled(logicalTime);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 8.6
  virtual void
  timeConstrainedEnabled(const rti1516::LogicalTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->timeConstrainedEnabled(logicalTime);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  // 8.13
  virtual void
  timeAdvanceGrant(const rti1516::LogicalTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->timeAdvanceGrant(logicalTime);
    } catch (const rti1516::Exception& e) {
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
      rti1516::MessageRetractionHandle rti1516MessageRetractionHandle;
      rti1516MessageRetractionHandle = rti1516::MessageRetractionHandleFriend::createHandle(messageRetractionHandle);
      _federateAmbassador->requestRetraction(rti1516MessageRetractionHandle);
    } catch (const rti1516::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an rti1516 exception in callback: " << e.what() << std::endl;
    }
  }

  virtual TimeManagement<RTI1516Traits>* createTimeManagement(Federate& federate)
  {
    std::string logicalTimeFactoryName = federate.getLogicalTimeFactoryName();
    std::auto_ptr<rti1516::LogicalTimeFactory> logicalTimeFactory;
    logicalTimeFactory = rti1516::LogicalTimeFactoryFactory::makeLogicalTimeFactory(utf8ToUcs(logicalTimeFactoryName));
    if (!logicalTimeFactory.get())
      return 0;

    // Get logical time and logical time interval. If they are the well known ones,
    // try to use the optimized implementation using the native time data types directly.
    // An implementation is considered equal if the implementation name is the same and they are assignable in each direction,
    // Also add a flag that forces the to use the opaque factory

    if (!_forceOpaqueLogicalTime) {
      std::auto_ptr<rti1516::LogicalTime> logicalTime = logicalTimeFactory->makeLogicalTime();
      std::auto_ptr<rti1516::LogicalTimeInterval> logicalTimeInterval = logicalTimeFactory->makeLogicalTimeInterval();
      try {
        HLAinteger64Time time;
        HLAinteger64Interval interval;
        if (time.implementationName() == logicalTime->implementationName() &&
            interval.implementationName() == logicalTimeInterval->implementationName()) {
          time = *logicalTime;
          interval = *logicalTimeInterval;
          *logicalTime = time;
          *logicalTimeInterval = interval;
          if (*logicalTime == time && *logicalTimeInterval == interval) {
            return new TemplateTimeManagement<RTI1516Traits, RTI1516integer64TimeFactory>(RTI1516integer64TimeFactory(ucsToUtf8(time.implementationName())));
          }
        }
      } catch (...) {
      }

      try {
        HLAfloat64Time time;
        HLAfloat64Interval interval;
        if (time.implementationName() == logicalTime->implementationName() &&
            interval.implementationName() == logicalTimeInterval->implementationName()) {
          time = *logicalTime;
          interval = *logicalTimeInterval;
          *logicalTime = time;
          *logicalTimeInterval = interval;
          if (*logicalTime == time && *logicalTimeInterval == interval) {
            return new TemplateTimeManagement<RTI1516Traits, RTI1516float64TimeFactory>(RTI1516float64TimeFactory(ucsToUtf8(time.implementationName())));
          }
        }
      } catch (...) {
      }
    }

    // Ok, we will just need to use the opaque logical time factory
    return new TemplateTimeManagement<RTI1516Traits, RTI1516LogicalTimeFactory>(RTI1516LogicalTimeFactory(logicalTimeFactory));
  }

  void ensureConnected(const std::string& federationExecutionName)
  {
    URL url;

    if (federationExecutionName.find("://") != std::string::npos)
      url = URL::fromUrl(federationExecutionName);

    if (url.getProtocol().empty())
      url.setProtocol(_defaultUrl.getProtocol());
    if (url.getHost().empty())
      url.setHost(_defaultUrl.getHost());
    if (url.getService().empty())
      url.setService(_defaultUrl.getService());
    if (url.getPath().empty())
      url.setPath(_defaultUrl.getPath());

    if (!isConnected()) {
      Ambassador<RTI1516Traits>::connect(url, _stringStringListMap);

      _connectedUrl = url;;
    } else if (_connectedUrl != url) {
      throw rti1516::RTIinternalError(L"Connect url does not point to the same connection.");
    }
  }

  // The instantiation time argument list
  rti1516::FederateAmbassador* _federateAmbassador;
  URL _defaultUrl;
  StringStringListMap _stringStringListMap;
  URL _connectedUrl;
  bool _forceOpaqueLogicalTime;
};

RTIambassadorImplementation::RTIambassadorImplementation(const std::vector<std::wstring>& args) throw () :
  _ambassadorInterface(new RTI1516AmbassadorInterface(args))
{
}

RTIambassadorImplementation::~RTIambassadorImplementation()
{
  delete _ambassadorInterface;
  _ambassadorInterface = 0;
}

void
RTIambassadorImplementation::createFederationExecution(std::wstring const & federationExecutionName,
                                                       std::wstring const & fullPathNameToTheFDDfile,
                                                       std::wstring const & logicalTimeImplementationName)
  throw (rti1516::FederationExecutionAlreadyExists,
         rti1516::CouldNotOpenFDD,
         rti1516::ErrorReadingFDD,
         rti1516::CouldNotCreateLogicalTimeFactory,
         rti1516::RTIinternalError)
{
  // Make sure we can read the fdd file
  std::ifstream stream(OpenRTI::ucsToLocale(fullPathNameToTheFDDfile).c_str());
  if (!stream.is_open())
    throw rti1516::CouldNotOpenFDD(fullPathNameToTheFDDfile);

  OpenRTI::FOMStringModuleList fomModuleList;
  try {
    fomModuleList.push_back(OpenRTI::FDD1516FileReader::read(stream));
  } catch (const OpenRTI::Exception& e) {
    throw rti1516::ErrorReadingFDD(OpenRTI::utf8ToUcs(e.getReason()));
  } catch (...) {
    throw rti1516::RTIinternalError(L"Unknown error while reading fdd file");
  }

  std::string utf8FederationExecutionName = OpenRTI::ucsToUtf8(federationExecutionName);
  _ambassadorInterface->ensureConnected(utf8FederationExecutionName);
  _ambassadorInterface->createFederationExecution(getFilePart(utf8FederationExecutionName), fomModuleList, ucsToUtf8(logicalTimeImplementationName));
}

// 4.3
void
RTIambassadorImplementation::destroyFederationExecution(std::wstring const & federationExecutionName)
  throw (rti1516::FederatesCurrentlyJoined,
         rti1516::FederationExecutionDoesNotExist,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->ensureConnected(ucsToUtf8(federationExecutionName));

  _ambassadorInterface->destroyFederationExecution(getFilePart(ucsToUtf8(federationExecutionName)));
}

// 4.4
rti1516::FederateHandle
RTIambassadorImplementation::joinFederationExecution(std::wstring const & federateType,
                                                     std::wstring const & federationExecutionName,
                                                     rti1516::FederateAmbassador & federateAmbassador)
  throw (rti1516::FederateAlreadyExecutionMember,
         rti1516::FederationExecutionDoesNotExist,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::CouldNotCreateLogicalTimeFactory,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->ensureConnected(ucsToUtf8(federationExecutionName));
  _ambassadorInterface->_federateAmbassador = &federateAmbassador;

  FederateHandle federateHandle = _ambassadorInterface->joinFederationExecution(std::string(), ucsToUtf8(federateType),
                                                                                getFilePart(ucsToUtf8(federationExecutionName)),
                                                                                OpenRTI::FOMStringModuleList(), NULL /*FIXME*/);
  return rti1516::FederateHandleFriend::createHandle(federateHandle);
}

// 4.5
void
RTIambassadorImplementation::resignFederationExecution(rti1516::ResignAction rti1516ResignAction)
  throw (rti1516::OwnershipAcquisitionPending,
         rti1516::FederateOwnsAttributes,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  OpenRTI::ResignAction resignAction;
  switch (rti1516ResignAction) {
  case rti1516::UNCONDITIONALLY_DIVEST_ATTRIBUTES:
    resignAction = OpenRTI::UNCONDITIONALLY_DIVEST_ATTRIBUTES;
    break;
  case rti1516::DELETE_OBJECTS:
    resignAction = OpenRTI::DELETE_OBJECTS;
    break;
  case rti1516::CANCEL_PENDING_OWNERSHIP_ACQUISITIONS:
    resignAction = OpenRTI::CANCEL_PENDING_OWNERSHIP_ACQUISITIONS;
    break;
  case rti1516::DELETE_OBJECTS_THEN_DIVEST:
    resignAction = OpenRTI::DELETE_OBJECTS_THEN_DIVEST;
    break;
  case rti1516::CANCEL_THEN_DELETE_THEN_DIVEST:
    resignAction = OpenRTI::CANCEL_THEN_DELETE_THEN_DIVEST;
    break;
  case rti1516::NO_ACTION:
    resignAction = OpenRTI::NO_ACTION;
    break;
  default:
    resignAction = OpenRTI::CANCEL_THEN_DELETE_THEN_DIVEST;
  }

  _ambassadorInterface->_federateAmbassador = 0;
  _ambassadorInterface->resignFederationExecution(resignAction);
}

// 4.6
void
RTIambassadorImplementation::registerFederationSynchronizationPoint(std::wstring const & label,
                                                                    rti1516::VariableLengthData const & rti1516Tag)
  throw (rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::VariableLengthData tag = rti1516::VariableLengthDataFriend::readPointer(rti1516Tag);
  // According to the standard, an empty set also means all federates currently joined.
  _ambassadorInterface->registerFederationSynchronizationPoint(ucsToUtf8(label), tag, OpenRTI::FederateHandleSet());
}

void
RTIambassadorImplementation::registerFederationSynchronizationPoint(std::wstring const & label,
                                                                    rti1516::VariableLengthData const & rti1516Tag,
                                                                    rti1516::FederateHandleSet const & rti1516FederateHandleSet)
  throw (rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::VariableLengthData tag = rti1516::VariableLengthDataFriend::readPointer(rti1516Tag);

  OpenRTI::FederateHandleSet federateHandleSet;
  for (rti1516::FederateHandleSet::const_iterator i = rti1516FederateHandleSet.begin(); i != rti1516FederateHandleSet.end(); ++i)
    federateHandleSet.insert(rti1516::FederateHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->registerFederationSynchronizationPoint(ucsToUtf8(label), tag, federateHandleSet);
}

// 4.9
void
RTIambassadorImplementation::synchronizationPointAchieved(std::wstring const & label)
  throw (rti1516::SynchronizationPointLabelNotAnnounced,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->synchronizationPointAchieved(ucsToUtf8(label));
}

// 4.11
void
RTIambassadorImplementation::requestFederationSave(std::wstring const & label)
  throw (rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->requestFederationSave(ucsToUtf8(label));
}

void
RTIambassadorImplementation::requestFederationSave(const std::wstring& label,
                                                   const rti1516::LogicalTime& rti1516LogicalTime)
  throw (rti1516::LogicalTimeAlreadyPassed,
         rti1516::InvalidLogicalTime,
         rti1516::FederateUnableToUseTime,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->requestFederationSave(ucsToUtf8(label), rti1516LogicalTime);
}

// 4.13
void
RTIambassadorImplementation::federateSaveBegun()
  throw (rti1516::SaveNotInitiated,
         rti1516::FederateNotExecutionMember,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->federateSaveBegun();
}

// 4.14
void
RTIambassadorImplementation::federateSaveComplete()
  throw (rti1516::FederateHasNotBegunSave,
         rti1516::FederateNotExecutionMember,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->federateSaveComplete();
}

void
RTIambassadorImplementation::federateSaveNotComplete()
  throw (rti1516::FederateHasNotBegunSave,
         rti1516::FederateNotExecutionMember,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->federateSaveNotComplete();
}

// 4.16
void
RTIambassadorImplementation::queryFederationSaveStatus ()
  throw (rti1516::FederateNotExecutionMember,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->queryFederationSaveStatus();
}

// 4.18
void
RTIambassadorImplementation::requestFederationRestore(std::wstring const & label)
  throw (rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->requestFederationRestore(ucsToUtf8(label));
}

// 4.22
void
RTIambassadorImplementation::federateRestoreComplete()
  throw (rti1516::RestoreNotRequested,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->federateRestoreComplete();
}

void
RTIambassadorImplementation::federateRestoreNotComplete()
  throw (rti1516::RestoreNotRequested,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->federateRestoreNotComplete();
}

// 4.24
void
RTIambassadorImplementation::queryFederationRestoreStatus()
  throw (rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->queryFederationRestoreStatus();
}

/////////////////////////////////////
// Declaration Management Services //
/////////////////////////////////////

// 5.2
void
RTIambassadorImplementation::publishObjectClassAttributes(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                                          rti1516::AttributeHandleSet const & rti1516AttributeHandleSet)
  throw (rti1516::ObjectClassNotDefined,
         rti1516::AttributeNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516::AttributeHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->publishObjectClassAttributes(objectClassHandle, attributeHandleSet);
}

// 5.3
void
RTIambassadorImplementation::unpublishObjectClass(rti1516::ObjectClassHandle rti1516ObjectClassHandle)
  throw (rti1516::ObjectClassNotDefined,
         rti1516::OwnershipAcquisitionPending,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);
  _ambassadorInterface->unpublishObjectClass(objectClassHandle);
}

void
RTIambassadorImplementation::unpublishObjectClassAttributes(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                                            rti1516::AttributeHandleSet const & rti1516AttributeHandleSet)
  throw (rti1516::ObjectClassNotDefined,
         rti1516::AttributeNotDefined,
         rti1516::OwnershipAcquisitionPending,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516::AttributeHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->unpublishObjectClassAttributes(objectClassHandle, attributeHandleSet);
}

// 5.4
void
RTIambassadorImplementation::publishInteractionClass(rti1516::InteractionClassHandle rti1516InteractionClassHandle)
  throw (rti1516::InteractionClassNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);
  _ambassadorInterface->publishInteractionClass(interactionClassHandle);
}

// 5.5
void
RTIambassadorImplementation::unpublishInteractionClass(rti1516::InteractionClassHandle rti1516InteractionClassHandle)
  throw (rti1516::InteractionClassNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);
  _ambassadorInterface->unpublishInteractionClass(interactionClassHandle);
}

// 5.6
void
RTIambassadorImplementation::subscribeObjectClassAttributes(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                                            rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                            bool active)
  throw (rti1516::ObjectClassNotDefined,
         rti1516::AttributeNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516::AttributeHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->subscribeObjectClassAttributes(objectClassHandle, attributeHandleSet, active);
}

// 5.7
void
RTIambassadorImplementation::unsubscribeObjectClass(rti1516::ObjectClassHandle rti1516ObjectClassHandle)
  throw (rti1516::ObjectClassNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);
  _ambassadorInterface->unsubscribeObjectClass(objectClassHandle);
}

void
RTIambassadorImplementation::unsubscribeObjectClassAttributes(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                                              rti1516::AttributeHandleSet const & rti1516AttributeHandleSet)
  throw (rti1516::ObjectClassNotDefined,
         rti1516::AttributeNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516::AttributeHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->unsubscribeObjectClassAttributes(objectClassHandle, attributeHandleSet);
}

// 5.8
void
RTIambassadorImplementation::subscribeInteractionClass(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                                       bool active)
  throw (rti1516::InteractionClassNotDefined,
         rti1516::FederateServiceInvocationsAreBeingReportedViaMOM,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);
  _ambassadorInterface->subscribeInteractionClass(interactionClassHandle, active);
}

// 5.9
void
RTIambassadorImplementation::unsubscribeInteractionClass(rti1516::InteractionClassHandle rti1516InteractionClassHandle)
  throw (rti1516::InteractionClassNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);
  _ambassadorInterface->unsubscribeInteractionClass(interactionClassHandle);
}

////////////////////////////////
// Object Management Services //
////////////////////////////////

// 6.2
void
RTIambassadorImplementation::reserveObjectInstanceName(std::wstring const & objectInstanceName)
  throw (rti1516::IllegalName,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->reserveObjectInstanceName(ucsToUtf8(objectInstanceName));
}

// 6.4
rti1516::ObjectInstanceHandle
RTIambassadorImplementation::registerObjectInstance(rti1516::ObjectClassHandle rti1516ObjectClassHandle)
  throw (rti1516::ObjectClassNotDefined,
         rti1516::ObjectClassNotPublished,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = _ambassadorInterface->registerObjectInstance(objectClassHandle);
  return rti1516::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);
}

rti1516::ObjectInstanceHandle
RTIambassadorImplementation::registerObjectInstance(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                                    std::wstring const & objectInstanceName)
  throw (rti1516::ObjectClassNotDefined,
         rti1516::ObjectClassNotPublished,
         rti1516::ObjectInstanceNameNotReserved,
         rti1516::ObjectInstanceNameInUse,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = _ambassadorInterface->registerObjectInstance(objectClassHandle, ucsToUtf8(objectInstanceName), false);
  return rti1516::ObjectInstanceHandleFriend::createHandle(objectInstanceHandle);
}

// 6.6
void
RTIambassadorImplementation::updateAttributeValues(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                   const rti1516::AttributeHandleValueMap& rti1516AttributeValues,
                                                   const rti1516::VariableLengthData& rti1516Tag)
  throw (rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotOwned,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::VariableLengthData tag = rti1516::VariableLengthDataFriend::readPointer(rti1516Tag);

  std::vector<OpenRTI::AttributeValue> attributeValueVector;
  attributeValueVector.reserve(rti1516AttributeValues.size());
  for (rti1516::AttributeHandleValueMap::const_iterator i = rti1516AttributeValues.begin(); i != rti1516AttributeValues.end(); ++i) {
    attributeValueVector.push_back(OpenRTI::AttributeValue());
    OpenRTI::AttributeHandle attributeHandle = rti1516::AttributeHandleFriend::getOpenRTIHandle(i->first);
    attributeValueVector.back().setAttributeHandle(attributeHandle);
    attributeValueVector.back().getValue() = rti1516::VariableLengthDataFriend::readPointer(i->second);
  }

  _ambassadorInterface->updateAttributeValues(objectInstanceHandle, attributeValueVector, tag);
}

rti1516::MessageRetractionHandle
RTIambassadorImplementation::updateAttributeValues(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                   const rti1516::AttributeHandleValueMap& rti1516AttributeValues,
                                                   const rti1516::VariableLengthData& rti1516Tag,
                                                   const rti1516::LogicalTime& rti1516LogicalTime)
  throw (rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotOwned,
         rti1516::InvalidLogicalTime,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);
  OpenRTI::VariableLengthData tag = rti1516::VariableLengthDataFriend::readPointer(rti1516Tag);

  std::vector<OpenRTI::AttributeValue> attributeValueVector;
  attributeValueVector.reserve(rti1516AttributeValues.size());
  for (rti1516::AttributeHandleValueMap::const_iterator i = rti1516AttributeValues.begin(); i != rti1516AttributeValues.end(); ++i) {
    attributeValueVector.push_back(OpenRTI::AttributeValue());
    OpenRTI::AttributeHandle attributeHandle = rti1516::AttributeHandleFriend::getOpenRTIHandle(i->first);
    attributeValueVector.back().setAttributeHandle(attributeHandle);
    attributeValueVector.back().getValue() = rti1516::VariableLengthDataFriend::readPointer(i->second);
  }

  OpenRTI::MessageRetractionHandle messageRetractionHandle;
  messageRetractionHandle = _ambassadorInterface->updateAttributeValues(objectInstanceHandle, attributeValueVector, tag, rti1516LogicalTime);
  return rti1516::MessageRetractionHandleFriend::createHandle(messageRetractionHandle);
}

// 6.8
void
RTIambassadorImplementation::sendInteraction(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                             const rti1516::ParameterHandleValueMap& rti1516ParameterValues,
                                             const rti1516::VariableLengthData& rti1516Tag)
  throw (rti1516::InteractionClassNotPublished,
         rti1516::InteractionClassNotDefined,
         rti1516::InteractionParameterNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle;
  interactionClassHandle = rti1516::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);

  std::vector<OpenRTI::ParameterValue> parameterValueVector;
  parameterValueVector.reserve(rti1516ParameterValues.size());
  for (rti1516::ParameterHandleValueMap::const_iterator i = rti1516ParameterValues.begin(); i != rti1516ParameterValues.end(); ++i) {
    parameterValueVector.push_back(OpenRTI::ParameterValue());
    OpenRTI::ParameterHandle parameterHandle = rti1516::ParameterHandleFriend::getOpenRTIHandle(i->first);
    parameterValueVector.back().setParameterHandle(parameterHandle);
    parameterValueVector.back().getValue() = rti1516::VariableLengthDataFriend::readPointer(i->second);
  }

  OpenRTI::VariableLengthData tag = rti1516::VariableLengthDataFriend::readPointer(rti1516Tag);
  _ambassadorInterface->sendInteraction(interactionClassHandle, parameterValueVector, tag);
}

rti1516::MessageRetractionHandle
RTIambassadorImplementation::sendInteraction(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                             const rti1516::ParameterHandleValueMap& rti1516ParameterValues,
                                             const rti1516::VariableLengthData& rti1516Tag,
                                             const rti1516::LogicalTime& rti1516LogicalTime)
  throw (rti1516::InteractionClassNotPublished,
         rti1516::InteractionClassNotDefined,
         rti1516::InteractionParameterNotDefined,
         rti1516::InvalidLogicalTime,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle;
  interactionClassHandle = rti1516::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);

  std::vector<OpenRTI::ParameterValue> parameterValueVector;
  parameterValueVector.reserve(rti1516ParameterValues.size());
  for (rti1516::ParameterHandleValueMap::const_iterator i = rti1516ParameterValues.begin(); i != rti1516ParameterValues.end(); ++i) {
    parameterValueVector.push_back(OpenRTI::ParameterValue());
    OpenRTI::ParameterHandle parameterHandle = rti1516::ParameterHandleFriend::getOpenRTIHandle(i->first);
    parameterValueVector.back().setParameterHandle(parameterHandle);
    parameterValueVector.back().getValue() = rti1516::VariableLengthDataFriend::readPointer(i->second);
  }

  OpenRTI::VariableLengthData tag = rti1516::VariableLengthDataFriend::readPointer(rti1516Tag);
  OpenRTI::MessageRetractionHandle messageRetractionHandle;
  messageRetractionHandle = _ambassadorInterface->sendInteraction(interactionClassHandle, parameterValueVector, tag, rti1516LogicalTime);
  return rti1516::MessageRetractionHandleFriend::createHandle(messageRetractionHandle);
}

// 6.10
void
RTIambassadorImplementation::deleteObjectInstance(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                  const rti1516::VariableLengthData& rti1516Tag)
  throw (rti1516::DeletePrivilegeNotHeld,
         rti1516::ObjectInstanceNotKnown,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);
  OpenRTI::VariableLengthData tag = rti1516::VariableLengthDataFriend::readPointer(rti1516Tag);
  _ambassadorInterface->deleteObjectInstance(objectInstanceHandle, tag);
}

rti1516::MessageRetractionHandle
RTIambassadorImplementation::deleteObjectInstance(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                  const rti1516::VariableLengthData& rti1516Tag,
                                                  const rti1516::LogicalTime& rti1516LogicalTime)
  throw (rti1516::DeletePrivilegeNotHeld,
         rti1516::ObjectInstanceNotKnown,
         rti1516::InvalidLogicalTime,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);
  OpenRTI::VariableLengthData tag = rti1516::VariableLengthDataFriend::readPointer(rti1516Tag);
  OpenRTI::MessageRetractionHandle messageRetractionHandle = _ambassadorInterface->deleteObjectInstance(objectInstanceHandle, tag, rti1516LogicalTime);
  return rti1516::MessageRetractionHandleFriend::createHandle(messageRetractionHandle);
}

// 6.12
void
RTIambassadorImplementation::localDeleteObjectInstance(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle)
  throw (rti1516::ObjectInstanceNotKnown,
         rti1516::FederateOwnsAttributes,
         rti1516::OwnershipAcquisitionPending,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);
  _ambassadorInterface->localDeleteObjectInstance(objectInstanceHandle);
}

// 6.13
void
RTIambassadorImplementation::changeAttributeTransportationType(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                               rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                               rti1516::TransportationType rti1516TransportationType)
  throw (rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotOwned,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516::AttributeHandleFriend::getOpenRTIHandle(*i));

  OpenRTI::TransportationType transportationType = translate(rti1516TransportationType);

  _ambassadorInterface->changeAttributeTransportationType(objectInstanceHandle, attributeHandleSet, transportationType);
}

// 6.14
void
RTIambassadorImplementation::changeInteractionTransportationType(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                                                 rti1516::TransportationType rti1516TransportationType)
  throw (rti1516::InteractionClassNotDefined,
         rti1516::InteractionClassNotPublished,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);
  OpenRTI::TransportationType transportationType = translate(rti1516TransportationType);
  _ambassadorInterface->changeInteractionTransportationType(interactionClassHandle, transportationType);
}

// 6.17
void
RTIambassadorImplementation::requestAttributeValueUpdate(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                         rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                         rti1516::VariableLengthData const & rti1516Tag)
  throw (rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516::AttributeHandleFriend::getOpenRTIHandle(*i));

  OpenRTI::VariableLengthData tag = rti1516::VariableLengthDataFriend::readPointer(rti1516Tag);

  _ambassadorInterface->requestAttributeValueUpdate(objectInstanceHandle, attributeHandleSet, tag);
}

void
RTIambassadorImplementation::requestAttributeValueUpdate(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                                         rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                         rti1516::VariableLengthData const & rti1516Tag)
  throw (rti1516::ObjectClassNotDefined,
         rti1516::AttributeNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516::AttributeHandleFriend::getOpenRTIHandle(*i));

  OpenRTI::VariableLengthData tag = rti1516::VariableLengthDataFriend::readPointer(rti1516Tag);

  _ambassadorInterface->requestAttributeValueUpdate(objectClassHandle, attributeHandleSet, tag);
}

///////////////////////////////////
// Ownership Management Services //
///////////////////////////////////
// 7.2
void
RTIambassadorImplementation::unconditionalAttributeOwnershipDivestiture(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                        rti1516::AttributeHandleSet const & rti1516AttributeHandleSet)
  throw (rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotOwned,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516::AttributeHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->unconditionalAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleSet);
}

// 7.3
void
RTIambassadorImplementation::negotiatedAttributeOwnershipDivestiture(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                     rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                                     rti1516::VariableLengthData const & rti1516Tag)
  throw (rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotOwned,
         rti1516::AttributeAlreadyBeingDivested,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516::AttributeHandleFriend::getOpenRTIHandle(*i));

  OpenRTI::VariableLengthData tag = rti1516::VariableLengthDataFriend::readPointer(rti1516Tag);

  _ambassadorInterface->negotiatedAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleSet, tag);
}

// 7.6
void
RTIambassadorImplementation::confirmDivestiture(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                rti1516::AttributeHandleSet const& rti1516AttributeHandleSet,
                                                rti1516::VariableLengthData const& rti1516Tag)
  throw (rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotOwned,
         rti1516::AttributeDivestitureWasNotRequested,
         rti1516::NoAcquisitionPending,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516::AttributeHandleFriend::getOpenRTIHandle(*i));

  OpenRTI::VariableLengthData tag = rti1516::VariableLengthDataFriend::readPointer(rti1516Tag);

  _ambassadorInterface->confirmDivestiture(objectInstanceHandle, attributeHandleSet, tag);
}

// 7.8
void
RTIambassadorImplementation::attributeOwnershipAcquisition(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                           rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                           rti1516::VariableLengthData const & rti1516Tag)
  throw (rti1516::ObjectInstanceNotKnown,
         rti1516::ObjectClassNotPublished,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotPublished,
         rti1516::FederateOwnsAttributes,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516::AttributeHandleFriend::getOpenRTIHandle(*i));

  OpenRTI::VariableLengthData tag = rti1516::VariableLengthDataFriend::readPointer(rti1516Tag);

  _ambassadorInterface->attributeOwnershipAcquisition(objectInstanceHandle, attributeHandleSet, tag);
}

// 7.9
void
RTIambassadorImplementation::attributeOwnershipAcquisitionIfAvailable(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                      rti1516::AttributeHandleSet const & rti1516AttributeHandleSet)
  throw (rti1516::ObjectInstanceNotKnown,
         rti1516::ObjectClassNotPublished,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotPublished,
         rti1516::FederateOwnsAttributes,
         rti1516::AttributeAlreadyBeingAcquired,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516::AttributeHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->attributeOwnershipAcquisitionIfAvailable(objectInstanceHandle, attributeHandleSet);
}

// 7.12
void
RTIambassadorImplementation::attributeOwnershipDivestitureIfWanted(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                   rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                                   rti1516::AttributeHandleSet & rti1516DivestedAttributeSet)
  throw (rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotOwned,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516::AttributeHandleFriend::getOpenRTIHandle(*i));

  OpenRTI::AttributeHandleSet divestedAttributeHandleSet;
  _ambassadorInterface->attributeOwnershipDivestitureIfWanted(objectInstanceHandle, attributeHandleSet, divestedAttributeHandleSet);

  rti1516DivestedAttributeSet.clear();
  for (OpenRTI::AttributeHandleSet::const_iterator i = divestedAttributeHandleSet.begin(); i != divestedAttributeHandleSet.end(); ++i)
    rti1516DivestedAttributeSet.insert(rti1516::AttributeHandleFriend::createHandle(*i));
}

// 7.13
void
RTIambassadorImplementation::cancelNegotiatedAttributeOwnershipDivestiture(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                           rti1516::AttributeHandleSet const & rti1516AttributeHandleSet)
  throw (rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotOwned,
         rti1516::AttributeDivestitureWasNotRequested,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516::AttributeHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->cancelNegotiatedAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleSet);
}

// 7.14
void
RTIambassadorImplementation::cancelAttributeOwnershipAcquisition(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                                 rti1516::AttributeHandleSet const & rti1516AttributeHandleSet)
  throw (rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::AttributeAlreadyOwned,
         rti1516::AttributeAcquisitionWasNotRequested,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516::AttributeHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->cancelAttributeOwnershipAcquisition(objectInstanceHandle, attributeHandleSet);
}

// 7.16
void
RTIambassadorImplementation::queryAttributeOwnership(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                     rti1516::AttributeHandle rti1516AttributeHandle)
  throw (rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);
  OpenRTI::AttributeHandle attributeHandle = rti1516::AttributeHandleFriend::getOpenRTIHandle(rti1516AttributeHandle);
  _ambassadorInterface->queryAttributeOwnership(objectInstanceHandle, attributeHandle);
}

// 7.18
bool
RTIambassadorImplementation::isAttributeOwnedByFederate(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                        rti1516::AttributeHandle rti1516AttributeHandle)
  throw (rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);
  OpenRTI::AttributeHandle attributeHandle = rti1516::AttributeHandleFriend::getOpenRTIHandle(rti1516AttributeHandle);
  return _ambassadorInterface->isAttributeOwnedByFederate(objectInstanceHandle, attributeHandle);
}

//////////////////////////////
// Time Management Services //
//////////////////////////////

// 8.2
void
RTIambassadorImplementation::enableTimeRegulation(const rti1516::LogicalTimeInterval& rti1516Lookahead)
  throw (rti1516::TimeRegulationAlreadyEnabled,
         rti1516::InvalidLookahead,
         rti1516::InTimeAdvancingState,
         rti1516::RequestForTimeRegulationPending,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->enableTimeRegulation(rti1516Lookahead);
}

// 8.4
void
RTIambassadorImplementation::disableTimeRegulation()
  throw (rti1516::TimeRegulationIsNotEnabled,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->disableTimeRegulation();
}

// 8.5
void
RTIambassadorImplementation::enableTimeConstrained()
  throw (rti1516::TimeConstrainedAlreadyEnabled,
         rti1516::InTimeAdvancingState,
         rti1516::RequestForTimeConstrainedPending,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->enableTimeConstrained();
}

// 8.7
void
RTIambassadorImplementation::disableTimeConstrained()
  throw (rti1516::TimeConstrainedIsNotEnabled,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->disableTimeConstrained();
}

// 8.8
void
RTIambassadorImplementation::timeAdvanceRequest(const rti1516::LogicalTime& logicalTime)
  throw (rti1516::InvalidLogicalTime,
         rti1516::LogicalTimeAlreadyPassed,
         rti1516::InTimeAdvancingState,
         rti1516::RequestForTimeRegulationPending,
         rti1516::RequestForTimeConstrainedPending,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->timeAdvanceRequest(logicalTime);
}

// 8.9
void
RTIambassadorImplementation::timeAdvanceRequestAvailable(const rti1516::LogicalTime& logicalTime)
  throw (rti1516::InvalidLogicalTime,
         rti1516::LogicalTimeAlreadyPassed,
         rti1516::InTimeAdvancingState,
         rti1516::RequestForTimeRegulationPending,
         rti1516::RequestForTimeConstrainedPending,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->timeAdvanceRequestAvailable(logicalTime);
}

// 8.10
void
RTIambassadorImplementation::nextMessageRequest(const rti1516::LogicalTime& logicalTime)
  throw (rti1516::InvalidLogicalTime,
         rti1516::LogicalTimeAlreadyPassed,
         rti1516::InTimeAdvancingState,
         rti1516::RequestForTimeRegulationPending,
         rti1516::RequestForTimeConstrainedPending,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->nextMessageRequest(logicalTime);
}

// 8.11
void
RTIambassadorImplementation::nextMessageRequestAvailable(const rti1516::LogicalTime& logicalTime)
  throw (rti1516::InvalidLogicalTime,
         rti1516::LogicalTimeAlreadyPassed,
         rti1516::InTimeAdvancingState,
         rti1516::RequestForTimeRegulationPending,
         rti1516::RequestForTimeConstrainedPending,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->nextMessageRequestAvailable(logicalTime);
}

// 8.12
void
RTIambassadorImplementation::flushQueueRequest(const rti1516::LogicalTime& logicalTime)
  throw (rti1516::InvalidLogicalTime,
         rti1516::LogicalTimeAlreadyPassed,
         rti1516::InTimeAdvancingState,
         rti1516::RequestForTimeRegulationPending,
         rti1516::RequestForTimeConstrainedPending,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->flushQueueRequest(logicalTime);
}

// 8.14
void
RTIambassadorImplementation::enableAsynchronousDelivery()
  throw (rti1516::AsynchronousDeliveryAlreadyEnabled,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->enableAsynchronousDelivery();
}

// 8.15
void
RTIambassadorImplementation::disableAsynchronousDelivery()
  throw (rti1516::AsynchronousDeliveryAlreadyDisabled,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->disableAsynchronousDelivery();
}

// 8.16
bool
RTIambassadorImplementation::queryGALT(rti1516::LogicalTime& logicalTime)
  throw (rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  return _ambassadorInterface->queryGALT(logicalTime);
}

// 8.17
void
RTIambassadorImplementation::queryLogicalTime(rti1516::LogicalTime& logicalTime)
  throw (rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->queryLogicalTime(logicalTime);
}

// 8.18
bool
RTIambassadorImplementation::queryLITS(rti1516::LogicalTime& logicalTime)
  throw (rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  return _ambassadorInterface->queryLITS(logicalTime);
}

// 8.19
void
RTIambassadorImplementation::modifyLookahead(const rti1516::LogicalTimeInterval& lookahead)
  throw (rti1516::TimeRegulationIsNotEnabled,
         rti1516::InvalidLookahead,
         rti1516::InTimeAdvancingState,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->modifyLookahead(lookahead);
}

// 8.20
void
RTIambassadorImplementation::queryLookahead(rti1516::LogicalTimeInterval& lookahead)
  throw (rti1516::TimeRegulationIsNotEnabled,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->queryLookahead(lookahead);
}

// 8.21
void
RTIambassadorImplementation::retract(rti1516::MessageRetractionHandle rti1516MessageRetractionHandle)
  throw (rti1516::InvalidRetractionHandle,
         rti1516::TimeRegulationIsNotEnabled,
         rti1516::MessageCanNoLongerBeRetracted,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::MessageRetractionHandle messageRetractionHandle = rti1516::MessageRetractionHandleFriend::getOpenRTIHandle(rti1516MessageRetractionHandle);
  _ambassadorInterface->retract(messageRetractionHandle);
}

// 8.23
void
RTIambassadorImplementation::changeAttributeOrderType(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                      rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                                      rti1516::OrderType rti1516OrderType)
  throw (rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotOwned,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);

  OpenRTI::AttributeHandleSet attributeHandleSet;
  for (rti1516::AttributeHandleSet::const_iterator i = rti1516AttributeHandleSet.begin(); i != rti1516AttributeHandleSet.end(); ++i)
    attributeHandleSet.insert(rti1516::AttributeHandleFriend::getOpenRTIHandle(*i));

  OpenRTI::OrderType orderType = translate(rti1516OrderType);

  _ambassadorInterface->changeAttributeOrderType(objectInstanceHandle, attributeHandleSet, orderType);
}

// 8.24
void
RTIambassadorImplementation::changeInteractionOrderType(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                                        rti1516::OrderType rti1516OrderType)
  throw (rti1516::InteractionClassNotDefined,
         rti1516::InteractionClassNotPublished,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);

  OpenRTI::OrderType orderType = translate(rti1516OrderType);

  _ambassadorInterface->changeInteractionOrderType(interactionClassHandle, orderType);
}

//////////////////////////////////
// Data Distribution Management //
//////////////////////////////////

// 9.2
rti1516::RegionHandle
RTIambassadorImplementation::createRegion(rti1516::DimensionHandleSet const & rti1516DimensionHandleSet)
  throw (rti1516::InvalidDimensionHandle,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::DimensionHandleSet dimensionHandleSet;
  for (rti1516::DimensionHandleSet::const_iterator i = rti1516DimensionHandleSet.begin(); i != rti1516DimensionHandleSet.end(); ++i)
    dimensionHandleSet.insert(rti1516::DimensionHandleFriend::getOpenRTIHandle(*i));

  return rti1516::RegionHandleFriend::createHandle(_ambassadorInterface->createRegion(dimensionHandleSet));
}

// 9.3
void
RTIambassadorImplementation::commitRegionModifications(rti1516::RegionHandleSet const & rti1516RegionHandleSet)
  throw (rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::RegionHandleSet regionHandleSet;
  for (rti1516::RegionHandleSet::const_iterator i = rti1516RegionHandleSet.begin(); i != rti1516RegionHandleSet.end(); ++i)
    regionHandleSet.insert(rti1516::RegionHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->commitRegionModifications(regionHandleSet);
}

// 9.4
void
RTIambassadorImplementation::deleteRegion(rti1516::RegionHandle rti1516RegionHandle)
  throw (rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::RegionInUseForUpdateOrSubscription,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::RegionHandle regionHandle = rti1516::RegionHandleFriend::getOpenRTIHandle(rti1516RegionHandle);
  _ambassadorInterface->deleteRegion(regionHandle);
}

// 9.5
rti1516::ObjectInstanceHandle
RTIambassadorImplementation::registerObjectInstanceWithRegions(rti1516::ObjectClassHandle theClass,
                                                               rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                                                               theAttributeHandleSetRegionHandleSetPairVector)
  throw (rti1516::ObjectClassNotDefined,
         rti1516::ObjectClassNotPublished,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotPublished,
         rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::InvalidRegionContext,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  throw rti1516::RTIinternalError(L"Not implemented");
  // FIXME
  /* _ambassadorInterface->registerObjectInstanceWithRegions(); */
}

rti1516::ObjectInstanceHandle
RTIambassadorImplementation::registerObjectInstanceWithRegions(rti1516::ObjectClassHandle theClass,
                                                               rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                                                               theAttributeHandleSetRegionHandleSetPairVector,
                                                               std::wstring const & theObjectInstanceName)
  throw (rti1516::ObjectClassNotDefined,
         rti1516::ObjectClassNotPublished,
         rti1516::AttributeNotDefined,
         rti1516::AttributeNotPublished,
         rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::InvalidRegionContext,
         rti1516::ObjectInstanceNameNotReserved,
         rti1516::ObjectInstanceNameInUse,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  // FIXME
  /* _ambassadorInterface->registerObjectInstanceWithRegions(); */
  throw rti1516::RTIinternalError(L"not implemented");
}

// 9.6
void
RTIambassadorImplementation::associateRegionsForUpdates(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                        rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                                                        theAttributeHandleSetRegionHandleSetPairVector)
  throw (rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::InvalidRegionContext,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);
  throw rti1516::RTIinternalError(L"Not implemented");
}

// 9.7
void
RTIambassadorImplementation::unassociateRegionsForUpdates(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                          rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                                                          theAttributeHandleSetRegionHandleSetPairVector)
  throw (rti1516::ObjectInstanceNotKnown,
         rti1516::AttributeNotDefined,
         rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);
  // FIXME
  /* _ambassadorInterface->registerObjectInstanceWithRegions(); */
}

// 9.8
void
RTIambassadorImplementation::subscribeObjectClassAttributesWithRegions(rti1516::ObjectClassHandle theClass,
                                                                       rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                                                                       theAttributeHandleSetRegionHandleSetPairVector,
                                                                       bool active)
  throw (rti1516::ObjectClassNotDefined,
         rti1516::AttributeNotDefined,
         rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::InvalidRegionContext,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  // FIXME
  /* _ambassadorInterface->registerObjectInstanceWithRegions(); */
}

// 9.9
void
RTIambassadorImplementation::unsubscribeObjectClassAttributesWithRegions(rti1516::ObjectClassHandle theClass,
                                                                         rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                                                                         theAttributeHandleSetRegionHandleSetPairVector)
  throw (rti1516::ObjectClassNotDefined,
         rti1516::AttributeNotDefined,
         rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  // FIXME
  /* _ambassadorInterface->registerObjectInstanceWithRegions(); */
}

// 9.10
void
RTIambassadorImplementation::subscribeInteractionClassWithRegions(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                                                  rti1516::RegionHandleSet const & rti1516RegionHandleSet,
                                                                  bool active)
  throw (rti1516::InteractionClassNotDefined,
         rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::InvalidRegionContext,
         rti1516::FederateServiceInvocationsAreBeingReportedViaMOM,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);

  OpenRTI::RegionHandleSet regionHandleSet;
  for (rti1516::RegionHandleSet::const_iterator i = rti1516RegionHandleSet.begin(); i != rti1516RegionHandleSet.end(); ++i)
    regionHandleSet.insert(rti1516::RegionHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->subscribeInteractionClassWithRegions(interactionClassHandle, regionHandleSet, active);
}

// 9.11
void
RTIambassadorImplementation::unsubscribeInteractionClassWithRegions(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                                                    rti1516::RegionHandleSet const & rti1516RegionHandleSet)
  throw (rti1516::InteractionClassNotDefined,
         rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);

  OpenRTI::RegionHandleSet regionHandleSet;
  for (rti1516::RegionHandleSet::const_iterator i = rti1516RegionHandleSet.begin(); i != rti1516RegionHandleSet.end(); ++i)
    regionHandleSet.insert(rti1516::RegionHandleFriend::getOpenRTIHandle(*i));

  _ambassadorInterface->unsubscribeInteractionClassWithRegions(interactionClassHandle, regionHandleSet);
}

// 9.12
void
RTIambassadorImplementation::sendInteractionWithRegions(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                                        rti1516::ParameterHandleValueMap const & rti1516ParameterHandleValueMap,
                                                        rti1516::RegionHandleSet const & rti1516RegionHandleSet,
                                                        rti1516::VariableLengthData const & rti1516Tag)
  throw (rti1516::InteractionClassNotDefined,
         rti1516::InteractionClassNotPublished,
         rti1516::InteractionParameterNotDefined,
         rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::InvalidRegionContext,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);

  OpenRTI::RegionHandleSet regionHandleSet;
  for (rti1516::RegionHandleSet::const_iterator i = rti1516RegionHandleSet.begin(); i != rti1516RegionHandleSet.end(); ++i)
    regionHandleSet.insert(rti1516::RegionHandleFriend::getOpenRTIHandle(*i));

  OpenRTI::VariableLengthData tag = rti1516::VariableLengthDataFriend::readPointer(rti1516Tag);

  // FIXME
  /* _ambassadorInterface->sendInteractionWithRegions(); */
}

rti1516::MessageRetractionHandle
RTIambassadorImplementation::sendInteractionWithRegions(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                                        rti1516::ParameterHandleValueMap const & rti1516ParameterHandleValueMap,
                                                        rti1516::RegionHandleSet const & rti1516RegionHandleSet,
                                                        rti1516::VariableLengthData const & rti1516Tag,
                                                        rti1516::LogicalTime const & rti1516LogicalTime)
  throw (rti1516::InteractionClassNotDefined,
         rti1516::InteractionClassNotPublished,
         rti1516::InteractionParameterNotDefined,
         rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::InvalidRegionContext,
         rti1516::InvalidLogicalTime,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);

  OpenRTI::RegionHandleSet regionHandleSet;
  for (rti1516::RegionHandleSet::const_iterator i = rti1516RegionHandleSet.begin(); i != rti1516RegionHandleSet.end(); ++i)
    regionHandleSet.insert(rti1516::RegionHandleFriend::getOpenRTIHandle(*i));

  OpenRTI::VariableLengthData tag = rti1516::VariableLengthDataFriend::readPointer(rti1516Tag);
  OpenRTI::MessageRetractionHandle messageRetractionHandle;
  // messageRetractionHandle = _ambassadorInterface->sendInteractionWithRegions(interactionClassHandle, parameterValueVector, tag, logicalTime);
  return rti1516::MessageRetractionHandleFriend::createHandle(messageRetractionHandle);
}

// 9.13
void
RTIambassadorImplementation::requestAttributeValueUpdateWithRegions(rti1516::ObjectClassHandle theClass,
                                                                    rti1516::AttributeHandleSetRegionHandleSetPairVector const & theSet,
                                                                    rti1516::VariableLengthData const & rti1516Tag)
  throw (rti1516::ObjectClassNotDefined,
         rti1516::AttributeNotDefined,
         rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::InvalidRegionContext,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::VariableLengthData tag = rti1516::VariableLengthDataFriend::readPointer(rti1516Tag);

  // FIXME
  /* _ambassadorInterface->registerObjectInstanceWithRegions(); */
}

//////////////////////////
// RTI Support Services //
//////////////////////////

// 10.2
rti1516::ObjectClassHandle
RTIambassadorImplementation::getObjectClassHandle(std::wstring const & name)
  throw (rti1516::NameNotFound,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectClassHandle handle = _ambassadorInterface->getObjectClassHandle(ucsToUtf8(name));
  return rti1516::ObjectClassHandleFriend::createHandle(handle);
}

// 10.3
std::wstring
RTIambassadorImplementation::getObjectClassName(rti1516::ObjectClassHandle rti1516ObjectClassHandle)
  throw (rti1516::InvalidObjectClassHandle,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);
  return utf8ToUcs(_ambassadorInterface->getObjectClassName(objectClassHandle));
}

// 10.4
rti1516::AttributeHandle
RTIambassadorImplementation::getAttributeHandle(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                                std::wstring const & attributeName)
  throw (rti1516::InvalidObjectClassHandle,
         rti1516::NameNotFound,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);
  OpenRTI::AttributeHandle handle = _ambassadorInterface->getAttributeHandle(objectClassHandle, ucsToUtf8(attributeName));
  return rti1516::AttributeHandleFriend::createHandle(handle);
}

// 10.5
std::wstring
RTIambassadorImplementation::getAttributeName(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                              rti1516::AttributeHandle rti1516AttributeHandle)
  throw (rti1516::InvalidObjectClassHandle,
         rti1516::InvalidAttributeHandle,
         rti1516::AttributeNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);
  OpenRTI::AttributeHandle attributeHandle = rti1516::AttributeHandleFriend::getOpenRTIHandle(rti1516AttributeHandle);
  return utf8ToUcs(_ambassadorInterface->getAttributeName(objectClassHandle, attributeHandle));
}

// 10.6
rti1516::InteractionClassHandle
RTIambassadorImplementation::getInteractionClassHandle(std::wstring const & name)
  throw (rti1516::NameNotFound,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  OpenRTI::InteractionClassHandle handle = _ambassadorInterface->getInteractionClassHandle(ucsToUtf8(name));
  return rti1516::InteractionClassHandleFriend::createHandle(handle);
}

// 10.7
std::wstring
RTIambassadorImplementation::getInteractionClassName(rti1516::InteractionClassHandle rti1516InteractionClassHandle)
  throw (rti1516::InvalidInteractionClassHandle,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);
  return utf8ToUcs(_ambassadorInterface->getInteractionClassName(interactionClassHandle));
}

// 10.8
rti1516::ParameterHandle
RTIambassadorImplementation::getParameterHandle(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                                std::wstring const & parameterName)
  throw (rti1516::InvalidInteractionClassHandle,
         rti1516::NameNotFound,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);
  OpenRTI::ParameterHandle handle = _ambassadorInterface->getParameterHandle(interactionClassHandle, ucsToUtf8(parameterName));
  return rti1516::ParameterHandleFriend::createHandle(handle);
}

// 10.9
std::wstring
RTIambassadorImplementation::getParameterName(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                              rti1516::ParameterHandle rti1516ParameterHandle)
  throw (rti1516::InvalidInteractionClassHandle,
         rti1516::InvalidParameterHandle,
         rti1516::InteractionParameterNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);
  OpenRTI::ParameterHandle parameterHandle = rti1516::ParameterHandleFriend::getOpenRTIHandle(rti1516ParameterHandle);
  return utf8ToUcs(_ambassadorInterface->getParameterName(interactionClassHandle, parameterHandle));
}

// 10.10
rti1516::ObjectInstanceHandle
RTIambassadorImplementation::getObjectInstanceHandle(std::wstring const & name)
  throw (rti1516::ObjectInstanceNotKnown,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle handle = _ambassadorInterface->getObjectInstanceHandle(ucsToUtf8(name));
  return rti1516::ObjectInstanceHandleFriend::createHandle(handle);
}

// 10.11
std::wstring
RTIambassadorImplementation::getObjectInstanceName(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle)
  throw (rti1516::ObjectInstanceNotKnown,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);
  return utf8ToUcs(_ambassadorInterface->getObjectInstanceName(objectInstanceHandle));
}

// 10.12
rti1516::DimensionHandle
RTIambassadorImplementation::getDimensionHandle(std::wstring const & name)
  throw (rti1516::NameNotFound,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  OpenRTI::DimensionHandle handle = _ambassadorInterface->getDimensionHandle(ucsToUtf8(name));
  return rti1516::DimensionHandleFriend::createHandle(handle);
}

// 10.13
std::wstring
RTIambassadorImplementation::getDimensionName(rti1516::DimensionHandle rti1516DimensionHandle)
  throw (rti1516::InvalidDimensionHandle,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  OpenRTI::DimensionHandle dimensionHandle = rti1516::DimensionHandleFriend::getOpenRTIHandle(rti1516DimensionHandle);
  return utf8ToUcs(_ambassadorInterface->getDimensionName(dimensionHandle));
}

// 10.14
unsigned long
RTIambassadorImplementation::getDimensionUpperBound(rti1516::DimensionHandle rti1516DimensionHandle)
  throw (rti1516::InvalidDimensionHandle,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  OpenRTI::DimensionHandle dimensionHandle = rti1516::DimensionHandleFriend::getOpenRTIHandle(rti1516DimensionHandle);
  return _ambassadorInterface->getDimensionUpperBound(dimensionHandle);
}

// 10.15
rti1516::DimensionHandleSet
RTIambassadorImplementation::getAvailableDimensionsForClassAttribute(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                                                     rti1516::AttributeHandle rti1516AttributeHandle)
  throw (rti1516::InvalidObjectClassHandle,
         rti1516::InvalidAttributeHandle,
         rti1516::AttributeNotDefined,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectClassHandle objectClassHandle = rti1516::ObjectClassHandleFriend::getOpenRTIHandle(rti1516ObjectClassHandle);

  OpenRTI::AttributeHandle attributeHandle = rti1516::AttributeHandleFriend::getOpenRTIHandle(rti1516AttributeHandle);

  OpenRTI::DimensionHandleSet dimensionHandleSet = _ambassadorInterface->getAvailableDimensionsForClassAttribute(objectClassHandle, attributeHandle);

  rti1516::DimensionHandleSet rti1516DimensionHandleSet;
  for (OpenRTI::DimensionHandleSet::const_iterator i = dimensionHandleSet.begin(); i != dimensionHandleSet.end(); ++i)
    rti1516DimensionHandleSet.insert(rti1516::DimensionHandleFriend::createHandle(*i));

  return rti1516DimensionHandleSet;
}

// 10.16
rti1516::ObjectClassHandle
RTIambassadorImplementation::getKnownObjectClassHandle(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle)
  throw (rti1516::ObjectInstanceNotKnown,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  OpenRTI::ObjectInstanceHandle objectInstanceHandle = rti1516::ObjectInstanceHandleFriend::getOpenRTIHandle(rti1516ObjectInstanceHandle);
  OpenRTI::ObjectClassHandle objectClassHandle = _ambassadorInterface->getKnownObjectClassHandle(objectInstanceHandle);
  return rti1516::ObjectClassHandleFriend::createHandle(objectClassHandle);
}

// 10.17
rti1516::DimensionHandleSet
RTIambassadorImplementation::getAvailableDimensionsForInteractionClass(rti1516::InteractionClassHandle rti1516InteractionClassHandle)
  throw (rti1516::InvalidInteractionClassHandle,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  OpenRTI::InteractionClassHandle interactionClassHandle = rti1516::InteractionClassHandleFriend::getOpenRTIHandle(rti1516InteractionClassHandle);

  OpenRTI::DimensionHandleSet dimensionHandleSet = _ambassadorInterface->getAvailableDimensionsForInteractionClass(interactionClassHandle);

  rti1516::DimensionHandleSet rti1516DimensionHandleSet;
  for (OpenRTI::DimensionHandleSet::const_iterator i = dimensionHandleSet.begin(); i != dimensionHandleSet.end(); ++i)
    rti1516DimensionHandleSet.insert(rti1516::DimensionHandleFriend::createHandle(*i));

  return rti1516DimensionHandleSet;
}

// 10.18
rti1516::TransportationType
RTIambassadorImplementation::getTransportationType(std::wstring const & transportationName)
  throw (rti1516::InvalidTransportationName,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  return translate(_ambassadorInterface->getTransportationType(ucsToUtf8(transportationName)));
}

// 10.19
std::wstring
RTIambassadorImplementation::getTransportationName(rti1516::TransportationType transportationType)
  throw (rti1516::InvalidTransportationType,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  return utf8ToUcs(_ambassadorInterface->getTransportationName(translate(transportationType)));
}

// 10.20
rti1516::OrderType
RTIambassadorImplementation::getOrderType(std::wstring const & orderName)
  throw (rti1516::InvalidOrderName,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  return translate(_ambassadorInterface->getOrderType(ucsToUtf8(orderName)));
}

// 10.21
std::wstring
RTIambassadorImplementation::getOrderName(rti1516::OrderType orderType)
  throw (rti1516::InvalidOrderType,
         rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  return utf8ToUcs(_ambassadorInterface->getOrderName(translate(orderType)));
}

// 10.22
void
RTIambassadorImplementation::enableObjectClassRelevanceAdvisorySwitch()
  throw (rti1516::ObjectClassRelevanceAdvisorySwitchIsOn,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->enableObjectClassRelevanceAdvisorySwitch();
}

// 10.23
void
RTIambassadorImplementation::disableObjectClassRelevanceAdvisorySwitch()
  throw (rti1516::ObjectClassRelevanceAdvisorySwitchIsOff,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->disableObjectClassRelevanceAdvisorySwitch();
}

// 10.24
void
RTIambassadorImplementation::enableAttributeRelevanceAdvisorySwitch ()
  throw (rti1516::AttributeRelevanceAdvisorySwitchIsOn,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->enableAttributeRelevanceAdvisorySwitch();
}

// 10.25
void
RTIambassadorImplementation::disableAttributeRelevanceAdvisorySwitch ()
  throw (rti1516::AttributeRelevanceAdvisorySwitchIsOff,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->disableAttributeRelevanceAdvisorySwitch();
}

// 10.26
void
RTIambassadorImplementation::enableAttributeScopeAdvisorySwitch ()
  throw (rti1516::AttributeScopeAdvisorySwitchIsOn,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->enableAttributeScopeAdvisorySwitch();
}

// 10.27
void
RTIambassadorImplementation::disableAttributeScopeAdvisorySwitch ()
  throw (rti1516::AttributeScopeAdvisorySwitchIsOff,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->disableAttributeScopeAdvisorySwitch();
}

// 10.28
void
RTIambassadorImplementation::enableInteractionRelevanceAdvisorySwitch ()
  throw (rti1516::InteractionRelevanceAdvisorySwitchIsOn,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->enableInteractionRelevanceAdvisorySwitch();
}

// 10.29
void
RTIambassadorImplementation::disableInteractionRelevanceAdvisorySwitch ()
  throw (rti1516::InteractionRelevanceAdvisorySwitchIsOff,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->disableInteractionRelevanceAdvisorySwitch();
}

// 10.30
rti1516::DimensionHandleSet
RTIambassadorImplementation::getDimensionHandleSet(rti1516::RegionHandle rti1516RegionHandle)
  throw (rti1516::InvalidRegion,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  OpenRTI::RegionHandle regionHandle = rti1516::RegionHandleFriend::getOpenRTIHandle(rti1516RegionHandle);

  OpenRTI::DimensionHandleSet dimensionHandleSet = _ambassadorInterface->getDimensionHandleSet(regionHandle);

  rti1516::DimensionHandleSet rti1516DimensionHandleSet;
  for (OpenRTI::DimensionHandleSet::const_iterator i = dimensionHandleSet.begin(); i != dimensionHandleSet.end(); ++i)
    rti1516DimensionHandleSet.insert(rti1516::DimensionHandleFriend::createHandle(*i));

  return rti1516DimensionHandleSet;
}

// 10.31
rti1516::RangeBounds
RTIambassadorImplementation::getRangeBounds(rti1516::RegionHandle theRegionHandle,
                                            rti1516::DimensionHandle theDimensionHandle)
  throw (rti1516::InvalidRegion,
         rti1516::RegionDoesNotContainSpecifiedDimension,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  throw rti1516::RTIinternalError(L"Not implemented");
  // FIXME
  /* _ambassadorInterface->setRangeBounds(); */
}

// 10.32
void
RTIambassadorImplementation::setRangeBounds(rti1516::RegionHandle theRegionHandle,
                                            rti1516::DimensionHandle theDimensionHandle,
                                            rti1516::RangeBounds const & theRangeBounds)
  throw (rti1516::InvalidRegion,
         rti1516::RegionNotCreatedByThisFederate,
         rti1516::RegionDoesNotContainSpecifiedDimension,
         rti1516::InvalidRangeBound,
         rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  // FIXME
  /* _ambassadorInterface->setRangeBounds(); */
}

// 10.33
unsigned long
RTIambassadorImplementation::normalizeFederateHandle(rti1516::FederateHandle rti1516FederateHandle)
  throw (rti1516::FederateNotExecutionMember,
         rti1516::InvalidFederateHandle,
         rti1516::RTIinternalError)
{
  OpenRTI::FederateHandle federateHandle = rti1516::FederateHandleFriend::getOpenRTIHandle(rti1516FederateHandle);
  return _ambassadorInterface->normalizeFederateHandle(federateHandle);
}

// 10.34
unsigned long
RTIambassadorImplementation::normalizeServiceGroup(rti1516::ServiceGroupIndicator rti1516ServiceGroup)
  throw (rti1516::FederateNotExecutionMember,
         rti1516::InvalidServiceGroup,
         rti1516::RTIinternalError)
{
  OpenRTI::ServiceGroupIndicator serviceGroup;
  switch (rti1516ServiceGroup) {
  case rti1516::FEDERATION_MANAGEMENT:
    serviceGroup = OpenRTI::FEDERATION_MANAGEMENT;
    break;
  case rti1516::DECLARATION_MANAGEMENT:
    serviceGroup = OpenRTI::DECLARATION_MANAGEMENT;
    break;
  case rti1516::OBJECT_MANAGEMENT:
    serviceGroup = OpenRTI::OBJECT_MANAGEMENT;
    break;
  case rti1516::OWNERSHIP_MANAGEMENT:
    serviceGroup = OpenRTI::OWNERSHIP_MANAGEMENT;
    break;
  case rti1516::TIME_MANAGEMENT:
    serviceGroup = OpenRTI::TIME_MANAGEMENT;
    break;
  case rti1516::DATA_DISTRIBUTION_MANAGEMENT:
    serviceGroup = OpenRTI::DATA_DISTRIBUTION_MANAGEMENT;
    break;
  case rti1516::SUPPORT_SERVICES:
    serviceGroup = OpenRTI::SUPPORT_SERVICES;
    break;
  default:
    throw rti1516::InvalidServiceGroup(L"");
  }

  return _ambassadorInterface->normalizeServiceGroup(serviceGroup);
}

// 10.37
bool
RTIambassadorImplementation::evokeCallback(double approximateMinimumTimeInSeconds)
  throw (rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  return _ambassadorInterface->evokeCallback(approximateMinimumTimeInSeconds);
}

// 10.38
bool
RTIambassadorImplementation::evokeMultipleCallbacks(double approximateMinimumTimeInSeconds,
                                                    double approximateMaximumTimeInSeconds)
  throw (rti1516::FederateNotExecutionMember,
         rti1516::RTIinternalError)
{
  return _ambassadorInterface->evokeMultipleCallbacks(approximateMinimumTimeInSeconds,
                                                      approximateMaximumTimeInSeconds);
}

// 10.39
void
RTIambassadorImplementation::enableCallbacks ()
  throw (rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->enableCallbacks();
}

// 10.40
void
RTIambassadorImplementation::disableCallbacks ()
  throw (rti1516::FederateNotExecutionMember,
         rti1516::SaveInProgress,
         rti1516::RestoreInProgress,
         rti1516::RTIinternalError)
{
  _ambassadorInterface->disableCallbacks();
}

rti1516::FederateHandle
RTIambassadorImplementation::decodeFederateHandle(rti1516::VariableLengthData const & encodedValue) const
{
  return rti1516::FederateHandleFriend::createHandle(encodedValue);
}

rti1516::ObjectClassHandle
RTIambassadorImplementation::decodeObjectClassHandle(rti1516::VariableLengthData const & encodedValue) const
{
  return rti1516::ObjectClassHandleFriend::createHandle(encodedValue);
}

rti1516::InteractionClassHandle
RTIambassadorImplementation::decodeInteractionClassHandle(rti1516::VariableLengthData const & encodedValue) const
{
  return rti1516::InteractionClassHandleFriend::createHandle(encodedValue);
}

rti1516::ObjectInstanceHandle
RTIambassadorImplementation::decodeObjectInstanceHandle(rti1516::VariableLengthData const & encodedValue) const
{
  return rti1516::ObjectInstanceHandleFriend::createHandle(encodedValue);
}

rti1516::AttributeHandle
RTIambassadorImplementation::decodeAttributeHandle(rti1516::VariableLengthData const & encodedValue) const
{
  return rti1516::AttributeHandleFriend::createHandle(encodedValue);
}

rti1516::ParameterHandle
RTIambassadorImplementation::decodeParameterHandle(rti1516::VariableLengthData const & encodedValue) const
{
  return rti1516::ParameterHandleFriend::createHandle(encodedValue);
}

rti1516::DimensionHandle
RTIambassadorImplementation::decodeDimensionHandle(rti1516::VariableLengthData const & encodedValue) const
{
  return rti1516::DimensionHandleFriend::createHandle(encodedValue);
}

rti1516::MessageRetractionHandle
RTIambassadorImplementation::decodeMessageRetractionHandle(rti1516::VariableLengthData const & encodedValue) const
{
  return rti1516::MessageRetractionHandleFriend::createHandle(encodedValue);
}

rti1516::RegionHandle
RTIambassadorImplementation::decodeRegionHandle(rti1516::VariableLengthData const & encodedValue) const
{
  return rti1516::RegionHandleFriend::createHandle(encodedValue);
}

}
