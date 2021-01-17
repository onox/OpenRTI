// HLA 1.3 Header "RTIambServices.hh"

typedef FederateAmbassador *FederateAmbassadorPtr ;

// Federation Management -------------------

void createFederationExecution(const char *executionName, const char *FEDid)
    RTI_THROW ((FederationExecutionAlreadyExists, CouldNotOpenFED, ErrorReadingFED,
	   ConcurrentAccessAttempted, RTIinternalError));

void destroyFederationExecution(const char *)
    RTI_THROW ((FederatesCurrentlyJoined, FederationExecutionDoesNotExist,
	   ConcurrentAccessAttempted, RTIinternalError));

FederateHandle joinFederationExecution(const char *, const char *, FederateAmbassadorPtr)
    RTI_THROW ((FederateAlreadyExecutionMember, FederationExecutionDoesNotExist,
	   CouldNotOpenFED, ErrorReadingFED, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void resignFederationExecution(ResignAction)
    RTI_THROW ((FederateOwnsAttributes, FederateNotExecutionMember, InvalidResignAction,
	   ConcurrentAccessAttempted, RTIinternalError));

void registerFederationSynchronizationPoint(const char *, const char *)
    RTI_THROW ((FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void registerFederationSynchronizationPoint(const char *, const char *, const FederateHandleSet &)
    RTI_THROW ((FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError));

void synchronizationPointAchieved(const char *)
    RTI_THROW ((SynchronizationPointLabelWasNotAnnounced, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

void requestFederationSave(const char *, const FedTime &)
    RTI_THROW ((FederationTimeAlreadyPassed, InvalidFederationTime, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

void requestFederationSave(const char *)
    RTI_THROW ((FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError));

void federateSaveBegun()
    RTI_THROW ((SaveNotInitiated, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   RestoreInProgress, RTIinternalError));

void federateSaveComplete()
    RTI_THROW ((SaveNotInitiated, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   RestoreInProgress, RTIinternalError));

void federateSaveNotComplete()
    RTI_THROW ((SaveNotInitiated, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   RestoreInProgress, RTIinternalError));

void requestFederationRestore(const char *)
    RTI_THROW ((FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError));

void federateRestoreComplete()
    RTI_THROW ((RestoreNotRequested, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RTIinternalError));

void federateRestoreNotComplete()
    RTI_THROW ((RestoreNotRequested, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RTIinternalError));

// Declaration Management -------------------

void publishObjectClass(ObjectClassHandle, const AttributeHandleSet &)
    RTI_THROW ((ObjectClassNotDefined, AttributeNotDefined, OwnershipAcquisitionPending,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError));

void unpublishObjectClass(ObjectClassHandle)
    RTI_THROW ((ObjectClassNotDefined, ObjectClassNotPublished, OwnershipAcquisitionPending,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError));

void publishInteractionClass(InteractionClassHandle)
    RTI_THROW ((InteractionClassNotDefined, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void unpublishInteractionClass(InteractionClassHandle)
    RTI_THROW ((InteractionClassNotDefined, InteractionClassNotPublished, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

void subscribeObjectClassAttributes(ObjectClassHandle, const AttributeHandleSet &, Boolean = RTI_TRUE)
    RTI_THROW ((ObjectClassNotDefined, AttributeNotDefined, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

void unsubscribeObjectClass(ObjectClassHandle)
    RTI_THROW ((ObjectClassNotDefined, ObjectClassNotSubscribed, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

void subscribeInteractionClass(InteractionClassHandle, Boolean = RTI_TRUE)
    RTI_THROW ((InteractionClassNotDefined, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   FederateLoggingServiceCalls, SaveInProgress, RestoreInProgress, RTIinternalError));

void unsubscribeInteractionClass(InteractionClassHandle)
    RTI_THROW ((InteractionClassNotDefined, InteractionClassNotSubscribed, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

// Object Management -------------------

ObjectHandle registerObjectInstance(ObjectClassHandle, const char *)
    RTI_THROW ((ObjectClassNotDefined, ObjectClassNotPublished, ObjectAlreadyRegistered,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError));

ObjectHandle registerObjectInstance(ObjectClassHandle)
    RTI_THROW ((ObjectClassNotDefined, ObjectClassNotPublished, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

EventRetractionHandle updateAttributeValues(ObjectHandle, const AttributeHandleValuePairSet &,
					    const FedTime &, const char *)
    RTI_THROW ((ObjectNotKnown, AttributeNotDefined, AttributeNotOwned, InvalidFederationTime,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError));

void updateAttributeValues(ObjectHandle, const AttributeHandleValuePairSet &, const char *)
    RTI_THROW ((ObjectNotKnown, AttributeNotDefined, AttributeNotOwned, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

EventRetractionHandle sendInteraction(InteractionClassHandle, const ParameterHandleValuePairSet &,
				      const FedTime &, const char *)
    RTI_THROW ((InteractionClassNotDefined, InteractionClassNotPublished, InteractionParameterNotDefined,
	   InvalidFederationTime, FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError));

void sendInteraction(InteractionClassHandle, const ParameterHandleValuePairSet &, const char *)
    RTI_THROW ((InteractionClassNotDefined, InteractionClassNotPublished, InteractionParameterNotDefined,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress,
	   RTIinternalError));

EventRetractionHandle deleteObjectInstance(ObjectHandle, const FedTime &, const char *)
    RTI_THROW ((ObjectNotKnown, DeletePrivilegeNotHeld, InvalidFederationTime, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

void deleteObjectInstance(ObjectHandle, const char *)
    RTI_THROW ((ObjectNotKnown, DeletePrivilegeNotHeld, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

void localDeleteObjectInstance(ObjectHandle)
    RTI_THROW ((ObjectNotKnown, FederateOwnsAttributes, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

void changeAttributeTransportationType(ObjectHandle, const AttributeHandleSet &, TransportationHandle)
    RTI_THROW ((ObjectNotKnown, AttributeNotDefined, AttributeNotOwned, InvalidTransportationHandle,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress,
	   RTIinternalError));

void changeInteractionTransportationType(InteractionClassHandle, TransportationHandle)
    RTI_THROW ((InteractionClassNotDefined, InteractionClassNotPublished, InvalidTransportationHandle,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress,
	   RTIinternalError));

void requestObjectAttributeValueUpdate(ObjectHandle, const AttributeHandleSet &)
    RTI_THROW ((ObjectNotKnown, AttributeNotDefined, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void requestClassAttributeValueUpdate(ObjectClassHandle, const AttributeHandleSet &)
    RTI_THROW ((ObjectClassNotDefined, AttributeNotDefined, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

// Ownership Management -------------------

void unconditionalAttributeOwnershipDivestiture(ObjectHandle, const AttributeHandleSet &)
    RTI_THROW ((ObjectNotKnown, AttributeNotDefined, AttributeNotOwned, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

void negotiatedAttributeOwnershipDivestiture(ObjectHandle, const AttributeHandleSet &, const char *)
    RTI_THROW ((ObjectNotKnown, AttributeNotDefined, AttributeNotOwned, AttributeAlreadyBeingDivested,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress,
	   RTIinternalError));

void attributeOwnershipAcquisition(ObjectHandle, const AttributeHandleSet &desiredAttributes, const char *)
    RTI_THROW ((ObjectNotKnown, ObjectClassNotPublished, AttributeNotDefined, AttributeNotPublished,
	   FederateOwnsAttributes, FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError));

void attributeOwnershipAcquisitionIfAvailable(ObjectHandle, const AttributeHandleSet &)
    RTI_THROW ((ObjectNotKnown, ObjectClassNotPublished, AttributeNotDefined, AttributeNotPublished,
	   FederateOwnsAttributes, AttributeAlreadyBeingAcquired, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

AttributeHandleSet *attributeOwnershipReleaseResponse(ObjectHandle, const AttributeHandleSet &)
    RTI_THROW ((ObjectNotKnown, AttributeNotDefined, AttributeNotOwned, FederateWasNotAskedToReleaseAttribute,
	   FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void cancelNegotiatedAttributeOwnershipDivestiture(ObjectHandle, const AttributeHandleSet &)
    RTI_THROW ((ObjectNotKnown, AttributeNotDefined, AttributeNotOwned, AttributeDivestitureWasNotRequested,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress,
	   RTIinternalError));

void cancelAttributeOwnershipAcquisition(ObjectHandle, const AttributeHandleSet &)
    RTI_THROW ((ObjectNotKnown, AttributeNotDefined, AttributeAlreadyOwned,
	   AttributeAcquisitionWasNotRequested, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

void queryAttributeOwnership(ObjectHandle, AttributeHandle)
    RTI_THROW ((ObjectNotKnown, AttributeNotDefined, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

Boolean isAttributeOwnedByFederate(ObjectHandle, AttributeHandle)
    RTI_THROW ((ObjectNotKnown, AttributeNotDefined, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

// Time Management -------------------

void enableTimeRegulation(const FedTime &, const FedTime &)
    RTI_THROW ((TimeRegulationAlreadyEnabled, EnableTimeRegulationPending, TimeAdvanceAlreadyInProgress,
	   InvalidFederationTime, InvalidLookahead, ConcurrentAccessAttempted,
	   FederateNotExecutionMember, SaveInProgress, RestoreInProgress, RTIinternalError));

void disableTimeRegulation()
    RTI_THROW ((TimeRegulationWasNotEnabled, ConcurrentAccessAttempted, FederateNotExecutionMember,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void enableTimeConstrained()
    RTI_THROW ((TimeConstrainedAlreadyEnabled, EnableTimeConstrainedPending, TimeAdvanceAlreadyInProgress,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress,
	   RTIinternalError));

void disableTimeConstrained()
    RTI_THROW ((TimeConstrainedWasNotEnabled, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void timeAdvanceRequest(const FedTime &)
    RTI_THROW ((InvalidFederationTime, FederationTimeAlreadyPassed, TimeAdvanceAlreadyInProgress,
	   EnableTimeRegulationPending, EnableTimeConstrainedPending, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

void timeAdvanceRequestAvailable(const FedTime &)
    RTI_THROW ((InvalidFederationTime, FederationTimeAlreadyPassed, TimeAdvanceAlreadyInProgress,
	   EnableTimeRegulationPending, EnableTimeConstrainedPending, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

void nextEventRequest(const FedTime &)
    RTI_THROW ((InvalidFederationTime, FederationTimeAlreadyPassed, TimeAdvanceAlreadyInProgress,
	   EnableTimeRegulationPending, EnableTimeConstrainedPending, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

void nextEventRequestAvailable(const FedTime &)
    RTI_THROW ((InvalidFederationTime, FederationTimeAlreadyPassed, TimeAdvanceAlreadyInProgress,
	   EnableTimeRegulationPending, EnableTimeConstrainedPending, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

void flushQueueRequest(const FedTime &)
    RTI_THROW ((InvalidFederationTime, FederationTimeAlreadyPassed, TimeAdvanceAlreadyInProgress,
	   EnableTimeRegulationPending, EnableTimeConstrainedPending, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

void enableAsynchronousDelivery()
    RTI_THROW ((AsynchronousDeliveryAlreadyEnabled, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

void disableAsynchronousDelivery()
    RTI_THROW ((AsynchronousDeliveryAlreadyDisabled, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

void queryLBTS(FedTime &)
    RTI_THROW ((FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void queryFederateTime(FedTime &)
    RTI_THROW ((FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void queryMinNextEventTime(FedTime &)
    RTI_THROW ((FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void modifyLookahead(const FedTime &)
    RTI_THROW ((InvalidLookahead, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void queryLookahead(FedTime &)
    RTI_THROW ((FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError));

void retract(EventRetractionHandle theHandle)
    RTI_THROW ((InvalidRetractionHandle, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void changeAttributeOrderType(ObjectHandle, const AttributeHandleSet &, OrderingHandle)
    RTI_THROW ((ObjectNotKnown, AttributeNotDefined, AttributeNotOwned, InvalidOrderingHandle,
	   FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void changeInteractionOrderType(InteractionClassHandle, OrderingHandle)
    RTI_THROW ((InteractionClassNotDefined, InteractionClassNotPublished, InvalidOrderingHandle,
	   FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

// Data Distribution Management -------------------

Region *createRegion(SpaceHandle, ULong)
    RTI_THROW ((SpaceNotDefined, InvalidExtents, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void notifyAboutRegionModification(Region &theRegion)
    RTI_THROW ((RegionNotKnown, InvalidExtents, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void deleteRegion(Region *)
    RTI_THROW ((RegionNotKnown, RegionInUse, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

ObjectHandle registerObjectInstanceWithRegion(ObjectClassHandle, const char *, AttributeHandle [],
					      Region *theRegions[], ULong)
    RTI_THROW ((ObjectClassNotDefined, ObjectClassNotPublished, AttributeNotDefined, AttributeNotPublished,
	   RegionNotKnown, InvalidRegionContext, ObjectAlreadyRegistered, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

ObjectHandle registerObjectInstanceWithRegion(ObjectClassHandle, AttributeHandle [], Region *regions[], ULong)
    RTI_THROW ((ObjectClassNotDefined, ObjectClassNotPublished, AttributeNotDefined, AttributeNotPublished,
	   RegionNotKnown, InvalidRegionContext, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void associateRegionForUpdates(Region &, ObjectHandle, const AttributeHandleSet &)
    RTI_THROW ((ObjectNotKnown, AttributeNotDefined, InvalidRegionContext, RegionNotKnown,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError));

void unassociateRegionForUpdates(Region &, ObjectHandle)
    RTI_THROW ((ObjectNotKnown, InvalidRegionContext, RegionNotKnown, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

void subscribeObjectClassAttributesWithRegion(ObjectClassHandle, Region &, const AttributeHandleSet &,
					      Boolean = RTI_TRUE)
    RTI_THROW ((ObjectClassNotDefined, AttributeNotDefined, RegionNotKnown, InvalidRegionContext,
	   FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void unsubscribeObjectClassWithRegion(ObjectClassHandle, Region &)
    RTI_THROW ((ObjectClassNotDefined, RegionNotKnown, ObjectClassNotSubscribed, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

void subscribeInteractionClassWithRegion(InteractionClassHandle, Region &, Boolean = RTI_TRUE)
    RTI_THROW ((InteractionClassNotDefined, RegionNotKnown, InvalidRegionContext, FederateLoggingServiceCalls,
	   FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void unsubscribeInteractionClassWithRegion(InteractionClassHandle, Region &)
    RTI_THROW ((InteractionClassNotDefined, InteractionClassNotSubscribed, RegionNotKnown,
	   FederateNotExecutionMember, ConcurrentAccessAttempted, SaveInProgress,
	   RestoreInProgress, RTIinternalError));

EventRetractionHandle sendInteractionWithRegion(InteractionClassHandle, const ParameterHandleValuePairSet &,
						const FedTime &, const char *, const Region &)
    RTI_THROW ((InteractionClassNotDefined, InteractionClassNotPublished, InteractionParameterNotDefined,
	   InvalidFederationTime, RegionNotKnown, InvalidRegionContext, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

void sendInteractionWithRegion(InteractionClassHandle, const ParameterHandleValuePairSet &,
			       const char *, const Region &)
    RTI_THROW ((InteractionClassNotDefined, InteractionClassNotPublished, InteractionParameterNotDefined,
	   RegionNotKnown, InvalidRegionContext, FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void requestClassAttributeValueUpdateWithRegion(ObjectClassHandle, const AttributeHandleSet &, const Region &)
    RTI_THROW ((ObjectClassNotDefined, AttributeNotDefined, RegionNotKnown, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, SaveInProgress, RestoreInProgress, RTIinternalError));

// Support Services -------------------

ObjectClassHandle getObjectClassHandle(const char *)
    RTI_THROW ((NameNotFound, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError));

char *getObjectClassName(ObjectClassHandle)
    RTI_THROW ((ObjectClassNotDefined, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError));

AttributeHandle getAttributeHandle(const char *, ObjectClassHandle)
    RTI_THROW ((ObjectClassNotDefined, NameNotFound, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, RTIinternalError));

char *getAttributeName(AttributeHandle, ObjectClassHandle)
    RTI_THROW ((ObjectClassNotDefined, AttributeNotDefined, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, RTIinternalError));

InteractionClassHandle getInteractionClassHandle(const char *)
    RTI_THROW ((NameNotFound, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError));

char *getInteractionClassName(InteractionClassHandle)
    RTI_THROW ((InteractionClassNotDefined, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError));

ParameterHandle getParameterHandle(const char *, InteractionClassHandle)
    RTI_THROW ((InteractionClassNotDefined, NameNotFound, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, RTIinternalError));

char *getParameterName(ParameterHandle, InteractionClassHandle)
    RTI_THROW ((InteractionClassNotDefined, InteractionParameterNotDefined, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, RTIinternalError));

ObjectHandle getObjectInstanceHandle(const char *)
    RTI_THROW ((ObjectNotKnown, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError));

char *getObjectInstanceName(ObjectHandle)
    RTI_THROW ((ObjectNotKnown, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError));

SpaceHandle getRoutingSpaceHandle(const char *)
    RTI_THROW ((NameNotFound, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError));

char *getRoutingSpaceName(SpaceHandle)
    RTI_THROW ((SpaceNotDefined, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError));

DimensionHandle getDimensionHandle(const char *, SpaceHandle)
    RTI_THROW ((SpaceNotDefined, NameNotFound, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, RTIinternalError));

char *getDimensionName(DimensionHandle, SpaceHandle)
    RTI_THROW ((SpaceNotDefined, DimensionNotDefined, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, RTIinternalError));

SpaceHandle getAttributeRoutingSpaceHandle(AttributeHandle, ObjectClassHandle)
    RTI_THROW ((ObjectClassNotDefined, AttributeNotDefined, FederateNotExecutionMember,
	   ConcurrentAccessAttempted, RTIinternalError));

ObjectClassHandle getObjectClass(ObjectHandle)
    RTI_THROW ((ObjectNotKnown, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError));

SpaceHandle getInteractionRoutingSpaceHandle(InteractionClassHandle)
    RTI_THROW ((InteractionClassNotDefined, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError));

TransportationHandle getTransportationHandle(const char *)
    RTI_THROW ((NameNotFound, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError));

char *getTransportationName(TransportationHandle)
    RTI_THROW ((InvalidTransportationHandle, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError));

OrderingHandle getOrderingHandle(const char *)
    RTI_THROW ((NameNotFound, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError));

char *getOrderingName(OrderingHandle)
    RTI_THROW ((InvalidOrderingHandle, FederateNotExecutionMember, ConcurrentAccessAttempted, RTIinternalError));

void enableClassRelevanceAdvisorySwitch()
    RTI_THROW ((FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void disableClassRelevanceAdvisorySwitch()
    RTI_THROW ((FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void enableAttributeRelevanceAdvisorySwitch()
    RTI_THROW ((FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void disableAttributeRelevanceAdvisorySwitch()
    RTI_THROW ((FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void enableAttributeScopeAdvisorySwitch()
    RTI_THROW ((FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void disableAttributeScopeAdvisorySwitch()
    RTI_THROW ((FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void enableInteractionRelevanceAdvisorySwitch()
    RTI_THROW ((FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

void disableInteractionRelevanceAdvisorySwitch()
    RTI_THROW ((FederateNotExecutionMember, ConcurrentAccessAttempted,
	   SaveInProgress, RestoreInProgress, RTIinternalError));

Boolean tick()
    RTI_THROW ((SpecifiedSaveLabelDoesNotExist, ConcurrentAccessAttempted, RTIinternalError));

Boolean tick(TickTime, TickTime)
    RTI_THROW ((SpecifiedSaveLabelDoesNotExist, ConcurrentAccessAttempted, RTIinternalError));

RegionToken getRegionToken(Region *)
    RTI_THROW ((FederateNotExecutionMember, ConcurrentAccessAttempted, RegionNotKnown, RTIinternalError));

Region *getRegion(RegionToken)
    RTI_THROW ((FederateNotExecutionMember, ConcurrentAccessAttempted, RegionNotKnown, RTIinternalError));

RTIambassador()
    RTI_THROW ((MemoryExhausted, RTIinternalError));

~RTIambassador()
    RTI_THROW ((RTIinternalError));
