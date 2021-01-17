/***********************************************************************
   The IEEE hereby grants a general, royalty-free license to copy, distribute,
   display and make derivative works from this material, for all purposes,
   provided that any use of the material contains the following
   attribution: "Reprinted with permission from IEEE 1516.1(TM)-2010".
   Should you require additional information, contact the Manager, Standards
   Intellectual Property, IEEE Standards Association (stds-ipr@ieee.org).
***********************************************************************/
/***********************************************************************
      IEEE 1516.1 High Level Architecture Interface Specification C++ API
      File: RTI/NullFederateAmbassador.h
***********************************************************************/
#if defined(_WIN32)
#pragma warning(disable : 4100)
#endif

#ifndef RTI_NullFederateAmbassador_h
#define RTI_NullFederateAmbassador_h

#include <RTI/Exception.h>
#include <RTI/FederateAmbassador.h>

namespace rti1516e
{
   class RTI_EXPORT NullFederateAmbassador : public FederateAmbassador
   {
   public:
      NullFederateAmbassador ()
         RTI_THROW ((
            FederateInternalError)) {}

      virtual ~NullFederateAmbassador ()
         RTI_NOEXCEPT {}

      // 4.4
      virtual void connectionLost (
         std::wstring const & faultDescription)
         RTI_THROW ((
            FederateInternalError)) {}

      // 4.8
      virtual void reportFederationExecutions (
         FederationExecutionInformationVector const &
         theFederationExecutionInformationList)
         RTI_THROW ((
            FederateInternalError)) {}

      // 4.12
      virtual void synchronizationPointRegistrationSucceeded (
         std::wstring const & label)
         RTI_THROW ((
            FederateInternalError)) {}

      virtual void synchronizationPointRegistrationFailed (
         std::wstring const & label,
         SynchronizationPointFailureReason reason)
         RTI_THROW ((
            FederateInternalError)) {}

      // 4.13
      virtual void announceSynchronizationPoint (
         std::wstring  const & label,
         VariableLengthData const & theUserSuppliedTag)
         RTI_THROW ((
            FederateInternalError)) {}

      // 4.15
      virtual void federationSynchronized (
         std::wstring const & label,
         FederateHandleSet const& failedToSyncSet)
         RTI_THROW ((
            FederateInternalError)) {}

      // 4.17
      virtual void initiateFederateSave (
         std::wstring const & label)
         RTI_THROW ((
            FederateInternalError)) {}

      virtual void initiateFederateSave (
         std::wstring const & label,
         LogicalTime const & theTime)
         RTI_THROW ((
            FederateInternalError)) {}

      // 4.20
      virtual void federationSaved ()
         RTI_THROW ((
            FederateInternalError)) {}

      virtual void federationNotSaved (
         SaveFailureReason theSaveFailureReason)
         RTI_THROW ((
            FederateInternalError)) {}


      // 4.23
      virtual void federationSaveStatusResponse (
         FederateHandleSaveStatusPairVector const &
         theFederateStatusVector)
         RTI_THROW ((
            FederateInternalError)) {}

      // 4.25
      virtual void requestFederationRestoreSucceeded (
         std::wstring const & label)
         RTI_THROW ((
            FederateInternalError)) {}

      virtual void requestFederationRestoreFailed (
         std::wstring const & label)
         RTI_THROW ((
            FederateInternalError)) {}

      // 4.26
      virtual void federationRestoreBegun ()
         RTI_THROW ((
            FederateInternalError)) {}

      // 4.27
      virtual void initiateFederateRestore (
         std::wstring const & label,
         std::wstring const & federateName,
         FederateHandle handle)
         RTI_THROW ((
            FederateInternalError)) {}

      // 4.29
      virtual void federationRestored ()
         RTI_THROW ((
            FederateInternalError)) {}

      virtual void federationNotRestored (
         RestoreFailureReason theRestoreFailureReason)
         RTI_THROW ((
            FederateInternalError)) {}

      // 4.32
      virtual void federationRestoreStatusResponse (
         FederateRestoreStatusVector const &
         theFederateRestoreStatusVector)
         RTI_THROW ((
            FederateInternalError)) {}

      /////////////////////////////////////
      // Declaration Management Services //
      /////////////////////////////////////

      // 5.10
      virtual void startRegistrationForObjectClass (
         ObjectClassHandle theClass)
         RTI_THROW ((
            FederateInternalError)) {}

      // 5.11
      virtual void stopRegistrationForObjectClass (
         ObjectClassHandle theClass)
         RTI_THROW ((
            FederateInternalError)) {}

      // 5.12
      virtual void turnInteractionsOn (
         InteractionClassHandle theHandle)
         RTI_THROW ((
            FederateInternalError)) {}

      // 5.13
      virtual void turnInteractionsOff (
         InteractionClassHandle theHandle)
         RTI_THROW ((
            FederateInternalError)) {}

      ////////////////////////////////
      // Object Management Services //
      ////////////////////////////////

      // 6.3
      virtual void objectInstanceNameReservationSucceeded (
         std::wstring const & theObjectInstanceName)
         RTI_THROW ((
            FederateInternalError)) {}

      virtual void objectInstanceNameReservationFailed (
         std::wstring const & theObjectInstanceName)
         RTI_THROW ((
            FederateInternalError)) {}

      // 6.6
      virtual void multipleObjectInstanceNameReservationSucceeded (
         std::set<std::wstring> const & theObjectInstanceNames)
         RTI_THROW ((
            FederateInternalError)) {}

      virtual void multipleObjectInstanceNameReservationFailed (
         std::set<std::wstring> const & theObjectInstanceNames)
         RTI_THROW ((
            FederateInternalError)) {}

      // 6.9
      virtual void discoverObjectInstance (
         ObjectInstanceHandle theObject,
         ObjectClassHandle theObjectClass,
         std::wstring const & theObjectInstanceName)
         RTI_THROW ((
            FederateInternalError)) {}

      virtual void discoverObjectInstance (
         ObjectInstanceHandle theObject,
         ObjectClassHandle theObjectClass,
         std::wstring const & theObjectInstanceName,
         FederateHandle producingFederate)
         RTI_THROW ((
            FederateInternalError)) {}

      // 6.11
      virtual void reflectAttributeValues (
         ObjectInstanceHandle theObject,
         AttributeHandleValueMap const & theAttributeValues,
         VariableLengthData const & theUserSuppliedTag,
         OrderType sentOrder,
         TransportationType theType,
         SupplementalReflectInfo theReflectInfo)
         RTI_THROW ((
            FederateInternalError)) {}

      virtual void reflectAttributeValues (
         ObjectInstanceHandle theObject,
         AttributeHandleValueMap const & theAttributeValues,
         VariableLengthData const & theUserSuppliedTag,
         OrderType sentOrder,
         TransportationType theType,
         LogicalTime const & theTime,
         OrderType receivedOrder,
         SupplementalReflectInfo theReflectInfo)
         RTI_THROW ((
            FederateInternalError)) {}

      virtual void reflectAttributeValues (
         ObjectInstanceHandle theObject,
         AttributeHandleValueMap const & theAttributeValues,
         VariableLengthData const & theUserSuppliedTag,
         OrderType sentOrder,
         TransportationType theType,
         LogicalTime const & theTime,
         OrderType receivedOrder,
         MessageRetractionHandle theHandle,
         SupplementalReflectInfo theReflectInfo)
         RTI_THROW ((
            FederateInternalError)) {}

      // 6.13
      virtual void receiveInteraction (
         InteractionClassHandle theInteraction,
         ParameterHandleValueMap const & theParameterValues,
         VariableLengthData const & theUserSuppliedTag,
         OrderType sentOrder,
         TransportationType theType,
         SupplementalReceiveInfo theReceiveInfo)
         RTI_THROW ((
            FederateInternalError)) {}

      virtual void receiveInteraction (
         InteractionClassHandle theInteraction,
         ParameterHandleValueMap const & theParameterValues,
         VariableLengthData const & theUserSuppliedTag,
         OrderType sentOrder,
         TransportationType theType,
         LogicalTime const & theTime,
         OrderType receivedOrder,
         SupplementalReceiveInfo theReceiveInfo)
         RTI_THROW ((
            FederateInternalError)) {}

      virtual void receiveInteraction (
         InteractionClassHandle theInteraction,
         ParameterHandleValueMap const & theParameterValues,
         VariableLengthData const & theUserSuppliedTag,
         OrderType sentOrder,
         TransportationType theType,
         LogicalTime const & theTime,
         OrderType receivedOrder,
         MessageRetractionHandle theHandle,
         SupplementalReceiveInfo theReceiveInfo)
         RTI_THROW ((
            FederateInternalError)) {}

      // 6.15
      virtual void removeObjectInstance (
         ObjectInstanceHandle theObject,
         VariableLengthData const & theUserSuppliedTag,
         OrderType sentOrder,
         SupplementalRemoveInfo theRemoveInfo)
         RTI_THROW ((
            FederateInternalError)) {}

      virtual void removeObjectInstance (
         ObjectInstanceHandle theObject,
         VariableLengthData const & theUserSuppliedTag,
         OrderType sentOrder,
         LogicalTime const & theTime,
         OrderType receivedOrder,
         SupplementalRemoveInfo theRemoveInfo)
         RTI_THROW ((
            FederateInternalError)) {}

      virtual void removeObjectInstance (
         ObjectInstanceHandle theObject,
         VariableLengthData const & theUserSuppliedTag,
         OrderType sentOrder,
         LogicalTime const & theTime,
         OrderType receivedOrder,
         MessageRetractionHandle theHandle,
         SupplementalRemoveInfo theRemoveInfo)
         RTI_THROW ((
            FederateInternalError)) {}

      // 6.17
      virtual void attributesInScope (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes)
         RTI_THROW ((
            FederateInternalError)) {}

      // 6.18
      virtual void attributesOutOfScope (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes)
         RTI_THROW ((
            FederateInternalError)) {}

      // 6.20
      virtual void provideAttributeValueUpdate (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes,
         VariableLengthData const & theUserSuppliedTag)
         RTI_THROW ((
            FederateInternalError)) {}

      // 6.21
      virtual void turnUpdatesOnForObjectInstance (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes)
         RTI_THROW ((
            FederateInternalError)) {}

      virtual void turnUpdatesOnForObjectInstance (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes,
         std::wstring const & updateRateDesignator)
         RTI_THROW ((
            FederateInternalError)) {}

      // 6.22
      virtual void turnUpdatesOffForObjectInstance (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes)
         RTI_THROW ((
            FederateInternalError)) {}

      // 6.24
      virtual void confirmAttributeTransportationTypeChange (
         ObjectInstanceHandle theObject,
         AttributeHandleSet theAttributes,
         TransportationType theTransportation)
         RTI_THROW ((
            FederateInternalError)) {}

      // 6.26
      virtual void reportAttributeTransportationType (
         ObjectInstanceHandle theObject,
         AttributeHandle theAttribute,
         TransportationType theTransportation)
         RTI_THROW ((
            FederateInternalError)) {}

      // 6.28
      virtual void confirmInteractionTransportationTypeChange (
         InteractionClassHandle theInteraction,
         TransportationType theTransportation)
         RTI_THROW ((
            FederateInternalError)) {}

      // 6.30
      virtual void reportInteractionTransportationType (
         FederateHandle federateHandle,
         InteractionClassHandle theInteraction,
         TransportationType  theTransportation)
         RTI_THROW ((
            FederateInternalError)) {}


      ///////////////////////////////////
      // Ownership Management Services //
      ///////////////////////////////////

      // 7.4
      virtual void requestAttributeOwnershipAssumption (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & offeredAttributes,
         VariableLengthData const & theUserSuppliedTag)
         RTI_THROW ((
            FederateInternalError)) {}

      // 7.5
      virtual void requestDivestitureConfirmation (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & releasedAttributes)
         RTI_THROW ((
            FederateInternalError)) {}

      // 7.7
      virtual void attributeOwnershipAcquisitionNotification (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & securedAttributes,
         VariableLengthData const & theUserSuppliedTag)
         RTI_THROW ((
            FederateInternalError)) {}

      // 7.10
      virtual void attributeOwnershipUnavailable (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes)
         RTI_THROW ((
            FederateInternalError)) {}

      // 7.11
      virtual void requestAttributeOwnershipRelease (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & candidateAttributes,
         VariableLengthData const & theUserSuppliedTag)
         RTI_THROW ((
            FederateInternalError)) {}

      // 7.16
      virtual void confirmAttributeOwnershipAcquisitionCancellation (
         ObjectInstanceHandle theObject,
         AttributeHandleSet const & theAttributes)
         RTI_THROW ((
            FederateInternalError)) {}

      // 7.18
      virtual void informAttributeOwnership (
         ObjectInstanceHandle theObject,
         AttributeHandle theAttribute,
         FederateHandle theOwner)
         RTI_THROW ((
            FederateInternalError)) {}

      virtual void attributeIsNotOwned (
         ObjectInstanceHandle theObject,
         AttributeHandle theAttribute)
         RTI_THROW ((
            FederateInternalError)) {}

      virtual void attributeIsOwnedByRTI (
         ObjectInstanceHandle theObject,
         AttributeHandle theAttribute)
         RTI_THROW ((
            FederateInternalError)) {}

      //////////////////////////////
      // Time Management Services //
      //////////////////////////////

      // 8.3
      virtual void timeRegulationEnabled (
         LogicalTime const & theFederateTime)
         RTI_THROW ((
            FederateInternalError)) {}

      // 8.6
      virtual void timeConstrainedEnabled (
         LogicalTime const & theFederateTime)
         RTI_THROW ((
            FederateInternalError)) {}

      // 8.13
      virtual void timeAdvanceGrant (
         LogicalTime const & theTime)
         RTI_THROW ((
            FederateInternalError)) {}

      // 8.22
      virtual void requestRetraction (
         MessageRetractionHandle theHandle)
         RTI_THROW ((
            FederateInternalError)) {}
   };
}

#endif // RTI_NullFederateAmbassador_h
