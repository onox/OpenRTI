/***********************************************************************
  IEEE 1516.1 High Level Architecture Interface Specification C++ API
  File: RTI/RTIambassador.h
***********************************************************************/

// This interface is used to access the services of the RTI. 

#ifndef RTI_RTIambassador_h
#define RTI_RTIambassador_h

namespace rti1516
{
  class FederateAmbassador;
  class LogicalTime;
  class LogicalTimeFactory;
  class LogicalTimeInterval;
  class RangeBounds;
}

#include <RTI/SpecificConfig.h>
#include <string>
#include <RTI/Typedefs.h>
#include <RTI/Exception.h>

namespace rti1516
{
  class RTI_EXPORT RTIambassador
  {
  protected:
    RTIambassador()
       RTI_NOEXCEPT;

  public:
    virtual
      ~RTIambassador();
    // RTI_NOEXCEPT

    // 4.2
    virtual void createFederationExecution
    (std::wstring const & federationExecutionName,
     std::wstring const & fullPathNameToTheFDDfile,
     std::wstring const & logicalTimeImplementationName = L"")
      RTI_THROW ((FederationExecutionAlreadyExists,
             CouldNotOpenFDD,
             ErrorReadingFDD,
             CouldNotCreateLogicalTimeFactory,
             RTIinternalError)) = 0;

    // 4.3
    virtual void destroyFederationExecution 
    (std::wstring const & federationExecutionName)
      RTI_THROW ((FederatesCurrentlyJoined,
             FederationExecutionDoesNotExist,
             RTIinternalError)) = 0;

    // 4.4
    virtual FederateHandle joinFederationExecution 
    (std::wstring const & federateType,
     std::wstring const & federationExecutionName,
     FederateAmbassador & federateAmbassador)
      RTI_THROW ((FederateAlreadyExecutionMember,
             FederationExecutionDoesNotExist,
             SaveInProgress,
             RestoreInProgress,
             CouldNotCreateLogicalTimeFactory,
             RTIinternalError)) = 0;

    // 4.5
    virtual void resignFederationExecution
    (ResignAction resignAction)
      RTI_THROW ((OwnershipAcquisitionPending,
             FederateOwnsAttributes,
             FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 4.6
    virtual void registerFederationSynchronizationPoint
    (std::wstring const & label,
     VariableLengthData const & theUserSuppliedTag)
      RTI_THROW ((FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    virtual void registerFederationSynchronizationPoint
    (std::wstring const & label,
     VariableLengthData const & theUserSuppliedTag,
     FederateHandleSet const & syncSet)
      RTI_THROW ((FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 4.9
    virtual void synchronizationPointAchieved
    (std::wstring const & label)
      RTI_THROW ((SynchronizationPointLabelNotAnnounced,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 4.11
    virtual void requestFederationSave
    (std::wstring const & label)
      RTI_THROW ((FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    virtual void requestFederationSave
    (std::wstring const & label,
     LogicalTime const & theTime)
      RTI_THROW ((LogicalTimeAlreadyPassed,
             InvalidLogicalTime,
             FederateUnableToUseTime,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 4.13
    virtual void federateSaveBegun ()
      RTI_THROW ((SaveNotInitiated,
             FederateNotExecutionMember,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 4.14
    virtual void federateSaveComplete ()
      RTI_THROW ((FederateHasNotBegunSave,
             FederateNotExecutionMember,
             RestoreInProgress,
             RTIinternalError)) = 0;

    virtual void federateSaveNotComplete()
      RTI_THROW ((FederateHasNotBegunSave,
             FederateNotExecutionMember,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 4.16
    virtual void queryFederationSaveStatus ()
      RTI_THROW ((FederateNotExecutionMember,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 4.18
    virtual void requestFederationRestore
    (std::wstring const & label)
      RTI_THROW ((FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 4.22
    virtual void federateRestoreComplete ()
      RTI_THROW ((RestoreNotRequested,
             FederateNotExecutionMember,
             SaveInProgress,
             RTIinternalError)) = 0;

    virtual void federateRestoreNotComplete ()
      RTI_THROW ((RestoreNotRequested,
             FederateNotExecutionMember,
             SaveInProgress,
             RTIinternalError)) = 0;

    // 4.24
    virtual void queryFederationRestoreStatus ()
      RTI_THROW ((FederateNotExecutionMember,
             SaveInProgress,
             RTIinternalError)) = 0;

    /////////////////////////////////////
    // Declaration Management Services //
    /////////////////////////////////////
  
    // 5.2
    virtual void publishObjectClassAttributes
    (ObjectClassHandle theClass,
     AttributeHandleSet const & attributeList)
      RTI_THROW ((ObjectClassNotDefined,
             AttributeNotDefined,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 5.3
    virtual void unpublishObjectClass
    (ObjectClassHandle theClass)
      RTI_THROW ((ObjectClassNotDefined,
             OwnershipAcquisitionPending,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    virtual void unpublishObjectClassAttributes
    (ObjectClassHandle theClass,
     AttributeHandleSet const & attributeList)
      RTI_THROW ((ObjectClassNotDefined,
             AttributeNotDefined,
             OwnershipAcquisitionPending,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 5.4
    virtual void publishInteractionClass
    (InteractionClassHandle theInteraction)
      RTI_THROW ((InteractionClassNotDefined,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 5.5
    virtual void unpublishInteractionClass
    (InteractionClassHandle theInteraction)
      RTI_THROW ((InteractionClassNotDefined,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 5.6
    virtual void subscribeObjectClassAttributes
    (ObjectClassHandle theClass,
     AttributeHandleSet const & attributeList,
     bool active = true)
      RTI_THROW ((ObjectClassNotDefined,
             AttributeNotDefined,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 5.7
    virtual void unsubscribeObjectClass
    (ObjectClassHandle theClass)
      RTI_THROW ((ObjectClassNotDefined,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    virtual void unsubscribeObjectClassAttributes
    (ObjectClassHandle theClass,
     AttributeHandleSet const & attributeList)
      RTI_THROW ((ObjectClassNotDefined,
             AttributeNotDefined,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 5.8
    virtual void subscribeInteractionClass
    (InteractionClassHandle theClass,
     bool active = true)
      RTI_THROW ((InteractionClassNotDefined,
             FederateServiceInvocationsAreBeingReportedViaMOM,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 5.9
    virtual void unsubscribeInteractionClass
    (InteractionClassHandle theClass)
      RTI_THROW ((InteractionClassNotDefined,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    ////////////////////////////////
    // Object Management Services //
    ////////////////////////////////
  
    // 6.2
    virtual void reserveObjectInstanceName
    (std::wstring const & theObjectInstanceName)
      RTI_THROW ((IllegalName,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 6.4
    virtual ObjectInstanceHandle registerObjectInstance
    (ObjectClassHandle theClass)
      RTI_THROW ((ObjectClassNotDefined,
             ObjectClassNotPublished,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    virtual ObjectInstanceHandle registerObjectInstance
    (ObjectClassHandle theClass,
     std::wstring const & theObjectInstanceName)
      RTI_THROW ((ObjectClassNotDefined,
             ObjectClassNotPublished,
             ObjectInstanceNameNotReserved,
             ObjectInstanceNameInUse,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 6.6
    virtual void updateAttributeValues
    (ObjectInstanceHandle theObject,
     AttributeHandleValueMap const & theAttributeValues,
     VariableLengthData const & theUserSuppliedTag)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotDefined,
             AttributeNotOwned,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    virtual MessageRetractionHandle updateAttributeValues
    (ObjectInstanceHandle theObject,
     AttributeHandleValueMap const & theAttributeValues,
     VariableLengthData const & theUserSuppliedTag,
     LogicalTime const & theTime)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotDefined,
             AttributeNotOwned,
             InvalidLogicalTime,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 6.8
    virtual void sendInteraction
    (InteractionClassHandle theInteraction,
     ParameterHandleValueMap const & theParameterValues,
     VariableLengthData const & theUserSuppliedTag)
      RTI_THROW ((InteractionClassNotPublished,
             InteractionClassNotDefined,
             InteractionParameterNotDefined,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    virtual MessageRetractionHandle sendInteraction
    (InteractionClassHandle theInteraction,
     ParameterHandleValueMap const & theParameterValues,
     VariableLengthData const & theUserSuppliedTag,
     LogicalTime const & theTime)
      RTI_THROW ((InteractionClassNotPublished,
             InteractionClassNotDefined,
             InteractionParameterNotDefined,
             InvalidLogicalTime,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 6.10
    virtual void deleteObjectInstance
    (ObjectInstanceHandle theObject,
     VariableLengthData const & theUserSuppliedTag)
      RTI_THROW ((DeletePrivilegeNotHeld,
             ObjectInstanceNotKnown,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    virtual MessageRetractionHandle deleteObjectInstance
    (ObjectInstanceHandle theObject,
     VariableLengthData const & theUserSuppliedTag,
     LogicalTime  const & theTime)
      RTI_THROW ((DeletePrivilegeNotHeld,
             ObjectInstanceNotKnown,
             InvalidLogicalTime,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 6.12
    virtual void localDeleteObjectInstance
    (ObjectInstanceHandle theObject)
      RTI_THROW ((ObjectInstanceNotKnown,
             FederateOwnsAttributes,
             OwnershipAcquisitionPending,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 6.13
    virtual void changeAttributeTransportationType
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & theAttributes,
     TransportationType theType)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotDefined,
             AttributeNotOwned,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 6.14
    virtual void changeInteractionTransportationType
    (InteractionClassHandle theClass,
     TransportationType theType)
      RTI_THROW ((InteractionClassNotDefined,
             InteractionClassNotPublished,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;
  
    // 6.17
    virtual void requestAttributeValueUpdate
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & theAttributes,
     VariableLengthData const & theUserSuppliedTag)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotDefined,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    virtual void requestAttributeValueUpdate
    (ObjectClassHandle theClass,
     AttributeHandleSet const & theAttributes,
     VariableLengthData const & theUserSuppliedTag)
      RTI_THROW ((ObjectClassNotDefined,
             AttributeNotDefined,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    ///////////////////////////////////
    // Ownership Management Services //
    ///////////////////////////////////
    // 7.2
    virtual void unconditionalAttributeOwnershipDivestiture
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & theAttributes)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotDefined,
             AttributeNotOwned,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 7.3
    virtual void negotiatedAttributeOwnershipDivestiture
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & theAttributes,
     VariableLengthData const & theUserSuppliedTag)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotDefined,
             AttributeNotOwned,
             AttributeAlreadyBeingDivested,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 7.6
    virtual void confirmDivestiture
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & confirmedAttributes,
     VariableLengthData const & theUserSuppliedTag)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotDefined,
             AttributeNotOwned,
             AttributeDivestitureWasNotRequested,
             NoAcquisitionPending,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 7.8
    virtual void attributeOwnershipAcquisition
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & desiredAttributes,
     VariableLengthData const & theUserSuppliedTag)
      RTI_THROW ((ObjectInstanceNotKnown,
             ObjectClassNotPublished,
             AttributeNotDefined,
             AttributeNotPublished,
             FederateOwnsAttributes,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 7.9
    virtual void attributeOwnershipAcquisitionIfAvailable
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & desiredAttributes)
      RTI_THROW ((ObjectInstanceNotKnown,
             ObjectClassNotPublished,
             AttributeNotDefined,
             AttributeNotPublished,
             FederateOwnsAttributes,
             AttributeAlreadyBeingAcquired,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 7.12
    virtual void attributeOwnershipDivestitureIfWanted
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & theAttributes,
     AttributeHandleSet & theDivestedAttributes) // filled by RTI
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotDefined,
             AttributeNotOwned,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 7.13
    virtual void cancelNegotiatedAttributeOwnershipDivestiture
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & theAttributes)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotDefined,
             AttributeNotOwned,
             AttributeDivestitureWasNotRequested,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 7.14
    virtual void cancelAttributeOwnershipAcquisition
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & theAttributes)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotDefined,
             AttributeAlreadyOwned,
             AttributeAcquisitionWasNotRequested,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 7.16
    virtual void queryAttributeOwnership
    (ObjectInstanceHandle theObject,
     AttributeHandle theAttribute)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotDefined,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 7.18
    virtual bool isAttributeOwnedByFederate
    (ObjectInstanceHandle theObject,
     AttributeHandle theAttribute)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotDefined,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    //////////////////////////////
    // Time Management Services //
    //////////////////////////////
  
    // 8.2
    virtual void enableTimeRegulation
    (LogicalTimeInterval const & theLookahead)
      RTI_THROW ((TimeRegulationAlreadyEnabled,
             InvalidLookahead,
             InTimeAdvancingState,
             RequestForTimeRegulationPending,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 8.4
    virtual void disableTimeRegulation ()
      RTI_THROW ((TimeRegulationIsNotEnabled,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 8.5
    virtual void enableTimeConstrained ()
      RTI_THROW ((TimeConstrainedAlreadyEnabled,
             InTimeAdvancingState,
             RequestForTimeConstrainedPending,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 8.7
    virtual void disableTimeConstrained ()
      RTI_THROW ((TimeConstrainedIsNotEnabled,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 8.8
    virtual void timeAdvanceRequest
    (LogicalTime const & theTime)
      RTI_THROW ((InvalidLogicalTime,
             LogicalTimeAlreadyPassed,
             InTimeAdvancingState,
             RequestForTimeRegulationPending,
             RequestForTimeConstrainedPending,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 8.9
    virtual void timeAdvanceRequestAvailable
    (LogicalTime const & theTime)
      RTI_THROW ((InvalidLogicalTime,
             LogicalTimeAlreadyPassed,
             InTimeAdvancingState,
             RequestForTimeRegulationPending,
             RequestForTimeConstrainedPending,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 8.10
    virtual void nextMessageRequest
    (LogicalTime const & theTime)
      RTI_THROW ((InvalidLogicalTime,
             LogicalTimeAlreadyPassed,
             InTimeAdvancingState,
             RequestForTimeRegulationPending,
             RequestForTimeConstrainedPending,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 8.11
    virtual void nextMessageRequestAvailable
    (LogicalTime const & theTime)
      RTI_THROW ((InvalidLogicalTime,
             LogicalTimeAlreadyPassed,
             InTimeAdvancingState,
             RequestForTimeRegulationPending,
             RequestForTimeConstrainedPending,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 8.12
    virtual void flushQueueRequest
    (LogicalTime const & theTime)
      RTI_THROW ((InvalidLogicalTime,
             LogicalTimeAlreadyPassed,
             InTimeAdvancingState,
             RequestForTimeRegulationPending,
             RequestForTimeConstrainedPending,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 8.14
    virtual void enableAsynchronousDelivery ()
      RTI_THROW ((AsynchronousDeliveryAlreadyEnabled,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 8.15
    virtual void disableAsynchronousDelivery ()
      RTI_THROW ((AsynchronousDeliveryAlreadyDisabled,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 8.16
    virtual bool queryGALT (LogicalTime & theTime)
      RTI_THROW ((FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 8.17
    virtual void queryLogicalTime (LogicalTime & theTime)
      RTI_THROW ((FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 8.18
    virtual bool queryLITS (LogicalTime & theTime)
      RTI_THROW ((FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 8.19
    virtual void modifyLookahead
    (LogicalTimeInterval const & theLookahead)
      RTI_THROW ((TimeRegulationIsNotEnabled,
             InvalidLookahead,
             InTimeAdvancingState,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 8.20
    virtual void queryLookahead (LogicalTimeInterval & interval)
      RTI_THROW ((TimeRegulationIsNotEnabled,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 8.21
    virtual void retract
    (MessageRetractionHandle theHandle)
      RTI_THROW ((InvalidRetractionHandle,
             TimeRegulationIsNotEnabled,
             MessageCanNoLongerBeRetracted,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 8.23
    virtual void changeAttributeOrderType
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & theAttributes,
     OrderType theType)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotDefined,
             AttributeNotOwned,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 8.24
    virtual void changeInteractionOrderType
    (InteractionClassHandle theClass,
     OrderType theType)
      RTI_THROW ((InteractionClassNotDefined,
             InteractionClassNotPublished,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    //////////////////////////////////
    // Data Distribution Management //
    //////////////////////////////////
  
    // 9.2
    virtual RegionHandle createRegion
    (DimensionHandleSet const & theDimensions)
      RTI_THROW ((InvalidDimensionHandle,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 9.3
    virtual void commitRegionModifications
    (RegionHandleSet const & theRegionHandleSet)
      RTI_THROW ((InvalidRegion,
             RegionNotCreatedByThisFederate,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 9.4
    virtual void deleteRegion
    (RegionHandle theRegion)
      RTI_THROW ((InvalidRegion,
             RegionNotCreatedByThisFederate,
             RegionInUseForUpdateOrSubscription,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 9.5
    virtual ObjectInstanceHandle registerObjectInstanceWithRegions
    (ObjectClassHandle theClass,
     AttributeHandleSetRegionHandleSetPairVector const &
     theAttributeHandleSetRegionHandleSetPairVector)
      RTI_THROW ((ObjectClassNotDefined,
             ObjectClassNotPublished,
             AttributeNotDefined,
             AttributeNotPublished,
             InvalidRegion,
             RegionNotCreatedByThisFederate,
             InvalidRegionContext,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    virtual ObjectInstanceHandle registerObjectInstanceWithRegions
    (ObjectClassHandle theClass,
     AttributeHandleSetRegionHandleSetPairVector const &
     theAttributeHandleSetRegionHandleSetPairVector,
     std::wstring const & theObjectInstanceName)
      RTI_THROW ((ObjectClassNotDefined,
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
             RTIinternalError)) = 0;

    // 9.6
    virtual void associateRegionsForUpdates
    (ObjectInstanceHandle theObject,
     AttributeHandleSetRegionHandleSetPairVector const &
     theAttributeHandleSetRegionHandleSetPairVector)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotDefined,
             InvalidRegion,
             RegionNotCreatedByThisFederate,
             InvalidRegionContext,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 9.7
    virtual void unassociateRegionsForUpdates
    (ObjectInstanceHandle theObject,
     AttributeHandleSetRegionHandleSetPairVector const &
     theAttributeHandleSetRegionHandleSetPairVector)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotDefined,
             InvalidRegion,
             RegionNotCreatedByThisFederate,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 9.8
    virtual void subscribeObjectClassAttributesWithRegions
    (ObjectClassHandle theClass,
     AttributeHandleSetRegionHandleSetPairVector const &
     theAttributeHandleSetRegionHandleSetPairVector,
     bool active = true)
      RTI_THROW ((ObjectClassNotDefined,
             AttributeNotDefined,
             InvalidRegion,
             RegionNotCreatedByThisFederate,
             InvalidRegionContext,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 9.9
    virtual void unsubscribeObjectClassAttributesWithRegions
    (ObjectClassHandle theClass,
     AttributeHandleSetRegionHandleSetPairVector const &
     theAttributeHandleSetRegionHandleSetPairVector)
      RTI_THROW ((ObjectClassNotDefined,
             AttributeNotDefined,
             InvalidRegion,
             RegionNotCreatedByThisFederate,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 9.10
    virtual void subscribeInteractionClassWithRegions
    (InteractionClassHandle theClass,
     RegionHandleSet const & theRegionHandleSet,
     bool active = true)
      RTI_THROW ((InteractionClassNotDefined,
             InvalidRegion,
             RegionNotCreatedByThisFederate,
             InvalidRegionContext,
             FederateServiceInvocationsAreBeingReportedViaMOM,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 9.11
    virtual void unsubscribeInteractionClassWithRegions
    (InteractionClassHandle theClass,
     RegionHandleSet const & theRegionHandleSet)
      RTI_THROW ((InteractionClassNotDefined,
             InvalidRegion,
             RegionNotCreatedByThisFederate,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 9.12
    virtual void sendInteractionWithRegions
    (InteractionClassHandle theInteraction,
     ParameterHandleValueMap const & theParameterValues,
     RegionHandleSet const & theRegionHandleSet,
     VariableLengthData const & theUserSuppliedTag)
      RTI_THROW ((InteractionClassNotDefined,
             InteractionClassNotPublished,
             InteractionParameterNotDefined,
             InvalidRegion,
             RegionNotCreatedByThisFederate,
             InvalidRegionContext,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    virtual MessageRetractionHandle sendInteractionWithRegions
    (InteractionClassHandle theInteraction,
     ParameterHandleValueMap const & theParameterValues,
     RegionHandleSet const & theRegionHandleSet,
     VariableLengthData const & theUserSuppliedTag,
     LogicalTime const & theTime)
      RTI_THROW ((InteractionClassNotDefined,
             InteractionClassNotPublished,
             InteractionParameterNotDefined,
             InvalidRegion,
             RegionNotCreatedByThisFederate,
             InvalidRegionContext,
             InvalidLogicalTime,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 9.13
    virtual void requestAttributeValueUpdateWithRegions
    (ObjectClassHandle theClass,
     AttributeHandleSetRegionHandleSetPairVector const & theSet,
     VariableLengthData const & theUserSuppliedTag)
      RTI_THROW ((ObjectClassNotDefined,
             AttributeNotDefined,
             InvalidRegion,
             RegionNotCreatedByThisFederate,
             InvalidRegionContext,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    //////////////////////////
    // RTI Support Services //
    //////////////////////////
  
    // 10.2
    virtual ObjectClassHandle getObjectClassHandle
    (std::wstring const & theName)
      RTI_THROW ((NameNotFound,
             FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 10.3
    virtual std::wstring getObjectClassName
    (ObjectClassHandle theHandle)
      RTI_THROW ((InvalidObjectClassHandle,
             FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 10.4
    virtual AttributeHandle getAttributeHandle
    (ObjectClassHandle whichClass,
     std::wstring const & theAttributeName)
      RTI_THROW ((InvalidObjectClassHandle,
             NameNotFound,
             FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 10.5
    virtual std::wstring getAttributeName
    (ObjectClassHandle whichClass,
     AttributeHandle theHandle)   
      RTI_THROW ((InvalidObjectClassHandle,
             InvalidAttributeHandle,
             AttributeNotDefined,
             FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 10.6
    virtual InteractionClassHandle getInteractionClassHandle
    (std::wstring const & theName)
      RTI_THROW ((NameNotFound,
             FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 10.7
    virtual std::wstring getInteractionClassName
    (InteractionClassHandle theHandle)
      RTI_THROW ((InvalidInteractionClassHandle,
             FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 10.8
    virtual ParameterHandle getParameterHandle
    (InteractionClassHandle whichClass,
     std::wstring const & theName)
      RTI_THROW ((InvalidInteractionClassHandle,
             NameNotFound,
             FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 10.9
    virtual std::wstring getParameterName
    (InteractionClassHandle whichClass,
     ParameterHandle theHandle)   
      RTI_THROW ((InvalidInteractionClassHandle,
             InvalidParameterHandle,
             InteractionParameterNotDefined,
             FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 10.10
    virtual ObjectInstanceHandle getObjectInstanceHandle
    (std::wstring const & theName)
      RTI_THROW ((ObjectInstanceNotKnown,
             FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 10.11
    virtual std::wstring getObjectInstanceName
    (ObjectInstanceHandle theHandle)
      RTI_THROW ((ObjectInstanceNotKnown,
             FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 10.12
    virtual DimensionHandle getDimensionHandle
    (std::wstring const & theName)
      RTI_THROW ((NameNotFound,
             FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 10.13
    virtual std::wstring getDimensionName
    (DimensionHandle theHandle)
      RTI_THROW ((InvalidDimensionHandle,
             FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 10.14
    virtual unsigned long getDimensionUpperBound
    (DimensionHandle theHandle)   
      RTI_THROW ((InvalidDimensionHandle,
             FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 10.15
    virtual DimensionHandleSet getAvailableDimensionsForClassAttribute
    (ObjectClassHandle theClass,
     AttributeHandle theHandle)   
      RTI_THROW ((InvalidObjectClassHandle,
             InvalidAttributeHandle,
             AttributeNotDefined,
             FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 10.16
    virtual ObjectClassHandle getKnownObjectClassHandle
    (ObjectInstanceHandle theObject)
      RTI_THROW ((ObjectInstanceNotKnown,
             FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 10.17
    virtual DimensionHandleSet getAvailableDimensionsForInteractionClass
    (InteractionClassHandle theClass)
      RTI_THROW ((InvalidInteractionClassHandle,
             FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 10.18
    virtual TransportationType getTransportationType
    (std::wstring const & transportationName)
      RTI_THROW ((InvalidTransportationName,
             FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 10.19
    virtual std::wstring getTransportationName
    (TransportationType transportationType)
      RTI_THROW ((InvalidTransportationType,
             FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 10.20
    virtual OrderType getOrderType
    (std::wstring const & orderName)
      RTI_THROW ((InvalidOrderName,
             FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 10.21
    virtual std::wstring getOrderName
    (OrderType orderType)
      RTI_THROW ((InvalidOrderType,
             FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 10.22
    virtual void enableObjectClassRelevanceAdvisorySwitch ()
      RTI_THROW ((ObjectClassRelevanceAdvisorySwitchIsOn,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 10.23
    virtual void disableObjectClassRelevanceAdvisorySwitch ()
      RTI_THROW ((ObjectClassRelevanceAdvisorySwitchIsOff,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 10.24
    virtual void enableAttributeRelevanceAdvisorySwitch ()
      RTI_THROW ((AttributeRelevanceAdvisorySwitchIsOn,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 10.25
    virtual void disableAttributeRelevanceAdvisorySwitch ()
      RTI_THROW ((AttributeRelevanceAdvisorySwitchIsOff,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 10.26
    virtual void enableAttributeScopeAdvisorySwitch ()
      RTI_THROW ((AttributeScopeAdvisorySwitchIsOn,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 10.27
    virtual void disableAttributeScopeAdvisorySwitch ()
      RTI_THROW ((AttributeScopeAdvisorySwitchIsOff,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 10.28
    virtual void enableInteractionRelevanceAdvisorySwitch ()
      RTI_THROW ((InteractionRelevanceAdvisorySwitchIsOn,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 10.29
    virtual void disableInteractionRelevanceAdvisorySwitch ()
      RTI_THROW ((InteractionRelevanceAdvisorySwitchIsOff,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 10.30
    virtual
    DimensionHandleSet getDimensionHandleSet
    (RegionHandle theRegionHandle)
      RTI_THROW ((InvalidRegion,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 10.31
    virtual
    RangeBounds getRangeBounds
    (RegionHandle theRegionHandle,
     DimensionHandle theDimensionHandle)
      RTI_THROW ((InvalidRegion,
             RegionDoesNotContainSpecifiedDimension,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 10.32
    virtual void setRangeBounds
    (RegionHandle theRegionHandle,
     DimensionHandle theDimensionHandle,
     RangeBounds const & theRangeBounds)
      RTI_THROW ((InvalidRegion,
             RegionNotCreatedByThisFederate,
             RegionDoesNotContainSpecifiedDimension,
             InvalidRangeBound,
             FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 10.33
    virtual unsigned long normalizeFederateHandle
    (FederateHandle theFederateHandle)
      RTI_THROW ((FederateNotExecutionMember,
             InvalidFederateHandle,
             RTIinternalError)) = 0;

    // 10.34
    virtual unsigned long normalizeServiceGroup
    (ServiceGroupIndicator theServiceGroup)
      RTI_THROW ((FederateNotExecutionMember,
             InvalidServiceGroup,
             RTIinternalError)) = 0;

    // 10.37
    virtual bool evokeCallback(double approximateMinimumTimeInSeconds)
      RTI_THROW ((FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 10.38
    virtual bool evokeMultipleCallbacks(double approximateMinimumTimeInSeconds,
                                        double approximateMaximumTimeInSeconds)
      RTI_THROW ((FederateNotExecutionMember,
             RTIinternalError)) = 0;

    // 10.39
    virtual void enableCallbacks ()
      RTI_THROW ((FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    // 10.40
    virtual void disableCallbacks ()
      RTI_THROW ((FederateNotExecutionMember,
             SaveInProgress,
             RestoreInProgress,
             RTIinternalError)) = 0;

    virtual FederateHandle decodeFederateHandle(
       VariableLengthData const & encodedValue) const = 0;

    virtual ObjectClassHandle decodeObjectClassHandle(
       VariableLengthData const & encodedValue) const = 0;
    
    virtual InteractionClassHandle decodeInteractionClassHandle(
       VariableLengthData const & encodedValue) const = 0;
 
    virtual ObjectInstanceHandle decodeObjectInstanceHandle(
       VariableLengthData const & encodedValue) const = 0;

    virtual AttributeHandle decodeAttributeHandle(
       VariableLengthData const & encodedValue) const = 0;

    virtual ParameterHandle decodeParameterHandle(
       VariableLengthData const & encodedValue) const = 0;

    virtual DimensionHandle decodeDimensionHandle(
       VariableLengthData const & encodedValue) const = 0;

    virtual MessageRetractionHandle decodeMessageRetractionHandle(
       VariableLengthData const & encodedValue) const = 0;

    virtual RegionHandle decodeRegionHandle(
       VariableLengthData const & encodedValue) const = 0;

  };
}

#endif // RTI_RTIambassador_h
