// HLA 1.3 Header "federateAmbServices.hh"

// Federation Management

virtual void synchronizationPointRegistrationSucceeded(const char*)
  RTI_THROW ((FederateInternalError)) = 0;

virtual void synchronizationPointRegistrationFailed(const char*)
  RTI_THROW ((FederateInternalError)) = 0;

virtual void announceSynchronizationPoint(const char*, const char*)
  RTI_THROW ((FederateInternalError)) = 0;

virtual void federationSynchronized(const char*)
  RTI_THROW ((FederateInternalError)) = 0;

virtual void initiateFederateSave (const char*)
  RTI_THROW ((UnableToPerformSave, FederateInternalError)) = 0;

virtual void federationSaved()
  RTI_THROW ((FederateInternalError)) = 0;

virtual void federationNotSaved ()
  RTI_THROW ((FederateInternalError)) = 0;

virtual void requestFederationRestoreSucceeded (const char*)
  RTI_THROW ((FederateInternalError)) = 0;

virtual void requestFederationRestoreFailed(const char*, const char*)
  RTI_THROW ((FederateInternalError)) = 0;

virtual void federationRestoreBegun()
  RTI_THROW ((FederateInternalError)) = 0;

virtual void initiateFederateRestore (const char*, FederateHandle)
  RTI_THROW ((SpecifiedSaveLabelDoesNotExist, CouldNotRestore, FederateInternalError)) = 0;

virtual void federationRestored()
  RTI_THROW ((FederateInternalError)) = 0;

virtual void federationNotRestored()
  RTI_THROW ((FederateInternalError)) = 0;

// Declaration Management

virtual void startRegistrationForObjectClass(ObjectClassHandle)
  RTI_THROW ((ObjectClassNotPublished, FederateInternalError)) = 0;

virtual void stopRegistrationForObjectClass(ObjectClassHandle)
  RTI_THROW ((ObjectClassNotPublished, FederateInternalError)) = 0;

virtual void turnInteractionsOn(InteractionClassHandle)
  RTI_THROW ((InteractionClassNotPublished, FederateInternalError)) = 0;

virtual void turnInteractionsOff(InteractionClassHandle)
  RTI_THROW ((InteractionClassNotPublished, FederateInternalError)) = 0;

// Object Management

virtual void discoverObjectInstance(ObjectHandle, ObjectClassHandle, const char*)
  RTI_THROW ((CouldNotDiscover, ObjectClassNotKnown, FederateInternalError)) = 0;

virtual void reflectAttributeValues(ObjectHandle, const AttributeHandleValuePairSet&,
				    const FedTime&, const char*, EventRetractionHandle)
  RTI_THROW ((ObjectNotKnown, AttributeNotKnown, FederateOwnsAttributes, InvalidFederationTime, 
         FederateInternalError)) = 0;

virtual void reflectAttributeValues(ObjectHandle, const AttributeHandleValuePairSet&, const char*)
  RTI_THROW ((ObjectNotKnown, AttributeNotKnown, FederateOwnsAttributes, FederateInternalError)) = 0;

virtual void receiveInteraction(InteractionClassHandle, const ParameterHandleValuePairSet&,
				const FedTime&, const char*, EventRetractionHandle)
  RTI_THROW ((InteractionClassNotKnown, InteractionParameterNotKnown, InvalidFederationTime,
         FederateInternalError)) = 0;

virtual void receiveInteraction(InteractionClassHandle, const ParameterHandleValuePairSet&, const char*)
  RTI_THROW ((InteractionClassNotKnown, InteractionParameterNotKnown, FederateInternalError)) = 0;

virtual void removeObjectInstance(ObjectHandle, const FedTime&, const char*, EventRetractionHandle)
  RTI_THROW ((ObjectNotKnown, InvalidFederationTime, FederateInternalError)) = 0;

virtual void removeObjectInstance(ObjectHandle, const char*)
  RTI_THROW ((ObjectNotKnown, FederateInternalError)) = 0;

virtual void attributesInScope (ObjectHandle, const AttributeHandleSet&)
  RTI_THROW ((ObjectNotKnown, AttributeNotKnown, FederateInternalError)) = 0;

virtual void attributesOutOfScope(ObjectHandle, const AttributeHandleSet&)
  RTI_THROW ((ObjectNotKnown, AttributeNotKnown, FederateInternalError)) = 0;

virtual void provideAttributeValueUpdate(ObjectHandle, const AttributeHandleSet&)
  RTI_THROW ((ObjectNotKnown, AttributeNotKnown, AttributeNotOwned, FederateInternalError)) = 0;

virtual void turnUpdatesOnForObjectInstance (ObjectHandle, const AttributeHandleSet&)
  RTI_THROW ((ObjectNotKnown, AttributeNotOwned, FederateInternalError)) = 0;

virtual void turnUpdatesOffForObjectInstance (ObjectHandle, const AttributeHandleSet&)
  RTI_THROW ((ObjectNotKnown, AttributeNotOwned, FederateInternalError)) = 0;

// Ownership Management

virtual void requestAttributeOwnershipAssumption(ObjectHandle, const AttributeHandleSet&, const char*)
  RTI_THROW ((ObjectNotKnown, AttributeNotKnown, AttributeAlreadyOwned, AttributeNotPublished,
         FederateInternalError)) = 0;

virtual void attributeOwnershipDivestitureNotification(ObjectHandle, const AttributeHandleSet&)
  RTI_THROW ((ObjectNotKnown, AttributeNotKnown, AttributeNotOwned, AttributeDivestitureWasNotRequested,
         FederateInternalError)) = 0;

virtual void attributeOwnershipAcquisitionNotification(ObjectHandle, const AttributeHandleSet&)
  RTI_THROW ((ObjectNotKnown, AttributeNotKnown, AttributeAcquisitionWasNotRequested, AttributeAlreadyOwned,
         AttributeNotPublished, FederateInternalError)) = 0;

virtual void attributeOwnershipUnavailable(ObjectHandle, const AttributeHandleSet&)
  RTI_THROW ((ObjectNotKnown, AttributeNotKnown, AttributeNotDefined, AttributeAlreadyOwned,
         AttributeAcquisitionWasNotRequested, FederateInternalError)) = 0;

virtual void requestAttributeOwnershipRelease(ObjectHandle, const AttributeHandleSet&, const char*)
  RTI_THROW ((ObjectNotKnown, AttributeNotKnown, AttributeNotOwned, FederateInternalError)) = 0;

virtual void confirmAttributeOwnershipAcquisitionCancellation(ObjectHandle, const AttributeHandleSet&)
  RTI_THROW ((ObjectNotKnown, AttributeNotKnown, AttributeNotDefined, AttributeAlreadyOwned,
         AttributeAcquisitionWasNotCanceled, FederateInternalError)) = 0;

virtual void informAttributeOwnership(ObjectHandle, AttributeHandle, FederateHandle)
  RTI_THROW ((ObjectNotKnown, AttributeNotKnown, FederateInternalError)) = 0;

virtual void attributeIsNotOwned(ObjectHandle, AttributeHandle)
  RTI_THROW ((ObjectNotKnown, AttributeNotKnown, FederateInternalError)) = 0;

virtual void attributeOwnedByRTI (ObjectHandle, AttributeHandle)
  RTI_THROW ((ObjectNotKnown, AttributeNotKnown, FederateInternalError)) = 0;

// Time Management

virtual void timeRegulationEnabled(const FedTime&)
  RTI_THROW ((InvalidFederationTime, EnableTimeRegulationWasNotPending, FederateInternalError)) = 0;

virtual void timeConstrainedEnabled(const FedTime&)
  RTI_THROW ((InvalidFederationTime, EnableTimeConstrainedWasNotPending, FederateInternalError)) = 0;

virtual void timeAdvanceGrant(const FedTime&)
  RTI_THROW ((InvalidFederationTime, TimeAdvanceWasNotInProgress, FederationTimeAlreadyPassed,
         FederateInternalError)) = 0;

virtual void requestRetraction(EventRetractionHandle)
  RTI_THROW ((EventNotKnown, FederateInternalError)) = 0;

virtual ~FederateAmbassador()
RTI_THROW ((FederateInternalError))
{ }
