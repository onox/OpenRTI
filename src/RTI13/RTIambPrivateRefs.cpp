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

#include <RTI.hh>

#include <cstring>
#include <fstream>
#include <memory>

#include "Ambassador.h"
#include "AttributeHandleSetCallback.h"
#include "AttributeHandleSetImplementation.h"
#include "AttributeHandleValuePairSetCallback.h"
#include "AttributeHandleValuePairSetImplementation.h"
#include "FederateHandleSetImplementation.h"
#include "FDD1516FileReader.h"
#include "FEDFileReader.h"
#include "LogStream.h"
#include "ParameterHandleValuePairSetCallback.h"
#include "ParameterHandleValuePairSetImplementation.h"
#include "RTI13LogicalTimeFactory.h"
#include "StringUtils.h"

/// Returns a new string as required with RTI13 interfaces
static char* newUtf8ToLocale(const std::string& utf8)
{
  std::string s = OpenRTI::utf8ToLocale(utf8);
  char* data = new char[s.size()+1];
  data[s.size()] = 0;
  return std::strncpy(data, s.c_str(), s.size());
}

static OpenRTI::VariableLengthData toOpenRTITag(const char* tag)
{
  if (!tag)
    return OpenRTI::VariableLengthData();
  return OpenRTI::VariableLengthData(tag);
}

/// Helper to get conformant handles from internal handles
template<typename T>
static RTI::ULong rti13Handle(const OpenRTI::Handle<T>& handle)
{
  if (!handle.valid())
    return ~RTI::ULong(0);
  return handle.getHandle();
}

static RTI::EventRetractionHandle rti13MessageRetractionHandle(OpenRTI::MessageRetractionHandle messageRetractionHandle)
{
  RTI::EventRetractionHandle eventRetractionHandle;
  eventRetractionHandle.sendingFederate = messageRetractionHandle.getFederateHandle();
  eventRetractionHandle.theSerialNumber = messageRetractionHandle.getSerial();
  return eventRetractionHandle;
}

static OpenRTI::MessageRetractionHandle toOpenRTIMessageRetractionHandle(RTI::EventRetractionHandle eventRetractionHandle)
{
  return OpenRTI::MessageRetractionHandle(eventRetractionHandle.sendingFederate, eventRetractionHandle.theSerialNumber);
}

static RTI::TransportType rti13TransportType(OpenRTI::TransportationType transportationType)
{
  switch (transportationType) {
  case OpenRTI::RELIABLE:
    return 0;
  default:
    return 1;
  }
}

static OpenRTI::TransportationType toOpenRTITransportationType(RTI::TransportType transportationType)
{
  switch (transportationType) {
  case 0:
    return OpenRTI::RELIABLE;
  default:
    return OpenRTI::BEST_EFFORT;
  }
}

static RTI::OrderType rti13OrderType(OpenRTI::OrderType orderType)
{
  switch (orderType) {
  case OpenRTI::TIMESTAMP:
    return 0;
  default:
    return 1;
  }
}

static OpenRTI::OrderType toOpenRTIOrderType(RTI::OrderType orderType)
{
  switch (orderType) {
  case 0:
    return OpenRTI::TIMESTAMP;
  default:
    return OpenRTI::RECEIVE;
  }
}

namespace OpenRTI {

class OPENRTI_LOCAL RTI13Traits {
public:
  // The bindings have different logical times
  typedef RTI::FedTime NativeLogicalTime;
  typedef RTI::FedTime NativeLogicalTimeInterval;

  typedef RTI::FederateAmbassador FederateAmbassador;

  // FIXME implement exceptions
  typedef RTI::Exception Exception;

#define MAP_EXCEPTION(Exception, MappedException) \
  typedef MappedException Exception; \
  static void throw ## Exception(const std::string& reason) { throw MappedException(utf8ToLocale(reason).c_str()); } \
  static void throw ## Exception() { throw MappedException(""); }

  MAP_EXCEPTION(AlreadyConnected, OpenRTI::IgnoredError)
  MAP_EXCEPTION(AsynchronousDeliveryAlreadyDisabled, RTI::AsynchronousDeliveryAlreadyDisabled)
  MAP_EXCEPTION(AsynchronousDeliveryAlreadyEnabled, RTI::AsynchronousDeliveryAlreadyEnabled)
  MAP_EXCEPTION(AttributeAcquisitionWasNotRequested, RTI::AttributeAcquisitionWasNotRequested)
  MAP_EXCEPTION(AttributeAlreadyBeingAcquired, RTI::AttributeAlreadyBeingAcquired)
  MAP_EXCEPTION(AttributeAlreadyBeingDivested, RTI::AttributeAlreadyBeingDivested)
  MAP_EXCEPTION(AttributeAlreadyOwned, RTI::AttributeAlreadyOwned)
  MAP_EXCEPTION(AttributeDivestitureWasNotRequested, RTI::AttributeDivestitureWasNotRequested)
  MAP_EXCEPTION(AttributeNotDefined, RTI::AttributeNotDefined)
  MAP_EXCEPTION(AttributeNotOwned, RTI::AttributeNotOwned)
  MAP_EXCEPTION(AttributeNotPublished, RTI::AttributeNotPublished)
  MAP_EXCEPTION(AttributeRelevanceAdvisorySwitchIsOff, OpenRTI::IgnoredError)
  MAP_EXCEPTION(AttributeRelevanceAdvisorySwitchIsOn, OpenRTI::IgnoredError)
  MAP_EXCEPTION(AttributeScopeAdvisorySwitchIsOff, OpenRTI::IgnoredError)
  MAP_EXCEPTION(AttributeScopeAdvisorySwitchIsOn, OpenRTI::IgnoredError)
  MAP_EXCEPTION(CallNotAllowedFromWithinCallback, RTI::RTIinternalError)
  MAP_EXCEPTION(ConnectionFailed, RTI::RTIinternalError)
  MAP_EXCEPTION(CouldNotCreateLogicalTimeFactory, OpenRTI::IgnoredError)
  MAP_EXCEPTION(CouldNotOpenFDD, RTI::CouldNotOpenFED)
  MAP_EXCEPTION(CouldNotOpenMIM, RTI::CouldNotOpenFED)
  MAP_EXCEPTION(DeletePrivilegeNotHeld, RTI::DeletePrivilegeNotHeld)
  MAP_EXCEPTION(ErrorReadingFDD, RTI::ErrorReadingFED)
  MAP_EXCEPTION(ErrorReadingMIM, RTI::ErrorReadingFED)
  MAP_EXCEPTION(FederateAlreadyExecutionMember, RTI::FederateAlreadyExecutionMember)
  MAP_EXCEPTION(FederateNameAlreadyInUse, RTI::RTIinternalError)
  MAP_EXCEPTION(FederateHasNotBegunSave, RTI::SaveNotInitiated)
  MAP_EXCEPTION(FederateIsExecutionMember, RTI::FederateAlreadyExecutionMember)
  MAP_EXCEPTION(FederateNotExecutionMember, RTI::FederateNotExecutionMember)
  MAP_EXCEPTION(FederateOwnsAttributes, RTI::FederateOwnsAttributes)
  MAP_EXCEPTION(FederatesCurrentlyJoined, RTI::FederatesCurrentlyJoined)
  MAP_EXCEPTION(FederateServiceInvocationsAreBeingReportedViaMOM, RTI::RTIinternalError)
  MAP_EXCEPTION(FederateUnableToUseTime, RTI::InvalidFederationTime)
  MAP_EXCEPTION(FederationExecutionAlreadyExists, RTI::FederationExecutionAlreadyExists)
  MAP_EXCEPTION(FederationExecutionDoesNotExist, RTI::FederationExecutionDoesNotExist)
  MAP_EXCEPTION(IllegalName, RTI::RTIinternalError) // does not happen, since call is not available in rti13
  MAP_EXCEPTION(InconsistentFDD, RTI::CouldNotOpenFED)
  MAP_EXCEPTION(InteractionClassNotDefined, RTI::InteractionClassNotDefined)
  MAP_EXCEPTION(InteractionClassNotPublished, RTI::InteractionClassNotPublished)
  MAP_EXCEPTION(InteractionParameterNotDefined, RTI::InteractionParameterNotDefined)
  MAP_EXCEPTION(InteractionRelevanceAdvisorySwitchIsOff, OpenRTI::IgnoredError)
  MAP_EXCEPTION(InteractionRelevanceAdvisorySwitchIsOn, OpenRTI::IgnoredError)
  MAP_EXCEPTION(InTimeAdvancingState, RTI::TimeAdvanceAlreadyInProgress)
  MAP_EXCEPTION(InvalidAttributeHandle, RTI::AttributeNotDefined)
  MAP_EXCEPTION(InvalidDimensionHandle, RTI::RTIinternalError)
  MAP_EXCEPTION(InvalidFederateHandle, RTI::RTIinternalError) // does not happen, since call is not available in rti13
  MAP_EXCEPTION(InvalidInteractionClassHandle, RTI::InteractionClassNotDefined)
  MAP_EXCEPTION(InvalidLocalSettingsDesignator, RTI::RTIinternalError)
  MAP_EXCEPTION(InvalidLogicalTime, RTI::InvalidFederationTime)
  MAP_EXCEPTION(InvalidLookahead, RTI::InvalidLookahead)
  MAP_EXCEPTION(InvalidObjectClassHandle, RTI::ObjectClassNotDefined)
  MAP_EXCEPTION(InvalidOrderName, RTI::NameNotFound)
  MAP_EXCEPTION(InvalidOrderType, RTI::InvalidOrderingHandle)
  MAP_EXCEPTION(InvalidParameterHandle, RTI::InteractionParameterNotDefined)
  MAP_EXCEPTION(InvalidRangeBound, RTI::RTIinternalError) // does not happen, since call is not available in rti13
  MAP_EXCEPTION(InvalidRegion, RTI::RegionNotKnown)
  MAP_EXCEPTION(InvalidRegionContext, RTI::InvalidRegionContext)
  MAP_EXCEPTION(InvalidResignAction, RTI::RTIinternalError)
  MAP_EXCEPTION(InvalidRetractionHandle, RTI::InvalidRetractionHandle)
  MAP_EXCEPTION(InvalidServiceGroup, RTI::RTIinternalError) // does not happen, since call is not available in rti13
  MAP_EXCEPTION(InvalidTransportationName, RTI::NameNotFound)
  MAP_EXCEPTION(InvalidTransportationType, RTI::InvalidTransportationHandle)
  MAP_EXCEPTION(LogicalTimeAlreadyPassed, RTI::FederationTimeAlreadyPassed)
  MAP_EXCEPTION(MessageCanNoLongerBeRetracted, RTI::InvalidRetractionHandle)
  MAP_EXCEPTION(NameNotFound, RTI::NameNotFound)
  MAP_EXCEPTION(NameSetWasEmpty, RTI::RTIinternalError) // does not happen, since call is not available in rti13
  MAP_EXCEPTION(NoAcquisitionPending, RTI::RTIinternalError)
  MAP_EXCEPTION(NotConnected, RTI::FederateNotExecutionMember)
  MAP_EXCEPTION(ObjectClassNotDefined, RTI::ObjectClassNotDefined)
  MAP_EXCEPTION(ObjectClassNotPublished, RTI::ObjectClassNotPublished)
  MAP_EXCEPTION(ObjectClassRelevanceAdvisorySwitchIsOff, OpenRTI::IgnoredError)
  MAP_EXCEPTION(ObjectClassRelevanceAdvisorySwitchIsOn, OpenRTI::IgnoredError)
  MAP_EXCEPTION(ObjectInstanceNameInUse, RTI::ObjectAlreadyRegistered)
  MAP_EXCEPTION(ObjectInstanceNameNotReserved, RTI::ObjectAlreadyRegistered) // can happen for an rti13 federate during registration
  MAP_EXCEPTION(ObjectInstanceNotKnown, RTI::ObjectNotKnown)
  MAP_EXCEPTION(OwnershipAcquisitionPending, RTI::OwnershipAcquisitionPending)
  MAP_EXCEPTION(RegionDoesNotContainSpecifiedDimension, RTI::RTIinternalError)
  MAP_EXCEPTION(RegionInUseForUpdateOrSubscription, RTI::RegionInUse)
  MAP_EXCEPTION(RegionNotCreatedByThisFederate, RTI::RegionNotKnown)
  MAP_EXCEPTION(RequestForTimeConstrainedPending, RTI::EnableTimeConstrainedPending)
  MAP_EXCEPTION(RequestForTimeRegulationPending, RTI::EnableTimeRegulationPending)
  MAP_EXCEPTION(RestoreInProgress, RTI::RestoreInProgress)
  MAP_EXCEPTION(RestoreNotRequested, RTI::RestoreNotRequested)
  MAP_EXCEPTION(RTIinternalError, RTI::RTIinternalError)
  MAP_EXCEPTION(SaveInProgress, RTI::SaveInProgress)
  MAP_EXCEPTION(SaveNotInitiated, RTI::SaveNotInitiated)
  MAP_EXCEPTION(SynchronizationPointLabelNotAnnounced, RTI::SynchronizationPointLabelWasNotAnnounced)
  MAP_EXCEPTION(TimeConstrainedAlreadyEnabled, RTI::TimeConstrainedAlreadyEnabled)
  MAP_EXCEPTION(TimeConstrainedIsNotEnabled, RTI::TimeConstrainedWasNotEnabled)
  MAP_EXCEPTION(TimeRegulationAlreadyEnabled, RTI::TimeRegulationAlreadyEnabled)
  MAP_EXCEPTION(TimeRegulationIsNotEnabled, RTI::TimeRegulationWasNotEnabled)
  MAP_EXCEPTION(UnsupportedCallbackModel, RTI::RTIinternalError)
#undef MAP_EXCEPTION
};

class OPENRTI_LOCAL RTI13Federate : public OpenRTI::Federate<OpenRTI::RTI13Traits, OpenRTI::RTI13LogicalTimeFactory> {
public:
  RTI13Federate(const std::string& federateType, const std::string& federateName,
                const OpenRTI::FederateHandle& federateHandle, SharedPtr<AbstractConnect> connect,
                const OpenRTI::InsertFederationExecutionMessage& insertFederationExecution,
                RTI::FederateAmbassador* federateAmbassador) :
    Federate<OpenRTI::RTI13Traits, OpenRTI::RTI13LogicalTimeFactory>(federateType, federateName, federateHandle, connect, insertFederationExecution,
                                                                     RTI13LogicalTimeFactory(insertFederationExecution.getLogicalTimeFactoryName())),
    _federateAmbassador(federateAmbassador)
  {
  }

  virtual void synchronizationPointRegistrationResponse(const std::string& label, OpenRTI::RegisterFederationSynchronizationPointResponseType reason)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      switch (reason) {
      case OpenRTI::RegisterFederationSynchronizationPointResponseSuccess:
        _federateAmbassador->synchronizationPointRegistrationSucceeded(OpenRTI::utf8ToLocale(label).c_str());
        break;
      default:
        _federateAmbassador->synchronizationPointRegistrationFailed(OpenRTI::utf8ToLocale(label).c_str());
        break;
      }
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
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
      _federateAmbassador->announceSynchronizationPoint(OpenRTI::utf8ToLocale(label).c_str(), rti13Tag(tag));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
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
      _federateAmbassador->federationSynchronized(OpenRTI::utf8ToLocale(label).c_str());
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
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
  //     _federateAmbassador->initiateFederateSave(OpenRTI::utf8ToLocale(label).c_str());
  //   } catch (const RTI::Exception& e) {
  //     throw OpenRTI::FederateInternalError(OpenRTI::localeToUtf8(e._reason));
  //   }
  // }

  // virtual
  // void
  // initiateFederateSave(std::string const& label,
  //                      OpenRTI::LogicalTime const&)
  //   throw (OpenRTI::UnableToPerformSave,
  //          OpenRTI::InvalidLogicalTime,
  //          OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->initiateFederateSave(OpenRTI::utf8ToLocale(label).c_str());
  //   } catch (const RTI::UnableToPerformSave& e) {
  //     throw OpenRTI::UnableToPerformSave(OpenRTI::localeToUtf8(e._reason));
  //   } catch (const RTI::Exception& e) {
  //     throw OpenRTI::FederateInternalError(OpenRTI::localeToUtf8(e._reason));
  //   } catch (...) {
  //     throw OpenRTI::FederateInternalError("Unknown exception");
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
  //   } catch (const RTI::Exception& e) {
  //     throw OpenRTI::FederateInternalError(OpenRTI::localeToUtf8(e._reason));
  //   } catch (...) {
  //     throw OpenRTI::FederateInternalError("Unknown exception");
  //   }
  // }

  // virtual void
  // federationNotSaved(OpenRTI::SaveFailureReason theSaveFailureReason)
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->federationNotSaved();
  //   } catch (const RTI::Exception& e) {
  //     throw OpenRTI::FederateInternalError(OpenRTI::localeToUtf8(e._reason));
  //   } catch (...) {
  //     throw OpenRTI::FederateInternalError("Unknown exception");
  //   }
  // }

  // // 4.17
  // virtual void
  // federationSaveStatusResponse(OpenRTI::FederateHandleSaveStatusPairVector const& theFederateStatusVector)
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   /// Should never be called since there is also no queryFederationSaveStatus call in RTI13
  //   throw OpenRTI::FederateInternalError();
  // }

  // // 4.19
  // virtual void
  // requestFederationRestoreSucceeded(std::string const& label)
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->requestFederationRestoreSucceeded(OpenRTI::utf8ToLocale(label).c_str());
  //   } catch (const RTI::Exception& e) {
  //     throw OpenRTI::FederateInternalError(OpenRTI::localeToUtf8(e._reason));
  //   } catch (...) {
  //     throw OpenRTI::FederateInternalError("Unknown exception");
  //   }
  // }

  // virtual void
  // requestFederationRestoreFailed(std::string const& label)
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     /// FIXME, we might route that second argument through the internal interface
  //     _federateAmbassador->requestFederationRestoreFailed(OpenRTI::utf8ToLocale(label).c_str(), "Don't know why");
  //   } catch (const RTI::Exception& e) {
  //     throw OpenRTI::FederateInternalError(OpenRTI::localeToUtf8(e._reason));
  //   } catch (...) {
  //     throw OpenRTI::FederateInternalError("Unknown exception");
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
  //   } catch (const RTI::Exception& e) {
  //     throw OpenRTI::FederateInternalError(OpenRTI::localeToUtf8(e._reason));
  //   } catch (...) {
  //     throw OpenRTI::FederateInternalError("Unknown exception");
  //   }
  // }

  // // 4.21
  // virtual void
  // initiateFederateRestore(std::string const & label,
  //                         OpenRTI::FederateHandle handle)
  //   throw (OpenRTI::SpecifiedSaveLabelDoesNotExist,
  //          OpenRTI::CouldNotInitiateRestore,
  //          OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->initiateFederateRestore(OpenRTI::utf8ToLocale(label).c_str(), rti13Handle(handle));
  //   } catch (const RTI::SpecifiedSaveLabelDoesNotExist& e) {
  //     throw OpenRTI::SpecifiedSaveLabelDoesNotExist(OpenRTI::localeToUtf8(e._reason));
  //   } catch (const RTI::CouldNotRestore& e) {
  //     throw OpenRTI::CouldNotInitiateRestore(OpenRTI::localeToUtf8(e._reason));
  //   } catch (const RTI::Exception& e) {
  //     throw OpenRTI::FederateInternalError(OpenRTI::localeToUtf8(e._reason));
  //   } catch (...) {
  //     throw OpenRTI::FederateInternalError("Unknown exception");
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
  //   } catch (const RTI::Exception& e) {
  //     throw OpenRTI::FederateInternalError(OpenRTI::localeToUtf8(e._reason));
  //   } catch (...) {
  //     throw OpenRTI::FederateInternalError("Unknown exception");
  //   }
  // }

  // virtual void
  // federationNotRestored(OpenRTI::RestoreFailureReason)
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     _federateAmbassador->federationNotRestored();
  //   } catch (const RTI::Exception& e) {
  //     throw OpenRTI::FederateInternalError(OpenRTI::localeToUtf8(e._reason));
  //   } catch (...) {
  //     throw OpenRTI::FederateInternalError("Unknown exception");
  //   }
  // }

  // // 4.25
  // virtual void
  // federationRestoreStatusResponse(OpenRTI::FederateHandleRestoreStatusPairVector const& theFederateStatusVector)
  //   throw (OpenRTI::FederateInternalError)
  // {
  //   if (!_federateAmbassador)
  //     throw OpenRTI::FederateInternalError();
  //   try {
  //     throw RTI::FederateInternalError("Not implemented");
  //     // _federateAmbassador
  //   } catch (const RTI::Exception& e) {
  //     throw OpenRTI::FederateInternalError(OpenRTI::localeToUtf8(e._reason));
  //   } catch (...) {
  //     throw OpenRTI::FederateInternalError("Unknown exception");
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
      if (start)
        _federateAmbassador->startRegistrationForObjectClass(rti13Handle(objectClassHandle));
      else
        _federateAmbassador->stopRegistrationForObjectClass(rti13Handle(objectClassHandle));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
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
      if (on)
        _federateAmbassador->turnInteractionsOn(rti13Handle(interactionClassHandle));
      else
        _federateAmbassador->turnInteractionsOff(rti13Handle(interactionClassHandle));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  ////////////////////////////////
  // Object Management Services //
  ////////////////////////////////

  virtual void objectInstanceNameReservation(const ReserveObjectInstanceNameResponseMessage&)
    throw ()
  {
    /// No name reservation in RTI13, so we do not need a callback
  }
  virtual void objectInstanceNameReservation(const ReserveMultipleObjectInstanceNameResponseMessage&)
    throw ()
  {
    /// No name reservation in RTI13, so we do not need a callback
  }

  // 6.5
  virtual
  void
  discoverObjectInstance(OpenRTI::ObjectInstanceHandle objectInstanceHandle,
                         OpenRTI::ObjectClassHandle objectClassHandle,
                         std::string const& name)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->discoverObjectInstance(rti13Handle(objectInstanceHandle),
                                                  rti13Handle(objectClassHandle),
                                                  OpenRTI::utf8ToLocale(name).c_str());
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  virtual void reflectAttributeValues(const ObjectClass& objectClass, const OpenRTI::AttributeUpdateMessage& message)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      RTI::TransportType transportType = rti13TransportType(message.getTransportationType());
      RTI::OrderType orderType = rti13OrderType(OpenRTI::RECEIVE);
      AttributeHandleValuePairSetCallback attributeHandleValues(transportType, orderType);
      attributeHandleValues.getAttributeValues().reserve(message.getAttributeValues().size());
      for (std::vector<OpenRTI::AttributeValue>::const_iterator i = message.getAttributeValues().begin();
           i != message.getAttributeValues().end(); ++i) {
        if (objectClass.getAttributeSubscriptionType(i->getAttributeHandle()) == Unsubscribed)
          continue;
        attributeHandleValues.getAttributeValues().push_back(*i);
      }
      if (!attributeHandleValues.getAttributeValues().empty())
        _federateAmbassador->reflectAttributeValues(rti13Handle(message.getObjectInstanceHandle()), attributeHandleValues,
                                                    rti13Tag(message.getTag()));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  virtual void reflectAttributeValues(const ObjectClass& objectClass, const OpenRTI::TimeStampedAttributeUpdateMessage& message,
                                      const RTI::FedTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      RTI::TransportType transportType = rti13TransportType(message.getTransportationType());
      if (getTimeConstrainedEnabled()) {
        RTI::OrderType orderType = rti13OrderType(OpenRTI::TIMESTAMP);
        AttributeHandleValuePairSetCallback attributeHandleValues(transportType, orderType);
        RTI::EventRetractionHandle eventRetractionHandle = rti13MessageRetractionHandle(message.getMessageRetractionHandle());
        attributeHandleValues.getAttributeValues().reserve(message.getAttributeValues().size());
        for (std::vector<OpenRTI::AttributeValue>::const_iterator i = message.getAttributeValues().begin();
             i != message.getAttributeValues().end(); ++i) {
          if (objectClass.getAttributeSubscriptionType(i->getAttributeHandle()) == Unsubscribed)
            continue;
          attributeHandleValues.getAttributeValues().push_back(*i);
        }
        if (!attributeHandleValues.getAttributeValues().empty())
          _federateAmbassador->reflectAttributeValues(rti13Handle(message.getObjectInstanceHandle()), attributeHandleValues,
                                                      logicalTime, rti13Tag(message.getTag()), eventRetractionHandle);
      } else {
        RTI::OrderType orderType = rti13OrderType(OpenRTI::RECEIVE);
        AttributeHandleValuePairSetCallback attributeHandleValues(transportType, orderType);
        attributeHandleValues.getAttributeValues().reserve(message.getAttributeValues().size());
        for (std::vector<OpenRTI::AttributeValue>::const_iterator i = message.getAttributeValues().begin();
             i != message.getAttributeValues().end(); ++i) {
          if (objectClass.getAttributeSubscriptionType(i->getAttributeHandle()) == Unsubscribed)
            continue;
          attributeHandleValues.getAttributeValues().push_back(*i);
        }
        if (!attributeHandleValues.getAttributeValues().empty())
          _federateAmbassador->reflectAttributeValues(rti13Handle(message.getObjectInstanceHandle()), attributeHandleValues,
                                                      rti13Tag(message.getTag()));
      }
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 6.11
  virtual
  void
  removeObjectInstance(const OpenRTI::DeleteObjectInstanceMessage& message)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->removeObjectInstance(rti13Handle(message.getObjectInstanceHandle()), rti13Tag(message.getTag()));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  virtual
  void
    removeObjectInstance(const OpenRTI::TimeStampedDeleteObjectInstanceMessage& message, const RTI::FedTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      if (getTimeConstrainedEnabled()) {
        RTI::EventRetractionHandle eventRetractionHandle = rti13MessageRetractionHandle(message.getMessageRetractionHandle());
        _federateAmbassador->removeObjectInstance(rti13Handle(message.getObjectInstanceHandle()), logicalTime,
                                                  rti13Tag(message.getTag()), eventRetractionHandle);
      } else {
        _federateAmbassador->removeObjectInstance(rti13Handle(message.getObjectInstanceHandle()), rti13Tag(message.getTag()));
      }
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 6.9
  virtual void
  receiveInteraction(const InteractionClassHandle& interactionClassHandle, const OpenRTI::InteractionMessage& message)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      const InteractionClass& interactionClass = getInteractionClass(interactionClassHandle);
      RTI::InteractionClassHandle rti13InteractionClassHandle = rti13Handle(interactionClassHandle);
      RTI::TransportType transportType = rti13TransportType(message.getTransportationType());
      RTI::OrderType orderType = rti13OrderType(OpenRTI::RECEIVE);
      // FIXME no regions so far
      ParameterHandleValuePairSetCallback parameterHandleArray(transportType, orderType, 0);
      parameterHandleArray.getParameterValues().reserve(message.getParameterValues().size());
      for (std::vector<OpenRTI::ParameterValue>::const_iterator i = message.getParameterValues().begin();
           i != message.getParameterValues().end(); ++i) {
        if (!interactionClass.isValidParameter(i->getParameterHandle()))
          continue;
        parameterHandleArray.getParameterValues().push_back(*i);
      }
      _federateAmbassador->receiveInteraction(rti13InteractionClassHandle, parameterHandleArray, rti13Tag(message.getTag()));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  virtual void
    receiveInteraction(const InteractionClassHandle& interactionClassHandle, const OpenRTI::TimeStampedInteractionMessage& message, const RTI::FedTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      const InteractionClass& interactionClass = getInteractionClass(interactionClassHandle);
      RTI::InteractionClassHandle rti13InteractionClassHandle = rti13Handle(interactionClassHandle);
      RTI::TransportType transportType = rti13TransportType(message.getTransportationType());
      RTI::OrderType orderType = rti13OrderType(OpenRTI::TIMESTAMP);
      RTI::EventRetractionHandle eventRetractionHandle = rti13MessageRetractionHandle(message.getMessageRetractionHandle());
      // FIXME no regions so far
      ParameterHandleValuePairSetCallback parameterHandleArray(transportType, orderType, 0);
      parameterHandleArray.getParameterValues().reserve(message.getParameterValues().size());
      for (std::vector<OpenRTI::ParameterValue>::const_iterator i = message.getParameterValues().begin();
           i != message.getParameterValues().end(); ++i) {
        if (!interactionClass.isValidParameter(i->getParameterHandle()))
          continue;
        parameterHandleArray.getParameterValues().push_back(*i);
      }
      _federateAmbassador->receiveInteraction(rti13InteractionClassHandle, parameterHandleArray, logicalTime,
                                              rti13Tag(message.getTag()), eventRetractionHandle);
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 6.15
  virtual
  void
  attributesInScope(OpenRTI::ObjectInstanceHandle objectInstanceHandle, const OpenRTI::AttributeHandleVector& attributeHandleVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->attributesInScope(rti13Handle(objectInstanceHandle), AttributeHandleSetCallback(attributeHandleVector));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 6.16
  virtual
  void
  attributesOutOfScope(OpenRTI::ObjectInstanceHandle objectInstanceHandle, const OpenRTI::AttributeHandleVector& attributeHandleVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->attributesOutOfScope(rti13Handle(objectInstanceHandle), AttributeHandleSetCallback(attributeHandleVector));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 6.18
  virtual
  void
  provideAttributeValueUpdate(OpenRTI::ObjectInstanceHandle objectInstanceHandle,
                              const OpenRTI::AttributeHandleVector& attributeHandleVector,
                              const OpenRTI::VariableLengthData& tag)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->provideAttributeValueUpdate(rti13Handle(objectInstanceHandle), AttributeHandleSetCallback(attributeHandleVector));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 6.19
  virtual
  void
  turnUpdatesOnForObjectInstance(OpenRTI::ObjectInstanceHandle objectInstanceHandle, const OpenRTI::AttributeHandleVector& attributeHandleVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->turnUpdatesOnForObjectInstance(rti13Handle(objectInstanceHandle), AttributeHandleSetCallback(attributeHandleVector));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 6.20
  virtual
  void
  turnUpdatesOffForObjectInstance(OpenRTI::ObjectInstanceHandle objectInstanceHandle, const OpenRTI::AttributeHandleVector& attributeHandleVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->turnUpdatesOffForObjectInstance(rti13Handle(objectInstanceHandle), AttributeHandleSetCallback(attributeHandleVector));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  ///////////////////////////////////
  // Ownership Management Services //
  ///////////////////////////////////

  // 7.4
  virtual
  void
  requestAttributeOwnershipAssumption(OpenRTI::ObjectInstanceHandle objectInstanceHandle, const OpenRTI::AttributeHandleVector& attributeHandleVector,
                                      const OpenRTI::VariableLengthData& tag)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->requestAttributeOwnershipAssumption(rti13Handle(objectInstanceHandle),
                                                               AttributeHandleSetCallback(attributeHandleVector), rti13Tag(tag));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 7.5
  virtual
  void
  requestDivestitureConfirmation(OpenRTI::ObjectInstanceHandle objectInstanceHandle, const OpenRTI::AttributeHandleVector& attributeHandleVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->attributeOwnershipDivestitureNotification(rti13Handle(objectInstanceHandle), AttributeHandleSetCallback(attributeHandleVector));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 7.7
  virtual
  void
  attributeOwnershipAcquisitionNotification(OpenRTI::ObjectInstanceHandle objectInstanceHandle, const OpenRTI::AttributeHandleVector& attributeHandleVector,
                                            const OpenRTI::VariableLengthData& tag)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->attributeOwnershipAcquisitionNotification(rti13Handle(objectInstanceHandle), AttributeHandleSetCallback(attributeHandleVector));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 7.10
  virtual
  void
  attributeOwnershipUnavailable(OpenRTI::ObjectInstanceHandle objectInstanceHandle, const OpenRTI::AttributeHandleVector& attributeHandleVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->attributeOwnershipUnavailable(rti13Handle(objectInstanceHandle), AttributeHandleSetCallback(attributeHandleVector));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 7.11
  virtual
  void
  requestAttributeOwnershipRelease(OpenRTI::ObjectInstanceHandle objectInstanceHandle, const OpenRTI::AttributeHandleVector& attributeHandleVector,
                                   const OpenRTI::VariableLengthData& tag)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->requestAttributeOwnershipRelease(rti13Handle(objectInstanceHandle),
                                                            AttributeHandleSetCallback(attributeHandleVector), rti13Tag(tag));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 7.15
  virtual
  void
  confirmAttributeOwnershipAcquisitionCancellation(OpenRTI::ObjectInstanceHandle objectInstanceHandle,
                                                   const OpenRTI::AttributeHandleVector& attributeHandleVector)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->confirmAttributeOwnershipAcquisitionCancellation(rti13Handle(objectInstanceHandle), AttributeHandleSetCallback(attributeHandleVector));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 7.17
  virtual
  void
  informAttributeOwnership(OpenRTI::ObjectInstanceHandle objectInstanceHandle,
                           OpenRTI::AttributeHandle attributeHandle,
                           OpenRTI::FederateHandle federateHandle)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->informAttributeOwnership(rti13Handle(objectInstanceHandle),
                                                    rti13Handle(attributeHandle),
                                                    rti13Handle(federateHandle));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  virtual
  void
  attributeIsNotOwned(OpenRTI::ObjectInstanceHandle objectInstanceHandle,
                      OpenRTI::AttributeHandle attributeHandle)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->attributeIsNotOwned(rti13Handle(objectInstanceHandle),
                                               rti13Handle(attributeHandle));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  virtual
  void
  attributeIsOwnedByRTI(OpenRTI::ObjectInstanceHandle objectInstanceHandle,
                        OpenRTI::AttributeHandle attributeHandle)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->attributeOwnedByRTI(rti13Handle(objectInstanceHandle),
                                               rti13Handle(attributeHandle));
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  //////////////////////////////
  // Time Management Services //
  //////////////////////////////

  // 8.3
  virtual
  void
  timeRegulationEnabled(const RTI::FedTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->timeRegulationEnabled(logicalTime);
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 8.6
  virtual
  void
  timeConstrainedEnabled(const RTI::FedTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->timeConstrainedEnabled(logicalTime);
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 8.13
  virtual
  void
    timeAdvanceGrant(const RTI::FedTime& logicalTime)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      _federateAmbassador->timeAdvanceGrant(logicalTime);
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  // 8.22
  virtual
  void
  requestRetraction(OpenRTI::MessageRetractionHandle theHandle)
    throw ()
  {
    if (!_federateAmbassador) {
      Log(FederateAmbassador, Warning) << "Calling callback with zero ambassador!" << std::endl;
      return;
    }
    try {
      // _federateAmbassador
      throw RTI::FederateInternalError("Not implemented");
    } catch (const RTI::Exception& e) {
      Log(FederateAmbassador, Warning) << "Caught an RTI exception in callback: " << e._reason << std::endl;
    }
  }

  /// Hmm, use that cached allocation, so that we only need to copy ...
  const char* rti13Tag(const OpenRTI::VariableLengthData& tag)
  {
    if (tag.empty())
      return "";
    _tagCache.assign(tag.charData(), tag.size());
    return _tagCache.c_str();
  }
  std::string _tagCache;

  RTI::FederateAmbassador* _federateAmbassador;
};

} // namespace OpenRTI

class OPENRTI_LOCAL RTIambPrivateRefs : public OpenRTI::Ambassador<OpenRTI::RTI13Traits> {
public:
  RTIambPrivateRefs() :
    _concurrentAccess(false)
  { }

  virtual OpenRTI::AbstractFederate<Traits>*
  createFederate(const std::string& federateType, const std::string& federateName,
                 const OpenRTI::FederateHandle& federateHandle, const std::string& federationName,
                 const OpenRTI::InsertFederationExecutionMessage& insertFederationExecution,
                 OpenRTI::SharedPtr<OpenRTI::AbstractConnect> connect, RTI::FederateAmbassador* federateAmbassador)
  {
    return new OpenRTI::RTI13Federate(federateType, federateName, federateHandle, connect, insertFederationExecution, federateAmbassador);
  }

  struct ConcurrentAccessGuard {
    ConcurrentAccessGuard(RTIambPrivateRefs& p) : _p(p)
    {
      if (_p._concurrentAccess)
        throw RTI::ConcurrentAccessAttempted("Calling ambassador from callback!");
      _p._concurrentAccess = true;
    }
    ~ConcurrentAccessGuard()
    {
      _p._concurrentAccess = false;
    }

    RTIambPrivateRefs& _p;
  };

  void ensureConnected(const std::string& federationExecutionName)
  {
    // FIXME allow the local settings stuff to be a configuration file and others
    OpenRTI::StringMap defaults;
    defaults["protocol"] = "thread";
    OpenRTI::StringMap stringMap = OpenRTI::getStringMapFromUrl(defaults, federationExecutionName);
    stringMap.erase("federationExecutionName");

    if (!isConnected()) {
      // FIXME make that again configurable
      OpenRTI::Clock abstime = OpenRTI::Clock::now() + OpenRTI::Clock::fromSeconds(70);

      connect(stringMap, abstime);

      _connectParameters = stringMap;
    } else if (_connectParameters != stringMap) {
      throw RTI::RTIinternalError("Connect url does not point to the same connection.");
    }
  }

  // The instantiation time argument list
  OpenRTI::StringMap _connectParameters;

  bool _concurrentAccess;
};

RTI::RTIambassador::RTIambassador()
  throw (MemoryExhausted, RTIinternalError) :
  privateData(0),
  privateRefs(new RTIambPrivateRefs)
{
}

RTI::RTIambassador::~RTIambassador()
  throw (RTI::RTIinternalError)
{
  delete privateRefs;
  privateRefs = 0;
}

void
RTI::RTIambassador::createFederationExecution(const char* federationExecutionName,
                                              const char* fedFile)
  throw (RTI::FederationExecutionAlreadyExists,
         RTI::CouldNotOpenFED,
         RTI::ErrorReadingFED,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);

  // Make sure we can read the fed file
  if (!fedFile)
    throw RTI::CouldNotOpenFED("fedFile is NULL!");
  std::ifstream stream(fedFile);
  if (!stream.is_open())
    throw RTI::CouldNotOpenFED(fedFile);

  OpenRTI::FOMStringModuleList fomModules;
  try {
    if (OpenRTI::matchExtension(fedFile, ".fed"))
      fomModules.push_back(OpenRTI::FEDFileReader::read(stream));
    else
      fomModules.push_back(OpenRTI::FDD1516FileReader::read(stream));
  } catch (const OpenRTI::Exception& e) {
    throw RTI::ErrorReadingFED(OpenRTI::utf8ToLocale(e.getReason()).c_str());
  } catch (...) {
    throw RTI::RTIinternalError("Unknown error");
  }

  std::string utf8FederationExecutionName = OpenRTI::localeToUtf8(federationExecutionName);
  privateRefs->ensureConnected(utf8FederationExecutionName);
  privateRefs->createFederationExecution(OpenRTI::getFilePart(utf8FederationExecutionName), fomModules, "HLAfloat64Time");
}

void
RTI::RTIambassador::destroyFederationExecution(const char* federationExecutionName)
    throw (RTI::FederatesCurrentlyJoined,
           RTI::FederationExecutionDoesNotExist,
	   RTI::ConcurrentAccessAttempted,
           RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  std::string utf8FederationExecutionName = OpenRTI::localeToUtf8(federationExecutionName);

  privateRefs->ensureConnected(utf8FederationExecutionName);

  privateRefs->destroyFederationExecution(OpenRTI::getFilePart(utf8FederationExecutionName));
}

RTI::FederateHandle
RTI::RTIambassador::joinFederationExecution(const char* federateType,
                                            const char* federationExecutionName,
                                            RTI::FederateAmbassadorPtr federateAmbassadorPointer)
    throw (RTI::FederateAlreadyExecutionMember,
           RTI::FederationExecutionDoesNotExist,
	   RTI::CouldNotOpenFED,
           RTI::ErrorReadingFED,
           RTI::ConcurrentAccessAttempted,
	   RTI::SaveInProgress,
           RTI::RestoreInProgress,
           RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  if (!federateAmbassadorPointer)
    throw RTI::FederationExecutionDoesNotExist("Joining with a zero federate ambassador pointer!");
  std::string utf8FederationExecutionName = OpenRTI::localeToUtf8(federationExecutionName);

  privateRefs->ensureConnected(utf8FederationExecutionName);

  std::string utf8FederateType = OpenRTI::localeToUtf8(federateType);
  FederateHandle federateHandle = privateRefs->joinFederationExecution(std::string(), utf8FederateType,
                                                                       OpenRTI::getFilePart(utf8FederationExecutionName),
                                                                       OpenRTI::FOMStringModuleList(), federateAmbassadorPointer);
  return federateHandle;
}

void
RTI::RTIambassador::resignFederationExecution(RTI::ResignAction RTI13resignAction)
  throw (RTI::FederateOwnsAttributes,
         RTI::FederateNotExecutionMember,
         RTI::InvalidResignAction,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  OpenRTI::ResignAction resignAction;
  switch (RTI13resignAction) {
  case RTI::RELEASE_ATTRIBUTES:
    resignAction = OpenRTI::UNCONDITIONALLY_DIVEST_ATTRIBUTES;
    break;
  case RTI::DELETE_OBJECTS:
    resignAction = OpenRTI::DELETE_OBJECTS;
    break;
  case RTI::DELETE_OBJECTS_AND_RELEASE_ATTRIBUTES:
    resignAction = OpenRTI::DELETE_OBJECTS_THEN_DIVEST;
    break;
  case RTI::NO_ACTION:
    resignAction = OpenRTI::NO_ACTION;
    break;
  default:
    throw RTI::InvalidResignAction("Except");
  }
  privateRefs->resignFederationExecution(resignAction);
}

void
RTI::RTIambassador::registerFederationSynchronizationPoint(const char* label,
                                                           const char* tag)
    throw (RTI::FederateNotExecutionMember,
           RTI::ConcurrentAccessAttempted,
	   RTI::SaveInProgress,
           RTI::RestoreInProgress,
           RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  std::string utf8Label = OpenRTI::localeToUtf8(label);
  privateRefs->registerFederationSynchronizationPoint(utf8Label, toOpenRTITag(tag));
}

void
RTI::RTIambassador::registerFederationSynchronizationPoint(const char* label,
                                                           const char* tag,
                                                           const RTI::FederateHandleSet& syncSet)
    throw (RTI::FederateNotExecutionMember,
           RTI::ConcurrentAccessAttempted,
           RTI::SaveInProgress,
	   RTI::RestoreInProgress,
           RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  std::string utf8Label = OpenRTI::localeToUtf8(label);

  OpenRTI::FederateHandleSet federateHandleSet;
  ULong syncSetSize = syncSet.size();
  for (ULong i = 0; i < syncSetSize; ++i)
    federateHandleSet.insert(syncSet.getHandle(i));

  privateRefs->registerFederationSynchronizationPoint(utf8Label, toOpenRTITag(tag), federateHandleSet);
}

void
RTI::RTIambassador::synchronizationPointAchieved(const char* label)
  throw (RTI::SynchronizationPointLabelWasNotAnnounced,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  std::string utf8Label = OpenRTI::localeToUtf8(label);
  privateRefs->synchronizationPointAchieved(utf8Label);
}

void
RTI::RTIambassador::requestFederationSave(const char* label, const RTI::FedTime& fedTime)
  throw (RTI::FederationTimeAlreadyPassed,
         RTI::InvalidFederationTime,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  std::string utf8Label = OpenRTI::localeToUtf8(label);
  privateRefs->requestFederationSave(utf8Label, fedTime);
}

void
RTI::RTIambassador::requestFederationSave(const char* label)
    throw (RTI::FederateNotExecutionMember,
           RTI::ConcurrentAccessAttempted,
           RTI::SaveInProgress,
	   RTI::RestoreInProgress,
           RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  std::string utf8Label = OpenRTI::localeToUtf8(label);
  privateRefs->requestFederationSave(utf8Label);
}

void
RTI::RTIambassador::federateSaveBegun()
  throw (RTI::SaveNotInitiated,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->federateSaveBegun();
}

void
RTI::RTIambassador::federateSaveComplete()
  throw (RTI::SaveNotInitiated,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->federateSaveComplete();
}

void
RTI::RTIambassador::federateSaveNotComplete()
  throw (RTI::SaveNotInitiated,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->federateSaveNotComplete();
}

void
RTI::RTIambassador::requestFederationRestore(const char* label)
  throw (RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  std::string utf8Label = OpenRTI::localeToUtf8(label);
  privateRefs->requestFederationRestore(utf8Label);
}

void
RTI::RTIambassador::federateRestoreComplete()
  throw (RTI::RestoreNotRequested,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->federateRestoreComplete();
}

void
RTI::RTIambassador::federateRestoreNotComplete()
  throw (RTI::RestoreNotRequested,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->federateRestoreNotComplete();
}

// Declaration Management -------------------

void
RTI::RTIambassador::publishObjectClass(RTI::ObjectClassHandle objectClassHandle,
                                       const RTI::AttributeHandleSet& attributeHandleSet)
  throw (RTI::ObjectClassNotDefined,
         RTI::AttributeNotDefined,
         RTI::OwnershipAcquisitionPending,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  OpenRTI::AttributeHandleSet OpenRTIAttributeHandleSet;
  ULong attributeHandleSetSize = attributeHandleSet.size();
  for (ULong i = 0; i < attributeHandleSetSize; ++i)
    OpenRTIAttributeHandleSet.insert(attributeHandleSet.getHandle(i));

  privateRefs->publishObjectClassAttributes(objectClassHandle, OpenRTIAttributeHandleSet);
}

void
RTI::RTIambassador::unpublishObjectClass(RTI::ObjectClassHandle objectClassHandle)
  throw (RTI::ObjectClassNotDefined,
         RTI::ObjectClassNotPublished,
         RTI::OwnershipAcquisitionPending,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->unpublishObjectClass(objectClassHandle);
}

void
RTI::RTIambassador::publishInteractionClass(RTI::InteractionClassHandle interactionClassHandle)
  throw (RTI::InteractionClassNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  OpenRTI::InteractionClassHandle OpenRTIInteractionClassHandle = interactionClassHandle;
  privateRefs->publishInteractionClass(OpenRTIInteractionClassHandle);
}

void
RTI::RTIambassador::unpublishInteractionClass(RTI::InteractionClassHandle interactionClassHandle)
  throw (RTI::InteractionClassNotDefined,
         RTI::InteractionClassNotPublished,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  OpenRTI::InteractionClassHandle OpenRTIInteractionClassHandle = interactionClassHandle;
  privateRefs->unpublishInteractionClass(OpenRTIInteractionClassHandle);
}

void
RTI::RTIambassador::subscribeObjectClassAttributes(RTI::ObjectClassHandle objectClassHandle,
                                                   const RTI::AttributeHandleSet& attributeHandleSet,
                                                   RTI::Boolean active)
  throw (RTI::ObjectClassNotDefined,
         RTI::AttributeNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  OpenRTI::AttributeHandleSet OpenRTIAttributeHandleSet;
  ULong attributeHandleSetSize = attributeHandleSet.size();
  for (ULong i = 0; i < attributeHandleSetSize; ++i)
    OpenRTIAttributeHandleSet.insert(attributeHandleSet.getHandle(i));

  privateRefs->subscribeObjectClassAttributes(objectClassHandle, OpenRTIAttributeHandleSet, active == RTI::RTI_TRUE);
}

void
RTI::RTIambassador::unsubscribeObjectClass(RTI::ObjectClassHandle objectClassHandle)
  throw (RTI::ObjectClassNotDefined,
         RTI::ObjectClassNotSubscribed,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->unsubscribeObjectClass(objectClassHandle);
}

void
RTI::RTIambassador::subscribeInteractionClass(RTI::InteractionClassHandle interactionClassHandle,
                                              RTI::Boolean active)
  throw (RTI::InteractionClassNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::FederateLoggingServiceCalls,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->subscribeInteractionClass(interactionClassHandle, active == RTI::RTI_TRUE);
}

void
RTI::RTIambassador::unsubscribeInteractionClass(RTI::InteractionClassHandle interactionClassHandle)
  throw (RTI::InteractionClassNotDefined,
         RTI::InteractionClassNotSubscribed,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->unsubscribeInteractionClass(interactionClassHandle);
}

// Object Management -------------------

RTI::ObjectHandle
RTI::RTIambassador::registerObjectInstance(RTI::ObjectClassHandle objectClassHandle,
                                           const char* name)
  throw (RTI::ObjectClassNotDefined,
         RTI::ObjectClassNotPublished,
         RTI::ObjectAlreadyRegistered,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  return privateRefs->registerObjectInstance(objectClassHandle, OpenRTI::localeToUtf8(name), true);
}

RTI::ObjectHandle
RTI::RTIambassador::registerObjectInstance(RTI::ObjectClassHandle objectClassHandle)
  throw (RTI::ObjectClassNotDefined,
         RTI::ObjectClassNotPublished,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  return privateRefs->registerObjectInstance(objectClassHandle);
}

RTI::EventRetractionHandle
RTI::RTIambassador::updateAttributeValues(RTI::ObjectHandle objectHandle,
                                          const RTI::AttributeHandleValuePairSet& attributeHandleArray,
                                          const RTI::FedTime& fedTime,
                                          const char* tag)
  throw (RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::AttributeNotOwned,
         RTI::InvalidFederationTime,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  std::vector<OpenRTI::AttributeValue> attributeValueVector;
  ULong attributeHandleArraySize = attributeHandleArray.size();
  attributeValueVector.resize(attributeHandleArraySize);
  for (ULong i = 0; i < attributeHandleArraySize; ++i) {
    ULong length;
    char* value = attributeHandleArray.getValuePointer(i, length);
    attributeValueVector[i].setAttributeHandle(attributeHandleArray.getHandle(i));
    attributeValueVector[i].getValue().setData(value, length);
  }

  OpenRTI::MessageRetractionHandle messageRetractionHandle;
  messageRetractionHandle = privateRefs->updateAttributeValues(objectHandle, attributeValueVector, toOpenRTITag(tag), fedTime);
  return rti13MessageRetractionHandle(messageRetractionHandle);
}

void
RTI::RTIambassador::updateAttributeValues(RTI::ObjectHandle objectHandle,
                                          const RTI::AttributeHandleValuePairSet& attributeHandleArray,
                                          const char* tag)
  throw (RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::AttributeNotOwned,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  std::vector<OpenRTI::AttributeValue> attributeValueVector;
  ULong attributeHandleArraySize = attributeHandleArray.size();
  attributeValueVector.resize(attributeHandleArraySize);
  for (ULong i = 0; i < attributeHandleArraySize; ++i) {
    ULong length;
    char* value = attributeHandleArray.getValuePointer(i, length);
    attributeValueVector[i].setAttributeHandle(attributeHandleArray.getHandle(i));
    attributeValueVector[i].getValue().setData(value, length);
  }

  privateRefs->updateAttributeValues(objectHandle, attributeValueVector, toOpenRTITag(tag));
}

RTI::EventRetractionHandle
RTI::RTIambassador::sendInteraction(RTI::InteractionClassHandle interactionClassHandle,
                                    const RTI::ParameterHandleValuePairSet& parameterHandleArray,
                                    const RTI::FedTime& fedTime,
                                    const char* tag)
  throw (RTI::InteractionClassNotDefined,
         RTI::InteractionClassNotPublished,
         RTI::InteractionParameterNotDefined,
         RTI::InvalidFederationTime,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  OpenRTI::InteractionClassHandle OpenRTIInteractionClassHandle = interactionClassHandle;

  std::vector<OpenRTI::ParameterValue> parameterValueVector;
  ULong parameterHandleArraySize = parameterHandleArray.size();
  parameterValueVector.resize(parameterHandleArraySize);
  for (ULong i = 0; i < parameterHandleArraySize; ++i) {
    ULong length;
    char* value = parameterHandleArray.getValuePointer(i, length);
    parameterValueVector[i].setParameterHandle(parameterHandleArray.getHandle(i));
    parameterValueVector[i].getValue().setData(value, length);
  }

  // Note that the parametervaluevector is taken by the sendInteraction call. Thus on return it is empty
  OpenRTI::MessageRetractionHandle messageRetractionHandle;
  messageRetractionHandle = privateRefs->sendInteraction(OpenRTIInteractionClassHandle, parameterValueVector,
                                                         toOpenRTITag(tag), fedTime);
  return rti13MessageRetractionHandle(messageRetractionHandle);
}

void
RTI::RTIambassador::sendInteraction(RTI::InteractionClassHandle interactionClassHandle,
                                    const RTI::ParameterHandleValuePairSet& parameterHandleArray,
                                    const char* tag)
  throw (RTI::InteractionClassNotDefined,
         RTI::InteractionClassNotPublished,
         RTI::InteractionParameterNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  OpenRTI::InteractionClassHandle OpenRTIInteractionClassHandle = interactionClassHandle;

  std::vector<OpenRTI::ParameterValue> parameterValueVector;
  ULong parameterHandleArraySize = parameterHandleArray.size();
  parameterValueVector.resize(parameterHandleArraySize);
  for (ULong i = 0; i < parameterHandleArraySize; ++i) {
    ULong length;
    char* value = parameterHandleArray.getValuePointer(i, length);
    parameterValueVector[i].setParameterHandle(parameterHandleArray.getHandle(i));
    parameterValueVector[i].getValue().setData(value, length);
  }

  // Note that the parametervaluevector is taken by the sendInteraction call. Thus on return it is empty
  privateRefs->sendInteraction(OpenRTIInteractionClassHandle, parameterValueVector, toOpenRTITag(tag));
}

RTI::EventRetractionHandle
RTI::RTIambassador::deleteObjectInstance(RTI::ObjectHandle objectHandle,
                                         const RTI::FedTime& fedTime,
                                         const char* tag)
  throw (RTI::ObjectNotKnown,
         RTI::DeletePrivilegeNotHeld,
         RTI::InvalidFederationTime,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  OpenRTI::MessageRetractionHandle messageRetractionHandle;
  messageRetractionHandle = privateRefs->deleteObjectInstance(objectHandle, toOpenRTITag(tag), fedTime);
  return rti13MessageRetractionHandle(messageRetractionHandle);
}

void
RTI::RTIambassador::deleteObjectInstance(RTI::ObjectHandle objectHandle,
                                         const char* tag)
  throw (RTI::ObjectNotKnown,
         RTI::DeletePrivilegeNotHeld,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->deleteObjectInstance(objectHandle, toOpenRTITag(tag));
}

void
RTI::RTIambassador::localDeleteObjectInstance(RTI::ObjectHandle objectHandle)
  throw (RTI::ObjectNotKnown,
         RTI::FederateOwnsAttributes,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->localDeleteObjectInstance(objectHandle);
}

void
RTI::RTIambassador::changeAttributeTransportationType(RTI::ObjectHandle objectHandle,
                                                      const RTI::AttributeHandleSet& attributeHandleSet,
                                                      RTI::TransportationHandle transportationHandle)
  throw (RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::AttributeNotOwned,
         RTI::InvalidTransportationHandle,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  OpenRTI::TransportationType transportationType = toOpenRTITransportationType(transportationHandle);
  OpenRTI::AttributeHandleSet OpenRTIAttributeHandleSet;
  ULong attributeHandleSetSize = attributeHandleSet.size();
  for (ULong i = 0; i < attributeHandleSetSize; ++i)
    OpenRTIAttributeHandleSet.insert(attributeHandleSet.getHandle(i));

  privateRefs->changeAttributeTransportationType(objectHandle, OpenRTIAttributeHandleSet, transportationType);
}

void
RTI::RTIambassador::changeInteractionTransportationType(RTI::InteractionClassHandle interactionClassHandle,
                                                        RTI::TransportationHandle transportationHandle)
  throw (RTI::InteractionClassNotDefined,
         RTI::InteractionClassNotPublished,
         RTI::InvalidTransportationHandle,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  OpenRTI::InteractionClassHandle OpenRTIInteractionClassHandle = interactionClassHandle;
  OpenRTI::TransportationType transportationType = toOpenRTITransportationType(transportationHandle);
  privateRefs->changeInteractionTransportationType(OpenRTIInteractionClassHandle, transportationType);
}

void
RTI::RTIambassador::requestObjectAttributeValueUpdate(RTI::ObjectHandle objectHandle,
                                                      const RTI::AttributeHandleSet& attributeHandleSet)
  throw (RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  OpenRTI::AttributeHandleSet OpenRTIAttributeHandleSet;
  ULong attributeHandleSetSize = attributeHandleSet.size();
  for (ULong i = 0; i < attributeHandleSetSize; ++i)
    OpenRTIAttributeHandleSet.insert(attributeHandleSet.getHandle(i));

  privateRefs->requestAttributeValueUpdate(OpenRTI::ObjectInstanceHandle(objectHandle), OpenRTIAttributeHandleSet, OpenRTI::VariableLengthData());
}

void
RTI::RTIambassador::requestClassAttributeValueUpdate(RTI::ObjectClassHandle objectClassHandle,
                                                     const RTI::AttributeHandleSet& attributeHandleSet)
  throw (RTI::ObjectClassNotDefined,
         RTI::AttributeNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  OpenRTI::AttributeHandleSet OpenRTIAttributeHandleSet;
  ULong attributeHandleSetSize = attributeHandleSet.size();
  for (ULong i = 0; i < attributeHandleSetSize; ++i)
    OpenRTIAttributeHandleSet.insert(attributeHandleSet.getHandle(i));

  privateRefs->requestAttributeValueUpdate(OpenRTI::ObjectClassHandle(objectClassHandle), OpenRTIAttributeHandleSet, OpenRTI::VariableLengthData());
}

// Ownership Management -------------------

void
RTI::RTIambassador::unconditionalAttributeOwnershipDivestiture(RTI::ObjectHandle objectHandle,
                                                               const RTI::AttributeHandleSet& attributeHandleSet)
  throw (RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::AttributeNotOwned,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  OpenRTI::AttributeHandleSet OpenRTIAttributeHandleSet;
  ULong attributeHandleSetSize = attributeHandleSet.size();
  for (ULong i = 0; i < attributeHandleSetSize; ++i)
    OpenRTIAttributeHandleSet.insert(attributeHandleSet.getHandle(i));

  privateRefs->unconditionalAttributeOwnershipDivestiture(objectHandle, OpenRTIAttributeHandleSet);
}

void
RTI::RTIambassador::negotiatedAttributeOwnershipDivestiture(RTI::ObjectHandle objectHandle,
                                                            const RTI::AttributeHandleSet& attributeHandleSet,
                                                            const char* tag)
  throw (RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::AttributeNotOwned,
         RTI::AttributeAlreadyBeingDivested,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  OpenRTI::AttributeHandleSet OpenRTIAttributeHandleSet;
  ULong attributeHandleSetSize = attributeHandleSet.size();
  for (ULong i = 0; i < attributeHandleSetSize; ++i)
    OpenRTIAttributeHandleSet.insert(attributeHandleSet.getHandle(i));

  privateRefs->negotiatedAttributeOwnershipDivestiture(objectHandle, OpenRTIAttributeHandleSet, toOpenRTITag(tag));
}

void
RTI::RTIambassador::attributeOwnershipAcquisition(RTI::ObjectHandle objectHandle,
                                                  const RTI::AttributeHandleSet& attributeHandleSet,
                                                  const char* tag)
  throw (RTI::ObjectNotKnown,
         RTI::ObjectClassNotPublished,
         RTI::AttributeNotDefined,
         RTI::AttributeNotPublished,
         RTI::FederateOwnsAttributes,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  OpenRTI::AttributeHandleSet OpenRTIAttributeHandleSet;
  ULong attributeHandleSetSize = attributeHandleSet.size();
  for (ULong i = 0; i < attributeHandleSetSize; ++i)
    OpenRTIAttributeHandleSet.insert(attributeHandleSet.getHandle(i));

  privateRefs->attributeOwnershipAcquisition(objectHandle, OpenRTIAttributeHandleSet, toOpenRTITag(tag));
}

void
RTI::RTIambassador::attributeOwnershipAcquisitionIfAvailable(RTI::ObjectHandle objectHandle,
                                                             const AttributeHandleSet& attributeHandleSet)
  throw (RTI::ObjectNotKnown,
         RTI::ObjectClassNotPublished,
         RTI::AttributeNotDefined,
         RTI::AttributeNotPublished,
         RTI::FederateOwnsAttributes,
         RTI::AttributeAlreadyBeingAcquired,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  OpenRTI::AttributeHandleSet OpenRTIAttributeHandleSet;
  ULong attributeHandleSetSize = attributeHandleSet.size();
  for (ULong i = 0; i < attributeHandleSetSize; ++i)
    OpenRTIAttributeHandleSet.insert(attributeHandleSet.getHandle(i));

  privateRefs->attributeOwnershipAcquisitionIfAvailable(objectHandle, OpenRTIAttributeHandleSet);
}

RTI::AttributeHandleSet*
RTI::RTIambassador::attributeOwnershipReleaseResponse(RTI::ObjectHandle objectHandle,
                                                      const RTI::AttributeHandleSet& attributeHandleSet)
  throw (RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::AttributeNotOwned,
         RTI::FederateWasNotAskedToReleaseAttribute,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  OpenRTI::AttributeHandleSet divestedAttributes;

  OpenRTI::AttributeHandleSet OpenRTIAttributeHandleSet;
  ULong attributeHandleSetSize = attributeHandleSet.size();
  for (ULong i = 0; i < attributeHandleSetSize; ++i)
    OpenRTIAttributeHandleSet.insert(attributeHandleSet.getHandle(i));

  privateRefs->attributeOwnershipDivestitureIfWanted(objectHandle, OpenRTIAttributeHandleSet, divestedAttributes);

  // FIXME, return getReason we got ...
  throw RTI::RTIinternalError("Not iplemented????");
  return 0;
}

void
RTI::RTIambassador::cancelNegotiatedAttributeOwnershipDivestiture(RTI::ObjectHandle objectHandle,
                                                                  const RTI::AttributeHandleSet& attributeHandleSet)
  throw (RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::AttributeNotOwned,
         RTI::AttributeDivestitureWasNotRequested,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  OpenRTI::AttributeHandleSet OpenRTIAttributeHandleSet;
  ULong attributeHandleSetSize = attributeHandleSet.size();
  for (ULong i = 0; i < attributeHandleSetSize; ++i)
    OpenRTIAttributeHandleSet.insert(attributeHandleSet.getHandle(i));

  privateRefs->cancelNegotiatedAttributeOwnershipDivestiture(objectHandle, OpenRTIAttributeHandleSet);
}

void
RTI::RTIambassador::cancelAttributeOwnershipAcquisition(RTI::ObjectHandle objectHandle,
                                                        const RTI::AttributeHandleSet& attributeHandleSet)
  throw (RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::AttributeAlreadyOwned,
         RTI::AttributeAcquisitionWasNotRequested,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  OpenRTI::AttributeHandleSet OpenRTIAttributeHandleSet;
  ULong attributeHandleSetSize = attributeHandleSet.size();
  for (ULong i = 0; i < attributeHandleSetSize; ++i)
    OpenRTIAttributeHandleSet.insert(attributeHandleSet.getHandle(i));

  privateRefs->cancelAttributeOwnershipAcquisition(objectHandle, OpenRTIAttributeHandleSet);
}

void
RTI::RTIambassador::queryAttributeOwnership(RTI::ObjectHandle objectHandle,
                                            RTI::AttributeHandle attributeHandle)
  throw (RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->queryAttributeOwnership(objectHandle, attributeHandle);
}

RTI::Boolean
RTI::RTIambassador::isAttributeOwnedByFederate(RTI::ObjectHandle objectHandle,
                                               RTI::AttributeHandle attributeHandle)
  throw (RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  bool owned = privateRefs->isAttributeOwnedByFederate(objectHandle, attributeHandle);
  return owned ? RTI::RTI_TRUE : RTI::RTI_FALSE;
}

// Time Management -------------------

void
RTI::RTIambassador::enableTimeRegulation(const RTI::FedTime& time, const RTI::FedTime& lookahead)
  throw (RTI::TimeRegulationAlreadyEnabled,
         RTI::EnableTimeRegulationPending,
         RTI::TimeAdvanceAlreadyInProgress,
         RTI::InvalidFederationTime,
         RTI::InvalidLookahead,
         RTI::ConcurrentAccessAttempted,
         RTI::FederateNotExecutionMember,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->enableTimeRegulation(time, lookahead);
}

void
RTI::RTIambassador::disableTimeRegulation()
  throw (RTI::TimeRegulationWasNotEnabled,
         RTI::ConcurrentAccessAttempted,
         RTI::FederateNotExecutionMember,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->disableTimeRegulation();
}

void
RTI::RTIambassador::enableTimeConstrained()
  throw (RTI::TimeConstrainedAlreadyEnabled,
         RTI::EnableTimeConstrainedPending,
         RTI::TimeAdvanceAlreadyInProgress,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->enableTimeConstrained();
}

void
RTI::RTIambassador::disableTimeConstrained()
  throw (RTI::TimeConstrainedWasNotEnabled,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->disableTimeConstrained();
}

void
RTI::RTIambassador::timeAdvanceRequest(const RTI::FedTime& fedTime)
  throw (RTI::InvalidFederationTime,
         RTI::FederationTimeAlreadyPassed,
         RTI::TimeAdvanceAlreadyInProgress,
         RTI::EnableTimeRegulationPending,
         RTI::EnableTimeConstrainedPending,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->timeAdvanceRequest(fedTime);
}

void
RTI::RTIambassador::timeAdvanceRequestAvailable(const RTI::FedTime& fedTime)
  throw (RTI::InvalidFederationTime,
         RTI::FederationTimeAlreadyPassed,
         RTI::TimeAdvanceAlreadyInProgress,
         RTI::EnableTimeRegulationPending,
         RTI::EnableTimeConstrainedPending,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->timeAdvanceRequestAvailable(fedTime);
}

void
RTI::RTIambassador::nextEventRequest(const RTI::FedTime& fedTime)
  throw (RTI::InvalidFederationTime,
         RTI::FederationTimeAlreadyPassed,
         RTI::TimeAdvanceAlreadyInProgress,
         RTI::EnableTimeRegulationPending,
         RTI::EnableTimeConstrainedPending,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->nextMessageRequest(fedTime);
}

void
RTI::RTIambassador::nextEventRequestAvailable(const RTI::FedTime& fedTime)
  throw (RTI::InvalidFederationTime,
         RTI::FederationTimeAlreadyPassed,
         RTI::TimeAdvanceAlreadyInProgress,
         RTI::EnableTimeRegulationPending,
         RTI::EnableTimeConstrainedPending,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->nextMessageRequestAvailable(fedTime);
}

void
RTI::RTIambassador::flushQueueRequest(const RTI::FedTime& fedTime)
  throw (RTI::InvalidFederationTime,
         RTI::FederationTimeAlreadyPassed,
         RTI::TimeAdvanceAlreadyInProgress,
         RTI::EnableTimeRegulationPending,
         RTI::EnableTimeConstrainedPending,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->flushQueueRequest(fedTime);
}

void
RTI::RTIambassador::enableAsynchronousDelivery()
  throw (RTI::AsynchronousDeliveryAlreadyEnabled,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->enableAsynchronousDelivery();
}

void
RTI::RTIambassador::disableAsynchronousDelivery()
  throw (RTI::AsynchronousDeliveryAlreadyDisabled,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->disableAsynchronousDelivery();
}

void
RTI::RTIambassador::queryLBTS(RTI::FedTime& fedTime)
  throw (RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  if (privateRefs->queryGALT(fedTime))
    return;
  fedTime.setPositiveInfinity();
}

void
RTI::RTIambassador::queryFederateTime(RTI::FedTime& fedTime)
  throw (RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->queryLogicalTime(fedTime);
}

void
RTI::RTIambassador::queryMinNextEventTime(RTI::FedTime& fedTime)
  throw (RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  if (privateRefs->queryLITS(fedTime))
    return;
  fedTime.setPositiveInfinity();
}

void
RTI::RTIambassador::modifyLookahead(const RTI::FedTime& fedTime)
  throw (RTI::InvalidLookahead,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->modifyLookahead(fedTime);
}

void
RTI::RTIambassador::queryLookahead(RTI::FedTime& fedTime)
  throw (RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->queryLookahead(fedTime);
}

void
RTI::RTIambassador::retract(RTI::EventRetractionHandle theHandle)
  throw (RTI::InvalidRetractionHandle,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->retract(toOpenRTIMessageRetractionHandle(theHandle));
}

void
RTI::RTIambassador::changeAttributeOrderType(RTI::ObjectHandle objectHandle,
                                             const RTI::AttributeHandleSet& attributeHandleSet,
                                             RTI::OrderingHandle orderingHandle)
  throw (RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::AttributeNotOwned,
         RTI::InvalidOrderingHandle,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  OpenRTI::OrderType orderType = toOpenRTIOrderType(orderingHandle);
  OpenRTI::AttributeHandleSet OpenRTIAttributeHandleSet;
  ULong attributeHandleSetSize = attributeHandleSet.size();
  for (ULong i = 0; i < attributeHandleSetSize; ++i)
    OpenRTIAttributeHandleSet.insert(attributeHandleSet.getHandle(i));

  privateRefs->changeAttributeOrderType(objectHandle, OpenRTIAttributeHandleSet, orderType);
}

void
RTI::RTIambassador::changeInteractionOrderType(RTI::InteractionClassHandle interactionClassHandle,
                                               RTI::OrderingHandle orderingHandle)
  throw (RTI::InteractionClassNotDefined,
         RTI::InteractionClassNotPublished,
         RTI::InvalidOrderingHandle,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  OpenRTI::OrderType orderType = toOpenRTIOrderType(orderingHandle);
  privateRefs->changeInteractionOrderType(interactionClassHandle, orderType);
}

// Data Distribution Management -------------------

RTI::Region*
RTI::RTIambassador::createRegion(RTI::SpaceHandle spaceHandle,
                                 RTI::ULong)
  throw (RTI::SpaceNotDefined,
         RTI::InvalidExtents,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

void
RTI::RTIambassador::notifyAboutRegionModification(RTI::Region& theRegion)
  throw (RTI::RegionNotKnown,
         RTI::InvalidExtents,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

void
RTI::RTIambassador::deleteRegion(RTI::Region* region)
  throw (RTI::RegionNotKnown,
         RTI::RegionInUse,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

RTI::ObjectHandle
RTI::RTIambassador::registerObjectInstanceWithRegion(RTI::ObjectClassHandle objectClassHandle,
                                                     const char* tag,
                                                     RTI::AttributeHandle [],
                                                     RTI::Region* theRegions[],
                                                     RTI::ULong)
  throw (RTI::ObjectClassNotDefined,
         RTI::ObjectClassNotPublished,
         RTI::AttributeNotDefined,
         RTI::AttributeNotPublished,
         RTI::RegionNotKnown,
         RTI::InvalidRegionContext,
         RTI::ObjectAlreadyRegistered,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

RTI::ObjectHandle
RTI::RTIambassador::registerObjectInstanceWithRegion(RTI::ObjectClassHandle objectClassHandle,
                                                     RTI::AttributeHandle [],
                                                     RTI::Region* regions[],
                                                     RTI::ULong)
  throw (RTI::ObjectClassNotDefined,
         RTI::ObjectClassNotPublished,
         RTI::AttributeNotDefined,
         RTI::AttributeNotPublished,
         RTI::RegionNotKnown,
         RTI::InvalidRegionContext,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

void
RTI::RTIambassador::associateRegionForUpdates(RTI::Region& ,
                                              RTI::ObjectHandle,
                                              const RTI::AttributeHandleSet& attributeHandleSet)
  throw (RTI::ObjectNotKnown,
         RTI::AttributeNotDefined,
         RTI::InvalidRegionContext,
         RTI::RegionNotKnown,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

void
RTI::RTIambassador::unassociateRegionForUpdates(RTI::Region&, RTI::ObjectHandle)
  throw (RTI::ObjectNotKnown,
         RTI::InvalidRegionContext,
         RTI::RegionNotKnown,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

void
RTI::RTIambassador::subscribeObjectClassAttributesWithRegion(RTI::ObjectClassHandle objectClassHandle,
                                                             RTI::Region&,
                                                             const RTI::AttributeHandleSet&,
                                                             RTI::Boolean active)
  throw (RTI::ObjectClassNotDefined,
         RTI::AttributeNotDefined,
         RTI::RegionNotKnown,
         RTI::InvalidRegionContext,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

void
RTI::RTIambassador::unsubscribeObjectClassWithRegion(RTI::ObjectClassHandle objectClassHandle,
                                                     RTI::Region&)
  throw (RTI::ObjectClassNotDefined,
         RTI::RegionNotKnown,
         RTI::ObjectClassNotSubscribed,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

void
RTI::RTIambassador::subscribeInteractionClassWithRegion(RTI::InteractionClassHandle interactionClassHandle,
                                                        RTI::Region&,
                                                        RTI::Boolean active)
  throw (RTI::InteractionClassNotDefined,
         RTI::RegionNotKnown,
         RTI::InvalidRegionContext,
         RTI::FederateLoggingServiceCalls,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

void
RTI::RTIambassador::unsubscribeInteractionClassWithRegion(RTI::InteractionClassHandle interactionClassHandle,
                                                          RTI::Region&)
  throw (RTI::InteractionClassNotDefined,
         RTI::InteractionClassNotSubscribed,
         RTI::RegionNotKnown,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

RTI::EventRetractionHandle
RTI::RTIambassador::sendInteractionWithRegion(RTI::InteractionClassHandle interactionClassHandle,
                                              const RTI::ParameterHandleValuePairSet&,
                                              const RTI::FedTime&,
                                              const char* tag,
                                              const RTI::Region&)
  throw (RTI::InteractionClassNotDefined,
         RTI::InteractionClassNotPublished,
         RTI::InteractionParameterNotDefined,
         RTI::InvalidFederationTime,
         RTI::RegionNotKnown,
         RTI::InvalidRegionContext,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

void
RTI::RTIambassador::sendInteractionWithRegion(RTI::InteractionClassHandle interactionClassHandle,
                                              const RTI::ParameterHandleValuePairSet& ,
                                              const char* tag,
                                              const RTI::Region& )
  throw (RTI::InteractionClassNotDefined,
         RTI::InteractionClassNotPublished,
         RTI::InteractionParameterNotDefined,
         RTI::RegionNotKnown,
         RTI::InvalidRegionContext,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

void
RTI::RTIambassador::requestClassAttributeValueUpdateWithRegion(RTI::ObjectClassHandle objectClassHandle,
                                                               const RTI::AttributeHandleSet&,
                                                               const RTI::Region&)
  throw (RTI::ObjectClassNotDefined,
         RTI::AttributeNotDefined,
         RTI::RegionNotKnown,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw OpenRTI::RTIinternalError("ENOIMP");
}

// Support Services -------------------

RTI::ObjectClassHandle
RTI::RTIambassador::getObjectClassHandle(const char* name)
  throw (RTI::NameNotFound,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  if (!name)
    throw RTI::NameNotFound("Zero name pointer.");
  OpenRTI::ObjectClassHandle objectClassHandle = privateRefs->getObjectClassHandle(OpenRTI::localeToUtf8(name));
  return objectClassHandle;
}

char*
RTI::RTIambassador::getObjectClassName(RTI::ObjectClassHandle objectClassHandle)
  throw (RTI::ObjectClassNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  return newUtf8ToLocale(privateRefs->getObjectClassName(objectClassHandle));
}

RTI::AttributeHandle
RTI::RTIambassador::getAttributeHandle(const char* name,
                                       RTI::ObjectClassHandle objectClassHandle)
  throw (RTI::ObjectClassNotDefined,
         RTI::NameNotFound,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  if (!name)
    throw RTI::NameNotFound("Zero name pointer.");
  return privateRefs->getAttributeHandle(objectClassHandle, OpenRTI::localeToUtf8(name));
}

char*
RTI::RTIambassador::getAttributeName(RTI::AttributeHandle attributeHandle,
                                     RTI::ObjectClassHandle objectClassHandle)
  throw (RTI::ObjectClassNotDefined,
         RTI::AttributeNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  return newUtf8ToLocale(privateRefs->getAttributeName(objectClassHandle, attributeHandle));
}

RTI::InteractionClassHandle
RTI::RTIambassador::getInteractionClassHandle(const char* name)
  throw (RTI::NameNotFound,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  if (!name)
    throw RTI::NameNotFound("Zero name pointer.");
  return privateRefs->getInteractionClassHandle(OpenRTI::localeToUtf8(name));
}

char*
RTI::RTIambassador::getInteractionClassName(RTI::InteractionClassHandle interactionClassHandle)
  throw (RTI::InteractionClassNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  return newUtf8ToLocale(privateRefs->getInteractionClassName(interactionClassHandle));
}

RTI::ParameterHandle
RTI::RTIambassador::getParameterHandle(const char* name,
                                       RTI::InteractionClassHandle interactionClassHandle)
  throw (RTI::InteractionClassNotDefined,
         RTI::NameNotFound,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  if (!name)
    throw RTI::NameNotFound("Zero name pointer.");
  return privateRefs->getParameterHandle(interactionClassHandle, OpenRTI::localeToUtf8(name));
}

char*
RTI::RTIambassador::getParameterName(RTI::ParameterHandle parameterHandle,
                                     RTI::InteractionClassHandle interactionClassHandle)
  throw (RTI::InteractionClassNotDefined,
         RTI::InteractionParameterNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  return newUtf8ToLocale(privateRefs->getParameterName(interactionClassHandle, parameterHandle));
}

RTI::ObjectHandle
RTI::RTIambassador::getObjectInstanceHandle(const char* name)
  throw (RTI::ObjectNotKnown,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  if (!name)
    throw RTI::NameNotFound("Zero name pointer.");
  return privateRefs->getObjectInstanceHandle(OpenRTI::localeToUtf8(name));
}

char*
RTI::RTIambassador::getObjectInstanceName(RTI::ObjectHandle objectHandle)
  throw (RTI::ObjectNotKnown,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  return newUtf8ToLocale(privateRefs->getObjectInstanceName(objectHandle));
}

RTI::SpaceHandle
RTI::RTIambassador::getRoutingSpaceHandle(const char* name)
  throw (RTI::NameNotFound,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  if (!name)
    throw RTI::NameNotFound("Zero name pointer.");
  throw RTI::RTIinternalError("Unknown exception");
}

char*
RTI::RTIambassador::getRoutingSpaceName(RTI::SpaceHandle spaceHandle)
  throw (RTI::SpaceNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw RTI::RTIinternalError("Unknown exception");
}

RTI::DimensionHandle
RTI::RTIambassador::getDimensionHandle(const char* name,
                                       RTI::SpaceHandle spaceHandle)
  throw (RTI::SpaceNotDefined,
         RTI::NameNotFound,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  if (!name)
    throw RTI::NameNotFound("Zero name pointer.");
  throw RTI::RTIinternalError("Unknown exception");
}

char*
RTI::RTIambassador::getDimensionName(RTI::DimensionHandle dimensionHandle,
                                     RTI::SpaceHandle spaceHandle)
  throw (RTI::SpaceNotDefined,
         RTI::DimensionNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw RTI::RTIinternalError("Unknown exception");
}

RTI::SpaceHandle
RTI::RTIambassador::getAttributeRoutingSpaceHandle(RTI::AttributeHandle attributeHandle,
                                                   RTI::ObjectClassHandle objectClassHandle)
  throw (RTI::ObjectClassNotDefined,
         RTI::AttributeNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw RTI::RTIinternalError("Unknown exception");
}

RTI::ObjectClassHandle
RTI::RTIambassador::getObjectClass(RTI::ObjectHandle objectHandle)
  throw (RTI::ObjectNotKnown,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  return privateRefs->getKnownObjectClassHandle(objectHandle);
}

RTI::SpaceHandle
RTI::RTIambassador::getInteractionRoutingSpaceHandle(InteractionClassHandle interactionClassHandle)
  throw (RTI::InteractionClassNotDefined,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw RTI::RTIinternalError("Unknown exception");
}

RTI::TransportationHandle
RTI::RTIambassador::getTransportationHandle(const char* name)
  throw (RTI::NameNotFound,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  if (!name)
    throw RTI::NameNotFound("Zero name pointer.");
  throw RTI::RTIinternalError("Unknown exception");
}

char*
RTI::RTIambassador::getTransportationName(RTI::TransportationHandle transportationHandle)
  throw (RTI::InvalidTransportationHandle,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw RTI::RTIinternalError("Unknown exception");
}

RTI::OrderingHandle
RTI::RTIambassador::getOrderingHandle(const char* name)
  throw (RTI::NameNotFound,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  if (!name)
    throw RTI::NameNotFound("Zero name pointer.");
  throw RTI::RTIinternalError("Unknown exception");
}

char*
RTI::RTIambassador::getOrderingName(RTI::OrderingHandle orderingHandle)
  throw (RTI::InvalidOrderingHandle,
         RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw RTI::RTIinternalError("Unknown exception");
}

void
RTI::RTIambassador::enableClassRelevanceAdvisorySwitch()
  throw (RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->enableObjectClassRelevanceAdvisorySwitch();
}

void
RTI::RTIambassador::disableClassRelevanceAdvisorySwitch()
  throw (RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->disableObjectClassRelevanceAdvisorySwitch();
}

void
RTI::RTIambassador::enableAttributeRelevanceAdvisorySwitch()
  throw (RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->enableAttributeRelevanceAdvisorySwitch();
}

void
RTI::RTIambassador::disableAttributeRelevanceAdvisorySwitch()
  throw (RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->disableAttributeRelevanceAdvisorySwitch();
}

void
RTI::RTIambassador::enableAttributeScopeAdvisorySwitch()
  throw (RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->enableAttributeScopeAdvisorySwitch();
}

void
RTI::RTIambassador::disableAttributeScopeAdvisorySwitch()
  throw (RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->enableAttributeScopeAdvisorySwitch();
}

void
RTI::RTIambassador::enableInteractionRelevanceAdvisorySwitch()
  throw (RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->enableInteractionRelevanceAdvisorySwitch();
}

void
RTI::RTIambassador::disableInteractionRelevanceAdvisorySwitch()
  throw (RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::SaveInProgress,
         RTI::RestoreInProgress,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  privateRefs->disableInteractionRelevanceAdvisorySwitch();
}

RTI::Boolean
RTI::RTIambassador::tick()
  throw (RTI::SpecifiedSaveLabelDoesNotExist,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  bool pendingCallback = privateRefs->evokeMultipleCallbacks(0, std::numeric_limits<double>::infinity());
  return pendingCallback ? RTI::RTI_TRUE : RTI::RTI_FALSE;
}

RTI::Boolean
RTI::RTIambassador::tick(RTI::TickTime minTime,
                         RTI::TickTime maxTime)
  throw (RTI::SpecifiedSaveLabelDoesNotExist,
         RTI::ConcurrentAccessAttempted,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  bool pendingCallback = privateRefs->evokeMultipleCallbacks(minTime, maxTime);
  return pendingCallback ? RTI::RTI_TRUE : RTI::RTI_FALSE;
}

RTI::RegionToken
RTI::RTIambassador::getRegionToken(RTI::Region*)
  throw (RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RegionNotKnown,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw RTI::RTIinternalError("Unknown exception");
}

RTI::Region*
RTI::RTIambassador::getRegion(RTI::RegionToken)
  throw (RTI::FederateNotExecutionMember,
         RTI::ConcurrentAccessAttempted,
         RTI::RegionNotKnown,
         RTI::RTIinternalError)
{
  RTIambPrivateRefs::ConcurrentAccessGuard concurrentAccessGuard(*privateRefs);
  throw RTI::RTIinternalError("Unknown exception");
}
