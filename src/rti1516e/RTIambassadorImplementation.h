/* -*-c++-*- OpenRTI - Copyright (C) 2009-2022 Mathias Froehlich
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

#ifndef RTIambassadorImplementation_h
#define RTIambassadorImplementation_h

#include <Export.h>
#include <RTI/RTIambassador.h>

namespace OpenRTI {

class OPENRTI_LOCAL RTIambassadorImplementation : public rti1516e::RTIambassador {
public:
  RTIambassadorImplementation() RTI_NOEXCEPT;
  virtual ~RTIambassadorImplementation();

  virtual void
  connect(rti1516e::FederateAmbassador & federateAmbassador, rti1516e::CallbackModel theCallbackModel,
          std::wstring const & localSettingsDesignator)
    RTI_THROW ((rti1516e::ConnectionFailed,
           rti1516e::InvalidLocalSettingsDesignator,
           rti1516e::UnsupportedCallbackModel,
           rti1516e::AlreadyConnected,
           rti1516e::CallNotAllowedFromWithinCallback,
           rti1516e::RTIinternalError));

  virtual void
  disconnect()
    RTI_THROW ((rti1516e::FederateIsExecutionMember,
           rti1516e::CallNotAllowedFromWithinCallback,
           rti1516e::RTIinternalError));

  virtual void
  createFederationExecution(std::wstring const & federationExecutionName,
                            std::wstring const & fomModule,
                            std::wstring const & logicalTimeImplementationName)
    RTI_THROW ((rti1516e::CouldNotCreateLogicalTimeFactory,
           rti1516e::InconsistentFDD,
           rti1516e::CouldNotOpenFDD,
           rti1516e::ErrorReadingFDD,
           rti1516e::FederationExecutionAlreadyExists,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual void
  createFederationExecution(std::wstring const & federationExecutionName,
                            std::vector<std::wstring> const & fomModules,
                            std::wstring const & logicalTimeImplementationName)
    RTI_THROW ((rti1516e::CouldNotCreateLogicalTimeFactory,
           rti1516e::InconsistentFDD,
           rti1516e::CouldNotOpenFDD,
           rti1516e::ErrorReadingFDD,
           rti1516e::FederationExecutionAlreadyExists,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual void
  createFederationExecutionWithMIM (std::wstring const & federationExecutionName,
                             std::vector<std::wstring> const & fomModules,
                             std::wstring const & mimModule,
                             std::wstring const & logicalTimeImplementationName)
    RTI_THROW ((rti1516e::CouldNotCreateLogicalTimeFactory,
           rti1516e::InconsistentFDD,
           rti1516e::CouldNotOpenFDD,
           rti1516e::ErrorReadingFDD,
           rti1516e::DesignatorIsHLAstandardMIM,
           rti1516e::ErrorReadingMIM,
           rti1516e::CouldNotOpenMIM,
           rti1516e::FederationExecutionAlreadyExists,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual void
  destroyFederationExecution(std::wstring const & federationExecutionName)
    RTI_THROW ((rti1516e::FederatesCurrentlyJoined,
           rti1516e::FederationExecutionDoesNotExist,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual void
  listFederationExecutions()
         RTI_THROW ((rti1516e::NotConnected,
                rti1516e::RTIinternalError));

  virtual rti1516e::FederateHandle
  joinFederationExecution(std::wstring const & federateType,
                          std::wstring const & federationExecutionName,
                          std::vector<std::wstring> const & additionalFomModules)
    RTI_THROW ((rti1516e::CouldNotCreateLogicalTimeFactory,
            rti1516e::FederationExecutionDoesNotExist,
            rti1516e::InconsistentFDD,
            rti1516e::ErrorReadingFDD,
            rti1516e::CouldNotOpenFDD,
            rti1516e::FederateAlreadyExecutionMember,
            rti1516e::SaveInProgress,
            rti1516e::RestoreInProgress,
            rti1516e::NotConnected,
            rti1516e::CallNotAllowedFromWithinCallback,
            rti1516e::RTIinternalError));

  virtual rti1516e::FederateHandle
  joinFederationExecution(std::wstring const & federateName,
                          std::wstring const & federateType,
                          std::wstring const & federationExecutionName,
                          std::vector<std::wstring> const & additionalFomModules)
    RTI_THROW ((rti1516e::CouldNotCreateLogicalTimeFactory,
           rti1516e::FederateNameAlreadyInUse,
           rti1516e::FederationExecutionDoesNotExist,
           rti1516e::InconsistentFDD,
           rti1516e::ErrorReadingFDD,
           rti1516e::CouldNotOpenFDD,
           rti1516e::FederateAlreadyExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::CallNotAllowedFromWithinCallback,
           rti1516e::RTIinternalError));

  // 4.5
  virtual void
  resignFederationExecution(rti1516e::ResignAction rti1516ResignAction)
    RTI_THROW ((rti1516e::OwnershipAcquisitionPending,
           rti1516e::FederateOwnsAttributes,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 4.6
  virtual void
  registerFederationSynchronizationPoint(std::wstring const & label,
                                         rti1516e::VariableLengthData const & rti1516Tag)
    RTI_THROW ((rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual void
  registerFederationSynchronizationPoint(std::wstring const & label,
                                         rti1516e::VariableLengthData const & rti1516Tag,
                                         rti1516e::FederateHandleSet const & rti1516FederateHandleSet)
    RTI_THROW ((rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 4.9
  virtual void
    synchronizationPointAchieved(std::wstring const & label, bool)
    RTI_THROW ((rti1516e::SynchronizationPointLabelNotAnnounced,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 4.11
  virtual void
  requestFederationSave(std::wstring const & label)
    RTI_THROW ((rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual void
  requestFederationSave(const std::wstring& label,
                        const rti1516e::LogicalTime& rti1516LogicalTime)
    RTI_THROW ((rti1516e::LogicalTimeAlreadyPassed,
           rti1516e::InvalidLogicalTime,
           rti1516e::FederateUnableToUseTime,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 4.13
  virtual void
  federateSaveBegun()
    RTI_THROW ((rti1516e::SaveNotInitiated,
           rti1516e::FederateNotExecutionMember,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 4.14
  virtual void
  federateSaveComplete()
    RTI_THROW ((rti1516e::FederateHasNotBegunSave,
           rti1516e::FederateNotExecutionMember,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual void
  federateSaveNotComplete()
    RTI_THROW ((rti1516e::FederateHasNotBegunSave,
           rti1516e::FederateNotExecutionMember,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual void
  abortFederationSave()
    RTI_THROW ((rti1516e::SaveNotInProgress,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 4.16
  virtual void
  queryFederationSaveStatus ()
    RTI_THROW ((rti1516e::FederateNotExecutionMember,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 4.18
  virtual void
  requestFederationRestore(std::wstring const & label)
    RTI_THROW ((rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 4.22
  virtual void
  federateRestoreComplete()
    RTI_THROW ((rti1516e::RestoreNotRequested,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual void
  federateRestoreNotComplete()
    RTI_THROW ((rti1516e::RestoreNotRequested,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual void
  abortFederationRestore ()
    RTI_THROW ((rti1516e::RestoreNotInProgress,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 4.24
  virtual void
  queryFederationRestoreStatus()
    RTI_THROW ((rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  /////////////////////////////////////
  // Declaration Management Services //
  /////////////////////////////////////

  // 5.2
  virtual void
  publishObjectClassAttributes(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                               rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet)
    RTI_THROW ((rti1516e::ObjectClassNotDefined,
           rti1516e::AttributeNotDefined,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 5.3
  virtual void
  unpublishObjectClass(rti1516e::ObjectClassHandle rti1516ObjectClassHandle)
    RTI_THROW ((rti1516e::ObjectClassNotDefined,
           rti1516e::OwnershipAcquisitionPending,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual void
  unpublishObjectClassAttributes(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                 rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet)
    RTI_THROW ((rti1516e::ObjectClassNotDefined,
           rti1516e::AttributeNotDefined,
           rti1516e::OwnershipAcquisitionPending,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 5.4
  virtual void
  publishInteractionClass(rti1516e::InteractionClassHandle rti1516InteractionClassHandle)
    RTI_THROW ((rti1516e::InteractionClassNotDefined,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 5.5
  virtual void
  unpublishInteractionClass(rti1516e::InteractionClassHandle rti1516InteractionClassHandle)
    RTI_THROW ((rti1516e::InteractionClassNotDefined,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 5.6
  virtual void
  subscribeObjectClassAttributes(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                 rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                                 bool active, std::wstring const & updateRateDesignator)
    RTI_THROW ((rti1516e::ObjectClassNotDefined,
           rti1516e::AttributeNotDefined,
           rti1516e::FederateNotExecutionMember,
           rti1516e::InvalidUpdateRateDesignator,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 5.7
  virtual void
  unsubscribeObjectClass(rti1516e::ObjectClassHandle rti1516ObjectClassHandle)
    RTI_THROW ((rti1516e::ObjectClassNotDefined,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual void
  unsubscribeObjectClassAttributes(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                   rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet)
    RTI_THROW ((rti1516e::ObjectClassNotDefined,
           rti1516e::AttributeNotDefined,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 5.8
  virtual void
  subscribeInteractionClass(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                            bool active)
    RTI_THROW ((rti1516e::InteractionClassNotDefined,
           rti1516e::FederateServiceInvocationsAreBeingReportedViaMOM,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 5.9
  virtual void
  unsubscribeInteractionClass(rti1516e::InteractionClassHandle rti1516InteractionClassHandle)
    RTI_THROW ((rti1516e::InteractionClassNotDefined,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  ////////////////////////////////
  // Object Management Services //
  ////////////////////////////////

  virtual void
  reserveObjectInstanceName(std::wstring const & objectInstanceName)
    RTI_THROW ((rti1516e::IllegalName,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual void
  releaseObjectInstanceName(std::wstring const & objectInstanceName)
    RTI_THROW ((rti1516e::ObjectInstanceNameNotReserved,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual void
  reserveMultipleObjectInstanceName(std::set<std::wstring> const & theObjectInstanceNames)
    RTI_THROW ((rti1516e::IllegalName,
           rti1516e::NameSetWasEmpty,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual void
  releaseMultipleObjectInstanceName(std::set<std::wstring> const & theObjectInstanceNames)
    RTI_THROW ((rti1516e::ObjectInstanceNameNotReserved,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 6.4
  virtual rti1516e::ObjectInstanceHandle
  registerObjectInstance(rti1516e::ObjectClassHandle rti1516ObjectClassHandle)
    RTI_THROW ((rti1516e::ObjectClassNotDefined,
           rti1516e::ObjectClassNotPublished,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual rti1516e::ObjectInstanceHandle
  registerObjectInstance(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                         std::wstring const & objectInstanceName)
    RTI_THROW ((rti1516e::ObjectClassNotDefined,
           rti1516e::ObjectClassNotPublished,
           rti1516e::ObjectInstanceNameNotReserved,
           rti1516e::ObjectInstanceNameInUse,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 6.6
  virtual void
  updateAttributeValues(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                        const rti1516e::AttributeHandleValueMap& rti1516AttributeValues,
                        const rti1516e::VariableLengthData& rti1516Tag)
    RTI_THROW ((rti1516e::ObjectInstanceNotKnown,
           rti1516e::AttributeNotDefined,
           rti1516e::AttributeNotOwned,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual rti1516e::MessageRetractionHandle
  updateAttributeValues(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                        const rti1516e::AttributeHandleValueMap& rti1516AttributeValues,
                        const rti1516e::VariableLengthData& rti1516Tag,
                        const rti1516e::LogicalTime& rti1516LogicalTime)
    RTI_THROW ((rti1516e::ObjectInstanceNotKnown,
           rti1516e::AttributeNotDefined,
           rti1516e::AttributeNotOwned,
           rti1516e::InvalidLogicalTime,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 6.8
  virtual void
  sendInteraction(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                  const rti1516e::ParameterHandleValueMap& rti1516ParameterValues,
                  const rti1516e::VariableLengthData& rti1516Tag)
    RTI_THROW ((rti1516e::InteractionClassNotPublished,
           rti1516e::InteractionClassNotDefined,
           rti1516e::InteractionParameterNotDefined,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual rti1516e::MessageRetractionHandle
  sendInteraction(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                  const rti1516e::ParameterHandleValueMap& rti1516ParameterValues,
                  const rti1516e::VariableLengthData& rti1516Tag,
                  const rti1516e::LogicalTime& rti1516LogicalTime)
    RTI_THROW ((rti1516e::InteractionClassNotPublished,
           rti1516e::InteractionClassNotDefined,
           rti1516e::InteractionParameterNotDefined,
           rti1516e::InvalidLogicalTime,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 6.10
  virtual void
  deleteObjectInstance(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                       const rti1516e::VariableLengthData& rti1516Tag)
    RTI_THROW ((rti1516e::DeletePrivilegeNotHeld,
           rti1516e::ObjectInstanceNotKnown,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual rti1516e::MessageRetractionHandle
  deleteObjectInstance(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                       const rti1516e::VariableLengthData& rti1516Tag,
                       const rti1516e::LogicalTime& rti1516LogicalTime)
    RTI_THROW ((rti1516e::DeletePrivilegeNotHeld,
           rti1516e::ObjectInstanceNotKnown,
           rti1516e::InvalidLogicalTime,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 6.12
  virtual void
  localDeleteObjectInstance(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle)
    RTI_THROW ((rti1516e::ObjectInstanceNotKnown,
           rti1516e::FederateOwnsAttributes,
           rti1516e::OwnershipAcquisitionPending,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 6.13
  virtual void
  changeAttributeTransportationType(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                    rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                                    rti1516e::TransportationType rti1516TransportationType)
    RTI_THROW ((rti1516e::ObjectInstanceNotKnown,
           rti1516e::AttributeNotDefined,
           rti1516e::AttributeNotOwned,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 6.14
  virtual void
  changeInteractionTransportationType(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                      rti1516e::TransportationType rti1516TransportationType)
    RTI_THROW ((rti1516e::InteractionClassNotDefined,
           rti1516e::InteractionClassNotPublished,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 6.17
  virtual void
  requestAttributeValueUpdate(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                              rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                              rti1516e::VariableLengthData const & rti1516Tag)
    RTI_THROW ((rti1516e::ObjectInstanceNotKnown,
           rti1516e::AttributeNotDefined,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual void
  requestAttributeValueUpdate(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                              rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                              rti1516e::VariableLengthData const & rti1516Tag)
    RTI_THROW ((rti1516e::ObjectClassNotDefined,
           rti1516e::AttributeNotDefined,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual void
  requestAttributeTransportationTypeChange(rti1516e::ObjectInstanceHandle theObject,
                                           rti1516e::AttributeHandleSet const & theAttributes,
                                           rti1516e::TransportationType theType)
    RTI_THROW ((rti1516e::AttributeAlreadyBeingChanged,
           rti1516e::AttributeNotOwned,
           rti1516e::AttributeNotDefined,
           rti1516e::ObjectInstanceNotKnown,
           rti1516e::InvalidTransportationType,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual void
  queryAttributeTransportationType(rti1516e::ObjectInstanceHandle theObject, rti1516e::AttributeHandle theAttribute)
    RTI_THROW ((rti1516e::AttributeNotDefined,
           rti1516e::ObjectInstanceNotKnown,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual void
  requestInteractionTransportationTypeChange(rti1516e::InteractionClassHandle theClass, rti1516e::TransportationType theType)
    RTI_THROW ((rti1516e::InteractionClassAlreadyBeingChanged,
           rti1516e::InteractionClassNotPublished,
           rti1516e::InteractionClassNotDefined,
           rti1516e::InvalidTransportationType,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

    virtual void
    queryInteractionTransportationType(rti1516e::FederateHandle theFederate, rti1516e::InteractionClassHandle theInteraction)
      RTI_THROW ((rti1516e::InteractionClassNotDefined,
             rti1516e::SaveInProgress,
             rti1516e::RestoreInProgress,
             rti1516e::FederateNotExecutionMember,
             rti1516e::NotConnected,
             rti1516e::RTIinternalError));

  ///////////////////////////////////
  // Ownership Management Services //
  ///////////////////////////////////
  // 7.2
  virtual void
  unconditionalAttributeOwnershipDivestiture(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                             rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet)
    RTI_THROW ((rti1516e::ObjectInstanceNotKnown,
           rti1516e::AttributeNotDefined,
           rti1516e::AttributeNotOwned,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 7.3
  virtual void
  negotiatedAttributeOwnershipDivestiture(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                          rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                                          rti1516e::VariableLengthData const & rti1516Tag)
    RTI_THROW ((rti1516e::ObjectInstanceNotKnown,
           rti1516e::AttributeNotDefined,
           rti1516e::AttributeNotOwned,
           rti1516e::AttributeAlreadyBeingDivested,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 7.6
  virtual void
  confirmDivestiture(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                     rti1516e::AttributeHandleSet const& rti1516AttributeHandleSet,
                     rti1516e::VariableLengthData const& rti1516Tag)
    RTI_THROW ((rti1516e::ObjectInstanceNotKnown,
           rti1516e::AttributeNotDefined,
           rti1516e::AttributeNotOwned,
           rti1516e::AttributeDivestitureWasNotRequested,
           rti1516e::NoAcquisitionPending,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 7.8
  virtual void
  attributeOwnershipAcquisition(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                                rti1516e::VariableLengthData const & rti1516Tag)
    RTI_THROW ((rti1516e::ObjectInstanceNotKnown,
           rti1516e::ObjectClassNotPublished,
           rti1516e::AttributeNotDefined,
           rti1516e::AttributeNotPublished,
           rti1516e::FederateOwnsAttributes,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 7.9
  virtual void
  attributeOwnershipAcquisitionIfAvailable(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                           rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet)
    RTI_THROW ((rti1516e::ObjectInstanceNotKnown,
           rti1516e::ObjectClassNotPublished,
           rti1516e::AttributeNotDefined,
           rti1516e::AttributeNotPublished,
           rti1516e::FederateOwnsAttributes,
           rti1516e::AttributeAlreadyBeingAcquired,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual void
  attributeOwnershipReleaseDenied(rti1516e::ObjectInstanceHandle theObject, rti1516e::AttributeHandleSet const & theAttributes)
    RTI_THROW ((rti1516e::AttributeNotOwned,
           rti1516e::AttributeNotDefined,
           rti1516e::ObjectInstanceNotKnown,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 7.12
  virtual void
  attributeOwnershipDivestitureIfWanted(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                        rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                                        rti1516e::AttributeHandleSet & rti1516DivestedAttributeSet)
    RTI_THROW ((rti1516e::ObjectInstanceNotKnown,
           rti1516e::AttributeNotDefined,
           rti1516e::AttributeNotOwned,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 7.13
  virtual void
  cancelNegotiatedAttributeOwnershipDivestiture(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet)
    RTI_THROW ((rti1516e::ObjectInstanceNotKnown,
           rti1516e::AttributeNotDefined,
           rti1516e::AttributeNotOwned,
           rti1516e::AttributeDivestitureWasNotRequested,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 7.14
  virtual void
  cancelAttributeOwnershipAcquisition(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                      rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet)
    RTI_THROW ((rti1516e::ObjectInstanceNotKnown,
           rti1516e::AttributeNotDefined,
           rti1516e::AttributeAlreadyOwned,
           rti1516e::AttributeAcquisitionWasNotRequested,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 7.16
  virtual void
  queryAttributeOwnership(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                          rti1516e::AttributeHandle rti1516AttributeHandle)
    RTI_THROW ((rti1516e::ObjectInstanceNotKnown,
           rti1516e::AttributeNotDefined,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 7.18
  virtual bool
  isAttributeOwnedByFederate(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                             rti1516e::AttributeHandle rti1516AttributeHandle)
    RTI_THROW ((rti1516e::ObjectInstanceNotKnown,
           rti1516e::AttributeNotDefined,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  //////////////////////////////
  // Time Management Services //
  //////////////////////////////

  // 8.2
  virtual void
  enableTimeRegulation(const rti1516e::LogicalTimeInterval& rti1516Lookahead)
    RTI_THROW ((rti1516e::TimeRegulationAlreadyEnabled,
           rti1516e::InvalidLookahead,
           rti1516e::InTimeAdvancingState,
           rti1516e::RequestForTimeRegulationPending,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 8.4
  virtual void
  disableTimeRegulation()
    RTI_THROW ((rti1516e::TimeRegulationIsNotEnabled,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 8.5
  virtual void
  enableTimeConstrained()
    RTI_THROW ((rti1516e::TimeConstrainedAlreadyEnabled,
           rti1516e::InTimeAdvancingState,
           rti1516e::RequestForTimeConstrainedPending,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 8.7
  virtual void
  disableTimeConstrained()
    RTI_THROW ((rti1516e::TimeConstrainedIsNotEnabled,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 8.8
  virtual void
  timeAdvanceRequest(const rti1516e::LogicalTime& logicalTime)
    RTI_THROW ((rti1516e::InvalidLogicalTime,
           rti1516e::LogicalTimeAlreadyPassed,
           rti1516e::InTimeAdvancingState,
           rti1516e::RequestForTimeRegulationPending,
           rti1516e::RequestForTimeConstrainedPending,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 8.9
  virtual void
  timeAdvanceRequestAvailable(const rti1516e::LogicalTime& logicalTime)
    RTI_THROW ((rti1516e::InvalidLogicalTime,
           rti1516e::LogicalTimeAlreadyPassed,
           rti1516e::InTimeAdvancingState,
           rti1516e::RequestForTimeRegulationPending,
           rti1516e::RequestForTimeConstrainedPending,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 8.10
  virtual void
  nextMessageRequest(const rti1516e::LogicalTime& logicalTime)
    RTI_THROW ((rti1516e::InvalidLogicalTime,
           rti1516e::LogicalTimeAlreadyPassed,
           rti1516e::InTimeAdvancingState,
           rti1516e::RequestForTimeRegulationPending,
           rti1516e::RequestForTimeConstrainedPending,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 8.11
  virtual void
  nextMessageRequestAvailable(const rti1516e::LogicalTime& logicalTime)
    RTI_THROW ((rti1516e::InvalidLogicalTime,
           rti1516e::LogicalTimeAlreadyPassed,
           rti1516e::InTimeAdvancingState,
           rti1516e::RequestForTimeRegulationPending,
           rti1516e::RequestForTimeConstrainedPending,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 8.12
  virtual void
  flushQueueRequest(const rti1516e::LogicalTime& logicalTime)
    RTI_THROW ((rti1516e::InvalidLogicalTime,
           rti1516e::LogicalTimeAlreadyPassed,
           rti1516e::InTimeAdvancingState,
           rti1516e::RequestForTimeRegulationPending,
           rti1516e::RequestForTimeConstrainedPending,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 8.14
  virtual void
  enableAsynchronousDelivery()
    RTI_THROW ((rti1516e::AsynchronousDeliveryAlreadyEnabled,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 8.15
  virtual void
  disableAsynchronousDelivery()
    RTI_THROW ((rti1516e::AsynchronousDeliveryAlreadyDisabled,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 8.16
  virtual bool
  queryGALT(rti1516e::LogicalTime& logicalTime)
    RTI_THROW ((rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 8.17
  virtual void
  queryLogicalTime(rti1516e::LogicalTime& logicalTime)
    RTI_THROW ((rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 8.18
  virtual bool
  queryLITS(rti1516e::LogicalTime& logicalTime)
    RTI_THROW ((rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 8.19
  virtual void
  modifyLookahead(const rti1516e::LogicalTimeInterval& lookahead)
    RTI_THROW ((rti1516e::TimeRegulationIsNotEnabled,
           rti1516e::InvalidLookahead,
           rti1516e::InTimeAdvancingState,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 8.20
  virtual void
  queryLookahead(rti1516e::LogicalTimeInterval& lookahead)
    RTI_THROW ((rti1516e::TimeRegulationIsNotEnabled,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 8.21
  virtual void
  retract(rti1516e::MessageRetractionHandle rti1516MessageRetractionHandle)
    RTI_THROW ((rti1516e::MessageCanNoLongerBeRetracted,
           rti1516e::InvalidMessageRetractionHandle,
           rti1516e::TimeRegulationIsNotEnabled,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 8.23
  virtual void
  changeAttributeOrderType(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                           rti1516e::AttributeHandleSet const & rti1516AttributeHandleSet,
                           rti1516e::OrderType rti1516OrderType)
    RTI_THROW ((rti1516e::ObjectInstanceNotKnown,
           rti1516e::AttributeNotDefined,
           rti1516e::AttributeNotOwned,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 8.24
  virtual void
  changeInteractionOrderType(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                             rti1516e::OrderType rti1516OrderType)
    RTI_THROW ((rti1516e::InteractionClassNotDefined,
           rti1516e::InteractionClassNotPublished,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  //////////////////////////////////
  // Data Distribution Management //
  //////////////////////////////////

  // 9.2
  virtual rti1516e::RegionHandle
  createRegion(rti1516e::DimensionHandleSet const & rti1516DimensionHandleSet)
    RTI_THROW ((rti1516e::InvalidDimensionHandle,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 9.3
  virtual void
  commitRegionModifications(rti1516e::RegionHandleSet const & rti1516RegionHandleSet)
    RTI_THROW ((rti1516e::InvalidRegion,
           rti1516e::RegionNotCreatedByThisFederate,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 9.4
  virtual void
  deleteRegion(const rti1516e::RegionHandle& rti1516RegionHandle)
    RTI_THROW ((rti1516e::InvalidRegion,
           rti1516e::RegionNotCreatedByThisFederate,
           rti1516e::RegionInUseForUpdateOrSubscription,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 9.5
  virtual rti1516e::ObjectInstanceHandle
  registerObjectInstanceWithRegions(rti1516e::ObjectClassHandle theClass,
                                    rti1516e::AttributeHandleSetRegionHandleSetPairVector const &
                                    theAttributeHandleSetRegionHandleSetPairVector)
    RTI_THROW ((rti1516e::ObjectClassNotDefined,
           rti1516e::ObjectClassNotPublished,
           rti1516e::AttributeNotDefined,
           rti1516e::AttributeNotPublished,
           rti1516e::InvalidRegion,
           rti1516e::RegionNotCreatedByThisFederate,
           rti1516e::InvalidRegionContext,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual rti1516e::ObjectInstanceHandle
  registerObjectInstanceWithRegions(rti1516e::ObjectClassHandle theClass,
                                    rti1516e::AttributeHandleSetRegionHandleSetPairVector const &
                                    theAttributeHandleSetRegionHandleSetPairVector,
                                    std::wstring const & theObjectInstanceName)
    RTI_THROW ((rti1516e::ObjectClassNotDefined,
           rti1516e::ObjectClassNotPublished,
           rti1516e::AttributeNotDefined,
           rti1516e::AttributeNotPublished,
           rti1516e::InvalidRegion,
           rti1516e::RegionNotCreatedByThisFederate,
           rti1516e::InvalidRegionContext,
           rti1516e::ObjectInstanceNameNotReserved,
           rti1516e::ObjectInstanceNameInUse,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 9.6
  virtual void
  associateRegionsForUpdates(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                             rti1516e::AttributeHandleSetRegionHandleSetPairVector const &
                             theAttributeHandleSetRegionHandleSetPairVector)
    RTI_THROW ((rti1516e::ObjectInstanceNotKnown,
           rti1516e::AttributeNotDefined,
           rti1516e::InvalidRegion,
           rti1516e::RegionNotCreatedByThisFederate,
           rti1516e::InvalidRegionContext,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 9.7
  virtual void
  unassociateRegionsForUpdates(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                               rti1516e::AttributeHandleSetRegionHandleSetPairVector const &
                               theAttributeHandleSetRegionHandleSetPairVector)
    RTI_THROW ((rti1516e::ObjectInstanceNotKnown,
           rti1516e::AttributeNotDefined,
           rti1516e::InvalidRegion,
           rti1516e::RegionNotCreatedByThisFederate,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual void
  subscribeObjectClassAttributesWithRegions(rti1516e::ObjectClassHandle theClass,
                                            rti1516e::AttributeHandleSetRegionHandleSetPairVector const &
                                            theAttributeHandleSetRegionHandleSetPairVector,
                                            bool active, std::wstring const & updateRateDesignator)
    RTI_THROW ((rti1516e::ObjectClassNotDefined,
           rti1516e::AttributeNotDefined,
           rti1516e::InvalidRegion,
           rti1516e::RegionNotCreatedByThisFederate,
           rti1516e::InvalidRegionContext,
           rti1516e::FederateNotExecutionMember,
           rti1516e::InvalidUpdateRateDesignator,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 9.9
  virtual void
  unsubscribeObjectClassAttributesWithRegions(rti1516e::ObjectClassHandle theClass,
                                              rti1516e::AttributeHandleSetRegionHandleSetPairVector const &
                                              theAttributeHandleSetRegionHandleSetPairVector)
    RTI_THROW ((rti1516e::ObjectClassNotDefined,
           rti1516e::AttributeNotDefined,
           rti1516e::InvalidRegion,
           rti1516e::RegionNotCreatedByThisFederate,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 9.10
  virtual void
  subscribeInteractionClassWithRegions(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                       rti1516e::RegionHandleSet const & rti1516RegionHandleSet,
                                       bool active)
    RTI_THROW ((rti1516e::InteractionClassNotDefined,
           rti1516e::InvalidRegion,
           rti1516e::RegionNotCreatedByThisFederate,
           rti1516e::InvalidRegionContext,
           rti1516e::FederateServiceInvocationsAreBeingReportedViaMOM,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 9.11
  virtual void
  unsubscribeInteractionClassWithRegions(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                                         rti1516e::RegionHandleSet const & rti1516RegionHandleSet)
    RTI_THROW ((rti1516e::InteractionClassNotDefined,
           rti1516e::InvalidRegion,
           rti1516e::RegionNotCreatedByThisFederate,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 9.12
  virtual void
  sendInteractionWithRegions(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                             rti1516e::ParameterHandleValueMap const & rti1516ParameterHandleValueMap,
                             rti1516e::RegionHandleSet const & rti1516RegionHandleSet,
                             rti1516e::VariableLengthData const & rti1516Tag)
    RTI_THROW ((rti1516e::InteractionClassNotDefined,
           rti1516e::InteractionClassNotPublished,
           rti1516e::InteractionParameterNotDefined,
           rti1516e::InvalidRegion,
           rti1516e::RegionNotCreatedByThisFederate,
           rti1516e::InvalidRegionContext,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual rti1516e::MessageRetractionHandle
  sendInteractionWithRegions(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                             rti1516e::ParameterHandleValueMap const & rti1516ParameterHandleValueMap,
                             rti1516e::RegionHandleSet const & rti1516RegionHandleSet,
                             rti1516e::VariableLengthData const & rti1516Tag,
                             rti1516e::LogicalTime const & rti1516LogicalTime)
    RTI_THROW ((rti1516e::InteractionClassNotDefined,
           rti1516e::InteractionClassNotPublished,
           rti1516e::InteractionParameterNotDefined,
           rti1516e::InvalidRegion,
           rti1516e::RegionNotCreatedByThisFederate,
           rti1516e::InvalidRegionContext,
           rti1516e::InvalidLogicalTime,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 9.13
  virtual void
  requestAttributeValueUpdateWithRegions(rti1516e::ObjectClassHandle theClass,
                                         rti1516e::AttributeHandleSetRegionHandleSetPairVector const & theSet,
                                         rti1516e::VariableLengthData const & rti1516Tag)
    RTI_THROW ((rti1516e::ObjectClassNotDefined,
           rti1516e::AttributeNotDefined,
           rti1516e::InvalidRegion,
           rti1516e::RegionNotCreatedByThisFederate,
           rti1516e::InvalidRegionContext,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  //////////////////////////
  // RTI Support Services //
  //////////////////////////

  virtual rti1516e::ResignAction
  getAutomaticResignDirective()
         RTI_THROW ((rti1516e::FederateNotExecutionMember,
                rti1516e::NotConnected,
                rti1516e::RTIinternalError));

  virtual void
  setAutomaticResignDirective(rti1516e::ResignAction resignAction)
    RTI_THROW ((rti1516e::InvalidResignAction,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual rti1516e::FederateHandle getFederateHandle(std::wstring const & theName)
    RTI_THROW ((rti1516e::NameNotFound,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual std::wstring getFederateName(rti1516e::FederateHandle theHandle)
    RTI_THROW ((rti1516e::InvalidFederateHandle,
           rti1516e::FederateHandleNotKnown,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.2
  virtual rti1516e::ObjectClassHandle
  getObjectClassHandle(std::wstring const & name)
    RTI_THROW ((rti1516e::NameNotFound,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.3
  virtual std::wstring
  getObjectClassName(rti1516e::ObjectClassHandle rti1516ObjectClassHandle)
    RTI_THROW ((rti1516e::InvalidObjectClassHandle,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.4
  virtual rti1516e::AttributeHandle
  getAttributeHandle(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                     std::wstring const & attributeName)
    RTI_THROW ((rti1516e::InvalidObjectClassHandle,
           rti1516e::NameNotFound,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.5
  virtual std::wstring
  getAttributeName(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                   rti1516e::AttributeHandle rti1516AttributeHandle)
    RTI_THROW ((rti1516e::InvalidObjectClassHandle,
           rti1516e::InvalidAttributeHandle,
           rti1516e::AttributeNotDefined,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.13
  virtual double
  getUpdateRateValue(std::wstring const & updateRateDesignator)
    RTI_THROW ((rti1516e::InvalidUpdateRateDesignator,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.14
  virtual double
  getUpdateRateValueForAttribute(rti1516e::ObjectInstanceHandle theObject, rti1516e::AttributeHandle theAttribute)
    RTI_THROW ((rti1516e::ObjectInstanceNotKnown,
           rti1516e::AttributeNotDefined,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.6
  virtual rti1516e::InteractionClassHandle
  getInteractionClassHandle(std::wstring const & name)
    RTI_THROW ((rti1516e::NameNotFound,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.7
  virtual std::wstring
  getInteractionClassName(rti1516e::InteractionClassHandle rti1516InteractionClassHandle)
    RTI_THROW ((rti1516e::InvalidInteractionClassHandle,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.8
  virtual rti1516e::ParameterHandle
  getParameterHandle(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                     std::wstring const & parameterName)
    RTI_THROW ((rti1516e::InvalidInteractionClassHandle,
           rti1516e::NameNotFound,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.9
  virtual std::wstring
  getParameterName(rti1516e::InteractionClassHandle rti1516InteractionClassHandle,
                   rti1516e::ParameterHandle rti1516ParameterHandle)
    RTI_THROW ((rti1516e::InvalidInteractionClassHandle,
           rti1516e::InvalidParameterHandle,
           rti1516e::InteractionParameterNotDefined,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.10
  virtual rti1516e::ObjectInstanceHandle
  getObjectInstanceHandle(std::wstring const & name)
    RTI_THROW ((rti1516e::ObjectInstanceNotKnown,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.11
  virtual std::wstring
  getObjectInstanceName(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle)
    RTI_THROW ((rti1516e::ObjectInstanceNotKnown,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.12
  virtual rti1516e::DimensionHandle
  getDimensionHandle(std::wstring const & name)
    RTI_THROW ((rti1516e::NameNotFound,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.13
  virtual std::wstring
  getDimensionName(rti1516e::DimensionHandle rti1516DimensionHandle)
    RTI_THROW ((rti1516e::InvalidDimensionHandle,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.14
  virtual unsigned long
  getDimensionUpperBound(rti1516e::DimensionHandle rti1516DimensionHandle)
    RTI_THROW ((rti1516e::InvalidDimensionHandle,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.15
  virtual rti1516e::DimensionHandleSet
  getAvailableDimensionsForClassAttribute(rti1516e::ObjectClassHandle rti1516ObjectClassHandle,
                                          rti1516e::AttributeHandle rti1516AttributeHandle)
    RTI_THROW ((rti1516e::InvalidObjectClassHandle,
           rti1516e::InvalidAttributeHandle,
           rti1516e::AttributeNotDefined,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.16
  virtual rti1516e::ObjectClassHandle
  getKnownObjectClassHandle(rti1516e::ObjectInstanceHandle rti1516ObjectInstanceHandle)
    RTI_THROW ((rti1516e::ObjectInstanceNotKnown,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.17
  virtual rti1516e::DimensionHandleSet
  getAvailableDimensionsForInteractionClass(rti1516e::InteractionClassHandle rti1516InteractionClassHandle)
    RTI_THROW ((rti1516e::InvalidInteractionClassHandle,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.18
  virtual rti1516e::TransportationType
  getTransportationType(std::wstring const & transportationName)
    RTI_THROW ((rti1516e::InvalidTransportationName,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.19
  virtual std::wstring
  getTransportationName(rti1516e::TransportationType transportationType)
    RTI_THROW ((rti1516e::InvalidTransportationType,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.20
  virtual rti1516e::OrderType
  getOrderType(std::wstring const & orderName)
    RTI_THROW ((rti1516e::InvalidOrderName,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.21
  virtual std::wstring
  getOrderName(rti1516e::OrderType orderType)
    RTI_THROW ((rti1516e::InvalidOrderType,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.22
  virtual void enableObjectClassRelevanceAdvisorySwitch()
    RTI_THROW ((rti1516e::ObjectClassRelevanceAdvisorySwitchIsOn,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.23
  virtual void disableObjectClassRelevanceAdvisorySwitch()
    RTI_THROW ((rti1516e::ObjectClassRelevanceAdvisorySwitchIsOff,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.24
  virtual void enableAttributeRelevanceAdvisorySwitch ()
    RTI_THROW ((rti1516e::AttributeRelevanceAdvisorySwitchIsOn,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.25
  virtual void disableAttributeRelevanceAdvisorySwitch ()
    RTI_THROW ((rti1516e::AttributeRelevanceAdvisorySwitchIsOff,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.26
  virtual void enableAttributeScopeAdvisorySwitch ()
    RTI_THROW ((rti1516e::AttributeScopeAdvisorySwitchIsOn,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.27
  virtual void disableAttributeScopeAdvisorySwitch ()
    RTI_THROW ((rti1516e::AttributeScopeAdvisorySwitchIsOff,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.28
  virtual void enableInteractionRelevanceAdvisorySwitch ()
    RTI_THROW ((rti1516e::InteractionRelevanceAdvisorySwitchIsOn,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.29
  virtual void disableInteractionRelevanceAdvisorySwitch ()
    RTI_THROW ((rti1516e::InteractionRelevanceAdvisorySwitchIsOff,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.30
  virtual
  rti1516e::DimensionHandleSet getDimensionHandleSet(rti1516e::RegionHandle rti1516RegionHandle)
    RTI_THROW ((rti1516e::InvalidRegion,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.31
  virtual rti1516e::RangeBounds
  getRangeBounds(rti1516e::RegionHandle theRegionHandle,
                 rti1516e::DimensionHandle theDimensionHandle)
    RTI_THROW ((rti1516e::InvalidRegion,
           rti1516e::RegionDoesNotContainSpecifiedDimension,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.32
  virtual void
  setRangeBounds(rti1516e::RegionHandle theRegionHandle,
                 rti1516e::DimensionHandle theDimensionHandle,
                 rti1516e::RangeBounds const & theRangeBounds)
    RTI_THROW ((rti1516e::InvalidRegion,
           rti1516e::RegionNotCreatedByThisFederate,
           rti1516e::RegionDoesNotContainSpecifiedDimension,
           rti1516e::InvalidRangeBound,
           rti1516e::FederateNotExecutionMember,
           rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.33
  virtual unsigned long normalizeFederateHandle(rti1516e::FederateHandle rti1516FederateHandle)
    RTI_THROW ((rti1516e::FederateNotExecutionMember,
           rti1516e::InvalidFederateHandle,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.34
  virtual unsigned long normalizeServiceGroup(rti1516e::ServiceGroup rti1516ServiceGroup)
    RTI_THROW ((rti1516e::FederateNotExecutionMember,
           rti1516e::InvalidServiceGroup,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // 10.37
  virtual bool evokeCallback(double approximateMinimumTimeInSeconds)
    RTI_THROW ((rti1516e::CallNotAllowedFromWithinCallback,
           rti1516e::RTIinternalError));

  // 10.38
  virtual bool evokeMultipleCallbacks(double approximateMinimumTimeInSeconds,
                                      double approximateMaximumTimeInSeconds)
    RTI_THROW ((rti1516e::CallNotAllowedFromWithinCallback,
           rti1516e::RTIinternalError));

  // 10.39
  virtual void enableCallbacks ()
    RTI_THROW ((rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::RTIinternalError));

  // 10.40
  virtual void disableCallbacks ()
    RTI_THROW ((rti1516e::SaveInProgress,
           rti1516e::RestoreInProgress,
           rti1516e::RTIinternalError));

  // Return instance of time factory being used by the federation
  virtual RTI_UNIQUE_PTR<rti1516e::LogicalTimeFactory> getTimeFactory() const
    RTI_THROW ((rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  // Decode handles
  virtual rti1516e::FederateHandle decodeFederateHandle(rti1516e::VariableLengthData const & encodedValue) const
    RTI_THROW ((rti1516e::CouldNotDecode,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual rti1516e::ObjectClassHandle decodeObjectClassHandle(rti1516e::VariableLengthData const & encodedValue) const
    RTI_THROW ((rti1516e::CouldNotDecode,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual rti1516e::InteractionClassHandle decodeInteractionClassHandle(rti1516e::VariableLengthData const & encodedValue) const
    RTI_THROW ((rti1516e::CouldNotDecode,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual rti1516e::ObjectInstanceHandle decodeObjectInstanceHandle(rti1516e::VariableLengthData const & encodedValue) const
    RTI_THROW ((rti1516e::CouldNotDecode,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual rti1516e::AttributeHandle decodeAttributeHandle(rti1516e::VariableLengthData const & encodedValue) const
    RTI_THROW ((rti1516e::CouldNotDecode,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual rti1516e::ParameterHandle decodeParameterHandle(rti1516e::VariableLengthData const & encodedValue) const
    RTI_THROW ((rti1516e::CouldNotDecode,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual rti1516e::DimensionHandle decodeDimensionHandle(rti1516e::VariableLengthData const & encodedValue) const
    RTI_THROW ((rti1516e::CouldNotDecode,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual rti1516e::MessageRetractionHandle decodeMessageRetractionHandle(rti1516e::VariableLengthData const & encodedValue) const
    RTI_THROW ((rti1516e::CouldNotDecode,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

  virtual rti1516e::RegionHandle decodeRegionHandle(rti1516e::VariableLengthData const & encodedValue) const
    RTI_THROW ((rti1516e::CouldNotDecode,
           rti1516e::FederateNotExecutionMember,
           rti1516e::NotConnected,
           rti1516e::RTIinternalError));

private:
  class RTI1516EAmbassadorInterface;
  RTI1516EAmbassadorInterface* _ambassadorInterface;
};

}

#endif
