/***********************************************************************
  IEEE 1516.1 High Level Architecture Interface Specification C++ API
  File: RTI/NullFederateAmbassador.h
***********************************************************************/

#ifndef RTI_NullFederateAmbassador_h
#define RTI_NullFederateAmbassador_h

#include <RTI/Exception.h>
#include <RTI/FederateAmbassador.h>

namespace rti1516
{
  class RTI_EXPORT NullFederateAmbassador : public FederateAmbassador
  {
  public:
    NullFederateAmbassador()
       RTI_THROW ((FederateInternalError)) {}

    virtual
    ~NullFederateAmbassador()
    RTI_NOEXCEPT {}

    // 4.7
    virtual
    void
    synchronizationPointRegistrationSucceeded(std::wstring const & label)
      RTI_THROW ((FederateInternalError)) {}

    virtual
    void
    synchronizationPointRegistrationFailed(std::wstring const & label,
                                           SynchronizationFailureReason reason)
      RTI_THROW ((FederateInternalError)) {}

    // 4.8
    virtual
    void
    announceSynchronizationPoint(std::wstring  const & label,
                                 VariableLengthData const & theUserSuppliedTag)
      RTI_THROW ((FederateInternalError)) {}

    // 4.10
    virtual
    void
    federationSynchronized(std::wstring const & label)
      RTI_THROW ((FederateInternalError)) {}

    // 4.12
    virtual
    void
    initiateFederateSave(std::wstring const & label)
      RTI_THROW ((UnableToPerformSave,
             FederateInternalError)) {}

    virtual
    void
    initiateFederateSave(std::wstring const & label,
                         LogicalTime const & theTime)
      RTI_THROW ((UnableToPerformSave,
             InvalidLogicalTime,
             FederateInternalError)) {}

    // 4.15
    virtual
    void
    federationSaved()
      RTI_THROW ((FederateInternalError)) {}

    virtual
    void
    federationNotSaved(SaveFailureReason theSaveFailureReason)
      RTI_THROW ((FederateInternalError)) {}


    // 4.17
    virtual
    void
    federationSaveStatusResponse(
      FederateHandleSaveStatusPairVector const & 
      theFederateStatusVector)
      RTI_THROW ((FederateInternalError)) {}

    // 4.19
    virtual
    void
    requestFederationRestoreSucceeded(std::wstring const & label)
      RTI_THROW ((FederateInternalError)) {}

    virtual
    void
    requestFederationRestoreFailed(std::wstring const & label)
      RTI_THROW ((FederateInternalError)) {}

    // 4.20
    virtual
    void
    federationRestoreBegun()
      RTI_THROW ((FederateInternalError)) {}

    // 4.21
    virtual
    void
    initiateFederateRestore(std::wstring         const & label,
                            FederateHandle handle)
      RTI_THROW ((SpecifiedSaveLabelDoesNotExist,
             CouldNotInitiateRestore,
             FederateInternalError)) {}

    // 4.23
    virtual
    void
    federationRestored()
      RTI_THROW ((FederateInternalError)) {}

    virtual
    void
    federationNotRestored(RestoreFailureReason theRestoreFailureReason)
      RTI_THROW ((FederateInternalError)) {}

    // 4.25
    virtual
    void
    federationRestoreStatusResponse(
      FederateHandleRestoreStatusPairVector  const & 
      theFederateStatusVector)
      RTI_THROW ((FederateInternalError)) {}

    /////////////////////////////////////
    // Declaration Management Services //
    /////////////////////////////////////
  
    // 5.10
    virtual
    void
    startRegistrationForObjectClass(ObjectClassHandle theClass)
      RTI_THROW ((ObjectClassNotPublished,
             FederateInternalError)) {}

    // 5.11
    virtual
    void
    stopRegistrationForObjectClass(ObjectClassHandle theClass)
      RTI_THROW ((ObjectClassNotPublished,
             FederateInternalError)) {}

    // 5.12
    virtual
    void
    turnInteractionsOn(InteractionClassHandle theHandle)
      RTI_THROW ((InteractionClassNotPublished,
             FederateInternalError)) {}

    // 5.13
    virtual
    void
    turnInteractionsOff(InteractionClassHandle theHandle)
      RTI_THROW ((InteractionClassNotPublished,
             FederateInternalError)) {}

    ////////////////////////////////
    // Object Management Services //
    ////////////////////////////////
  
    // 6.3
    virtual
    void
    objectInstanceNameReservationSucceeded(std::wstring const &
                                           theObjectInstanceName)
      RTI_THROW ((UnknownName,
             FederateInternalError)) {}

    virtual
    void
    objectInstanceNameReservationFailed(std::wstring const &
                                        theObjectInstanceName)
      RTI_THROW ((UnknownName,
             FederateInternalError)) {}

  
    // 6.5
    virtual
    void
    discoverObjectInstance(ObjectInstanceHandle theObject,
                           ObjectClassHandle theObjectClass,
                           std::wstring const & theObjectInstanceName)
      RTI_THROW ((CouldNotDiscover,
             ObjectClassNotKnown,
             FederateInternalError)) {}

    // 6.7
    virtual
    void
    reflectAttributeValues
    (ObjectInstanceHandle theObject,
     AttributeHandleValueMap const & theAttributeValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotRecognized,
             AttributeNotSubscribed,
             FederateInternalError)) {}

    virtual
    void
    reflectAttributeValues
    (ObjectInstanceHandle theObject,
     AttributeHandleValueMap const & theAttributeValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType,
     RegionHandleSet const & theSentRegionHandleSet)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotRecognized,
             AttributeNotSubscribed,
             FederateInternalError)) {}

    virtual
    void
    reflectAttributeValues
    (ObjectInstanceHandle theObject,
     AttributeHandleValueMap const & theAttributeValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType,
     LogicalTime const & theTime,
     OrderType receivedOrder)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotRecognized,
             AttributeNotSubscribed,
             FederateInternalError)) {}
  
    virtual
    void
    reflectAttributeValues
    (ObjectInstanceHandle theObject,
     AttributeHandleValueMap const & theAttributeValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType,
     LogicalTime const & theTime,
     OrderType receivedOrder,
     RegionHandleSet const & theSentRegionHandleSet)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotRecognized,
             AttributeNotSubscribed,
             FederateInternalError)) {}
  
    virtual
    void
    reflectAttributeValues
    (ObjectInstanceHandle theObject,
     AttributeHandleValueMap const & theAttributeValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType,
     LogicalTime const & theTime,
     OrderType receivedOrder,
     MessageRetractionHandle theHandle)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotRecognized,
             AttributeNotSubscribed,
             InvalidLogicalTime,
             FederateInternalError)) {}

    virtual
    void
    reflectAttributeValues
    (ObjectInstanceHandle theObject,
     AttributeHandleValueMap const & theAttributeValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType,
     LogicalTime const & theTime,
     OrderType receivedOrder,
     MessageRetractionHandle theHandle,
     RegionHandleSet const & theSentRegionHandleSet)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotRecognized,
             AttributeNotSubscribed,
             InvalidLogicalTime,
             FederateInternalError)) {}

    // 6.9
    virtual
    void
    receiveInteraction
    (InteractionClassHandle theInteraction,
     ParameterHandleValueMap const & theParameterValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType)
      RTI_THROW ((InteractionClassNotRecognized,
             InteractionParameterNotRecognized,
             InteractionClassNotSubscribed,
             FederateInternalError)) {}

    virtual
    void
    receiveInteraction
    (InteractionClassHandle theInteraction,
     ParameterHandleValueMap const & theParameterValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType,
     RegionHandleSet const & theSentRegionHandleSet)
      RTI_THROW ((InteractionClassNotRecognized,
             InteractionParameterNotRecognized,
             InteractionClassNotSubscribed,
             FederateInternalError)) {}

    virtual
    void
    receiveInteraction
    (InteractionClassHandle theInteraction,
     ParameterHandleValueMap const & theParameterValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType,
     LogicalTime const & theTime,
     OrderType receivedOrder)
      RTI_THROW ((InteractionClassNotRecognized,
             InteractionParameterNotRecognized,
             InteractionClassNotSubscribed,
             FederateInternalError)) {}

    virtual
    void
    receiveInteraction
    (InteractionClassHandle theInteraction,
     ParameterHandleValueMap const & theParameterValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType,
     LogicalTime const & theTime,
     OrderType receivedOrder,
     RegionHandleSet const & theSentRegionHandleSet)
      RTI_THROW ((InteractionClassNotRecognized,
             InteractionParameterNotRecognized,
             InteractionClassNotSubscribed,
             FederateInternalError)) {}

    virtual
    void
    receiveInteraction
    (InteractionClassHandle theInteraction,
     ParameterHandleValueMap const & theParameterValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType,
     LogicalTime const & theTime,
     OrderType receivedOrder,
     MessageRetractionHandle theHandle)
      RTI_THROW ((InteractionClassNotRecognized,
             InteractionParameterNotRecognized,
             InteractionClassNotSubscribed,
             InvalidLogicalTime,
             FederateInternalError)) {}

    virtual
    void
    receiveInteraction
    (InteractionClassHandle theInteraction,
     ParameterHandleValueMap const & theParameterValues,
     VariableLengthData const & theUserSuppliedTag,
     OrderType sentOrder,
     TransportationType theType,
     LogicalTime const & theTime,
     OrderType receivedOrder,
     MessageRetractionHandle theHandle,
     RegionHandleSet const & theSentRegionHandleSet)
      RTI_THROW ((InteractionClassNotRecognized,
             InteractionParameterNotRecognized,
             InteractionClassNotSubscribed,
             InvalidLogicalTime,
             FederateInternalError)) {}

    // 6.11
    virtual
    void
    removeObjectInstance(ObjectInstanceHandle theObject,
                         VariableLengthData const & theUserSuppliedTag,
                         OrderType sentOrder)
      RTI_THROW ((ObjectInstanceNotKnown,
             FederateInternalError)) {}

    virtual
    void
    removeObjectInstance(ObjectInstanceHandle theObject,
                         VariableLengthData const & theUserSuppliedTag,
                         OrderType sentOrder,
                         LogicalTime const & theTime,
                         OrderType receivedOrder)
      RTI_THROW ((ObjectInstanceNotKnown,
             FederateInternalError)) {}

    virtual
    void
    removeObjectInstance(ObjectInstanceHandle theObject,
                         VariableLengthData const & theUserSuppliedTag,
                         OrderType sentOrder,
                         LogicalTime const & theTime,
                         OrderType receivedOrder,
                         MessageRetractionHandle theHandle)
      RTI_THROW ((ObjectInstanceNotKnown,
             InvalidLogicalTime,
             FederateInternalError)) {}

    // 6.15
    virtual
    void
    attributesInScope
    (ObjectInstanceHandle theObject,
      AttributeHandleSet const & theAttributes)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotRecognized,
             AttributeNotSubscribed,
             FederateInternalError)) {}

    // 6.16
    virtual
    void
    attributesOutOfScope
    (ObjectInstanceHandle theObject,
      AttributeHandleSet const & theAttributes)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotRecognized,
             AttributeNotSubscribed,
             FederateInternalError)) {}

    // 6.18
    virtual
    void
    provideAttributeValueUpdate
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & theAttributes,
     VariableLengthData const & theUserSuppliedTag)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotRecognized,
             AttributeNotOwned,
             FederateInternalError)) {}

    // 6.19
    virtual
    void
    turnUpdatesOnForObjectInstance
    (ObjectInstanceHandle theObject,
      AttributeHandleSet const & theAttributes)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotRecognized,
             AttributeNotOwned,
             FederateInternalError)) {}

    // 6.20
    virtual
    void
    turnUpdatesOffForObjectInstance
    (ObjectInstanceHandle theObject,
      AttributeHandleSet const & theAttributes)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotRecognized,
             AttributeNotOwned,
             FederateInternalError)) {}

    ///////////////////////////////////
    // Ownership Management Services //
    ///////////////////////////////////
  
    // 7.4
    virtual
    void
    requestAttributeOwnershipAssumption
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & offeredAttributes,
     VariableLengthData const & theUserSuppliedTag)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotRecognized,
             AttributeAlreadyOwned,
             AttributeNotPublished,
             FederateInternalError)) {}

    // 7.5
    virtual
    void
    requestDivestitureConfirmation
    (ObjectInstanceHandle theObject,
      AttributeHandleSet const & releasedAttributes)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotRecognized,
             AttributeNotOwned,
             AttributeDivestitureWasNotRequested,
             FederateInternalError)) {}

    // 7.7
    virtual
    void
    attributeOwnershipAcquisitionNotification
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & securedAttributes,
     VariableLengthData const & theUserSuppliedTag)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotRecognized,
             AttributeAcquisitionWasNotRequested,
             AttributeAlreadyOwned,
             AttributeNotPublished,
             FederateInternalError)) {}

    // 7.10
    virtual
    void
    attributeOwnershipUnavailable
    (ObjectInstanceHandle theObject,
      AttributeHandleSet const & theAttributes)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotRecognized,
             AttributeAlreadyOwned,
             AttributeAcquisitionWasNotRequested,
             FederateInternalError)) {}

    // 7.11
    virtual
    void
    requestAttributeOwnershipRelease
    (ObjectInstanceHandle theObject,
     AttributeHandleSet const & candidateAttributes,
     VariableLengthData const & theUserSuppliedTag)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotRecognized,
             AttributeNotOwned,
             FederateInternalError)) {}

    // 7.15
    virtual
    void
    confirmAttributeOwnershipAcquisitionCancellation
    (ObjectInstanceHandle theObject,
      AttributeHandleSet const & theAttributes)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotRecognized,
             AttributeAlreadyOwned,
             AttributeAcquisitionWasNotCanceled,
             FederateInternalError)) {}

    // 7.17
    virtual
    void
    informAttributeOwnership(ObjectInstanceHandle theObject,
                             AttributeHandle theAttribute,
                             FederateHandle theOwner)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotRecognized,
             FederateInternalError)) {}

    virtual
    void
    attributeIsNotOwned(ObjectInstanceHandle theObject,
                        AttributeHandle theAttribute)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotRecognized,
             FederateInternalError)) {}

    virtual
    void
    attributeIsOwnedByRTI(ObjectInstanceHandle theObject,
                          AttributeHandle theAttribute)
      RTI_THROW ((ObjectInstanceNotKnown,
             AttributeNotRecognized,
             FederateInternalError)) {}

    //////////////////////////////
    // Time Management Services //
    //////////////////////////////
  
    // 8.3
    virtual
    void
    timeRegulationEnabled(LogicalTime const & theFederateTime)
      RTI_THROW ((InvalidLogicalTime,
             NoRequestToEnableTimeRegulationWasPending,
             FederateInternalError)) {}

    // 8.6
    virtual
    void
    timeConstrainedEnabled(LogicalTime const & theFederateTime)
      RTI_THROW ((InvalidLogicalTime,
             NoRequestToEnableTimeConstrainedWasPending,
             FederateInternalError)) {}

    // 8.13
    virtual
    void
    timeAdvanceGrant(LogicalTime const & theTime)
      RTI_THROW ((InvalidLogicalTime,
             JoinedFederateIsNotInTimeAdvancingState,
             FederateInternalError)) {}

    // 8.22
    virtual
    void
    requestRetraction(MessageRetractionHandle theHandle)
      RTI_THROW ((FederateInternalError)) {}
  };
}

#endif // RTI_NullFederateAmbassador_h
