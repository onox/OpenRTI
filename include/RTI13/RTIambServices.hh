// HLA 1.3 Header "RTIambServices.hh"

typedef FederateAmbassador *FederateAmbassadorPtr ;

// Federation Management -------------------

void createFederationExecution(const char *executionName, const char *FEDid)
    throw (FederationExecutionAlreadyExists, CouldNotOpenFED, ErrorReadingFED,
	   ConcurrentAccessAttempted, RTIinternalError);

void destroyFederationExecution(const char *)
    throw (FederatesCurrentlyJoined, FederationExecutionDoesNotExist,
	   ConcurrentAccessAttempted, RTIinternalError);

FederateHandle joinFederationExecution(const char *, const char *, FederateAmbassadorPtr)
    throw (FederateAlreadyExecutionMember, FederationExecutionDoesNotExist,
	   CouldNotOpenFED, ErrorReadingFED, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void resignFederationExecution(ResignAction)
    throw (FederateOwnsAttributes, FederateNotExecutionMember, InvalidResignAction,
	   ConcurrentAccessAttempted, RTIinternalError);

void registerFederationSynchronizationPoint(const char *, const char *)
    throw (FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void registerFederationSynchronizationPoint(const char *, const char *, const FederateHandleSet &)
    throw (FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError);

void synchronizationPointAchieved(const char *)
    throw (SynchronizationPointLabelWasNotAnnounced, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

void requestFederationSave(const char *, const FedTime &)
    throw (FederationTimeAlreadyPassed, InvalidFederationTime, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

void requestFederationSave(const char *)
    throw (FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError);

void federateSaveBegun()
    throw (SaveNotInitiated, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   RestoreInProgress, RTIinternalError);

void federateSaveComplete()
    throw (SaveNotInitiated, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   RestoreInProgress, RTIinternalError);

void federateSaveNotComplete()
    throw (SaveNotInitiated, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   RestoreInProgress, RTIinternalError);

void requestFederationRestore(const char *)
    throw (FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError);

void federateRestoreComplete()
    throw (RestoreNotRequested, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RTIinternalError);

void federateRestoreNotComplete()
    throw (RestoreNotRequested, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RTIinternalError);

// Declaration Management -------------------

void publishObjectClass(ObjectClassHandle, const AttributeHandleSet &)
    throw (ObjectClassNotDefined, AttributeNotDefined, OwnershipAcquisitionPending,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError);

void unpublishObjectClass(ObjectClassHandle)
    throw (ObjectClassNotDefined, ObjectClassNotPublished, OwnershipAcquisitionPending,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError);

void publishInteractionClass(InteractionClassHandle)
    throw (InteractionClassNotDefined, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void unpublishInteractionClass(InteractionClassHandle)
    throw (InteractionClassNotDefined, InteractionClassNotPublished, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

void subscribeObjectClassAttributes(ObjectClassHandle, const AttributeHandleSet &, Boolean = RTI_TRUE)
    throw (ObjectClassNotDefined, AttributeNotDefined, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

void unsubscribeObjectClass(ObjectClassHandle)
    throw (ObjectClassNotDefined, ObjectClassNotSubscribed, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

void subscribeInteractionClass(InteractionClassHandle, Boolean = RTI_TRUE)
    throw (InteractionClassNotDefined, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   FederateLoggingServiceCalls, SaveInProgress, RestoreInProgress, RTIinternalError);

void unsubscribeInteractionClass(InteractionClassHandle)
    throw (InteractionClassNotDefined, InteractionClassNotSubscribed, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

// Object Management -------------------

ObjectHandle registerObjectInstance(ObjectClassHandle, const char *)
    throw (ObjectClassNotDefined, ObjectClassNotPublished, ObjectAlreadyRegistered,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError);

ObjectHandle registerObjectInstance(ObjectClassHandle)
    throw (ObjectClassNotDefined, ObjectClassNotPublished, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

EventRetractionHandle updateAttributeValues(ObjectHandle, const AttributeHandleValuePairSet &,
					    const FedTime &, const char *)
    throw (ObjectNotKnown, AttributeNotDefined, AttributeNotOwned, InvalidFederationTime,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError);

void updateAttributeValues(ObjectHandle, const AttributeHandleValuePairSet &, const char *)
    throw (ObjectNotKnown, AttributeNotDefined, AttributeNotOwned, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

EventRetractionHandle sendInteraction(InteractionClassHandle, const ParameterHandleValuePairSet &,
				      const FedTime &, const char *)
    throw (InteractionClassNotDefined, InteractionClassNotPublished, InteractionParameterNotDefined,
	   InvalidFederationTime, FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError);

void sendInteraction(InteractionClassHandle, const ParameterHandleValuePairSet &, const char *)
    throw (InteractionClassNotDefined, InteractionClassNotPublished, InteractionParameterNotDefined,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress,
	   RTIinternalError);

EventRetractionHandle deleteObjectInstance(ObjectHandle, const FedTime &, const char *)
    throw (ObjectNotKnown, DeletePrivilegeNotHeld, InvalidFederationTime, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

void deleteObjectInstance(ObjectHandle, const char *)
    throw (ObjectNotKnown, DeletePrivilegeNotHeld, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

void localDeleteObjectInstance(ObjectHandle)
    throw (ObjectNotKnown, FederateOwnsAttributes, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

void changeAttributeTransportationType(ObjectHandle, const AttributeHandleSet &, TransportationHandle)
    throw (ObjectNotKnown, AttributeNotDefined, AttributeNotOwned, InvalidTransportationHandle,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress,
	   RTIinternalError);

void changeInteractionTransportationType(InteractionClassHandle, TransportationHandle)
    throw (InteractionClassNotDefined, InteractionClassNotPublished, InvalidTransportationHandle,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress,
	   RTIinternalError);

void requestObjectAttributeValueUpdate(ObjectHandle, const AttributeHandleSet &)
    throw (ObjectNotKnown, AttributeNotDefined, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void requestClassAttributeValueUpdate(ObjectClassHandle, const AttributeHandleSet &)
    throw (ObjectClassNotDefined, AttributeNotDefined, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

// Ownership Management -------------------

void unconditionalAttributeOwnershipDivestiture(ObjectHandle, const AttributeHandleSet &)
    throw (ObjectNotKnown, AttributeNotDefined, AttributeNotOwned, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

void negotiatedAttributeOwnershipDivestiture(ObjectHandle, const AttributeHandleSet &, const char *)
    throw (ObjectNotKnown, AttributeNotDefined, AttributeNotOwned, AttributeAlreadyBeingDivested,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress,
	   RTIinternalError);

void attributeOwnershipAcquisition(ObjectHandle, const AttributeHandleSet &desiredAttributes, const char *)
    throw (ObjectNotKnown, ObjectClassNotPublished, AttributeNotDefined, AttributeNotPublished,
	   FederateOwnsAttributes, FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError);

void attributeOwnershipAcquisitionIfAvailable(ObjectHandle, const AttributeHandleSet &)
    throw (ObjectNotKnown, ObjectClassNotPublished, AttributeNotDefined, AttributeNotPublished,
	   FederateOwnsAttributes, AttributeAlreadyBeingAcquired, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

AttributeHandleSet *attributeOwnershipReleaseResponse(ObjectHandle, const AttributeHandleSet &)
    throw (ObjectNotKnown, AttributeNotDefined, AttributeNotOwned, FederateWasNotAskedToReleaseAttribute,
	   FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void cancelNegotiatedAttributeOwnershipDivestiture(ObjectHandle, const AttributeHandleSet &)
    throw (ObjectNotKnown, AttributeNotDefined, AttributeNotOwned, AttributeDivestitureWasNotRequested,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress,
	   RTIinternalError);

void cancelAttributeOwnershipAcquisition(ObjectHandle, const AttributeHandleSet &)
    throw (ObjectNotKnown, AttributeNotDefined, AttributeAlreadyOwned,
	   AttributeAcquisitionWasNotRequested, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

void queryAttributeOwnership(ObjectHandle, AttributeHandle)
    throw (ObjectNotKnown, AttributeNotDefined, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

Boolean isAttributeOwnedByFederate(ObjectHandle, AttributeHandle)
    throw (ObjectNotKnown, AttributeNotDefined, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

// Time Management -------------------

void enableTimeRegulation(const FedTime &, const FedTime &)
    throw (TimeRegulationAlreadyEnabled, EnableTimeRegulationPending, TimeAdvanceAlreadyInProgress,
	   InvalidFederationTime, InvalidLookahead, ConcurrentAccessAttempted,
	   FederateNotExecutionMember, SaveInProgress, RestoreInProgress, RTIinternalError);

void disableTimeRegulation()
    throw (TimeRegulationWasNotEnabled, ConcurrentAccessAttempted, FederateNotExecutionMember,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void enableTimeConstrained()
    throw (TimeConstrainedAlreadyEnabled, EnableTimeConstrainedPending, TimeAdvanceAlreadyInProgress,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress,
	   RTIinternalError);

void disableTimeConstrained()
    throw (TimeConstrainedWasNotEnabled, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void timeAdvanceRequest(const FedTime &)
    throw (InvalidFederationTime, FederationTimeAlreadyPassed, TimeAdvanceAlreadyInProgress,
	   EnableTimeRegulationPending, EnableTimeConstrainedPending, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

void timeAdvanceRequestAvailable(const FedTime &)
    throw (InvalidFederationTime, FederationTimeAlreadyPassed, TimeAdvanceAlreadyInProgress,
	   EnableTimeRegulationPending, EnableTimeConstrainedPending, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

void nextEventRequest(const FedTime &)
    throw (InvalidFederationTime, FederationTimeAlreadyPassed, TimeAdvanceAlreadyInProgress,
	   EnableTimeRegulationPending, EnableTimeConstrainedPending, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

void nextEventRequestAvailable(const FedTime &)
    throw (InvalidFederationTime, FederationTimeAlreadyPassed, TimeAdvanceAlreadyInProgress,
	   EnableTimeRegulationPending, EnableTimeConstrainedPending, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

void flushQueueRequest(const FedTime &)
    throw (InvalidFederationTime, FederationTimeAlreadyPassed, TimeAdvanceAlreadyInProgress,
	   EnableTimeRegulationPending, EnableTimeConstrainedPending, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

void enableAsynchronousDelivery()
    throw (AsynchronousDeliveryAlreadyEnabled, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

void disableAsynchronousDelivery()
    throw (AsynchronousDeliveryAlreadyDisabled, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

void queryLBTS(FedTime &)
    throw (FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void queryFederateTime(FedTime &)
    throw (FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void queryMinNextEventTime(FedTime &)
    throw (FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void modifyLookahead(const FedTime &)
    throw (InvalidLookahead, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void queryLookahead(FedTime &)
    throw (FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError);

void retract(EventRetractionHandle theHandle)
    throw (InvalidRetractionHandle, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void changeAttributeOrderType(ObjectHandle, const AttributeHandleSet &, OrderingHandle)
    throw (ObjectNotKnown, AttributeNotDefined, AttributeNotOwned, InvalidOrderingHandle,
	   FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void changeInteractionOrderType(InteractionClassHandle, OrderingHandle)
    throw (InteractionClassNotDefined, InteractionClassNotPublished, InvalidOrderingHandle,
	   FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

// Data Distribution Management -------------------

Region *createRegion(SpaceHandle, ULong)
    throw (SpaceNotDefined, InvalidExtents, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void notifyAboutRegionModification(Region &theRegion)
    throw (RegionNotKnown, InvalidExtents, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void deleteRegion(Region *)
    throw (RegionNotKnown, RegionInUse, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

ObjectHandle registerObjectInstanceWithRegion(ObjectClassHandle, const char *, AttributeHandle [],
					      Region *theRegions[], ULong)
    throw (ObjectClassNotDefined, ObjectClassNotPublished, AttributeNotDefined, AttributeNotPublished,
	   RegionNotKnown, InvalidRegionContext, ObjectAlreadyRegistered, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

ObjectHandle registerObjectInstanceWithRegion(ObjectClassHandle, AttributeHandle [], Region *regions[], ULong)
    throw (ObjectClassNotDefined, ObjectClassNotPublished, AttributeNotDefined, AttributeNotPublished,
	   RegionNotKnown, InvalidRegionContext, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void associateRegionForUpdates(Region &, ObjectHandle, const AttributeHandleSet &)
    throw (ObjectNotKnown, AttributeNotDefined, InvalidRegionContext, RegionNotKnown,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError);

void unassociateRegionForUpdates(Region &, ObjectHandle)
    throw (ObjectNotKnown, InvalidRegionContext, RegionNotKnown, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

void subscribeObjectClassAttributesWithRegion(ObjectClassHandle, Region &, const AttributeHandleSet &,
					      Boolean = RTI_TRUE)
    throw (ObjectClassNotDefined, AttributeNotDefined, RegionNotKnown, InvalidRegionContext,
	   FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void unsubscribeObjectClassWithRegion(ObjectClassHandle, Region &)
    throw (ObjectClassNotDefined, RegionNotKnown, ObjectClassNotSubscribed, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

void subscribeInteractionClassWithRegion(InteractionClassHandle, Region &, Boolean = RTI_TRUE)
    throw (InteractionClassNotDefined, RegionNotKnown, InvalidRegionContext, FederateLoggingServiceCalls,
	   FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void unsubscribeInteractionClassWithRegion(InteractionClassHandle, Region &)
    throw (InteractionClassNotDefined, InteractionClassNotSubscribed, RegionNotKnown,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError);

EventRetractionHandle sendInteractionWithRegion(InteractionClassHandle, const ParameterHandleValuePairSet &,
						const FedTime &, const char *, const Region &)
    throw (InteractionClassNotDefined, InteractionClassNotPublished, InteractionParameterNotDefined,
	   InvalidFederationTime, RegionNotKnown, InvalidRegionContext, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

void sendInteractionWithRegion(InteractionClassHandle, const ParameterHandleValuePairSet &,
			       const char *, const Region &)
    throw (InteractionClassNotDefined, InteractionClassNotPublished, InteractionParameterNotDefined,
	   RegionNotKnown, InvalidRegionContext, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void requestClassAttributeValueUpdateWithRegion(ObjectClassHandle, const AttributeHandleSet &, const Region &)
    throw (ObjectClassNotDefined, AttributeNotDefined, RegionNotKnown, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError);

// Support Services -------------------

ObjectClassHandle getObjectClassHandle(const char *)
    throw (NameNotFound, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError);

char *getObjectClassName(ObjectClassHandle)
    throw (ObjectClassNotDefined, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError);

AttributeHandle getAttributeHandle(const char *, ObjectClassHandle)
    throw (ObjectClassNotDefined, NameNotFound, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, RTIinternalError);

char *getAttributeName(AttributeHandle, ObjectClassHandle)
    throw (ObjectClassNotDefined, AttributeNotDefined, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, RTIinternalError);

InteractionClassHandle getInteractionClassHandle(const char *)
    throw (NameNotFound, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError);

char *getInteractionClassName(InteractionClassHandle)
    throw (InteractionClassNotDefined, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError);

ParameterHandle getParameterHandle(const char *, InteractionClassHandle)
    throw (InteractionClassNotDefined, NameNotFound, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, RTIinternalError);

char *getParameterName(ParameterHandle, InteractionClassHandle)
    throw (InteractionClassNotDefined, InteractionParameterNotDefined, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, RTIinternalError);

ObjectHandle getObjectInstanceHandle(const char *)
    throw (ObjectNotKnown, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError);

char *getObjectInstanceName(ObjectHandle)
    throw (ObjectNotKnown, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError);

SpaceHandle getRoutingSpaceHandle(const char *)
    throw (NameNotFound, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError);

char *getRoutingSpaceName(SpaceHandle)
    throw (SpaceNotDefined, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError);

DimensionHandle getDimensionHandle(const char *, SpaceHandle)
    throw (SpaceNotDefined, NameNotFound, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, RTIinternalError);

char *getDimensionName(DimensionHandle, SpaceHandle)
    throw (SpaceNotDefined, DimensionNotDefined, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, RTIinternalError);

SpaceHandle getAttributeRoutingSpaceHandle(AttributeHandle, ObjectClassHandle)
    throw (ObjectClassNotDefined, AttributeNotDefined, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, RTIinternalError);

ObjectClassHandle getObjectClass(ObjectHandle)
    throw (ObjectNotKnown, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError);

SpaceHandle getInteractionRoutingSpaceHandle(InteractionClassHandle)
    throw (InteractionClassNotDefined, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError);

TransportationHandle getTransportationHandle(const char *)
    throw (NameNotFound, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError);

char *getTransportationName(TransportationHandle)
    throw (InvalidTransportationHandle, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError);

OrderingHandle getOrderingHandle(const char *)
    throw (NameNotFound, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError);

char *getOrderingName(OrderingHandle)
    throw (InvalidOrderingHandle, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError);

void enableClassRelevanceAdvisorySwitch()
    throw (FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void disableClassRelevanceAdvisorySwitch()
    throw (FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void enableAttributeRelevanceAdvisorySwitch()
    throw (FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void disableAttributeRelevanceAdvisorySwitch()
    throw (FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void enableAttributeScopeAdvisorySwitch()
    throw (FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void disableAttributeScopeAdvisorySwitch()
    throw (FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void enableInteractionRelevanceAdvisorySwitch()
    throw (FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

void disableInteractionRelevanceAdvisorySwitch()
    throw (FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError);

Boolean tick()
    throw (SpecifiedSaveLabelDoesNotExist, ConcurrentAccessAttempted, RTIinternalError);

Boolean tick(TickTime, TickTime)
    throw (SpecifiedSaveLabelDoesNotExist, ConcurrentAccessAttempted, RTIinternalError);

RegionToken getRegionToken(Region *)
    throw (FederateNotExecutionMember, ConcurrentAccessAttempted, RegionNotKnown, RTIinternalError);

Region *getRegion(RegionToken)
    throw (FederateNotExecutionMember, ConcurrentAccessAttempted, RegionNotKnown, RTIinternalError);

RTIambassador()
    throw (MemoryExhausted, RTIinternalError);

~RTIambassador()
    throw (RTIinternalError);
