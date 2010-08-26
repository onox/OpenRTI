// HLA 1.3 Header "federateAmbServices.hh"

// Federation Management

virtual void synchronizationPointRegistrationSucceeded(const char*)
  throw (FederateInternalError) = 0;

virtual void synchronizationPointRegistrationFailed(const char*)
  throw (FederateInternalError) = 0;

virtual void announceSynchronizationPoint(const char*, const char*)
  throw (FederateInternalError) = 0;

virtual void federationSynchronized(const char*)
  throw (FederateInternalError) = 0;

virtual void initiateFederateSave (const char*)
  throw (UnableToPerformSave, FederateInternalError) = 0;

virtual void federationSaved()
  throw (FederateInternalError) = 0;

virtual void federationNotSaved ()
  throw (FederateInternalError) = 0;

virtual void requestFederationRestoreSucceeded (const char*)
  throw (FederateInternalError) = 0;

virtual void requestFederationRestoreFailed(const char*, const char*)
  throw (FederateInternalError) = 0;

virtual void federationRestoreBegun()
  throw (FederateInternalError) = 0;

virtual void initiateFederateRestore (const char*, FederateHandle)
  throw (SpecifiedSaveLabelDoesNotExist, CouldNotRestore, FederateInternalError) = 0;

virtual void federationRestored()
  throw (FederateInternalError) = 0;

virtual void federationNotRestored()
  throw (FederateInternalError) = 0;

// Declaration Management

virtual void startRegistrationForObjectClass(ObjectClassHandle)
  throw (ObjectClassNotPublished, FederateInternalError) = 0;

virtual void stopRegistrationForObjectClass(ObjectClassHandle)
  throw (ObjectClassNotPublished, FederateInternalError) = 0;

virtual void turnInteractionsOn(InteractionClassHandle)
  throw (InteractionClassNotPublished, FederateInternalError) = 0;

virtual void turnInteractionsOff(InteractionClassHandle)
  throw (InteractionClassNotPublished, FederateInternalError) = 0;

// Object Management

virtual void discoverObjectInstance(ObjectHandle, ObjectClassHandle, const char*)
  throw (CouldNotDiscover, ObjectClassNotKnown, FederateInternalError) = 0;

virtual void reflectAttributeValues(ObjectHandle, const AttributeHandleValuePairSet&,
				    const FedTime&, const char*, EventRetractionHandle)
  throw (ObjectNotKnown, AttributeNotKnown, FederateOwnsAttributes, InvalidFederationTime, 
         FederateInternalError) = 0;

virtual void reflectAttributeValues(ObjectHandle, const AttributeHandleValuePairSet&, const char*)
  throw (ObjectNotKnown, AttributeNotKnown, FederateOwnsAttributes, FederateInternalError) = 0;

virtual void receiveInteraction(InteractionClassHandle, const ParameterHandleValuePairSet&,
				const FedTime&, const char*, EventRetractionHandle)
  throw (InteractionClassNotKnown, InteractionParameterNotKnown, InvalidFederationTime,
         FederateInternalError) = 0;

virtual void receiveInteraction(InteractionClassHandle, const ParameterHandleValuePairSet&, const char*)
  throw (InteractionClassNotKnown, InteractionParameterNotKnown, FederateInternalError) = 0;

virtual void removeObjectInstance(ObjectHandle, const FedTime&, const char*, EventRetractionHandle)
  throw (ObjectNotKnown, InvalidFederationTime, FederateInternalError) = 0;

virtual void removeObjectInstance(ObjectHandle, const char*)
  throw (ObjectNotKnown, FederateInternalError) = 0;

virtual void attributesInScope (ObjectHandle, const AttributeHandleSet&)
  throw (ObjectNotKnown, AttributeNotKnown, FederateInternalError) = 0;

virtual void attributesOutOfScope(ObjectHandle, const AttributeHandleSet&)
  throw (ObjectNotKnown, AttributeNotKnown, FederateInternalError) = 0;

virtual void provideAttributeValueUpdate(ObjectHandle, const AttributeHandleSet&)
  throw (ObjectNotKnown, AttributeNotKnown, AttributeNotOwned, FederateInternalError) = 0;

virtual void turnUpdatesOnForObjectInstance (ObjectHandle, const AttributeHandleSet&)
  throw (ObjectNotKnown, AttributeNotOwned, FederateInternalError) = 0;

virtual void turnUpdatesOffForObjectInstance (ObjectHandle, const AttributeHandleSet&)
  throw (ObjectNotKnown, AttributeNotOwned, FederateInternalError) = 0;

// Ownership Management

virtual void requestAttributeOwnershipAssumption(ObjectHandle, const AttributeHandleSet&, const char*)
  throw (ObjectNotKnown, AttributeNotKnown, AttributeAlreadyOwned, AttributeNotPublished,
         FederateInternalError) = 0;

virtual void attributeOwnershipDivestitureNotification(ObjectHandle, const AttributeHandleSet&)
  throw (ObjectNotKnown, AttributeNotKnown, AttributeNotOwned, AttributeDivestitureWasNotRequested,
         FederateInternalError) = 0;

virtual void attributeOwnershipAcquisitionNotification(ObjectHandle, const AttributeHandleSet&)
  throw (ObjectNotKnown, AttributeNotKnown, AttributeAcquisitionWasNotRequested, AttributeAlreadyOwned,
         AttributeNotPublished, FederateInternalError) = 0;

virtual void attributeOwnershipUnavailable(ObjectHandle, const AttributeHandleSet&)
  throw (ObjectNotKnown, AttributeNotKnown, AttributeNotDefined, AttributeAlreadyOwned,
         AttributeAcquisitionWasNotRequested, FederateInternalError) = 0;

virtual void requestAttributeOwnershipRelease(ObjectHandle, const AttributeHandleSet&, const char*)
  throw (ObjectNotKnown, AttributeNotKnown, AttributeNotOwned, FederateInternalError) = 0;

virtual void confirmAttributeOwnershipAcquisitionCancellation(ObjectHandle, const AttributeHandleSet&)
  throw (ObjectNotKnown, AttributeNotKnown, AttributeNotDefined, AttributeAlreadyOwned,
         AttributeAcquisitionWasNotCanceled, FederateInternalError) = 0;

virtual void informAttributeOwnership(ObjectHandle, AttributeHandle, FederateHandle)
  throw (ObjectNotKnown, AttributeNotKnown, FederateInternalError) = 0;

virtual void attributeIsNotOwned(ObjectHandle, AttributeHandle)
  throw (ObjectNotKnown, AttributeNotKnown, FederateInternalError) = 0;

virtual void attributeOwnedByRTI (ObjectHandle, AttributeHandle)
  throw (ObjectNotKnown, AttributeNotKnown, FederateInternalError) = 0;

// Time Management

virtual void timeRegulationEnabled(const FedTime&)
  throw (InvalidFederationTime, EnableTimeRegulationWasNotPending, FederateInternalError) = 0;

virtual void timeConstrainedEnabled(const FedTime&)
  throw (InvalidFederationTime, EnableTimeConstrainedWasNotPending, FederateInternalError) = 0;

virtual void timeAdvanceGrant(const FedTime&)
  throw (InvalidFederationTime, TimeAdvanceWasNotInProgress, FederationTimeAlreadyPassed,
         FederateInternalError) = 0;

virtual void requestRetraction(EventRetractionHandle)
  throw (EventNotKnown, FederateInternalError) = 0;

virtual ~FederateAmbassador()
throw (FederateInternalError)
{ }
