/* -*-c++-*- OpenRTI - Copyright (C) 2010-2011 Mathias Froehlich
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

#ifndef QRTI1516Federate_h
#define QRTI1516Federate_h

#include "QRTIFederate.h"

#include <RTI/FederateAmbassador.h>
#include <RTI/RTIambassadorFactory.h>
#include <RTI/RTIambassador.h>
#include <RTI/LogicalTime.h>
#include <RTI/LogicalTimeInterval.h>
#include <RTI/LogicalTimeFactory.h>
#include <RTI/RangeBounds.h>
#include <RTI/HLAfloat64Time.h>
#include <RTI/HLAinteger64Time.h>


class QRTI1516Federate : public QRTIFederate {
public:
  class Ambassador : public rti1516::FederateAmbassador {
  public:
    Ambassador(QRTI1516Federate& federate) :
      _federate(federate),
      _timeRegulationEnabled(false),
      _timeConstrainedEnabled(false),
      _timeAdvancePending(false)
    { }
    virtual ~Ambassador()
      throw ()
    { }

    bool getTimeRegulationEnabled() const
    { return _timeRegulationEnabled; }
    bool getTimeConstrainedEnabled() const
    { return _timeConstrainedEnabled; }
    bool getTimeAdvancePending() const
    { return _timeAdvancePending; }

    void connect(std::vector<std::wstring> args)
    {
      rti1516::RTIambassadorFactory factory;
      _ambassador = factory.createRTIambassador(args);
      setLogicalTimeFactory();
    }

    void setLogicalTimeFactory(const std::wstring& logicalTimeImplementationName = std::wstring())
    {
      _logicalTimeImplementationName = logicalTimeImplementationName;
      _logicalTimeFactory = rti1516::LogicalTimeFactoryFactory::makeLogicalTimeFactory(logicalTimeImplementationName);
    }

    void createFederationExecution(const std::wstring& federationExecutionName, const std::wstring& fddFile)
    {
      _ambassador->createFederationExecution(federationExecutionName, fddFile, _logicalTimeImplementationName);
    }

    void destroyFederationExecution(const std::wstring& federationExecutionName)
    {
      _ambassador->destroyFederationExecution(federationExecutionName);
    }

    const rti1516::FederateHandle& joinFederationExecution(const std::wstring& federateType,
                                                           const std::wstring& federationExecutionName)
    {
      _federateHandle = _ambassador->joinFederationExecution(federateType, federationExecutionName, *this);
      _grantedLogicalTime = _logicalTimeFactory->makeLogicalTime();
      _grantedLogicalTime->setInitial();
      return _federateHandle;
    }

    void resignFederationExecution(rti1516::ResignAction resignAction)
    {
      _ambassador->resignFederationExecution(resignAction);
      _federateHandle = rti1516::FederateHandle();
    }

    void registerFederationSynchronizationPoint(const std::wstring& label, const rti1516::VariableLengthData& tag)
    {
      _ambassador->registerFederationSynchronizationPoint(label, tag);
    }

    void registerFederationSynchronizationPoint(const std::wstring& label, const rti1516::VariableLengthData& tag,
                                                const rti1516::FederateHandleSet& federateHandleSet)
    {
      _ambassador->registerFederationSynchronizationPoint(label, tag, federateHandleSet);
    }

    void synchronizationPointAchieved(const std::wstring& label)
    {
      _ambassador->synchronizationPointAchieved(label);
    }

    void requestFederationSave(const std::wstring& label)
    {
      _ambassador->requestFederationSave(label);
    }

    void requestFederationSave(const std::wstring& label, const rti1516::LogicalTime& logicalTime)
    {
      _ambassador->requestFederationSave(label, logicalTime);
    }

    void federateSaveComplete()
    {
      _ambassador->federateSaveComplete();
    }

    void federateSaveNotComplete()
    {
      _ambassador->federateSaveNotComplete();
    }

    void queryFederationSaveStatus()
    {
      _ambassador->queryFederationSaveStatus();
    }

    void requestFederationRestore(const std::wstring& label)
    {
      _ambassador->requestFederationRestore(label);
    }

    void federateRestoreComplete()
    {
      _ambassador->federateRestoreComplete();
    }

    void federateRestoreNotComplete()
    {
      _ambassador->federateRestoreNotComplete();
    }

    void queryFederationRestoreStatus()
    {
      _ambassador->queryFederationRestoreStatus();
    }

    void publishObjectClassAttributes(const rti1516::ObjectClassHandle& objectClassHandle,
                                      const rti1516::AttributeHandleSet& attributeList)
    {
      _ambassador->publishObjectClassAttributes(objectClassHandle, attributeList);
    }

    void unpublishObjectClass(const rti1516::ObjectClassHandle& objectClassHandle)
    {
      _ambassador->unpublishObjectClass(objectClassHandle);
    }

    void unpublishObjectClassAttributes(const rti1516::ObjectClassHandle& objectClassHandle,
                                        const rti1516::AttributeHandleSet& attributeList)
    {
      _ambassador->unpublishObjectClassAttributes(objectClassHandle, attributeList);
    }

    void publishInteractionClass(const rti1516::InteractionClassHandle& interactionClassHandle)
    {
      _ambassador->publishInteractionClass(interactionClassHandle);
    }

    void unpublishInteractionClass(const rti1516::InteractionClassHandle& interactionClassHandle)
    {
      _ambassador->unpublishInteractionClass(interactionClassHandle);
    }

    void subscribeObjectClassAttributes(const rti1516::ObjectClassHandle& objectClassHandle,
                                        const rti1516::AttributeHandleSet& attributeHandleSet,
                                        bool active = true)
    {
      _ambassador->subscribeObjectClassAttributes(objectClassHandle, attributeHandleSet, active);
    }

    void unsubscribeObjectClass(const rti1516::ObjectClassHandle& objectClassHandle)
    {
      _ambassador->unsubscribeObjectClass(objectClassHandle);
    }

    void unsubscribeObjectClassAttributes(const rti1516::ObjectClassHandle& objectClassHandle,
                                          const rti1516::AttributeHandleSet& attributeHandleSet)
    {
      _ambassador->unsubscribeObjectClassAttributes(objectClassHandle, attributeHandleSet);
    }

    void subscribeInteractionClass(const rti1516::InteractionClassHandle& interactionClassHandle, bool active = true)
    {
      _ambassador->subscribeInteractionClass(interactionClassHandle, active);
    }

    void unsubscribeInteractionClass(const rti1516::InteractionClassHandle& interactionClassHandle)
    {
      _ambassador->unsubscribeInteractionClass(interactionClassHandle);
    }

    void reserveObjectInstanceName(const std::wstring& objectInstanceName)
    {
      _ambassador->reserveObjectInstanceName(objectInstanceName);
    }

    rti1516::ObjectInstanceHandle registerObjectInstance(const rti1516::ObjectClassHandle& objectClassHandle)
    {
      return _ambassador->registerObjectInstance(objectClassHandle);
    }

    rti1516::ObjectInstanceHandle registerObjectInstance(const rti1516::ObjectClassHandle& objectClassHandle,
                                                         const std::wstring& objectInstanceName)
    {
      return _ambassador->registerObjectInstance(objectClassHandle, objectInstanceName);
    }

    void updateAttributeValues(const rti1516::ObjectInstanceHandle& objectInstanceHandle,
                               const rti1516::AttributeHandleValueMap& attributeHandleValueMap,
                               const rti1516::VariableLengthData& tag)
    {
      _ambassador->updateAttributeValues(objectInstanceHandle, attributeHandleValueMap, tag);
    }

    rti1516::MessageRetractionHandle updateAttributeValues(const rti1516::ObjectInstanceHandle& objectInstanceHandle,
                                                           const rti1516::AttributeHandleValueMap& attributeHandleValueMap,
                                                           const rti1516::VariableLengthData& tag,
                                                           const rti1516::LogicalTime& logicalTime)
    {
      return _ambassador->updateAttributeValues(objectInstanceHandle, attributeHandleValueMap, tag, logicalTime);
    }

    void sendInteraction(const rti1516::InteractionClassHandle& interactionClassHandle,
                         const rti1516::ParameterHandleValueMap& parameterHandleValueMap,
                         const rti1516::VariableLengthData& tag)
    {
      _ambassador->sendInteraction(interactionClassHandle, parameterHandleValueMap, tag);
    }

    rti1516::MessageRetractionHandle sendInteraction(const rti1516::InteractionClassHandle& interactionClassHandle,
                                                     const rti1516::ParameterHandleValueMap& parameterHandleValueMap,
                                                     const rti1516::VariableLengthData& tag,
                                                     const rti1516::LogicalTime& logicalTime)
    {
      return _ambassador->sendInteraction(interactionClassHandle, parameterHandleValueMap, tag, logicalTime);
    }

    void deleteObjectInstance(const rti1516::ObjectInstanceHandle& objectInstanceHandle,
                              const rti1516::VariableLengthData& tag)
    {
      _ambassador->deleteObjectInstance(objectInstanceHandle, tag);
    }

    rti1516::MessageRetractionHandle deleteObjectInstance(const rti1516::ObjectInstanceHandle& objectInstanceHandle,
                                                          const rti1516::VariableLengthData& tag,
                                                          const rti1516::LogicalTime& logicalTime)
    {
      return _ambassador->deleteObjectInstance(objectInstanceHandle, tag, logicalTime);
    }

    void localDeleteObjectInstance(const rti1516::ObjectInstanceHandle& objectInstanceHandle)
    {
      _ambassador->localDeleteObjectInstance(objectInstanceHandle);
    }

    void changeAttributeTransportationType(const rti1516::ObjectInstanceHandle& objectInstanceHandle,
                                           const rti1516::AttributeHandleSet& attributeHandleSet,
                                           const rti1516::TransportationType& transportationType)
    {
      _ambassador->changeAttributeTransportationType(objectInstanceHandle, attributeHandleSet, transportationType);
    }

    void changeInteractionTransportationType(const rti1516::InteractionClassHandle& interactionClassHandle,
                                             const rti1516::TransportationType& transportationType)
    {
      _ambassador->changeInteractionTransportationType(interactionClassHandle, transportationType);
    }

    void requestAttributeValueUpdate(const rti1516::ObjectInstanceHandle& objectInstanceHandle,
                                     const rti1516::AttributeHandleSet& attributeHandleSet,
                                     const rti1516::VariableLengthData& tag)
    {
      _ambassador->requestAttributeValueUpdate(objectInstanceHandle, attributeHandleSet, tag);
    }

    void requestAttributeValueUpdate(const rti1516::ObjectClassHandle& objectClassHandle,
                                     const rti1516::AttributeHandleSet& attributeHandleSet,
                                     const rti1516::VariableLengthData& tag)
    {
      _ambassador->requestAttributeValueUpdate(objectClassHandle, attributeHandleSet, tag);
    }

    void unconditionalAttributeOwnershipDivestiture(const rti1516::ObjectInstanceHandle& objectInstanceHandle,
                                                    const rti1516::AttributeHandleSet& attributeHandleSet)
    {
      _ambassador->unconditionalAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleSet);
    }

    void negotiatedAttributeOwnershipDivestiture(const rti1516::ObjectInstanceHandle& objectInstanceHandle,
                                                 const rti1516::AttributeHandleSet& attributeHandleSet,
                                                 const rti1516::VariableLengthData& tag)
    {
      _ambassador->negotiatedAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleSet, tag);
    }

    void confirmDivestiture(const rti1516::ObjectInstanceHandle& objectInstanceHandle,
                            const rti1516::AttributeHandleSet& attributeHandleSet,
                            const rti1516::VariableLengthData& tag)
    {
      _ambassador->confirmDivestiture(objectInstanceHandle, attributeHandleSet, tag);
    }

    void attributeOwnershipAcquisition(const rti1516::ObjectInstanceHandle& objectInstanceHandle,
                                       const rti1516::AttributeHandleSet& attributeHandleSet,
                                       const rti1516::VariableLengthData& tag)
    {
      _ambassador->attributeOwnershipAcquisition(objectInstanceHandle, attributeHandleSet, tag);
    }

    void attributeOwnershipAcquisitionIfAvailable(const rti1516::ObjectInstanceHandle& objectInstanceHandle,
                                                  const rti1516::AttributeHandleSet& attributeHandleSet)
    {
      _ambassador->attributeOwnershipAcquisitionIfAvailable(objectInstanceHandle, attributeHandleSet);
    }

    void attributeOwnershipDivestitureIfWanted(const rti1516::ObjectInstanceHandle& objectInstanceHandle,
                                               const rti1516::AttributeHandleSet& attributeHandleSet,
                                               rti1516::AttributeHandleSet& divestedAttributeHandleSet)
    {
      _ambassador->attributeOwnershipDivestitureIfWanted(objectInstanceHandle, attributeHandleSet, divestedAttributeHandleSet);
    }

    void cancelNegotiatedAttributeOwnershipDivestiture(const rti1516::ObjectInstanceHandle& objectInstanceHandle,
                                                       const rti1516::AttributeHandleSet& attributeHandleSet)
    {
      _ambassador->cancelNegotiatedAttributeOwnershipDivestiture(objectInstanceHandle, attributeHandleSet);
    }

    void cancelAttributeOwnershipAcquisition(const rti1516::ObjectInstanceHandle& objectInstanceHandle,
                                             const rti1516::AttributeHandleSet& attributeHandleSet)
    {
      _ambassador->cancelAttributeOwnershipAcquisition(objectInstanceHandle, attributeHandleSet);
    }

    void queryAttributeOwnership(const rti1516::ObjectInstanceHandle& objectInstanceHandle,
                                 const rti1516::AttributeHandle& attributeHandle)
    {
      _ambassador->queryAttributeOwnership(objectInstanceHandle, attributeHandle);
    }

    bool isAttributeOwnedByFederate(const rti1516::ObjectInstanceHandle& objectInstanceHandle,
                                    const rti1516::AttributeHandle& attributeHandle)
    {
      return _ambassador->isAttributeOwnedByFederate(objectInstanceHandle, attributeHandle);
    }

    void enableTimeRegulation(const rti1516::LogicalTimeInterval& logicalTimeInterval)
    {
      _ambassador->enableTimeRegulation(logicalTimeInterval);
    }

    void disableTimeRegulation()
    {
      _timeRegulationEnabled = false;
      _ambassador->disableTimeRegulation();
    }

    void enableTimeConstrained()
    {
      _ambassador->enableTimeConstrained();
    }

    void disableTimeConstrained()
    {
      _timeConstrainedEnabled = false;
      _ambassador->disableTimeConstrained();
    }

    void timeAdvanceRequest(const rti1516::LogicalTime& logicalTime)
    {
      _timeAdvancePending = true;
      _ambassador->timeAdvanceRequest(logicalTime);
    }

    void timeAdvanceRequestAvailable(const rti1516::LogicalTime& logicalTime)
    {
      _timeAdvancePending = true;
      _ambassador->timeAdvanceRequestAvailable(logicalTime);
    }

    void nextMessageRequest(const rti1516::LogicalTime& logicalTime)
    {
      _timeAdvancePending = true;
      _ambassador->nextMessageRequest(logicalTime);
    }

    void nextMessageRequestAvailable(const rti1516::LogicalTime& logicalTime)
    {
      _timeAdvancePending = true;
      _ambassador->nextMessageRequestAvailable(logicalTime);
    }

    void flushQueueRequest(const rti1516::LogicalTime& logicalTime)
    {
      // _timeAdvancePending = true;
      _ambassador->flushQueueRequest(logicalTime);
    }

    void enableAsynchronousDelivery()
    {
      _ambassador->enableAsynchronousDelivery();
    }

    void disableAsynchronousDelivery()
    {
      _ambassador->disableAsynchronousDelivery();
    }

    bool queryGALT(rti1516::LogicalTime& logicalTime)
    {
      return _ambassador->queryGALT(logicalTime);
    }

    void queryLogicalTime(rti1516::LogicalTime& logicalTime)
    {
      _ambassador->queryLogicalTime(logicalTime);
    }

    bool queryLITS(rti1516::LogicalTime& logicalTime)
    {
      return _ambassador->queryLITS(logicalTime);
    }

    void modifyLookahead(const rti1516::LogicalTimeInterval& logicalTimeInterval)
    {
      _ambassador->modifyLookahead(logicalTimeInterval);
    }

    void queryLookahead(rti1516::LogicalTimeInterval& logicalTimeInterval)
    {
      _ambassador->queryLookahead(logicalTimeInterval);
    }

    void retract(const rti1516::MessageRetractionHandle& messageRetractionHandle)
    {
      _ambassador->retract(messageRetractionHandle);
    }

    void changeAttributeOrderType(const rti1516::ObjectInstanceHandle& objectInstanceHandle,
                                  const rti1516::AttributeHandleSet& attributeHandleSet,
                                  const rti1516::OrderType& orderType)
    {
      _ambassador->changeAttributeOrderType(objectInstanceHandle, attributeHandleSet, orderType);
    }

    void changeInteractionOrderType(const rti1516::InteractionClassHandle& interactionClassHandle,
                                    const rti1516::OrderType& orderType)
    {
      _ambassador->changeInteractionOrderType(interactionClassHandle, orderType);
    }

    rti1516::RegionHandle createRegion(const rti1516::DimensionHandleSet& dimensionHandleSet)
    {
      return _ambassador->createRegion(dimensionHandleSet);
    }

    void commitRegionModifications(const rti1516::RegionHandleSet& regionHandleSet)
    {
      _ambassador->commitRegionModifications(regionHandleSet);
    }

    void deleteRegion(const rti1516::RegionHandle& regionHandle)
    {
      _ambassador->deleteRegion(regionHandle);
    }

    rti1516::ObjectInstanceHandle registerObjectInstanceWithRegions(const rti1516::ObjectClassHandle& objectClassHandle,
                                                                    const rti1516::AttributeHandleSetRegionHandleSetPairVector& attributeHandleSetRegionHandleSetPairVector)
    {
      return _ambassador->registerObjectInstanceWithRegions(objectClassHandle, attributeHandleSetRegionHandleSetPairVector);
    }

    rti1516::ObjectInstanceHandle registerObjectInstanceWithRegions(const rti1516::ObjectClassHandle& objectClassHandle,
                                                                    const rti1516::AttributeHandleSetRegionHandleSetPairVector& attributeHandleSetRegionHandleSetPairVector,
                                                                    const std::wstring& objectInstanceName)
    {
      return _ambassador->registerObjectInstanceWithRegions(objectClassHandle, attributeHandleSetRegionHandleSetPairVector, objectInstanceName);
    }

    void associateRegionsForUpdates(const rti1516::ObjectInstanceHandle& objectInstanceHandle,
                                    const rti1516::AttributeHandleSetRegionHandleSetPairVector& attributeHandleSetRegionHandleSetPairVector)
    {
      _ambassador->associateRegionsForUpdates(objectInstanceHandle, attributeHandleSetRegionHandleSetPairVector);
    }

    void unassociateRegionsForUpdates(const rti1516::ObjectInstanceHandle& objectInstanceHandle,
                                      const rti1516::AttributeHandleSetRegionHandleSetPairVector& attributeHandleSetRegionHandleSetPairVector)
    {
      _ambassador->unassociateRegionsForUpdates(objectInstanceHandle, attributeHandleSetRegionHandleSetPairVector);
    }

    void subscribeObjectClassAttributesWithRegions(const rti1516::ObjectClassHandle& objectClassHandle,
                                                   const rti1516::AttributeHandleSetRegionHandleSetPairVector& attributeHandleSetRegionHandleSetPairVector,
                                                   bool active = true)
    {
      _ambassador->subscribeObjectClassAttributesWithRegions(objectClassHandle, attributeHandleSetRegionHandleSetPairVector, active);
    }

    void unsubscribeObjectClassAttributesWithRegions(const rti1516::ObjectClassHandle& objectClassHandle,
                                                     const rti1516::AttributeHandleSetRegionHandleSetPairVector& attributeHandleSetRegionHandleSetPairVector)
    {
      _ambassador->unsubscribeObjectClassAttributesWithRegions(objectClassHandle, attributeHandleSetRegionHandleSetPairVector);
    }

    void subscribeInteractionClassWithRegions(const rti1516::InteractionClassHandle& interactionClassHandle,
                                              const rti1516::RegionHandleSet& regionHandleSet,
                                              bool active = true)
    {
      _ambassador->subscribeInteractionClassWithRegions(interactionClassHandle, regionHandleSet, active);
    }

    void unsubscribeInteractionClassWithRegions(const rti1516::InteractionClassHandle& interactionClassHandle,
                                                const rti1516::RegionHandleSet& regionHandleSet)
    {
      _ambassador->unsubscribeInteractionClassWithRegions(interactionClassHandle, regionHandleSet);
    }

    void sendInteractionWithRegions(const rti1516::InteractionClassHandle& interactionClassHandle,
                                    const rti1516::ParameterHandleValueMap& parameterHandleValueMap,
                                    const rti1516::RegionHandleSet& regionHandleSet,
                                    const rti1516::VariableLengthData& tag)
    {
      _ambassador->sendInteractionWithRegions(interactionClassHandle, parameterHandleValueMap, regionHandleSet, tag);
    }

    rti1516::MessageRetractionHandle sendInteractionWithRegions(const rti1516::InteractionClassHandle& interactionClassHandle,
                                                                const rti1516::ParameterHandleValueMap& parameterHandleValueMap,
                                                                const rti1516::RegionHandleSet& regionHandleSet,
                                                                const rti1516::VariableLengthData& tag,
                                                                const rti1516::LogicalTime& logicalTime)
    {
      return _ambassador->sendInteractionWithRegions(interactionClassHandle, parameterHandleValueMap, regionHandleSet, tag, logicalTime);
    }

    void requestAttributeValueUpdateWithRegions(const rti1516::ObjectClassHandle& objectClassHandle,
                                                const rti1516::AttributeHandleSetRegionHandleSetPairVector& attributeHandleSetRegionHandleSetPairVector,
                                                const rti1516::VariableLengthData& tag)
    {
      _ambassador->requestAttributeValueUpdateWithRegions(objectClassHandle, attributeHandleSetRegionHandleSetPairVector, tag);
    }

    rti1516::ObjectClassHandle getObjectClassHandle(std::wstring const & theName)
    {
      return _ambassador->getObjectClassHandle(theName);
    }

    std::wstring getObjectClassName(rti1516::ObjectClassHandle theHandle)
    {
      return _ambassador->getObjectClassName(theHandle);
    }

    rti1516::AttributeHandle getAttributeHandle(rti1516::ObjectClassHandle whichClass, std::wstring const & attributeName)
    {
      return _ambassador->getAttributeHandle(whichClass, attributeName);
    }

    std::wstring getAttributeName(rti1516::ObjectClassHandle whichClass, rti1516::AttributeHandle theHandle)
    {
      return _ambassador->getAttributeName(whichClass, theHandle);
    }

    rti1516::InteractionClassHandle getInteractionClassHandle(std::wstring const & theName)
    {
      return _ambassador->getInteractionClassHandle(theName);
    }

    std::wstring getInteractionClassName(rti1516::InteractionClassHandle theHandle)
    {
      return _ambassador->getInteractionClassName(theHandle);
    }

    rti1516::ParameterHandle getParameterHandle(rti1516::InteractionClassHandle whichClass, std::wstring const & theName)
    {
      return _ambassador->getParameterHandle(whichClass, theName);
    }

    std::wstring getParameterName(rti1516::InteractionClassHandle whichClass, rti1516::ParameterHandle theHandle)
    {
      return _ambassador->getParameterName(whichClass, theHandle);
    }

    rti1516::ObjectInstanceHandle getObjectInstanceHandle(std::wstring const & theName)
    {
      return _ambassador->getObjectInstanceHandle(theName);
    }

    std::wstring getObjectInstanceName(rti1516::ObjectInstanceHandle theHandle)
    {
      return _ambassador->getObjectInstanceName(theHandle);
    }

    rti1516::DimensionHandle getDimensionHandle(std::wstring const & theName)
    {
      return _ambassador->getDimensionHandle(theName);
    }

    std::wstring getDimensionName(rti1516::DimensionHandle theHandle)
    {
      return _ambassador->getDimensionName(theHandle);
    }

    unsigned long getDimensionUpperBound(rti1516::DimensionHandle theHandle)
    {
      return _ambassador->getDimensionUpperBound(theHandle);
    }

    rti1516::DimensionHandleSet getAvailableDimensionsForClassAttribute(rti1516::ObjectClassHandle theClass,
                                                                        rti1516::AttributeHandle theHandle)
    {
      return _ambassador->getAvailableDimensionsForClassAttribute(theClass, theHandle);
    }

    rti1516::ObjectClassHandle getKnownObjectClassHandle(rti1516::ObjectInstanceHandle object)
    {
      return _ambassador->getKnownObjectClassHandle(object);
    }

    rti1516::DimensionHandleSet getAvailableDimensionsForInteractionClass(rti1516::InteractionClassHandle theClass)
    {
      return _ambassador->getAvailableDimensionsForInteractionClass(theClass);
    }

    rti1516::TransportationType getTransportationType(std::wstring const & transportationName)
    {
      return _ambassador->getTransportationType(transportationName);
    }

    std::wstring getTransportationName(rti1516::TransportationType transportationType)
    {
      return _ambassador->getTransportationName(transportationType);
    }

    rti1516::OrderType getOrderType(std::wstring const & orderName)
    {
      return _ambassador->getOrderType(orderName);
    }

    std::wstring getOrderName(rti1516::OrderType orderType)
    {
      return _ambassador->getOrderName(orderType);
    }

    void enableObjectClassRelevanceAdvisorySwitch()
    {
      _ambassador->enableObjectClassRelevanceAdvisorySwitch();
    }

    void disableObjectClassRelevanceAdvisorySwitch()
    {
      _ambassador->disableObjectClassRelevanceAdvisorySwitch();
    }

    void enableAttributeRelevanceAdvisorySwitch()
    {
      _ambassador->enableAttributeRelevanceAdvisorySwitch();
    }

    void disableAttributeRelevanceAdvisorySwitch()
    {
      _ambassador->disableAttributeRelevanceAdvisorySwitch();
    }

    void enableAttributeScopeAdvisorySwitch()
    {
      _ambassador->enableAttributeScopeAdvisorySwitch();
    }

    void disableAttributeScopeAdvisorySwitch()
    {
      _ambassador->disableAttributeScopeAdvisorySwitch();
    }

    void enableInteractionRelevanceAdvisorySwitch()
    {
      _ambassador->enableInteractionRelevanceAdvisorySwitch();
    }

    void disableInteractionRelevanceAdvisorySwitch()
    {
      _ambassador->disableInteractionRelevanceAdvisorySwitch();
    }

    rti1516::DimensionHandleSet getDimensionHandleSet(rti1516::RegionHandle regionHandle)
    {
      return _ambassador->getDimensionHandleSet(regionHandle);
    }

    rti1516::RangeBounds getRangeBounds(rti1516::RegionHandle regionHandle, rti1516::DimensionHandle theDimensionHandle)
    {
      return _ambassador->getRangeBounds(regionHandle, theDimensionHandle);
    }

    void setRangeBounds(rti1516::RegionHandle regionHandle, rti1516::DimensionHandle theDimensionHandle,
                        rti1516::RangeBounds const & rangeBounds)
    {
      return _ambassador->setRangeBounds(regionHandle, theDimensionHandle, rangeBounds);
    }

    unsigned long normalizeFederateHandle(rti1516::FederateHandle federateHandle)
    {
      return _ambassador->normalizeFederateHandle(federateHandle);
    }

    unsigned long normalizeServiceGroup(rti1516::ServiceGroupIndicator theServiceGroup)
    {
      return _ambassador->normalizeServiceGroup(theServiceGroup);
    }

    bool evokeCallback(double approximateMinimumTimeInSeconds)
    {
      return _ambassador->evokeCallback(approximateMinimumTimeInSeconds);
    }

    bool evokeMultipleCallbacks(double approximateMinimumTimeInSeconds,
                                double approximateMaximumTimeInSeconds)
    {
      return _ambassador->evokeMultipleCallbacks(approximateMinimumTimeInSeconds, approximateMaximumTimeInSeconds);
    }

    void enableCallbacks()
    {
      _ambassador->enableCallbacks();
    }

    void disableCallbacks()
    {
      _ambassador->disableCallbacks();
    }

    rti1516::FederateHandle decodeFederateHandle(rti1516::VariableLengthData const& encodedValue) const
    {
      return _ambassador->decodeFederateHandle(encodedValue);
    }

    rti1516::ObjectClassHandle decodeObjectClassHandle(rti1516::VariableLengthData const & encodedValue) const
    {
      return _ambassador->decodeObjectClassHandle(encodedValue);
    }

    rti1516::InteractionClassHandle decodeInteractionClassHandle(rti1516::VariableLengthData const & encodedValue) const
    {
      return _ambassador->decodeInteractionClassHandle(encodedValue);
    }

    rti1516::ObjectInstanceHandle decodeObjectInstanceHandle(rti1516::VariableLengthData const & encodedValue) const
    {
      return _ambassador->decodeObjectInstanceHandle(encodedValue);
    }

    rti1516::AttributeHandle decodeAttributeHandle(rti1516::VariableLengthData const & encodedValue) const
    {
      return _ambassador->decodeAttributeHandle(encodedValue);
    }

    rti1516::ParameterHandle decodeParameterHandle(rti1516::VariableLengthData const & encodedValue) const
    {
      return _ambassador->decodeParameterHandle(encodedValue);
    }

    rti1516::DimensionHandle decodeDimensionHandle(rti1516::VariableLengthData const & encodedValue) const
    {
      return _ambassador->decodeDimensionHandle(encodedValue);
    }

    rti1516::MessageRetractionHandle decodeMessageRetractionHandle(rti1516::VariableLengthData const & encodedValue) const
    {
      return _ambassador->decodeMessageRetractionHandle(encodedValue);
    }

    rti1516::RegionHandle decodeRegionHandle(rti1516::VariableLengthData const & encodedValue) const
    {
      return _ambassador->decodeRegionHandle(encodedValue);
    }

  protected:
    virtual void synchronizationPointRegistrationSucceeded(const std::wstring& label)
      throw (rti1516::FederateInternalError)
    {
    }

    virtual void synchronizationPointRegistrationFailed(const std::wstring& label, rti1516::SynchronizationFailureReason reason)
      throw (rti1516::FederateInternalError)
    {
    }

    virtual void announceSynchronizationPoint(const std::wstring& label, const rti1516::VariableLengthData& tag)
      throw (rti1516::FederateInternalError)
    {
    }

    virtual void federationSynchronized(const std::wstring& label)
      throw (rti1516::FederateInternalError)
    {
    }

    virtual void initiateFederateSave(const std::wstring& label)
      throw (rti1516::UnableToPerformSave,
             rti1516::FederateInternalError)
    {
    }

    virtual void initiateFederateSave(const std::wstring& label, const rti1516::LogicalTime& logicalTime)
      throw (rti1516::UnableToPerformSave,
             rti1516::InvalidLogicalTime,
             rti1516::FederateInternalError)
    {
    }

    virtual void federationSaved()
      throw (rti1516::FederateInternalError)
    {
    }

    virtual void federationNotSaved(rti1516::SaveFailureReason theSaveFailureReason)
      throw (rti1516::FederateInternalError)
    {
    }

    virtual void federationSaveStatusResponse(const rti1516::FederateHandleSaveStatusPairVector& federateStatusVector)
      throw (rti1516::FederateInternalError)
    {
    }

    virtual void requestFederationRestoreSucceeded(const std::wstring& label)
      throw (rti1516::FederateInternalError)
    {
    }

    virtual void requestFederationRestoreFailed(const std::wstring& label)
      throw (rti1516::FederateInternalError)
    {
    }

    virtual void federationRestoreBegun()
      throw (rti1516::FederateInternalError)
    {
    }

    virtual void initiateFederateRestore(const std::wstring& label, rti1516::FederateHandle handle)
      throw (rti1516::SpecifiedSaveLabelDoesNotExist,
             rti1516::CouldNotInitiateRestore,
             rti1516::FederateInternalError)
    {
    }

    virtual void federationRestored()
      throw (rti1516::FederateInternalError)
    {
    }

    virtual void federationNotRestored(rti1516::RestoreFailureReason restoreFailureReason)
      throw (rti1516::FederateInternalError)
    {
    }

    virtual void federationRestoreStatusResponse(const rti1516::FederateHandleRestoreStatusPairVector& federateStatusVector)
      throw (rti1516::FederateInternalError)
    {
    }

    virtual void startRegistrationForObjectClass(rti1516::ObjectClassHandle objectClassHandle)
      throw (rti1516::ObjectClassNotPublished,
             rti1516::FederateInternalError)
    {
    }

    virtual void stopRegistrationForObjectClass(rti1516::ObjectClassHandle objectClassHandle)
      throw (rti1516::ObjectClassNotPublished,
             rti1516::FederateInternalError)
    {
    }

    virtual void turnInteractionsOn(rti1516::InteractionClassHandle interactionClassHandle)
      throw (rti1516::InteractionClassNotPublished,
             rti1516::FederateInternalError)
    {
    }

    virtual void turnInteractionsOff(rti1516::InteractionClassHandle interactionClassHandle)
      throw (rti1516::InteractionClassNotPublished,
             rti1516::FederateInternalError)
    {
    }

    virtual void objectInstanceNameReservationSucceeded(const std::wstring& objectInstanceName)
      throw (rti1516::UnknownName,
             rti1516::FederateInternalError)
    {
    }

    virtual void objectInstanceNameReservationFailed(const std::wstring& objectInstanceName)
      throw (rti1516::UnknownName,
             rti1516::FederateInternalError)
    {
    }

    virtual void discoverObjectInstance(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                        rti1516::ObjectClassHandle objectClassHandle,
                                        const std::wstring& objectInstanceName)
      throw (rti1516::CouldNotDiscover,
             rti1516::ObjectClassNotKnown,
             rti1516::FederateInternalError)
    {
      _federate._discoverObjectInstance(objectInstanceHandle, objectClassHandle, objectInstanceName);
    }

    virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                        const rti1516::AttributeHandleValueMap& attributeHandleValueMap,
                                        const rti1516::VariableLengthData& tag,
                                        rti1516::OrderType orderType,
                                        rti1516::TransportationType transportationType)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::AttributeNotRecognized,
             rti1516::AttributeNotSubscribed,
             rti1516::FederateInternalError)
    {
      _federate._reflectAttributeValues(objectInstanceHandle, attributeHandleValueMap, tag);
    }

    virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                        const rti1516::AttributeHandleValueMap& attributeHandleValueMap,
                                        const rti1516::VariableLengthData& tag, rti1516::OrderType, rti1516::TransportationType,
                                        const rti1516::RegionHandleSet&)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::AttributeNotRecognized,
             rti1516::AttributeNotSubscribed,
             rti1516::FederateInternalError)
    {
      _federate._reflectAttributeValues(objectInstanceHandle, attributeHandleValueMap, tag);
    }

    virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                        const rti1516::AttributeHandleValueMap& attributeHandleValueMap,
                                        const rti1516::VariableLengthData& tag, rti1516::OrderType, rti1516::TransportationType,
                                        const rti1516::LogicalTime& logicalTime, rti1516::OrderType)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::AttributeNotRecognized,
             rti1516::AttributeNotSubscribed,
             rti1516::FederateInternalError)
    {
      _federate._reflectAttributeValues(objectInstanceHandle, attributeHandleValueMap, tag, logicalTime);
    }

    virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                        const rti1516::AttributeHandleValueMap& attributeHandleValueMap,
                                        const rti1516::VariableLengthData& tag, rti1516::OrderType, rti1516::TransportationType,
                                        const rti1516::LogicalTime& logicalTime, rti1516::OrderType, const rti1516::RegionHandleSet&)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::AttributeNotRecognized,
             rti1516::AttributeNotSubscribed,
             rti1516::FederateInternalError)
    {
      _federate._reflectAttributeValues(objectInstanceHandle, attributeHandleValueMap, tag, logicalTime);
    }

    virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                        const rti1516::AttributeHandleValueMap& attributeHandleValueMap,
                                        const rti1516::VariableLengthData& tag, rti1516::OrderType, rti1516::TransportationType,
                                        const rti1516::LogicalTime& logicalTime, rti1516::OrderType, rti1516::MessageRetractionHandle)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::AttributeNotRecognized,
             rti1516::AttributeNotSubscribed,
             rti1516::InvalidLogicalTime,
             rti1516::FederateInternalError)
    {
      _federate._reflectAttributeValues(objectInstanceHandle, attributeHandleValueMap, tag, logicalTime);
    }

    virtual void reflectAttributeValues(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                        const rti1516::AttributeHandleValueMap& attributeHandleValueMap,
                                        const rti1516::VariableLengthData& tag, rti1516::OrderType, rti1516::TransportationType,
                                        const rti1516::LogicalTime& logicalTime, rti1516::OrderType, rti1516::MessageRetractionHandle,
                                        const rti1516::RegionHandleSet&)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::AttributeNotRecognized,
             rti1516::AttributeNotSubscribed,
             rti1516::InvalidLogicalTime,
             rti1516::FederateInternalError)
    {
      _federate._reflectAttributeValues(objectInstanceHandle, attributeHandleValueMap, tag, logicalTime);
    }

    virtual void receiveInteraction(rti1516::InteractionClassHandle, const rti1516::ParameterHandleValueMap&,
                                    const rti1516::VariableLengthData&, rti1516::OrderType, rti1516::TransportationType)
      throw (rti1516::InteractionClassNotRecognized,
             rti1516::InteractionParameterNotRecognized,
             rti1516::InteractionClassNotSubscribed,
             rti1516::FederateInternalError)
    {
    }

    virtual void receiveInteraction(rti1516::InteractionClassHandle, const rti1516::ParameterHandleValueMap&,
                                    const rti1516::VariableLengthData&, rti1516::OrderType, rti1516::TransportationType,
                                    const rti1516::RegionHandleSet&)
      throw (rti1516::InteractionClassNotRecognized,
             rti1516::InteractionParameterNotRecognized,
             rti1516::InteractionClassNotSubscribed,
             rti1516::FederateInternalError)
    {
    }

    virtual void receiveInteraction(rti1516::InteractionClassHandle interaction,
                                    rti1516::ParameterHandleValueMap const & parameterValues,
                                    rti1516::VariableLengthData const & tag,
                                    rti1516::OrderType sentOrder,
                                    rti1516::TransportationType theType,
                                    rti1516::LogicalTime const & logicalTime,
                                    rti1516::OrderType receivedOrder)
      throw (rti1516::InteractionClassNotRecognized,
             rti1516::InteractionParameterNotRecognized,
             rti1516::InteractionClassNotSubscribed,
             rti1516::FederateInternalError)
    {
    }

    virtual void receiveInteraction(rti1516::InteractionClassHandle interaction,
                                    rti1516::ParameterHandleValueMap const & parameterValues,
                                    rti1516::VariableLengthData const & tag,
                                    rti1516::OrderType sentOrder,
                                    rti1516::TransportationType theType,
                                    rti1516::LogicalTime const & logicalTime,
                                    rti1516::OrderType receivedOrder,
                                    rti1516::RegionHandleSet const & theSentRegionHandleSet)
      throw (rti1516::InteractionClassNotRecognized,
             rti1516::InteractionParameterNotRecognized,
             rti1516::InteractionClassNotSubscribed,
             rti1516::FederateInternalError)
    {
    }

    virtual void receiveInteraction(rti1516::InteractionClassHandle interaction,
                                    rti1516::ParameterHandleValueMap const & parameterValues,
                                    rti1516::VariableLengthData const & tag,
                                    rti1516::OrderType sentOrder,
                                    rti1516::TransportationType theType,
                                    rti1516::LogicalTime const & logicalTime,
                                    rti1516::OrderType receivedOrder,
                                    rti1516::MessageRetractionHandle theHandle)
      throw (rti1516::InteractionClassNotRecognized,
             rti1516::InteractionParameterNotRecognized,
             rti1516::InteractionClassNotSubscribed,
             rti1516::InvalidLogicalTime,
             rti1516::FederateInternalError)
    {
    }

    virtual void receiveInteraction(rti1516::InteractionClassHandle interaction,
                                    rti1516::ParameterHandleValueMap const & parameterValues,
                                    rti1516::VariableLengthData const & tag,
                                    rti1516::OrderType sentOrder,
                                    rti1516::TransportationType theType,
                                    rti1516::LogicalTime const & logicalTime,
                                    rti1516::OrderType receivedOrder,
                                    rti1516::MessageRetractionHandle theHandle,
                                    rti1516::RegionHandleSet const & theSentRegionHandleSet)
      throw (rti1516::InteractionClassNotRecognized,
             rti1516::InteractionParameterNotRecognized,
             rti1516::InteractionClassNotSubscribed,
             rti1516::InvalidLogicalTime,
             rti1516::FederateInternalError)
    {
    }

    virtual void removeObjectInstance(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                      rti1516::VariableLengthData const & tag,
                                      rti1516::OrderType sentOrder)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::FederateInternalError)
    {
      _federate._removeObjectInstance(objectInstanceHandle);
    }

    virtual void removeObjectInstance(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                      rti1516::VariableLengthData const & tag,
                                      rti1516::OrderType sentOrder,
                                      rti1516::LogicalTime const & logicalTime,
                                      rti1516::OrderType receivedOrder)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::FederateInternalError)
    {
      _federate._removeObjectInstance(objectInstanceHandle);
    }

    virtual void removeObjectInstance(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                      rti1516::VariableLengthData const & tag,
                                      rti1516::OrderType sentOrder,
                                      rti1516::LogicalTime const & logicalTime,
                                      rti1516::OrderType receivedOrder,
                                      rti1516::MessageRetractionHandle theHandle)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::InvalidLogicalTime,
             rti1516::FederateInternalError)
    {
      _federate._removeObjectInstance(objectInstanceHandle);
    }

    virtual void attributesInScope(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                   rti1516::AttributeHandleSet const & attributes)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::AttributeNotRecognized,
             rti1516::AttributeNotSubscribed,
             rti1516::FederateInternalError)
    {
    }

    virtual void attributesOutOfScope(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                      rti1516::AttributeHandleSet const & attributes)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::AttributeNotRecognized,
             rti1516::AttributeNotSubscribed,
             rti1516::FederateInternalError)
    {
    }

    virtual void provideAttributeValueUpdate(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                             rti1516::AttributeHandleSet const & attributes,
                                             rti1516::VariableLengthData const & tag)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::AttributeNotRecognized,
             rti1516::AttributeNotOwned,
             rti1516::FederateInternalError)
    {
    }

    virtual void turnUpdatesOnForObjectInstance(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                                rti1516::AttributeHandleSet const & attributes)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::AttributeNotRecognized,
             rti1516::AttributeNotOwned,
             rti1516::FederateInternalError)
    {
    }

    virtual void turnUpdatesOffForObjectInstance(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                                 rti1516::AttributeHandleSet const & attributes)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::AttributeNotRecognized,
             rti1516::AttributeNotOwned,
             rti1516::FederateInternalError)
    {
    }

    virtual void requestAttributeOwnershipAssumption(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                                     rti1516::AttributeHandleSet const & offeredAttributes,
                                                     rti1516::VariableLengthData const & tag)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::AttributeNotRecognized,
             rti1516::AttributeAlreadyOwned,
             rti1516::AttributeNotPublished,
             rti1516::FederateInternalError)
    {
    }

    virtual void requestDivestitureConfirmation(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                                rti1516::AttributeHandleSet const & releasedAttributes)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::AttributeNotRecognized,
             rti1516::AttributeNotOwned,
             rti1516::AttributeDivestitureWasNotRequested,
             rti1516::FederateInternalError)
    {
    }

    virtual void attributeOwnershipAcquisitionNotification(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                                           rti1516::AttributeHandleSet const & securedAttributes,
                                                           rti1516::VariableLengthData const & tag)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::AttributeNotRecognized,
             rti1516::AttributeAcquisitionWasNotRequested,
             rti1516::AttributeAlreadyOwned,
             rti1516::AttributeNotPublished,
             rti1516::FederateInternalError)
    {
    }

    virtual void attributeOwnershipUnavailable(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                               rti1516::AttributeHandleSet const & attributes)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::AttributeNotRecognized,
             rti1516::AttributeAlreadyOwned,
             rti1516::AttributeAcquisitionWasNotRequested,
             rti1516::FederateInternalError)
    {
    }

    virtual void requestAttributeOwnershipRelease(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                                  rti1516::AttributeHandleSet const & candidateAttributes,
                                                  rti1516::VariableLengthData const & tag)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::AttributeNotRecognized,
             rti1516::AttributeNotOwned,
             rti1516::FederateInternalError)
    {
    }

    virtual void confirmAttributeOwnershipAcquisitionCancellation(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                                                  rti1516::AttributeHandleSet const & attributes)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::AttributeNotRecognized,
             rti1516::AttributeAlreadyOwned,
             rti1516::AttributeAcquisitionWasNotCanceled,
             rti1516::FederateInternalError)
    {
    }

    virtual void informAttributeOwnership(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                          rti1516::AttributeHandle attribute,
                                          rti1516::FederateHandle owner)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::AttributeNotRecognized,
             rti1516::FederateInternalError)
    {
    }

    virtual void attributeIsNotOwned(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                     rti1516::AttributeHandle attribute)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::AttributeNotRecognized,
             rti1516::FederateInternalError)
    {
    }

    virtual void attributeIsOwnedByRTI(rti1516::ObjectInstanceHandle objectInstanceHandle,
                                       rti1516::AttributeHandle attribute)
      throw (rti1516::ObjectInstanceNotKnown,
             rti1516::AttributeNotRecognized,
             rti1516::FederateInternalError)
    {
    }

    virtual void timeRegulationEnabled(const rti1516::LogicalTime& logicalTime)
      throw (rti1516::InvalidLogicalTime,
             rti1516::NoRequestToEnableTimeRegulationWasPending,
             rti1516::FederateInternalError)
    {
      _timeRegulationEnabled = true;
      *_grantedLogicalTime = logicalTime;
    }

    virtual void timeConstrainedEnabled(const rti1516::LogicalTime& logicalTime)
      throw (rti1516::InvalidLogicalTime,
             rti1516::NoRequestToEnableTimeConstrainedWasPending,
             rti1516::FederateInternalError)
    {
      _timeConstrainedEnabled = true;
      *_grantedLogicalTime = logicalTime;
    }

    virtual void timeAdvanceGrant(const rti1516::LogicalTime& logicalTime)
      throw (rti1516::InvalidLogicalTime,
             rti1516::JoinedFederateIsNotInTimeAdvancingState,
             rti1516::FederateInternalError)
    {
      _timeAdvancePending = false;
      *_grantedLogicalTime = logicalTime;
    }

    virtual void requestRetraction(rti1516::MessageRetractionHandle messageRetractionHandle)
      throw (rti1516::FederateInternalError)
    {
    }

    QRTI1516Federate& _federate;

    std::auto_ptr<rti1516::RTIambassador> _ambassador;

    std::wstring _logicalTimeImplementationName;
    std::auto_ptr<rti1516::LogicalTimeFactory> _logicalTimeFactory;

    rti1516::FederateHandle _federateHandle;

    std::auto_ptr<rti1516::LogicalTime> _grantedLogicalTime;
    bool _timeRegulationEnabled;
    bool _timeConstrainedEnabled;
    bool _timeAdvancePending;
  };

  QRTI1516Federate(QObject* parent = 0) :
    QRTIFederate(parent),
    _ambassador(*this)
  {
    startTimer(10);
  }

  virtual void join(const QString& federationExecution)
  {
    std::vector<std::wstring> args;
    _ambassador.connect(args);
    // ????
    _ambassador.setLogicalTimeFactory();
    // _ambassador.createFederationExecution(const std::wstring& federationExecutionName, const std::wstring& fddFile)
    _ambassador.joinFederationExecution(L"HLABrowser", federationExecution.toStdWString());
  }

  virtual void timerEvent(QTimerEvent*)
  {
    while (_ambassador.evokeCallback(0));
  }

  virtual bool readObjectModel(const QString& objectModelFile)
  {
    QFile file(objectModelFile);
    QXmlInputSource source(&file);
    QRTI1516OMTHandler handler;
    QXmlSimpleReader simpleReader;
    simpleReader.setErrorHandler(&handler);
    simpleReader.setContentHandler(&handler);
    if (!simpleReader.parse(&source, false))
      return false;

    return handler.mergeToFederate(*this);
  }

  virtual QRTIObjectClass* _createObjectClass(const QString& name, QRTIObjectClass* parentObjectClass = 0)
  {
    rti1516::ObjectClassHandle objectClassHandle;
    try {
      objectClassHandle = _ambassador.getObjectClassHandle(name.toStdWString());
    } catch (...) {
      return 0;
    }

    QRTI1516ObjectClass* objectClass = new QRTI1516ObjectClass(this, name, objectClassHandle);
    objectClass->_ambassador = &_ambassador;
    // register the object class with the federate.
    _insertObjectClass(objectClass);
    _objectClassMap[objectClassHandle] = objectClass;
    // register the object class with the parent object class.
    if (parentObjectClass)
      parentObjectClass->insertDerivedObjectClass(objectClass);
    return objectClass;
  }

  void _discoverObjectInstance(const rti1516::ObjectInstanceHandle& objectInstanceHandle,
                               const rti1516::ObjectClassHandle& objectClassHandle,
                               const std::wstring& objectInstanceName)
  {
    ObjectClassMap::const_iterator i = _objectClassMap.find(objectClassHandle);
    if (i == _objectClassMap.end())
      throw rti1516::ObjectClassNotKnown(objectClassHandle.toString());
    QRTI1516ObjectInstance* objectInstance;
    objectInstance = new QRTI1516ObjectInstance(this, i->second, QString::fromStdWString(objectInstanceName), objectInstanceHandle);
    // FIXME maintain a back reference to the object class to the object instances of this class
    /// i->second->_insertObjectInstance(objectInstance);
    // or may be also do this in the constructor ...
    _insertObjectInstance(objectInstance);
    _objectInstanceMap[objectInstanceHandle] = objectInstance;
  }

  void _reflectAttributeValues(const rti1516::ObjectInstanceHandle& objectInstanceHandle,
                               const rti1516::AttributeHandleValueMap& attributeHandleValueMap,
                               const rti1516::VariableLengthData& tag)
  {
    ObjectInstanceMap::const_iterator i = _objectInstanceMap.find(objectInstanceHandle);
    if (i == _objectInstanceMap.end())
      throw rti1516::ObjectInstanceNotKnown(objectInstanceHandle.toString());
    QRTI1516ObjectInstance* objectInstance = i->second;
    for (rti1516::AttributeHandleValueMap::const_iterator j = attributeHandleValueMap.begin();
         j != attributeHandleValueMap.end(); ++j)
      objectInstance->reflectObjectInstanceAttribute(j->first, j->second);
  }

  void _reflectAttributeValues(const rti1516::ObjectInstanceHandle& objectInstanceHandle,
                               const rti1516::AttributeHandleValueMap& attributeHandleValueMap,
                               const rti1516::VariableLengthData& tag, const rti1516::LogicalTime& logicalTime)
  {
    ObjectInstanceMap::const_iterator i = _objectInstanceMap.find(objectInstanceHandle);
    if (i == _objectInstanceMap.end())
      throw rti1516::ObjectInstanceNotKnown(objectInstanceHandle.toString());
    QRTI1516ObjectInstance* objectInstance = i->second;
    for (rti1516::AttributeHandleValueMap::const_iterator j = attributeHandleValueMap.begin();
         j != attributeHandleValueMap.end(); ++j)
      objectInstance->reflectObjectInstanceAttribute(j->first, j->second);
  }

  void _removeObjectInstance(const rti1516::ObjectInstanceHandle& objectInstanceHandle)
  {
    ObjectInstanceMap::iterator i = _objectInstanceMap.find(objectInstanceHandle);
    if (i == _objectInstanceMap.end())
      throw rti1516::ObjectInstanceNotKnown(objectInstanceHandle.toString());
    _eraseObjectInstance(i->second);
    _objectInstanceMap.erase(i);
  }

private:
  class QRTI1516ObjectClass;

  class QRTI1516ObjectClassAttribute : public QRTIObjectClassAttribute {
  public:
    QRTI1516ObjectClassAttribute(QRTIObjectClass* objectClass, const QString& name, const rti1516::AttributeHandle& attributeHandle) :
      QRTIObjectClassAttribute(objectClass, name),
      _objectClass(objectClass),
      _attributeHandle(attributeHandle)
    { }

    virtual QRTIObjectClass* getObjectClass()
    { return _objectClass; }

    QRTIObjectClass* _objectClass;
    rti1516::AttributeHandle _attributeHandle;
  };

  class QRTI1516ObjectClass : public QRTIObjectClass {
  public:
    QRTI1516ObjectClass(QObject* parent, const QString& name, const rti1516::ObjectClassHandle& objectClassHandle) :
      QRTIObjectClass(parent, name),
      _objectClassHandle(objectClassHandle),
      _ambassador(0)
    { }

    using QRTIObjectClass::getObjectClassAttributeIndex;
    int getObjectClassAttributeIndex(const rti1516::AttributeHandle& attributeHandle)
    {
      AttributeHandleIndexMap::const_iterator i = _attributeHandleIndexMap.find(attributeHandle);
      if (i == _attributeHandleIndexMap.end())
        return -1;
      return i->second;
    }

    using QRTIObjectClass::getObjectClassAttribute;
    QRTIObjectClassAttribute* getObjectClassAttribute(const rti1516::AttributeHandle& attributeHandle)
    {
      return QRTIObjectClass::getObjectClassAttribute(getObjectClassAttributeIndex(attributeHandle));
    }
    virtual QRTIObjectClassAttribute* _createObjectClassAttribute(const QString& name)
    {
      rti1516::AttributeHandle attributeHandle;
      try {
        attributeHandle = _ambassador->getAttributeHandle(_objectClassHandle, name.toStdWString());
      } catch (...) {
        return 0;
      }

      QRTI1516ObjectClassAttribute* objectClassAttribute = new QRTI1516ObjectClassAttribute(this, name, attributeHandle);
      // register the object class with the federate.
      // _insertObjectClassAttribute(objectClassAttribute);
      _attributeHandleIndexMap[attributeHandle] = objectClassAttribute->_indexInObjectClass;
      return objectClassAttribute;
    }

    virtual void subscribe()
    {
      /// FIXME think about something that sets things dirty and schedules an idle timeout timer to actually execute what is changed
      rti1516::AttributeHandleSet attributeHandleSet;
      for (AttributeHandleIndexMap::const_iterator i = _attributeHandleIndexMap.begin();
           i != _attributeHandleIndexMap.end(); ++i) {
        attributeHandleSet.insert(i->first);
      }

      try {
         _ambassador->subscribeObjectClassAttributes(_objectClassHandle, attributeHandleSet);
      } catch (...) {
      }
    }

    rti1516::ObjectClassHandle _objectClassHandle;
    Ambassador* _ambassador;

    typedef std::map<rti1516::AttributeHandle, int> AttributeHandleIndexMap;
    AttributeHandleIndexMap _attributeHandleIndexMap;
  };

  class QRTI1516ObjectInstance : public QRTIObjectInstance {
  public:
    QRTI1516ObjectInstance(QObject* parent, QRTI1516ObjectClass* objectClass, const QString& name, const rti1516::ObjectInstanceHandle& objectInstanceHandle) :
      QRTIObjectInstance(parent, objectClass, name),
      _objectInstanceHandle(objectInstanceHandle)
    { }

    QRTI1516ObjectClass* getObjectClass()
    { return static_cast<QRTI1516ObjectClass*>(_objectClass); }

    using QRTIObjectInstance::getObjectInstanceAttributeIndex;
    int getObjectInstanceAttributeIndex(const rti1516::AttributeHandle& attributeHandle)
    { return getObjectClass()->getObjectClassAttributeIndex(attributeHandle); }
    using QRTIObjectInstance::getObjectInstanceAttribute;
    QRTIObjectInstanceAttribute* getObjectInstanceAttribute(const rti1516::AttributeHandle& attributeHandle)
    { return getObjectInstanceAttribute(getObjectInstanceAttributeIndex(attributeHandle)); }
    void reflectObjectInstanceAttribute(const rti1516::AttributeHandle& attributeHandle,
                                        const rti1516::VariableLengthData& variableLengthData)
    {
      QRTIObjectInstanceAttribute* objectInstanceAttribute = getObjectInstanceAttribute(attributeHandle);
      if (!objectInstanceAttribute)
        return;
      unsigned long size = variableLengthData.size();
      objectInstanceAttribute->_rawData.resize(size);
      memcpy(objectInstanceAttribute->_rawData.data(), variableLengthData.data(), size);
      if (objectInstanceAttribute->_dataElement)
        objectInstanceAttribute->_dataElement->decode(objectInstanceAttribute->_rawData, 0);
      /// FIXME
      QRTIFederate* federate = static_cast<QRTIFederate*>(parent());
      federate->_objectInstanceModel->objectAttributeChanged(objectInstanceAttribute);
    }

    rti1516::ObjectInstanceHandle _objectInstanceHandle;
  };

  Ambassador _ambassador;

  typedef std::map<rti1516::ObjectClassHandle, QRTI1516ObjectClass*> ObjectClassMap;
  ObjectClassMap _objectClassMap;

  typedef std::map<rti1516::ObjectInstanceHandle, QRTI1516ObjectInstance*> ObjectInstanceMap;
  ObjectInstanceMap _objectInstanceMap;
};

#endif
