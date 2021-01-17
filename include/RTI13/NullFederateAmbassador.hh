// HLA 1.3 Header "NullFederateAmbassador.hh"

#ifndef NullFederateAmbassador_h
#define NullFederateAmbassador_h

#include <RTI.hh>

class NullFederateAmbassador : public RTI::FederateAmbassador {
public:
  NullFederateAmbassador() { }
  virtual ~NullFederateAmbassador() 
  RTI_THROW ((RTI::FederateInternalError)) { }
  
  // Federation Management
  
  virtual void synchronizationPointRegistrationSucceeded(const char *) 
    RTI_THROW ((RTI::FederateInternalError)) { }
  
  virtual void synchronizationPointRegistrationFailed(const char *) 
    RTI_THROW ((RTI::FederateInternalError)) { }

  virtual void announceSynchronizationPoint(const char *, const char *) 
    RTI_THROW ((RTI::FederateInternalError)) { }

  virtual void federationSynchronized(const char *) 
    RTI_THROW ((RTI::FederateInternalError)) { }

  virtual void initiateFederateSave(const char *) 
    RTI_THROW ((RTI::UnableToPerformSave, RTI::FederateInternalError)) { }

  virtual void federationSaved()
    RTI_THROW ((RTI::FederateInternalError)) { }

  virtual void federationNotSaved()
    RTI_THROW ((RTI::FederateInternalError)) { }

  virtual void requestFederationRestoreSucceeded(const char *) 
    RTI_THROW ((RTI::FederateInternalError)) { }

  virtual void requestFederationRestoreFailed(const char *, const char *) 
    RTI_THROW ((RTI::FederateInternalError)) { }

  virtual void federationRestoreBegun()
    RTI_THROW ((RTI::FederateInternalError)) { }

  virtual void initiateFederateRestore(const char *, RTI::FederateHandle) 
    RTI_THROW ((RTI::SpecifiedSaveLabelDoesNotExist, RTI::CouldNotRestore, RTI::FederateInternalError)) { }

  virtual void federationRestored()
    RTI_THROW ((RTI::FederateInternalError)) { }

  virtual void federationNotRestored()
    RTI_THROW ((RTI::FederateInternalError)) { }

  // Declaration Management

  virtual void startRegistrationForObjectClass(RTI::ObjectClassHandle) 
    RTI_THROW ((RTI::ObjectClassNotPublished, RTI::FederateInternalError)) { }

  virtual void stopRegistrationForObjectClass(RTI::ObjectClassHandle) 
    RTI_THROW ((RTI::ObjectClassNotPublished, RTI::FederateInternalError)) { }

  virtual void turnInteractionsOn(RTI::InteractionClassHandle) 
    RTI_THROW ((RTI::InteractionClassNotPublished, RTI::FederateInternalError)) { }

  virtual void turnInteractionsOff(RTI::InteractionClassHandle) 
    RTI_THROW ((RTI::InteractionClassNotPublished, RTI::FederateInternalError)) { }

  // Object Management

  virtual void discoverObjectInstance(RTI::ObjectHandle , RTI::ObjectClassHandle , const char *) 
    RTI_THROW ((RTI::CouldNotDiscover, RTI::ObjectClassNotKnown, RTI::FederateInternalError)) { }

  virtual void reflectAttributeValues(RTI::ObjectHandle, const RTI::AttributeHandleValuePairSet &, 
                                      const RTI::FedTime &, const char *, RTI::EventRetractionHandle) 
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateOwnsAttributes,
           RTI::InvalidFederationTime, RTI::FederateInternalError)) { }

  virtual void reflectAttributeValues(RTI::ObjectHandle, const RTI::AttributeHandleValuePairSet &, const char *) 
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateOwnsAttributes,
           RTI::FederateInternalError)) { }

  virtual void receiveInteraction(RTI::InteractionClassHandle, const RTI::ParameterHandleValuePairSet &, 
                                  const RTI::FedTime &, const char *, RTI::EventRetractionHandle) 
    RTI_THROW ((RTI::InteractionClassNotKnown, RTI::InteractionParameterNotKnown, RTI::InvalidFederationTime,
           RTI::FederateInternalError)) { }

  virtual void receiveInteraction(RTI::InteractionClassHandle, const RTI::ParameterHandleValuePairSet &, 
                                  const char *) 
    RTI_THROW ((RTI::InteractionClassNotKnown, RTI::InteractionParameterNotKnown, RTI::FederateInternalError)) { }

  virtual void removeObjectInstance(RTI::ObjectHandle, const RTI::FedTime &, const char *, 
                                    RTI::EventRetractionHandle) 
    RTI_THROW ((RTI::ObjectNotKnown, RTI::InvalidFederationTime, RTI::FederateInternalError)) { }

  virtual void removeObjectInstance(RTI::ObjectHandle, const char *) 
    RTI_THROW ((RTI::ObjectNotKnown, RTI::FederateInternalError)) { }

  virtual void attributesInScope(RTI::ObjectHandle, const RTI::AttributeHandleSet &) 
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateInternalError)) { }

  virtual void attributesOutOfScope(RTI::ObjectHandle, const RTI::AttributeHandleSet &) 
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateInternalError)) { }

  virtual void provideAttributeValueUpdate(RTI::ObjectHandle, const RTI::AttributeHandleSet &) 
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeNotOwned,
           RTI::FederateInternalError)) { }

  virtual void turnUpdatesOnForObjectInstance(RTI::ObjectHandle, const RTI::AttributeHandleSet &) 
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotOwned, RTI::FederateInternalError)) { }

  virtual void turnUpdatesOffForObjectInstance(RTI::ObjectHandle, const RTI::AttributeHandleSet &) 
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotOwned, RTI::FederateInternalError)) { }

  // Ownership Management

  virtual void requestAttributeOwnershipAssumption(RTI::ObjectHandle, const RTI::AttributeHandleSet &, 
                                                   const char *) 
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeAlreadyOwned,
           RTI::AttributeNotPublished, RTI::FederateInternalError)) { }

  virtual void attributeOwnershipDivestitureNotification(RTI::ObjectHandle, const RTI::AttributeHandleSet &) 
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeNotOwned,
           RTI::AttributeDivestitureWasNotRequested, RTI::FederateInternalError)) { }

  virtual void attributeOwnershipAcquisitionNotification(RTI::ObjectHandle, const RTI::AttributeHandleSet &) 
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeAcquisitionWasNotRequested,
           RTI::AttributeAlreadyOwned, RTI::AttributeNotPublished, RTI::FederateInternalError)) { }

  virtual void attributeOwnershipUnavailable(RTI::ObjectHandle, const RTI::AttributeHandleSet &) 
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeAlreadyOwned,
           RTI::AttributeAcquisitionWasNotRequested, RTI::FederateInternalError)) { }

  virtual void requestAttributeOwnershipRelease(RTI::ObjectHandle, const RTI::AttributeHandleSet &, 
                                                const char *) 
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeNotOwned, RTI::FederateInternalError)) { }

  virtual void confirmAttributeOwnershipAcquisitionCancellation(RTI::ObjectHandle,
                                                                const RTI::AttributeHandleSet &) 
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeAlreadyOwned,
           RTI::AttributeAcquisitionWasNotCanceled, RTI::FederateInternalError)) { }

  virtual void informAttributeOwnership(RTI::ObjectHandle, RTI::AttributeHandle, RTI::FederateHandle) 
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateInternalError)) { }

  virtual void attributeIsNotOwned(RTI::ObjectHandle, RTI::AttributeHandle) 
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateInternalError)) { }

  virtual void attributeOwnedByRTI(RTI::ObjectHandle, RTI::AttributeHandle) 
    RTI_THROW ((RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateInternalError)) { }

  // Time Management

  virtual void timeRegulationEnabled(const RTI::FedTime &) 
    RTI_THROW ((RTI::InvalidFederationTime, RTI::EnableTimeRegulationWasNotPending, RTI::FederateInternalError)) { }

  virtual void timeConstrainedEnabled(const RTI::FedTime &) 
    RTI_THROW ((RTI::InvalidFederationTime, RTI::EnableTimeConstrainedWasNotPending, RTI::FederateInternalError)) { }

  virtual void timeAdvanceGrant(const RTI::FedTime &) 
    RTI_THROW ((RTI::InvalidFederationTime, RTI::TimeAdvanceWasNotInProgress, RTI::FederateInternalError)) { }

  virtual void requestRetraction(RTI::EventRetractionHandle) 
    RTI_THROW ((RTI::EventNotKnown, RTI::FederateInternalError)) { }
};

#endif
