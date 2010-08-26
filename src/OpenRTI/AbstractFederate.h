/* -*-c++-*- OpenRTI - Copyright (C) 2009-2010 Mathias Froehlich
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

#ifndef OpenRTI_AbstractFederate_h
#define OpenRTI_AbstractFederate_h

#include "Message.h"
#include "RangeBounds.h"
#include "Referenced.h"

namespace OpenRTI {

template<typename T>
class OPENRTI_LOCAL AbstractFederate : public Referenced {
public:
  typedef T Traits;

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

  virtual ~AbstractFederate()
  { }

  virtual const FederateHandle& getFederateHandle() const = 0;
  virtual std::wstring getLogicalTimeFactoryName() const = 0;

  // that might move to something internal?? May be some join call doing that reservation and returning that handle?? FIXME
  virtual void requestObjectInstanceHandles(unsigned count) = 0;
  // virtual void joinFederationExecution() = 0

  virtual void dispatchInternalMessage(const AbstractMessage& message) = 0;

  virtual void resignFederationExecution(ResignAction resignAction, const Clock& clock)
    throw (OwnershipAcquisitionPending,
           FederateOwnsAttributes,
           RTIinternalError) = 0;

  virtual void registerFederationSynchronizationPoint(const std::wstring& label, const VariableLengthData& tag, const FederateHandleSet& syncSet)
    throw (SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void synchronizationPointAchieved(const std::wstring& label)
    throw (SynchronizationPointLabelNotAnnounced,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void requestFederationSave(const std::wstring& label)
    throw (SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void requestFederationSave(const std::wstring& label, const NativeLogicalTime& locicalTime)
    throw (LogicalTimeAlreadyPassed,
           InvalidLogicalTime,
           FederateUnableToUseTime,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void federateSaveBegun()
    throw (SaveNotInitiated,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void federateSaveComplete()
    throw (FederateHasNotBegunSave,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void federateSaveNotComplete()
    throw (FederateHasNotBegunSave,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void queryFederationSaveStatus()
    throw (RestoreInProgress,
           RTIinternalError) = 0;

  virtual void requestFederationRestore(const std::wstring& label)
    throw (SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void federateRestoreComplete()
    throw (RestoreNotRequested,
           SaveInProgress,
           RTIinternalError) = 0;

  virtual void federateRestoreNotComplete()
    throw (RestoreNotRequested,
           SaveInProgress,
           RTIinternalError) = 0;

  virtual void queryFederationRestoreStatus()
    throw (SaveInProgress,
           RTIinternalError) = 0;

  virtual void publishObjectClassAttributes(ObjectClassHandle objectClassHandle, const AttributeHandleSet& attributeList)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void unpublishObjectClass(ObjectClassHandle objectClassHandle)
    throw (ObjectClassNotDefined,
           OwnershipAcquisitionPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void unpublishObjectClassAttributes(ObjectClassHandle objectClassHandle, const AttributeHandleSet& attributeList)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           OwnershipAcquisitionPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void publishInteractionClass(InteractionClassHandle interactionClassHandle)
    throw (InteractionClassNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void unpublishInteractionClass(InteractionClassHandle interactionClassHandle)
    throw (InteractionClassNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void subscribeObjectClassAttributes(ObjectClassHandle objectClassHandle, const AttributeHandleSet& attributeList, bool active)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void unsubscribeObjectClass(ObjectClassHandle objectClassHandle)
    throw (ObjectClassNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void unsubscribeObjectClassAttributes(ObjectClassHandle objectClassHandle, const AttributeHandleSet& attributeList)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void subscribeInteractionClass(InteractionClassHandle interactionClassHandle, bool active)
    throw (InteractionClassNotDefined,
           FederateServiceInvocationsAreBeingReportedViaMOM,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void unsubscribeInteractionClass(InteractionClassHandle interactionClassHandle)
    throw (InteractionClassNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void reserveObjectInstanceName(const std::wstring& objectInstanceName)
    throw (IllegalName,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void releaseObjectInstanceName(const std::wstring& objectInstanceName)
    throw (ObjectInstanceNameNotReserved,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void reserveMultipleObjectInstanceName(const std::set<std::wstring>& objectInstanceNameSet)
    throw (IllegalName,
           NameSetWasEmpty,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void releaseMultipleObjectInstanceName(const std::set<std::wstring>& objectInstanceNameSet)
    throw (ObjectInstanceNameNotReserved,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual ObjectInstanceHandle registerObjectInstance(ObjectClassHandle objectClassHandle)
    throw (ObjectClassNotDefined,
           ObjectClassNotPublished,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual ObjectInstanceHandle registerObjectInstance(ObjectClassHandle objectClassHandle, const std::wstring& objectInstanceName,
                                                      bool allowUnreservedObjectNames)
    throw (ObjectClassNotDefined,
           ObjectClassNotPublished,
           ObjectInstanceNameNotReserved,
           ObjectInstanceNameInUse,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void
  updateAttributeValues(ObjectInstanceHandle objectClassHandle, std::vector<OpenRTI::AttributeValue>& attributeValues,
                        const VariableLengthData& tag)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual MessageRetractionHandle
  updateAttributeValues(ObjectInstanceHandle objectClassHandle, std::vector<OpenRTI::AttributeValue>& attributeValues,
                        const VariableLengthData& tag, const NativeLogicalTime& logicalTime)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           InvalidLogicalTime,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void sendInteraction(InteractionClassHandle interactionClassHandle, std::vector<ParameterValue>& parameterValues,
                       const VariableLengthData& tag)
    throw (InteractionClassNotPublished,
           InteractionClassNotDefined,
           InteractionParameterNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual MessageRetractionHandle sendInteraction(InteractionClassHandle interactionClassHandle,
                                          std::vector<ParameterValue>& parameterValues,
                                          const VariableLengthData& tag,
                                          const NativeLogicalTime& logicalTime)
    throw (InteractionClassNotPublished,
           InteractionClassNotDefined,
           InteractionParameterNotDefined,
           InvalidLogicalTime,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void deleteObjectInstance(ObjectInstanceHandle objectInstanceHandle, const VariableLengthData& tag)
    throw (DeletePrivilegeNotHeld,
           ObjectInstanceNotKnown,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual MessageRetractionHandle deleteObjectInstance(ObjectInstanceHandle objectInstanceHandle, const VariableLengthData& tag,
                                                       const NativeLogicalTime& logicalTime)
    throw (DeletePrivilegeNotHeld,
           ObjectInstanceNotKnown,
           InvalidLogicalTime,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void localDeleteObjectInstance(ObjectInstanceHandle objectInstanceHandle)
    throw (ObjectInstanceNotKnown,
           FederateOwnsAttributes,
           OwnershipAcquisitionPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void changeAttributeTransportationType(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet, TransportationType transportationType)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void changeInteractionTransportationType(InteractionClassHandle interactionClassHandle, TransportationType transportationType)
    throw (InteractionClassNotDefined,
           InteractionClassNotPublished,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void requestAttributeValueUpdate(ObjectInstanceHandle objectInstanceHandle,
                                   const AttributeHandleSet& attributeHandleSet,
                                   const VariableLengthData& tag)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void requestAttributeValueUpdate(ObjectClassHandle objectClassHandle,
                                   const AttributeHandleSet& attributeHandleSet,
                                   const VariableLengthData& tag)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void unconditionalAttributeOwnershipDivestiture(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void negotiatedAttributeOwnershipDivestiture(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet, const VariableLengthData& tag)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           AttributeAlreadyBeingDivested,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void confirmDivestiture(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet, const VariableLengthData& tag)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           AttributeDivestitureWasNotRequested,
           NoAcquisitionPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void attributeOwnershipAcquisition(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet, const VariableLengthData& tag)
    throw (ObjectInstanceNotKnown,
           ObjectClassNotPublished,
           AttributeNotDefined,
           AttributeNotPublished,
           FederateOwnsAttributes,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void attributeOwnershipAcquisitionIfAvailable(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet)
    throw (ObjectInstanceNotKnown,
           ObjectClassNotPublished,
           AttributeNotDefined,
           AttributeNotPublished,
           FederateOwnsAttributes,
           AttributeAlreadyBeingAcquired,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void attributeOwnershipDivestitureIfWanted(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet,
                                             AttributeHandleSet& divestedAttributes)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void cancelNegotiatedAttributeOwnershipDivestiture(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           AttributeDivestitureWasNotRequested,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void cancelAttributeOwnershipAcquisition(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeAlreadyOwned,
           AttributeAcquisitionWasNotRequested,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void queryAttributeOwnership(ObjectInstanceHandle objectInstanceHandle, AttributeHandle attributeHandle)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual bool isAttributeOwnedByFederate(ObjectInstanceHandle objectInstanceHandle, AttributeHandle attributeHandle) const
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void enableTimeRegulation(const NativeLogicalTimeInterval& lookahead)
    throw (TimeRegulationAlreadyEnabled,
           InvalidLookahead,
           InTimeAdvancingState,
           RequestForTimeRegulationPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void enableTimeRegulation(const NativeLogicalTime& logicalTime, const NativeLogicalTimeInterval& lookahead)
    throw (TimeRegulationAlreadyEnabled,
           InvalidLogicalTime,
           InvalidLookahead,
           InTimeAdvancingState,
           RequestForTimeRegulationPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void disableTimeRegulation()
    throw (TimeRegulationIsNotEnabled,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void enableTimeConstrained()
    throw (TimeConstrainedAlreadyEnabled,
           InTimeAdvancingState,
           RequestForTimeConstrainedPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void disableTimeConstrained()
    throw (TimeConstrainedIsNotEnabled,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void timeAdvanceRequest(const NativeLogicalTime& logicalTime)
    throw (InvalidLogicalTime,
           LogicalTimeAlreadyPassed,
           InTimeAdvancingState,
           RequestForTimeRegulationPending,
           RequestForTimeConstrainedPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void timeAdvanceRequestAvailable(const NativeLogicalTime& logicalTime)
    throw (InvalidLogicalTime,
           LogicalTimeAlreadyPassed,
           InTimeAdvancingState,
           RequestForTimeRegulationPending,
           RequestForTimeConstrainedPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void nextMessageRequest(const NativeLogicalTime& logicalTime)
    throw (InvalidLogicalTime,
           LogicalTimeAlreadyPassed,
           InTimeAdvancingState,
           RequestForTimeRegulationPending,
           RequestForTimeConstrainedPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void nextMessageRequestAvailable(const NativeLogicalTime& logicalTime)
    throw (InvalidLogicalTime,
           LogicalTimeAlreadyPassed,
           InTimeAdvancingState,
           RequestForTimeRegulationPending,
           RequestForTimeConstrainedPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void flushQueueRequest(const NativeLogicalTime& logicalTime)
    throw (InvalidLogicalTime,
           LogicalTimeAlreadyPassed,
           InTimeAdvancingState,
           RequestForTimeRegulationPending,
           RequestForTimeConstrainedPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void enableAsynchronousDelivery()
    throw (AsynchronousDeliveryAlreadyEnabled,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void disableAsynchronousDelivery()
    throw (AsynchronousDeliveryAlreadyDisabled,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual bool queryGALT(NativeLogicalTime& logicalTime)
    throw (SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void queryLogicalTime(NativeLogicalTime& logicalTime)
    throw (SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual bool queryLITS(NativeLogicalTime& logicalTime)
    throw (SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void modifyLookahead(const NativeLogicalTimeInterval& lookahead)
    throw (TimeRegulationIsNotEnabled,
           InvalidLookahead,
           InTimeAdvancingState,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void queryLookahead(NativeLogicalTimeInterval& logicalTimeInterval)
    throw (TimeRegulationIsNotEnabled,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void retract(MessageRetractionHandle messageRetractionHandle)
    throw (InvalidRetractionHandle,
           TimeRegulationIsNotEnabled,
           MessageCanNoLongerBeRetracted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void changeAttributeOrderType(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet, OrderType orderType)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void changeInteractionOrderType(InteractionClassHandle interactionClassHandle, OrderType orderType)
    throw (InteractionClassNotDefined,
           InteractionClassNotPublished,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual RegionHandle createRegion(const DimensionHandleSet& dimensionHandleSet)
    throw (InvalidDimensionHandle,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void commitRegionModifications(const RegionHandleSet& regionHandleSet)
    throw (InvalidRegion,
           RegionNotCreatedByThisFederate,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void deleteRegion(RegionHandle regionHandle)
    throw (InvalidRegion,
           RegionNotCreatedByThisFederate,
           RegionInUseForUpdateOrSubscription,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual ObjectInstanceHandle
  registerObjectInstanceWithRegions(ObjectClassHandle objectClassHandle,
                                    const AttributeHandleSetRegionHandleSetPairVector& attributeHandleSetRegionHandleSetPairVector)
    throw (ObjectClassNotDefined,
           ObjectClassNotPublished,
           AttributeNotDefined,
           AttributeNotPublished,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual ObjectInstanceHandle
  registerObjectInstanceWithRegions(ObjectClassHandle objectClassHandle,
                                    const AttributeHandleSetRegionHandleSetPairVector& attributeHandleSetRegionHandleSetPairVector,
                                    const std::wstring& objectInstanceName)
    throw (ObjectClassNotDefined,
           ObjectClassNotPublished,
           AttributeNotDefined,
           AttributeNotPublished,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           ObjectInstanceNameNotReserved,
           ObjectInstanceNameInUse,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void associateRegionsForUpdates(ObjectInstanceHandle objectInstanceHandle,
                                  const AttributeHandleSetRegionHandleSetPairVector& attributeHandleHandleSetRegionHandleSetPairVector)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void unassociateRegionsForUpdates(ObjectInstanceHandle objectInstanceHandle,
                                    const AttributeHandleSetRegionHandleSetPairVector& attributeHandleHandleSetRegionHandleSetPairVector)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void subscribeObjectClassAttributesWithRegions(ObjectClassHandle objectClassHandle,
                                                 const AttributeHandleSetRegionHandleSetPairVector& attributeHandleHandleSetRegionHandleSetPairVector,
                                                 bool active)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void unsubscribeObjectClassAttributesWithRegions(ObjectClassHandle objectClassHandle,
                                                   const AttributeHandleSetRegionHandleSetPairVector& attributeHandleHandleSetRegionHandleSetPairVector)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void subscribeInteractionClassWithRegions(InteractionClassHandle interactionClassHandle, const RegionHandleSet& regionHandleSet, bool active)
    throw (InteractionClassNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           FederateServiceInvocationsAreBeingReportedViaMOM,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void unsubscribeInteractionClassWithRegions(InteractionClassHandle interactionClassHandle, const RegionHandleSet& regionHandleSet)
    throw (InteractionClassNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void sendInteractionWithRegions(InteractionClassHandle interactionClassHandle,
                                  std::vector<ParameterValue>& parameterValues,
                                  const RegionHandleSet& regionHandleSet,
                                  const VariableLengthData& tag)
    throw (InteractionClassNotDefined,
           InteractionClassNotPublished,
           InteractionParameterNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual MessageRetractionHandle
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
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void requestAttributeValueUpdateWithRegions(ObjectClassHandle objectClassHandle, const AttributeHandleSetRegionHandleSetPairVector& theSet,
                                              const VariableLengthData& tag)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual ResignAction getAutomaticResignDirective()
    throw (RTIinternalError) = 0;

  virtual void setAutomaticResignDirective(ResignAction resignAction)
    throw (InvalidResignAction,
           RTIinternalError) = 0;

  virtual const ObjectClassHandle& getObjectClassHandle(const std::wstring& name) const
    throw (NameNotFound,
           RTIinternalError) = 0;

  virtual const std::wstring& getObjectClassName(const ObjectClassHandle& objectClassHandle) const
    throw (InvalidObjectClassHandle,
           RTIinternalError) = 0;

  virtual const AttributeHandle& getAttributeHandle(const ObjectClassHandle& objectClassHandle, const std::wstring& name) const
    throw (InvalidObjectClassHandle,
           NameNotFound,
           RTIinternalError) = 0;

  virtual const std::wstring& getAttributeName(const ObjectClassHandle& objectClassHandle, const AttributeHandle& attributeHandle) const
    throw (InvalidObjectClassHandle,
           InvalidAttributeHandle,
           AttributeNotDefined,
           RTIinternalError) = 0;

  virtual const InteractionClassHandle& getInteractionClassHandle(const std::wstring& name) const
    throw (NameNotFound,
           RTIinternalError) = 0;

  virtual const std::wstring& getInteractionClassName(const InteractionClassHandle& interactionClassHandle) const
    throw (InvalidInteractionClassHandle,
           RTIinternalError) = 0;

  virtual const ParameterHandle& getParameterHandle(const InteractionClassHandle& interactionClassHandle, const std::wstring& name) const
    throw (InvalidInteractionClassHandle,
           NameNotFound,
           RTIinternalError) = 0;

  virtual const std::wstring& getParameterName(const InteractionClassHandle& interactionClassHandle, const ParameterHandle& parameterHandle) const
    throw (InvalidInteractionClassHandle,
           InvalidParameterHandle,
           InteractionParameterNotDefined,
           RTIinternalError) = 0;

  virtual const ObjectInstanceHandle& getObjectInstanceHandle(const std::wstring& name) const
    throw (ObjectInstanceNotKnown,
           RTIinternalError) = 0;

  virtual const std::wstring& getObjectInstanceName(const ObjectInstanceHandle& objectInstanceHandle) const
    throw (ObjectInstanceNotKnown,
           RTIinternalError) = 0;

  virtual const DimensionHandle& getDimensionHandle(const std::wstring& name) const
    throw (NameNotFound,
           RTIinternalError) = 0;

  virtual const std::wstring& getDimensionName(const DimensionHandle& dimensionHandle) const
    throw (InvalidDimensionHandle,
           RTIinternalError) = 0;

  virtual unsigned long getDimensionUpperBound(const DimensionHandle& dimensionHandle) const
    throw (InvalidDimensionHandle,
           RTIinternalError) = 0;

  virtual const DimensionHandleSet& getAvailableDimensionsForClassAttribute(const ObjectClassHandle& objectClassHandle,
                                                                     const AttributeHandle& attributeHandle) const
    throw (InvalidObjectClassHandle,
           InvalidAttributeHandle,
           AttributeNotDefined,
           RTIinternalError) = 0;

  virtual const ObjectClassHandle& getKnownObjectClassHandle(const ObjectInstanceHandle& objectInstanceHandle) const
    throw (ObjectInstanceNotKnown,
           RTIinternalError) = 0;

  virtual const DimensionHandleSet& getAvailableDimensionsForInteractionClass(const InteractionClassHandle& interactionClassHandle) const
    throw (InvalidInteractionClassHandle,
           RTIinternalError) = 0;

  virtual TransportationType getTransportationType(const std::wstring& name) const
    throw (InvalidTransportationName,
           RTIinternalError) = 0;

  virtual const std::wstring& getTransportationName(TransportationType transportationType) const
    throw (InvalidTransportationType,
           RTIinternalError) = 0;

  virtual OrderType getOrderType(const std::wstring& name) const
    throw (InvalidOrderName,
           RTIinternalError) = 0;

  virtual const std::wstring& getOrderName(OrderType orderType) const
    throw (InvalidOrderType,
           RTIinternalError) = 0;

  virtual void enableObjectClassRelevanceAdvisorySwitch()
    throw (ObjectClassRelevanceAdvisorySwitchIsOn,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void disableObjectClassRelevanceAdvisorySwitch()
    throw (ObjectClassRelevanceAdvisorySwitchIsOff,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void enableAttributeRelevanceAdvisorySwitch()
    throw (AttributeRelevanceAdvisorySwitchIsOn,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void disableAttributeRelevanceAdvisorySwitch()
    throw (AttributeRelevanceAdvisorySwitchIsOff,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void enableAttributeScopeAdvisorySwitch()
    throw (AttributeScopeAdvisorySwitchIsOn,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void disableAttributeScopeAdvisorySwitch()
    throw (AttributeScopeAdvisorySwitchIsOff,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void enableInteractionRelevanceAdvisorySwitch()
    throw (InteractionRelevanceAdvisorySwitchIsOn,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void disableInteractionRelevanceAdvisorySwitch()
    throw (InteractionRelevanceAdvisorySwitchIsOff,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual DimensionHandleSet getDimensionHandleSet(RegionHandle regionHandle)
    throw (InvalidRegion,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual RangeBounds getRangeBounds(RegionHandle regionHandle, DimensionHandle dimensionHandle)
    throw (InvalidRegion,
           RegionDoesNotContainSpecifiedDimension,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void setRangeBounds(RegionHandle regionHandle, DimensionHandle dimensionHandle, const RangeBounds& rangeBounds)
    throw (InvalidRegion,
           RegionNotCreatedByThisFederate,
           RegionDoesNotContainSpecifiedDimension,
           InvalidRangeBound,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual unsigned long normalizeFederateHandle(FederateHandle federateHandle)
    throw (InvalidFederateHandle,
           RTIinternalError) = 0;

  virtual unsigned long normalizeServiceGroup(ServiceGroupIndicator serviceGroup)
    throw (InvalidServiceGroup,
           RTIinternalError) = 0;

  virtual bool evokeCallback(double approximateMinimumTimeInSeconds)
    throw (RTIinternalError) = 0;

  virtual bool evokeMultipleCallbacks(double approximateMinimumTimeInSeconds, double approximateMaximumTimeInSeconds)
    throw (RTIinternalError) = 0;

  virtual void enableCallbacks()
    throw (SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;

  virtual void disableCallbacks()
    throw (SaveInProgress,
           RestoreInProgress,
           RTIinternalError) = 0;
};

} // namespace OpenRTI

#endif
