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

class OPENRTI_LOCAL RTIambassadorImplementation : public rti1516::RTIambassador {
public:
  RTIambassadorImplementation(const std::vector<std::wstring>& args) RTI_NOEXCEPT;
  virtual ~RTIambassadorImplementation();

  // 4.2
  virtual void
  createFederationExecution(std::wstring const & federationExecutionName,
                            std::wstring const & fullPathNameToTheFDDfile,
                            std::wstring const & logicalTimeImplementationName)
    RTI_THROW ((rti1516::FederationExecutionAlreadyExists,
           rti1516::CouldNotOpenFDD,
           rti1516::ErrorReadingFDD,
           rti1516::CouldNotCreateLogicalTimeFactory,
           rti1516::RTIinternalError));

  // 4.3
  virtual void
  destroyFederationExecution(std::wstring const & federationExecutionName)
    RTI_THROW ((rti1516::FederatesCurrentlyJoined,
           rti1516::FederationExecutionDoesNotExist,
           rti1516::RTIinternalError));

  // 4.4
  virtual rti1516::FederateHandle
  joinFederationExecution(std::wstring const & federateType,
                          std::wstring const & federationExecutionName,
                          rti1516::FederateAmbassador & federateAmbassador)
    RTI_THROW ((rti1516::FederateAlreadyExecutionMember,
           rti1516::FederationExecutionDoesNotExist,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::CouldNotCreateLogicalTimeFactory,
           rti1516::RTIinternalError));

  // 4.5
  virtual void
  resignFederationExecution(rti1516::ResignAction rti1516ResignAction)
    RTI_THROW ((rti1516::OwnershipAcquisitionPending,
           rti1516::FederateOwnsAttributes,
           rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 4.6
  virtual void
  registerFederationSynchronizationPoint(std::wstring const & label,
                                         rti1516::VariableLengthData const & rti1516Tag)
    RTI_THROW ((rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  virtual void
  registerFederationSynchronizationPoint(std::wstring const & label,
                                         rti1516::VariableLengthData const & rti1516Tag,
                                         rti1516::FederateHandleSet const & rti1516FederateHandleSet)
    RTI_THROW ((rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 4.9
  virtual void
  synchronizationPointAchieved(std::wstring const & label)
    RTI_THROW ((rti1516::SynchronizationPointLabelNotAnnounced,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 4.11
  virtual void
  requestFederationSave(std::wstring const & label)
    RTI_THROW ((rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  virtual void
  requestFederationSave(const std::wstring& label,
                        const rti1516::LogicalTime& rti1516LogicalTime)
    RTI_THROW ((rti1516::LogicalTimeAlreadyPassed,
           rti1516::InvalidLogicalTime,
           rti1516::FederateUnableToUseTime,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 4.13
  virtual void
  federateSaveBegun()
    RTI_THROW ((rti1516::SaveNotInitiated,
           rti1516::FederateNotExecutionMember,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 4.14
  virtual void
  federateSaveComplete()
    RTI_THROW ((rti1516::FederateHasNotBegunSave,
           rti1516::FederateNotExecutionMember,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  virtual void
  federateSaveNotComplete()
    RTI_THROW ((rti1516::FederateHasNotBegunSave,
           rti1516::FederateNotExecutionMember,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 4.16
  virtual void
  queryFederationSaveStatus ()
    RTI_THROW ((rti1516::FederateNotExecutionMember,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 4.18
  virtual void
  requestFederationRestore(std::wstring const & label)
    RTI_THROW ((rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 4.22
  virtual void
  federateRestoreComplete()
    RTI_THROW ((rti1516::RestoreNotRequested,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RTIinternalError));

  virtual void
  federateRestoreNotComplete()
    RTI_THROW ((rti1516::RestoreNotRequested,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RTIinternalError));

  // 4.24
  virtual void
  queryFederationRestoreStatus()
    RTI_THROW ((rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RTIinternalError));

  /////////////////////////////////////
  // Declaration Management Services //
  /////////////////////////////////////

  // 5.2
  virtual void
  publishObjectClassAttributes(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                               rti1516::AttributeHandleSet const & rti1516AttributeHandleSet)
    RTI_THROW ((rti1516::ObjectClassNotDefined,
           rti1516::AttributeNotDefined,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 5.3
  virtual void
  unpublishObjectClass(rti1516::ObjectClassHandle rti1516ObjectClassHandle)
    RTI_THROW ((rti1516::ObjectClassNotDefined,
           rti1516::OwnershipAcquisitionPending,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  virtual void
  unpublishObjectClassAttributes(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                 rti1516::AttributeHandleSet const & rti1516AttributeHandleSet)
    RTI_THROW ((rti1516::ObjectClassNotDefined,
           rti1516::AttributeNotDefined,
           rti1516::OwnershipAcquisitionPending,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 5.4
  virtual void
  publishInteractionClass(rti1516::InteractionClassHandle rti1516InteractionClassHandle)
    RTI_THROW ((rti1516::InteractionClassNotDefined,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 5.5
  virtual void
  unpublishInteractionClass(rti1516::InteractionClassHandle rti1516InteractionClassHandle)
    RTI_THROW ((rti1516::InteractionClassNotDefined,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 5.6
  virtual void
  subscribeObjectClassAttributes(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                 rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                 bool active)
    RTI_THROW ((rti1516::ObjectClassNotDefined,
           rti1516::AttributeNotDefined,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 5.7
  virtual void
  unsubscribeObjectClass(rti1516::ObjectClassHandle rti1516ObjectClassHandle)
    RTI_THROW ((rti1516::ObjectClassNotDefined,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  virtual void
  unsubscribeObjectClassAttributes(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                   rti1516::AttributeHandleSet const & rti1516AttributeHandleSet)
    RTI_THROW ((rti1516::ObjectClassNotDefined,
           rti1516::AttributeNotDefined,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 5.8
  virtual void
  subscribeInteractionClass(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                            bool active)
    RTI_THROW ((rti1516::InteractionClassNotDefined,
           rti1516::FederateServiceInvocationsAreBeingReportedViaMOM,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 5.9
  virtual void
  unsubscribeInteractionClass(rti1516::InteractionClassHandle rti1516InteractionClassHandle)
    RTI_THROW ((rti1516::InteractionClassNotDefined,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  ////////////////////////////////
  // Object Management Services //
  ////////////////////////////////

  // 6.2
  virtual void
  reserveObjectInstanceName(std::wstring const & objectInstanceName)
    RTI_THROW ((rti1516::IllegalName,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 6.4
  virtual rti1516::ObjectInstanceHandle
  registerObjectInstance(rti1516::ObjectClassHandle rti1516ObjectClassHandle)
    RTI_THROW ((rti1516::ObjectClassNotDefined,
           rti1516::ObjectClassNotPublished,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  virtual rti1516::ObjectInstanceHandle
  registerObjectInstance(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                         std::wstring const & objectInstanceName)
    RTI_THROW ((rti1516::ObjectClassNotDefined,
           rti1516::ObjectClassNotPublished,
           rti1516::ObjectInstanceNameNotReserved,
           rti1516::ObjectInstanceNameInUse,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 6.6
  virtual void
  updateAttributeValues(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                        const rti1516::AttributeHandleValueMap& rti1516AttributeValues,
                        const rti1516::VariableLengthData& rti1516Tag)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotDefined,
           rti1516::AttributeNotOwned,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  virtual rti1516::MessageRetractionHandle
  updateAttributeValues(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                        const rti1516::AttributeHandleValueMap& rti1516AttributeValues,
                        const rti1516::VariableLengthData& rti1516Tag,
                        const rti1516::LogicalTime& rti1516LogicalTime)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotDefined,
           rti1516::AttributeNotOwned,
           rti1516::InvalidLogicalTime,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 6.8
  virtual void
  sendInteraction(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                  const rti1516::ParameterHandleValueMap& rti1516ParameterValues,
                  const rti1516::VariableLengthData& rti1516Tag)
    RTI_THROW ((rti1516::InteractionClassNotPublished,
           rti1516::InteractionClassNotDefined,
           rti1516::InteractionParameterNotDefined,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  virtual rti1516::MessageRetractionHandle
  sendInteraction(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                  const rti1516::ParameterHandleValueMap& rti1516ParameterValues,
                  const rti1516::VariableLengthData& rti1516Tag,
                  const rti1516::LogicalTime& rti1516LogicalTime)
    RTI_THROW ((rti1516::InteractionClassNotPublished,
           rti1516::InteractionClassNotDefined,
           rti1516::InteractionParameterNotDefined,
           rti1516::InvalidLogicalTime,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 6.10
  virtual void
  deleteObjectInstance(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                       const rti1516::VariableLengthData& rti1516Tag)
    RTI_THROW ((rti1516::DeletePrivilegeNotHeld,
           rti1516::ObjectInstanceNotKnown,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  virtual rti1516::MessageRetractionHandle
  deleteObjectInstance(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                       const rti1516::VariableLengthData& rti1516Tag,
                       const rti1516::LogicalTime& rti1516LogicalTime)
    RTI_THROW ((rti1516::DeletePrivilegeNotHeld,
           rti1516::ObjectInstanceNotKnown,
           rti1516::InvalidLogicalTime,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 6.12
  virtual void
  localDeleteObjectInstance(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::FederateOwnsAttributes,
           rti1516::OwnershipAcquisitionPending,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 6.13
  virtual void
  changeAttributeTransportationType(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                    rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                    rti1516::TransportationType rti1516TransportationType)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotDefined,
           rti1516::AttributeNotOwned,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 6.14
  virtual void
  changeInteractionTransportationType(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                      rti1516::TransportationType rti1516TransportationType)
    RTI_THROW ((rti1516::InteractionClassNotDefined,
           rti1516::InteractionClassNotPublished,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 6.17
  virtual void
  requestAttributeValueUpdate(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                              rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                              rti1516::VariableLengthData const & rti1516Tag)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotDefined,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  virtual void
  requestAttributeValueUpdate(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                              rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                              rti1516::VariableLengthData const & rti1516Tag)
    RTI_THROW ((rti1516::ObjectClassNotDefined,
           rti1516::AttributeNotDefined,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  ///////////////////////////////////
  // Ownership Management Services //
  ///////////////////////////////////
  // 7.2
  virtual void
  unconditionalAttributeOwnershipDivestiture(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                             rti1516::AttributeHandleSet const & rti1516AttributeHandleSet)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotDefined,
           rti1516::AttributeNotOwned,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 7.3
  virtual void
  negotiatedAttributeOwnershipDivestiture(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                          rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                          rti1516::VariableLengthData const & rti1516Tag)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotDefined,
           rti1516::AttributeNotOwned,
           rti1516::AttributeAlreadyBeingDivested,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 7.6
  virtual void
  confirmDivestiture(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                     rti1516::AttributeHandleSet const& rti1516AttributeHandleSet,
                     rti1516::VariableLengthData const& rti1516Tag)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotDefined,
           rti1516::AttributeNotOwned,
           rti1516::AttributeDivestitureWasNotRequested,
           rti1516::NoAcquisitionPending,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 7.8
  virtual void
  attributeOwnershipAcquisition(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                rti1516::VariableLengthData const & rti1516Tag)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::ObjectClassNotPublished,
           rti1516::AttributeNotDefined,
           rti1516::AttributeNotPublished,
           rti1516::FederateOwnsAttributes,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 7.9
  virtual void
  attributeOwnershipAcquisitionIfAvailable(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                           rti1516::AttributeHandleSet const & rti1516AttributeHandleSet)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::ObjectClassNotPublished,
           rti1516::AttributeNotDefined,
           rti1516::AttributeNotPublished,
           rti1516::FederateOwnsAttributes,
           rti1516::AttributeAlreadyBeingAcquired,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 7.12
  virtual void
  attributeOwnershipDivestitureIfWanted(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                        rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                                        rti1516::AttributeHandleSet & rti1516DivestedAttributeSet)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotDefined,
           rti1516::AttributeNotOwned,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 7.13
  virtual void
  cancelNegotiatedAttributeOwnershipDivestiture(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                                rti1516::AttributeHandleSet const & rti1516AttributeHandleSet)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotDefined,
           rti1516::AttributeNotOwned,
           rti1516::AttributeDivestitureWasNotRequested,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 7.14
  virtual void
  cancelAttributeOwnershipAcquisition(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                                      rti1516::AttributeHandleSet const & rti1516AttributeHandleSet)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotDefined,
           rti1516::AttributeAlreadyOwned,
           rti1516::AttributeAcquisitionWasNotRequested,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 7.16
  virtual void
  queryAttributeOwnership(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                          rti1516::AttributeHandle rti1516AttributeHandle)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotDefined,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 7.18
  virtual bool
  isAttributeOwnedByFederate(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                             rti1516::AttributeHandle rti1516AttributeHandle)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotDefined,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  //////////////////////////////
  // Time Management Services //
  //////////////////////////////

  // 8.2
  virtual void
  enableTimeRegulation(const rti1516::LogicalTimeInterval& rti1516Lookahead)
    RTI_THROW ((rti1516::TimeRegulationAlreadyEnabled,
           rti1516::InvalidLookahead,
           rti1516::InTimeAdvancingState,
           rti1516::RequestForTimeRegulationPending,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 8.4
  virtual void
  disableTimeRegulation()
    RTI_THROW ((rti1516::TimeRegulationIsNotEnabled,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 8.5
  virtual void
  enableTimeConstrained()
    RTI_THROW ((rti1516::TimeConstrainedAlreadyEnabled,
           rti1516::InTimeAdvancingState,
           rti1516::RequestForTimeConstrainedPending,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 8.7
  virtual void
  disableTimeConstrained()
    RTI_THROW ((rti1516::TimeConstrainedIsNotEnabled,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 8.8
  virtual void
  timeAdvanceRequest(const rti1516::LogicalTime& logicalTime)
    RTI_THROW ((rti1516::InvalidLogicalTime,
           rti1516::LogicalTimeAlreadyPassed,
           rti1516::InTimeAdvancingState,
           rti1516::RequestForTimeRegulationPending,
           rti1516::RequestForTimeConstrainedPending,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 8.9
  virtual void
  timeAdvanceRequestAvailable(const rti1516::LogicalTime& logicalTime)
    RTI_THROW ((rti1516::InvalidLogicalTime,
           rti1516::LogicalTimeAlreadyPassed,
           rti1516::InTimeAdvancingState,
           rti1516::RequestForTimeRegulationPending,
           rti1516::RequestForTimeConstrainedPending,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 8.10
  virtual void
  nextMessageRequest(const rti1516::LogicalTime& logicalTime)
    RTI_THROW ((rti1516::InvalidLogicalTime,
           rti1516::LogicalTimeAlreadyPassed,
           rti1516::InTimeAdvancingState,
           rti1516::RequestForTimeRegulationPending,
           rti1516::RequestForTimeConstrainedPending,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 8.11
  virtual void
  nextMessageRequestAvailable(const rti1516::LogicalTime& logicalTime)
    RTI_THROW ((rti1516::InvalidLogicalTime,
           rti1516::LogicalTimeAlreadyPassed,
           rti1516::InTimeAdvancingState,
           rti1516::RequestForTimeRegulationPending,
           rti1516::RequestForTimeConstrainedPending,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 8.12
  virtual void
  flushQueueRequest(const rti1516::LogicalTime& logicalTime)
    RTI_THROW ((rti1516::InvalidLogicalTime,
           rti1516::LogicalTimeAlreadyPassed,
           rti1516::InTimeAdvancingState,
           rti1516::RequestForTimeRegulationPending,
           rti1516::RequestForTimeConstrainedPending,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 8.14
  virtual void
  enableAsynchronousDelivery()
    RTI_THROW ((rti1516::AsynchronousDeliveryAlreadyEnabled,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 8.15
  virtual void
  disableAsynchronousDelivery()
    RTI_THROW ((rti1516::AsynchronousDeliveryAlreadyDisabled,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 8.16
  virtual bool
  queryGALT(rti1516::LogicalTime& logicalTime)
    RTI_THROW ((rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 8.17
  virtual void
  queryLogicalTime(rti1516::LogicalTime& logicalTime)
    RTI_THROW ((rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 8.18
  virtual bool
  queryLITS(rti1516::LogicalTime& logicalTime)
    RTI_THROW ((rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 8.19
  virtual void
  modifyLookahead(const rti1516::LogicalTimeInterval& lookahead)
    RTI_THROW ((rti1516::TimeRegulationIsNotEnabled,
           rti1516::InvalidLookahead,
           rti1516::InTimeAdvancingState,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 8.20
  virtual void
  queryLookahead(rti1516::LogicalTimeInterval& lookahead)
    RTI_THROW ((rti1516::TimeRegulationIsNotEnabled,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 8.21
  virtual void
  retract(rti1516::MessageRetractionHandle rti1516MessageRetractionHandle)
    RTI_THROW ((rti1516::InvalidRetractionHandle,
           rti1516::TimeRegulationIsNotEnabled,
           rti1516::MessageCanNoLongerBeRetracted,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 8.23
  virtual void
  changeAttributeOrderType(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                           rti1516::AttributeHandleSet const & rti1516AttributeHandleSet,
                           rti1516::OrderType rti1516OrderType)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotDefined,
           rti1516::AttributeNotOwned,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 8.24
  virtual void
  changeInteractionOrderType(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                             rti1516::OrderType rti1516OrderType)
    RTI_THROW ((rti1516::InteractionClassNotDefined,
           rti1516::InteractionClassNotPublished,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  //////////////////////////////////
  // Data Distribution Management //
  //////////////////////////////////

  // 9.2
  virtual rti1516::RegionHandle
  createRegion(rti1516::DimensionHandleSet const & rti1516DimensionHandleSet)
    RTI_THROW ((rti1516::InvalidDimensionHandle,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 9.3
  virtual void
  commitRegionModifications(rti1516::RegionHandleSet const & rti1516RegionHandleSet)
    RTI_THROW ((rti1516::InvalidRegion,
           rti1516::RegionNotCreatedByThisFederate,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 9.4
  virtual void
  deleteRegion(rti1516::RegionHandle rti1516RegionHandle)
    RTI_THROW ((rti1516::InvalidRegion,
           rti1516::RegionNotCreatedByThisFederate,
           rti1516::RegionInUseForUpdateOrSubscription,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 9.5
  virtual rti1516::ObjectInstanceHandle
  registerObjectInstanceWithRegions(rti1516::ObjectClassHandle theClass,
                                    rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                                    theAttributeHandleSetRegionHandleSetPairVector)
    RTI_THROW ((rti1516::ObjectClassNotDefined,
           rti1516::ObjectClassNotPublished,
           rti1516::AttributeNotDefined,
           rti1516::AttributeNotPublished,
           rti1516::InvalidRegion,
           rti1516::RegionNotCreatedByThisFederate,
           rti1516::InvalidRegionContext,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  virtual rti1516::ObjectInstanceHandle
  registerObjectInstanceWithRegions(rti1516::ObjectClassHandle theClass,
                                    rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                                    theAttributeHandleSetRegionHandleSetPairVector,
                                    std::wstring const & theObjectInstanceName)
    RTI_THROW ((rti1516::ObjectClassNotDefined,
           rti1516::ObjectClassNotPublished,
           rti1516::AttributeNotDefined,
           rti1516::AttributeNotPublished,
           rti1516::InvalidRegion,
           rti1516::RegionNotCreatedByThisFederate,
           rti1516::InvalidRegionContext,
           rti1516::ObjectInstanceNameNotReserved,
           rti1516::ObjectInstanceNameInUse,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 9.6
  virtual void
  associateRegionsForUpdates(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                             rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                             theAttributeHandleSetRegionHandleSetPairVector)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotDefined,
           rti1516::InvalidRegion,
           rti1516::RegionNotCreatedByThisFederate,
           rti1516::InvalidRegionContext,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 9.7
  virtual void
  unassociateRegionsForUpdates(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle,
                               rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                               theAttributeHandleSetRegionHandleSetPairVector)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::AttributeNotDefined,
           rti1516::InvalidRegion,
           rti1516::RegionNotCreatedByThisFederate,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 9.8
  virtual void
  subscribeObjectClassAttributesWithRegions(rti1516::ObjectClassHandle theClass,
                                            rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                                            theAttributeHandleSetRegionHandleSetPairVector,
                                            bool active)
    RTI_THROW ((rti1516::ObjectClassNotDefined,
           rti1516::AttributeNotDefined,
           rti1516::InvalidRegion,
           rti1516::RegionNotCreatedByThisFederate,
           rti1516::InvalidRegionContext,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 9.9
  virtual void
  unsubscribeObjectClassAttributesWithRegions(rti1516::ObjectClassHandle theClass,
                                              rti1516::AttributeHandleSetRegionHandleSetPairVector const &
                                              theAttributeHandleSetRegionHandleSetPairVector)
    RTI_THROW ((rti1516::ObjectClassNotDefined,
           rti1516::AttributeNotDefined,
           rti1516::InvalidRegion,
           rti1516::RegionNotCreatedByThisFederate,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 9.10
  virtual void
  subscribeInteractionClassWithRegions(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                       rti1516::RegionHandleSet const & rti1516RegionHandleSet,
                                       bool active)
    RTI_THROW ((rti1516::InteractionClassNotDefined,
           rti1516::InvalidRegion,
           rti1516::RegionNotCreatedByThisFederate,
           rti1516::InvalidRegionContext,
           rti1516::FederateServiceInvocationsAreBeingReportedViaMOM,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 9.11
  virtual void
  unsubscribeInteractionClassWithRegions(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                                         rti1516::RegionHandleSet const & rti1516RegionHandleSet)
    RTI_THROW ((rti1516::InteractionClassNotDefined,
           rti1516::InvalidRegion,
           rti1516::RegionNotCreatedByThisFederate,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 9.12
  virtual void
  sendInteractionWithRegions(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                             rti1516::ParameterHandleValueMap const & rti1516ParameterHandleValueMap,
                             rti1516::RegionHandleSet const & rti1516RegionHandleSet,
                             rti1516::VariableLengthData const & rti1516Tag)
    RTI_THROW ((rti1516::InteractionClassNotDefined,
           rti1516::InteractionClassNotPublished,
           rti1516::InteractionParameterNotDefined,
           rti1516::InvalidRegion,
           rti1516::RegionNotCreatedByThisFederate,
           rti1516::InvalidRegionContext,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  virtual rti1516::MessageRetractionHandle
  sendInteractionWithRegions(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                             rti1516::ParameterHandleValueMap const & rti1516ParameterHandleValueMap,
                             rti1516::RegionHandleSet const & rti1516RegionHandleSet,
                             rti1516::VariableLengthData const & rti1516Tag,
                             rti1516::LogicalTime const & rti1516LogicalTime)
    RTI_THROW ((rti1516::InteractionClassNotDefined,
           rti1516::InteractionClassNotPublished,
           rti1516::InteractionParameterNotDefined,
           rti1516::InvalidRegion,
           rti1516::RegionNotCreatedByThisFederate,
           rti1516::InvalidRegionContext,
           rti1516::InvalidLogicalTime,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 9.13
  virtual void
  requestAttributeValueUpdateWithRegions(rti1516::ObjectClassHandle theClass,
                                         rti1516::AttributeHandleSetRegionHandleSetPairVector const & theSet,
                                         rti1516::VariableLengthData const & rti1516Tag)
    RTI_THROW ((rti1516::ObjectClassNotDefined,
           rti1516::AttributeNotDefined,
           rti1516::InvalidRegion,
           rti1516::RegionNotCreatedByThisFederate,
           rti1516::InvalidRegionContext,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  //////////////////////////
  // RTI Support Services //
  //////////////////////////

  // 10.2
  virtual rti1516::ObjectClassHandle
  getObjectClassHandle(std::wstring const & name)
    RTI_THROW ((rti1516::NameNotFound,
           rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 10.3
  virtual std::wstring
  getObjectClassName(rti1516::ObjectClassHandle rti1516ObjectClassHandle)
    RTI_THROW ((rti1516::InvalidObjectClassHandle,
           rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 10.4
  virtual rti1516::AttributeHandle
  getAttributeHandle(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                     std::wstring const & attributeName)
    RTI_THROW ((rti1516::InvalidObjectClassHandle,
           rti1516::NameNotFound,
           rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 10.5
  virtual std::wstring
  getAttributeName(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                   rti1516::AttributeHandle rti1516AttributeHandle)
    RTI_THROW ((rti1516::InvalidObjectClassHandle,
           rti1516::InvalidAttributeHandle,
           rti1516::AttributeNotDefined,
           rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 10.6
  virtual rti1516::InteractionClassHandle
  getInteractionClassHandle(std::wstring const & name)
    RTI_THROW ((rti1516::NameNotFound,
           rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 10.7
  virtual std::wstring
  getInteractionClassName(rti1516::InteractionClassHandle rti1516InteractionClassHandle)
    RTI_THROW ((rti1516::InvalidInteractionClassHandle,
           rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 10.8
  virtual rti1516::ParameterHandle
  getParameterHandle(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                     std::wstring const & parameterName)
    RTI_THROW ((rti1516::InvalidInteractionClassHandle,
           rti1516::NameNotFound,
           rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 10.9
  virtual std::wstring
  getParameterName(rti1516::InteractionClassHandle rti1516InteractionClassHandle,
                   rti1516::ParameterHandle rti1516ParameterHandle)
    RTI_THROW ((rti1516::InvalidInteractionClassHandle,
           rti1516::InvalidParameterHandle,
           rti1516::InteractionParameterNotDefined,
           rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 10.10
  virtual rti1516::ObjectInstanceHandle
  getObjectInstanceHandle(std::wstring const & name)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 10.11
  virtual std::wstring
  getObjectInstanceName(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 10.12
  virtual rti1516::DimensionHandle
  getDimensionHandle(std::wstring const & name)
    RTI_THROW ((rti1516::NameNotFound,
           rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 10.13
  virtual std::wstring
  getDimensionName(rti1516::DimensionHandle rti1516DimensionHandle)
    RTI_THROW ((rti1516::InvalidDimensionHandle,
           rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 10.14
  virtual unsigned long
  getDimensionUpperBound(rti1516::DimensionHandle rti1516DimensionHandle)
    RTI_THROW ((rti1516::InvalidDimensionHandle,
           rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 10.15
  virtual rti1516::DimensionHandleSet
  getAvailableDimensionsForClassAttribute(rti1516::ObjectClassHandle rti1516ObjectClassHandle,
                                          rti1516::AttributeHandle rti1516AttributeHandle)
    RTI_THROW ((rti1516::InvalidObjectClassHandle,
           rti1516::InvalidAttributeHandle,
           rti1516::AttributeNotDefined,
           rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 10.16
  virtual rti1516::ObjectClassHandle
  getKnownObjectClassHandle(rti1516::ObjectInstanceHandle rti1516ObjectInstanceHandle)
    RTI_THROW ((rti1516::ObjectInstanceNotKnown,
           rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 10.17
  virtual rti1516::DimensionHandleSet
  getAvailableDimensionsForInteractionClass(rti1516::InteractionClassHandle rti1516InteractionClassHandle)
    RTI_THROW ((rti1516::InvalidInteractionClassHandle,
           rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 10.18
  virtual rti1516::TransportationType
  getTransportationType(std::wstring const & transportationName)
    RTI_THROW ((rti1516::InvalidTransportationName,
           rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 10.19
  virtual std::wstring
  getTransportationName(rti1516::TransportationType transportationType)
    RTI_THROW ((rti1516::InvalidTransportationType,
           rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 10.20
  virtual rti1516::OrderType
  getOrderType(std::wstring const & orderName)
    RTI_THROW ((rti1516::InvalidOrderName,
           rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 10.21
  virtual std::wstring
  getOrderName(rti1516::OrderType orderType)
    RTI_THROW ((rti1516::InvalidOrderType,
           rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 10.22
  virtual void enableObjectClassRelevanceAdvisorySwitch()
    RTI_THROW ((rti1516::ObjectClassRelevanceAdvisorySwitchIsOn,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 10.23
  virtual void disableObjectClassRelevanceAdvisorySwitch()
    RTI_THROW ((rti1516::ObjectClassRelevanceAdvisorySwitchIsOff,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 10.24
  virtual void enableAttributeRelevanceAdvisorySwitch ()
    RTI_THROW ((rti1516::AttributeRelevanceAdvisorySwitchIsOn,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 10.25
  virtual void disableAttributeRelevanceAdvisorySwitch ()
    RTI_THROW ((rti1516::AttributeRelevanceAdvisorySwitchIsOff,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 10.26
  virtual void enableAttributeScopeAdvisorySwitch ()
    RTI_THROW ((rti1516::AttributeScopeAdvisorySwitchIsOn,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 10.27
  virtual void disableAttributeScopeAdvisorySwitch ()
    RTI_THROW ((rti1516::AttributeScopeAdvisorySwitchIsOff,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 10.28
  virtual void enableInteractionRelevanceAdvisorySwitch ()
    RTI_THROW ((rti1516::InteractionRelevanceAdvisorySwitchIsOn,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 10.29
  virtual void disableInteractionRelevanceAdvisorySwitch ()
    RTI_THROW ((rti1516::InteractionRelevanceAdvisorySwitchIsOff,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 10.30
  virtual
  rti1516::DimensionHandleSet getDimensionHandleSet(rti1516::RegionHandle rti1516RegionHandle)
    RTI_THROW ((rti1516::InvalidRegion,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 10.31
  virtual rti1516::RangeBounds
  getRangeBounds(rti1516::RegionHandle theRegionHandle,
                 rti1516::DimensionHandle theDimensionHandle)
    RTI_THROW ((rti1516::InvalidRegion,
           rti1516::RegionDoesNotContainSpecifiedDimension,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 10.32
  virtual void
  setRangeBounds(rti1516::RegionHandle theRegionHandle,
                 rti1516::DimensionHandle theDimensionHandle,
                 rti1516::RangeBounds const & theRangeBounds)
    RTI_THROW ((rti1516::InvalidRegion,
           rti1516::RegionNotCreatedByThisFederate,
           rti1516::RegionDoesNotContainSpecifiedDimension,
           rti1516::InvalidRangeBound,
           rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 10.33
  virtual unsigned long normalizeFederateHandle(rti1516::FederateHandle rti1516FederateHandle)
    RTI_THROW ((rti1516::FederateNotExecutionMember,
           rti1516::InvalidFederateHandle,
           rti1516::RTIinternalError));

  // 10.34
  virtual unsigned long normalizeServiceGroup(rti1516::ServiceGroupIndicator rti1516ServiceGroup)
    RTI_THROW ((rti1516::FederateNotExecutionMember,
           rti1516::InvalidServiceGroup,
           rti1516::RTIinternalError));

  // 10.37
  virtual bool evokeCallback(double approximateMinimumTimeInSeconds)
    RTI_THROW ((rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 10.38
  virtual bool evokeMultipleCallbacks(double approximateMinimumTimeInSeconds,
                                      double approximateMaximumTimeInSeconds)
    RTI_THROW ((rti1516::FederateNotExecutionMember,
           rti1516::RTIinternalError));

  // 10.39
  virtual void enableCallbacks ()
    RTI_THROW ((rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  // 10.40
  virtual void disableCallbacks ()
    RTI_THROW ((rti1516::FederateNotExecutionMember,
           rti1516::SaveInProgress,
           rti1516::RestoreInProgress,
           rti1516::RTIinternalError));

  virtual rti1516::FederateHandle
  decodeFederateHandle(rti1516::VariableLengthData const & encodedValue) const;

  virtual rti1516::ObjectClassHandle
  decodeObjectClassHandle(rti1516::VariableLengthData const & encodedValue) const;

  virtual rti1516::InteractionClassHandle
  decodeInteractionClassHandle(rti1516::VariableLengthData const & encodedValue) const;

  virtual rti1516::ObjectInstanceHandle
  decodeObjectInstanceHandle(rti1516::VariableLengthData const & encodedValue) const;

  virtual rti1516::AttributeHandle
  decodeAttributeHandle(rti1516::VariableLengthData const & encodedValue) const;

  virtual rti1516::ParameterHandle
  decodeParameterHandle(rti1516::VariableLengthData const & encodedValue) const;

  virtual rti1516::DimensionHandle
  decodeDimensionHandle(rti1516::VariableLengthData const & encodedValue) const;

  virtual rti1516::MessageRetractionHandle
  decodeMessageRetractionHandle(rti1516::VariableLengthData const & encodedValue) const;

  virtual rti1516::RegionHandle
  decodeRegionHandle(rti1516::VariableLengthData const & encodedValue) const;

private:
  class RTI1516AmbassadorInterface;
  RTI1516AmbassadorInterface* _ambassadorInterface;
};

}

#endif
