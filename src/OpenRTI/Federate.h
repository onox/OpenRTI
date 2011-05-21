/* -*-c++-*- OpenRTI - Copyright (C) 2009-2011 Mathias Froehlich
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

#ifndef OpenRTI_Federate_h
#define OpenRTI_Federate_h

#include <list>

#include "AbstractFederate.h"
#include "HandleAllocator.h"
#include "LogStream.h"
#include "Region.h"

namespace OpenRTI {

template<typename T, typename L>
class OPENRTI_LOCAL Federate : public AbstractFederate<T> {
public:
  typedef typename AbstractFederate<T>::Traits Traits;

  typedef typename AbstractFederate<T>::NativeLogicalTime NativeLogicalTime;
  typedef typename AbstractFederate<T>::NativeLogicalTimeInterval NativeLogicalTimeInterval;

  typedef L LogicalTimeFactory;
  typedef typename LogicalTimeFactory::LogicalTime LogicalTime;
  typedef typename LogicalTimeFactory::LogicalTimeInterval LogicalTimeInterval;

  // The exceptions
  typedef typename AbstractFederate<T>::AlreadyConnected AlreadyConnected;
  typedef typename AbstractFederate<T>::AsynchronousDeliveryAlreadyDisabled AsynchronousDeliveryAlreadyDisabled;
  typedef typename AbstractFederate<T>::AsynchronousDeliveryAlreadyEnabled AsynchronousDeliveryAlreadyEnabled;
  typedef typename AbstractFederate<T>::AttributeAcquisitionWasNotRequested AttributeAcquisitionWasNotRequested;
  typedef typename AbstractFederate<T>::AttributeAlreadyBeingAcquired AttributeAlreadyBeingAcquired;
  typedef typename AbstractFederate<T>::AttributeAlreadyBeingDivested AttributeAlreadyBeingDivested;
  typedef typename AbstractFederate<T>::AttributeAlreadyOwned AttributeAlreadyOwned;
  typedef typename AbstractFederate<T>::AttributeDivestitureWasNotRequested AttributeDivestitureWasNotRequested;
  typedef typename AbstractFederate<T>::AttributeNotDefined AttributeNotDefined;
  typedef typename AbstractFederate<T>::AttributeNotOwned AttributeNotOwned;
  typedef typename AbstractFederate<T>::AttributeNotPublished AttributeNotPublished;
  typedef typename AbstractFederate<T>::AttributeRelevanceAdvisorySwitchIsOff AttributeRelevanceAdvisorySwitchIsOff;
  typedef typename AbstractFederate<T>::AttributeRelevanceAdvisorySwitchIsOn AttributeRelevanceAdvisorySwitchIsOn;
  typedef typename AbstractFederate<T>::AttributeScopeAdvisorySwitchIsOff AttributeScopeAdvisorySwitchIsOff;
  typedef typename AbstractFederate<T>::AttributeScopeAdvisorySwitchIsOn AttributeScopeAdvisorySwitchIsOn;
  typedef typename AbstractFederate<T>::CallNotAllowedFromWithinCallback CallNotAllowedFromWithinCallback;
  typedef typename AbstractFederate<T>::ConnectionFailed ConnectionFailed;
  typedef typename AbstractFederate<T>::CouldNotCreateLogicalTimeFactory CouldNotCreateLogicalTimeFactory;
  typedef typename AbstractFederate<T>::CouldNotOpenFDD CouldNotOpenFDD;
  typedef typename AbstractFederate<T>::CouldNotOpenMIM CouldNotOpenMIM;
  typedef typename AbstractFederate<T>::DeletePrivilegeNotHeld DeletePrivilegeNotHeld;
  typedef typename AbstractFederate<T>::ErrorReadingFDD ErrorReadingFDD;
  typedef typename AbstractFederate<T>::ErrorReadingMIM ErrorReadingMIM;
  typedef typename AbstractFederate<T>::FederateAlreadyExecutionMember FederateAlreadyExecutionMember;
  typedef typename AbstractFederate<T>::FederateNameAlreadyInUse FederateNameAlreadyInUse;
  typedef typename AbstractFederate<T>::FederateHasNotBegunSave FederateHasNotBegunSave;
  typedef typename AbstractFederate<T>::FederateIsExecutionMember FederateIsExecutionMember;
  typedef typename AbstractFederate<T>::FederateNotExecutionMember FederateNotExecutionMember;
  typedef typename AbstractFederate<T>::FederateOwnsAttributes FederateOwnsAttributes;
  typedef typename AbstractFederate<T>::FederatesCurrentlyJoined FederatesCurrentlyJoined;
  typedef typename AbstractFederate<T>::FederateServiceInvocationsAreBeingReportedViaMOM FederateServiceInvocationsAreBeingReportedViaMOM;
  typedef typename AbstractFederate<T>::FederateUnableToUseTime FederateUnableToUseTime;
  typedef typename AbstractFederate<T>::FederationExecutionAlreadyExists FederationExecutionAlreadyExists;
  typedef typename AbstractFederate<T>::FederationExecutionDoesNotExist FederationExecutionDoesNotExist;
  typedef typename AbstractFederate<T>::IllegalName IllegalName;
  typedef typename AbstractFederate<T>::InconsistentFDD InconsistentFDD;
  typedef typename AbstractFederate<T>::InteractionClassNotDefined InteractionClassNotDefined;
  typedef typename AbstractFederate<T>::InteractionClassNotPublished InteractionClassNotPublished;
  typedef typename AbstractFederate<T>::InteractionParameterNotDefined InteractionParameterNotDefined;
  typedef typename AbstractFederate<T>::InteractionRelevanceAdvisorySwitchIsOff InteractionRelevanceAdvisorySwitchIsOff;
  typedef typename AbstractFederate<T>::InteractionRelevanceAdvisorySwitchIsOn InteractionRelevanceAdvisorySwitchIsOn;
  typedef typename AbstractFederate<T>::InTimeAdvancingState InTimeAdvancingState;
  typedef typename AbstractFederate<T>::InvalidAttributeHandle InvalidAttributeHandle;
  typedef typename AbstractFederate<T>::InvalidDimensionHandle InvalidDimensionHandle;
  typedef typename AbstractFederate<T>::InvalidFederateHandle InvalidFederateHandle;
  typedef typename AbstractFederate<T>::InvalidInteractionClassHandle InvalidInteractionClassHandle;
  typedef typename AbstractFederate<T>::InvalidLocalSettingsDesignator InvalidLocalSettingsDesignator;
  typedef typename AbstractFederate<T>::InvalidLogicalTime InvalidLogicalTime;
  typedef typename AbstractFederate<T>::InvalidLookahead InvalidLookahead;
  typedef typename AbstractFederate<T>::InvalidObjectClassHandle InvalidObjectClassHandle;
  typedef typename AbstractFederate<T>::InvalidOrderName InvalidOrderName;
  typedef typename AbstractFederate<T>::InvalidOrderType InvalidOrderType;
  typedef typename AbstractFederate<T>::InvalidParameterHandle InvalidParameterHandle;
  typedef typename AbstractFederate<T>::InvalidRangeBound InvalidRangeBound;
  typedef typename AbstractFederate<T>::InvalidRegion InvalidRegion;
  typedef typename AbstractFederate<T>::InvalidRegionContext InvalidRegionContext;
  typedef typename AbstractFederate<T>::InvalidResignAction InvalidResignAction;
  typedef typename AbstractFederate<T>::InvalidRetractionHandle InvalidRetractionHandle;
  typedef typename AbstractFederate<T>::InvalidServiceGroup InvalidServiceGroup;
  typedef typename AbstractFederate<T>::InvalidTransportationName InvalidTransportationName;
  typedef typename AbstractFederate<T>::InvalidTransportationType InvalidTransportationType;
  typedef typename AbstractFederate<T>::LogicalTimeAlreadyPassed LogicalTimeAlreadyPassed;
  typedef typename AbstractFederate<T>::MessageCanNoLongerBeRetracted MessageCanNoLongerBeRetracted;
  typedef typename AbstractFederate<T>::NameNotFound NameNotFound;
  typedef typename AbstractFederate<T>::NameSetWasEmpty NameSetWasEmpty;
  typedef typename AbstractFederate<T>::NoAcquisitionPending NoAcquisitionPending;
  typedef typename AbstractFederate<T>::NotConnected NotConnected;
  typedef typename AbstractFederate<T>::ObjectClassNotDefined ObjectClassNotDefined;
  typedef typename AbstractFederate<T>::ObjectClassNotPublished ObjectClassNotPublished;
  typedef typename AbstractFederate<T>::ObjectClassRelevanceAdvisorySwitchIsOff ObjectClassRelevanceAdvisorySwitchIsOff;
  typedef typename AbstractFederate<T>::ObjectClassRelevanceAdvisorySwitchIsOn ObjectClassRelevanceAdvisorySwitchIsOn;
  typedef typename AbstractFederate<T>::ObjectInstanceNameInUse ObjectInstanceNameInUse;
  typedef typename AbstractFederate<T>::ObjectInstanceNameNotReserved ObjectInstanceNameNotReserved;
  typedef typename AbstractFederate<T>::ObjectInstanceNotKnown ObjectInstanceNotKnown;
  typedef typename AbstractFederate<T>::OwnershipAcquisitionPending OwnershipAcquisitionPending;
  typedef typename AbstractFederate<T>::RegionDoesNotContainSpecifiedDimension RegionDoesNotContainSpecifiedDimension;
  typedef typename AbstractFederate<T>::RegionInUseForUpdateOrSubscription RegionInUseForUpdateOrSubscription;
  typedef typename AbstractFederate<T>::RegionNotCreatedByThisFederate RegionNotCreatedByThisFederate;
  typedef typename AbstractFederate<T>::RequestForTimeConstrainedPending RequestForTimeConstrainedPending;
  typedef typename AbstractFederate<T>::RequestForTimeRegulationPending RequestForTimeRegulationPending;
  typedef typename AbstractFederate<T>::RestoreInProgress RestoreInProgress;
  typedef typename AbstractFederate<T>::RestoreNotRequested RestoreNotRequested;
  typedef typename AbstractFederate<T>::RTIinternalError RTIinternalError;
  typedef typename AbstractFederate<T>::SaveInProgress SaveInProgress;
  typedef typename AbstractFederate<T>::SaveNotInitiated SaveNotInitiated;
  typedef typename AbstractFederate<T>::SynchronizationPointLabelNotAnnounced SynchronizationPointLabelNotAnnounced;
  typedef typename AbstractFederate<T>::TimeConstrainedAlreadyEnabled TimeConstrainedAlreadyEnabled;
  typedef typename AbstractFederate<T>::TimeConstrainedIsNotEnabled TimeConstrainedIsNotEnabled;
  typedef typename AbstractFederate<T>::TimeRegulationAlreadyEnabled TimeRegulationAlreadyEnabled;
  typedef typename AbstractFederate<T>::TimeRegulationIsNotEnabled TimeRegulationIsNotEnabled;
  typedef typename AbstractFederate<T>::UnsupportedCallbackModel UnsupportedCallbackModel;

  typedef std::map<std::string, InteractionClassHandle> NameInteractionClassHandleMap;
  typedef std::map<std::string, ParameterHandle> NameParameterHandleMap;
  typedef std::map<std::string, ObjectClassHandle> NameObjectClassHandleMap;
  typedef std::map<std::string, ObjectInstanceHandle> NameObjectInstanceHandleMap;
  typedef std::map<std::string, AttributeHandle> NameAttributeHandleMap;
  typedef std::map<std::string, DimensionHandle> NameDimensionHandleMap;

  typedef std::map<std::string, TransportationType> NameTransportationTypeMap;
  typedef std::map<TransportationType, std::string> TransportationTypeNameMap;

  typedef std::map<std::string, OrderType> NameOrderTypeMap;
  typedef std::map<OrderType, std::string> OrderTypeNameMap;

  // Is used to map the available dimension handles to integer indices.
  // Internally, regions are only accessed by these indices.
  typedef std::map<DimensionHandle, unsigned> DimensionHandleIndexMap;

  // Do the maps with keys like std::pair<LogicalTime, bool>, where the unsigned is 0 when comparing with
  // strictly less and 1 when comparing with less equal. FIXME does this work??
  typedef std::pair<LogicalTime, bool> LogicalTimePair;

  struct OPENRTI_LOCAL PublishSubscribe : public Referenced {
    PublishSubscribe() :
      _orderType(RECEIVE),
      _transportationType(RELIABLE),
      _subscriptionType(Unsubscribed),
      _publicationType(Unpublished)
    { }
    std::string _name;
    OrderType _orderType;
    TransportationType _transportationType;
    DimensionHandleSet _dimensionHandleSet; // Make that vanish in favour of the below
    // DimensionHandleIndexMap _dimensionHandleIndexMap;
    SubscriptionType _subscriptionType;
    PublicationType _publicationType;

    SubscriptionType getSubscriptionType() const
    { return _subscriptionType; }
    bool setSubscriptionType(SubscriptionType subscriptionType)
    { std::swap(_subscriptionType, subscriptionType); return _subscriptionType != subscriptionType; }
    bool isSubscribed() const
    { return Unsubscribed != _subscriptionType; }

    PublicationType getPublicationType() const
    { return _publicationType; }
    bool setPublicationType(PublicationType publicationType)
    { std::swap(_publicationType, publicationType); return _publicationType != publicationType; }
    bool isPublished() const
    { return Unpublished != _publicationType; }
  };

  struct OPENRTI_LOCAL Dimension : public Referenced {
    Dimension(const std::string& name, unsigned long upperBound) :
      _name(name), _upperBound(upperBound)
    { }
    const std::string _name;
    const unsigned long _upperBound;
  };
  typedef std::vector<SharedPtr<Dimension> > DimensionVector;

  // struct OPENRTI_LOCAL RoutingSpace {
  //   DimensionHandleIndexMap _dimensionHandleIndexMap;
  // };

  struct OPENRTI_LOCAL RegionData {
    Region _region;
    DimensionHandleSet _dimensionHandleSet;
  };
  typedef std::map<RegionHandle, RegionData> RegionHandleRegionDataMap;

  struct OPENRTI_LOCAL Parameter : public Referenced {
    Parameter(const std::string& name) :
      _name(name)
    { }
    const std::string _name;
  };
  typedef std::vector<SharedPtr<Parameter> > ParameterVector;
  struct OPENRTI_LOCAL InteractionClass : public PublishSubscribe {
    std::string _fqnName;
    InteractionClassHandle _parentInteractionClassHandle;
    ParameterVector _parameterVector;
    NameParameterHandleMap _nameParameterHandleMap;

    bool isValidParameter(const ParameterHandle& parameterHandle) const
    { return parameterHandle.getHandle() < _parameterVector.size() && _parameterVector[parameterHandle.getHandle()].valid(); }
  };
  typedef std::vector<SharedPtr<InteractionClass> > InteractionClassVector;

  struct OPENRTI_LOCAL Attribute : public PublishSubscribe {
  };
  typedef std::vector<SharedPtr<Attribute> > AttributeVector;
  struct OPENRTI_LOCAL ObjectClass : public Referenced {
    ObjectClass() : _subscriptionType(Unsubscribed), _publicationType(Unpublished) { }
    std::string _name;
    std::string _fqnName;
    ObjectClassHandle _parentObjectClassHandle;
    AttributeVector _attributeVector;
    NameAttributeHandleMap _nameAttributeHandleMap;
    // Because of implicit publication/subscription
    SubscriptionType _subscriptionType;
    PublicationType _publicationType;

    // returns true if this is a valid object class attribute, note that the object class should be already checked
    bool isValidAttribute(const AttributeHandle& attributeHandle) const
    { return attributeHandle.getHandle() < _attributeVector.size() && _attributeVector[attributeHandle.getHandle()].valid(); }

    // Returns true if the attribute is subscribed.
    bool isAttributeSubscribed(const AttributeHandle& attributeHandle) const
    { return Unsubscribed != getEffectiveAttributeSubscriptionType(attributeHandle); }
    SubscriptionType getEffectiveAttributeSubscriptionType(const AttributeHandle& attributeHandle) const
    {
      SubscriptionType subscriptionType = _attributeVector[attributeHandle.getHandle()]->_subscriptionType;
      if (attributeHandle == AttributeHandle(0))
        subscriptionType = std::max(subscriptionType, _subscriptionType);
      return subscriptionType;
    }
    SubscriptionType getAttributeSubscriptionType(const AttributeHandle& attributeHandle) const
    { return _attributeVector[attributeHandle.getHandle()]->_subscriptionType; }
    // Sets the local subscription status.
    bool setAttributeSubscriptionType(const AttributeHandle& attributeHandle, SubscriptionType subscriptionType)
    {
      SubscriptionType oldSubscriptionType = getEffectiveAttributeSubscriptionType(attributeHandle);
      _attributeVector[attributeHandle.getHandle()]->_subscriptionType = subscriptionType;
      return oldSubscriptionType != getEffectiveAttributeSubscriptionType(attributeHandle);
    }

    SubscriptionType getEffectiveSubscriptionType() const
    { return std::max(_subscriptionType, _attributeVector[0]->_subscriptionType); }
    SubscriptionType getSubscriptionType() const
    { return _subscriptionType; }
    bool setSubscriptionType(SubscriptionType subscriptionType)
    {
      SubscriptionType oldSubscriptionType = getEffectiveSubscriptionType();
      _subscriptionType = subscriptionType;
      return oldSubscriptionType != getEffectiveSubscriptionType();
    }

    // Returns true if the attribute is published.
    bool isAttributePublished(const AttributeHandle& attributeHandle) const
    { return Unpublished != getEffectiveAttributePublicationType(attributeHandle); }
    PublicationType getEffectiveAttributePublicationType(const AttributeHandle& attributeHandle) const
    {
      PublicationType publicationType = _attributeVector[attributeHandle.getHandle()]->_publicationType;
      if (attributeHandle == AttributeHandle(0))
        publicationType = std::max(publicationType, _publicationType);
      return publicationType;
    }
    PublicationType getAttributePublicationType(const AttributeHandle& attributeHandle) const
    { return _attributeVector[attributeHandle.getHandle()]->_publicationType; }
    // Sets the local publish status. Returns true if the publish type changed with this call.
    bool setAttributePublicationType(const AttributeHandle& attributeHandle, PublicationType publicationType)
    {
      PublicationType oldPublicationType = getEffectiveAttributePublicationType(attributeHandle);
      _attributeVector[attributeHandle.getHandle()]->_publicationType = publicationType;
      return oldPublicationType != getEffectiveAttributePublicationType(attributeHandle);
    }

    PublicationType getEffectivePublicationType() const
    { return std::max(_publicationType, _attributeVector[0]->_publicationType); }
    PublicationType getPublicationType() const
    { return _publicationType; }
    bool setPublicationType(PublicationType publicationType)
    {
      PublicationType oldPublicationType = getEffectivePublicationType();
      _publicationType = publicationType;
      return oldPublicationType != getEffectivePublicationType();
    }
  };
  typedef std::vector<SharedPtr<ObjectClass> > ObjectClassVector;

  struct OPENRTI_LOCAL InstanceAttribute : public Referenced {
    InstanceAttribute(bool isOwnedByFederate = false, OrderType orderType = RECEIVE, TransportationType transportationType = RELIABLE) :
      _isOwnedByFederate(isOwnedByFederate),
      _orderType(orderType),
      _transportationType(transportationType)
    {}
    bool _isOwnedByFederate;
    OrderType _orderType;
    TransportationType _transportationType;
  };
  typedef std::vector<SharedPtr<InstanceAttribute> > InstanceAttributeVector;
  struct OPENRTI_LOCAL ObjectInstance {
    ObjectInstance(typename NameObjectInstanceHandleMap::iterator nameObjectInstanceHandleMapIterator) :
      _nameObjectInstanceHandleMapIterator(nameObjectInstanceHandleMapIterator)
    { }
    // This is the known object class of this instance, which might be something less than the original one
    ObjectClassHandle _objectClassHandle;
    typename NameObjectInstanceHandleMap::iterator _nameObjectInstanceHandleMapIterator;
    InstanceAttributeVector _instanceAttributeVector;
  };
  typedef std::map<ObjectInstanceHandle, ObjectInstance> ObjectInstanceHandleMap;

  Federate(const std::string& federateType, const std::string& federateName,
           const FederateHandle& federateHandle, SharedPtr<AbstractConnect> connect,
           const InsertFederationExecutionMessage& insertFederationExecution,
           const LogicalTimeFactory& logicalTimeFactory) :
    _federationConnect(connect),
    _federateType(federateType),
    _federateName(federateName),
    _federateHandle(federateHandle),
    _federationHandle(insertFederationExecution.getFederationHandle()),
    _defaultResignAction(CANCEL_THEN_DELETE_THEN_DIVEST),
    _objectClassRelevanceAdvisorySwitchEnabled(false),
    _attributeRelevanceAdvisorySwitchEnabled(false),
    _attributeScopeAdvisorySwitchEnabled(false),
    _interactionRelevanceAdvisorySwitchEnabled(false),
    _nextMessageMode(false),
    _timeRegulationEnablePending(false),
    _timeRegulationEnabled(false),
    _timeConstrainedEnablePending(false),
    _timeConstrainedEnabled(false),
    _timeAdvancePending(false),
    _flushQueueMode(false),
    _permitTimeRegulation(true),
    _asynchronousDeliveryEnabled(true),
    _messageRetractionSerial(0),
    _logicalTimeFactory(logicalTimeFactory)
  {
    for (FOMModuleList::const_iterator i = insertFederationExecution.getFOMModuleList().begin();
         i != insertFederationExecution.getFOMModuleList().end(); ++i)
      insert(*i);

    ConfigurationParameterMap::const_iterator i;
    // time regulation is by default permitted, but may be denied due to parent server policy
    i = insertFederationExecution.getConfigurationParameterMap().find("permitTimeRegulation");
    if (i != insertFederationExecution.getConfigurationParameterMap().end() && !i->second.empty() && i->second.front() != "true")
      _permitTimeRegulation = false;

    _logicalTime = _logicalTimeFactory.initialLogicalTime();
    _pendingLogicalTime.first = _logicalTime;
    _pendingLogicalTime.second = true;
    _localLowerBoundTimeStamp.first = _logicalTime;
    _localLowerBoundTimeStamp.second = true;
    _currentLookahead = _logicalTimeFactory.zeroLogicalTimeInterval();
    _targetLookahead = _currentLookahead;
  }
  virtual ~Federate()
  { }

  const std::string& getFederateType() const
  { return _federateType; }
  const std::string& getFederateName() const
  { return _federateName; }
  const FederateHandle& getFederateHandle() const
  { return _federateHandle; }
  const FederationHandle& getFederationHandle() const
  { return _federationHandle; }

  virtual std::string getLogicalTimeFactoryName() const
  { return _logicalTimeFactory.getName(); }

  // that might move to something internal??
  virtual void requestObjectInstanceHandles(unsigned count)
  {
    SharedPtr<ObjectInstanceHandlesRequestMessage> request;
    request = new ObjectInstanceHandlesRequestMessage;
    request->setFederationHandle(getFederationHandle());
    request->setFederateHandle(getFederateHandle());
    request->setCount(count);
    sendMessage(request);
  }

  virtual void resignFederationExecution(ResignAction resignAction, const Clock& clock)
    throw (OwnershipAcquisitionPending,
           FederateOwnsAttributes,
           RTIinternalError)
  {
    // Puh FIXME: have a concept for this in the server nodes instead of relying on that here
    try {
      for (size_t i = 0; i < _objectClassVector.size(); ++i) {
        unsubscribeObjectClass(i);
      }
      for (size_t i = 0; i < _interactionClassVector.size(); ++i) {
        unsubscribeInteractionClass(i);
      }
    } catch (...) {
    }

    try {
      // delete object instances if requested
      bool deleteObjects = resignAction == DELETE_OBJECTS ||
        resignAction == DELETE_OBJECTS_THEN_DIVEST || resignAction == CANCEL_THEN_DELETE_THEN_DIVEST;
      // FIXME: currently we do not have ownership management - so, if the owner dies the object needs to die too
      deleteObjects = true;

      if (deleteObjects) {
        // FIXME: do not walk all! have a list of 'own objects'
        for (typename ObjectInstanceHandleMap::iterator i = _objectInstanceHandleMap.begin(); i != _objectInstanceHandleMap.end();) {
          if (!i->second._instanceAttributeVector[AttributeHandle(0)]->_isOwnedByFederate)
            ++i;
          else {
            deleteObjectInstance((i++)->first, VariableLengthData("Delete on resign FIXME!"));
          }
        }
      }
    } catch (...) {
    }

    try {
      for (size_t i = 0; i < _objectClassVector.size(); ++i) {
        unpublishObjectClass(i);
      }
      for (size_t i = 0; i < _interactionClassVector.size(); ++i) {
        unpublishInteractionClass(i);
      }
    } catch (...) {
    }

    // Now release the resources this federate owns
    try {
      SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> message = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
      message->setFederationHandle(getFederationHandle());
      // FIXME
      ObjectInstanceHandleSet objectInstanceHandleSet;
      for (NameObjectInstanceHandleMap::iterator i = _reservedNameObjectInstanceHandlePairs.begin();
           i != _reservedNameObjectInstanceHandlePairs.end(); ++i) {
        objectInstanceHandleSet.insert(i->second);
      }
      _reservedNameObjectInstanceHandlePairs.clear();
      for (NameObjectInstanceHandleMap::iterator i = _privateNameObjectInstanceHandlePairs.begin();
           i != _privateNameObjectInstanceHandlePairs.end(); ++i) {
        objectInstanceHandleSet.insert(i->second);
      }
      _privateNameObjectInstanceHandlePairs.clear();
      for (typename ObjectInstanceHandleMap::iterator i = _objectInstanceHandleMap.begin(); i != _objectInstanceHandleMap.end(); ++i) {
        objectInstanceHandleSet.insert(i->first);
      }
      _objectInstanceHandleMap.clear();
      message->getObjectInstanceHandleVector().resize(objectInstanceHandleSet.size());
      std::copy(objectInstanceHandleSet.begin(), objectInstanceHandleSet.end(), message->getObjectInstanceHandleVector().begin());
      sendMessage(message);
    } catch (...) {
    }

    SharedPtr<EraseRegionMessage> eraseRegionRequest;
    for (typename RegionHandleRegionDataMap::iterator i = _regionHandleRegionDataMap.begin();
         i != _regionHandleRegionDataMap.end(); ++i) {
      if (i->first.getFederateHandle() != getFederateHandle())
        continue;
      if (!eraseRegionRequest.valid()) {
        eraseRegionRequest = new EraseRegionMessage;
        eraseRegionRequest->setFederationHandle(getFederationHandle());
        eraseRegionRequest->getRegionHandleVector().reserve(_regionHandleRegionDataMap.size());
      }
      eraseRegionRequest->getRegionHandleVector().push_back(i->first);
    }
    if (eraseRegionRequest.valid())
      sendMessage(eraseRegionRequest);

    SharedPtr<ResignFederationExecutionRequestMessage> request = new ResignFederationExecutionRequestMessage;
    request->setFederationHandle(getFederationHandle());
    request->setFederateHandle(getFederateHandle());
    sendMessage(request);

    SharedPtr<ShutdownFederationExecutionMessage> request2 = new ShutdownFederationExecutionMessage;
    request2->setFederationHandle(getFederationHandle());
    sendMessage(request2);

    for (;;) {
      // Skip everything that is not the resign response - don't need that anymore
      SharedPtr<AbstractMessage> message = receiveMessage(clock);
      if (!message.valid())
        Traits::throwRTIinternalError("resignFederationExecution hit timeout!");
      if (dynamic_cast<EraseFederationExecutionMessage*>(message.get()))
        break;
      if (InsertObjectInstanceMessage* insertMessage = dynamic_cast<InsertObjectInstanceMessage*>(message.get())) {
        SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> message = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
        message->setFederationHandle(getFederationHandle());
        message->getObjectInstanceHandleVector().push_back(insertMessage->getObjectInstanceHandle());
        sendMessage(message);
      }
      if (ObjectInstanceHandlesResponseMessage* reserveMessage = dynamic_cast<ObjectInstanceHandlesResponseMessage*>(message.get())) {
        SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> message = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
        message->setFederationHandle(getFederationHandle());
        for (ObjectInstanceHandleNamePairVector::const_iterator k = reserveMessage->getObjectInstanceHandleNamePairVector().begin();
             k != reserveMessage->getObjectInstanceHandleNamePairVector().end(); ++k) {
          message->getObjectInstanceHandleVector().push_back(k->first);
        }
        sendMessage(message);
      }
      if (ReserveObjectInstanceNameResponseMessage* reserveMessage = dynamic_cast<ReserveObjectInstanceNameResponseMessage*>(message.get())) {
        if (reserveMessage->getSuccess()) {
          SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> message = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
          message->setFederationHandle(getFederationHandle());
          message->getObjectInstanceHandleVector().push_back(reserveMessage->getObjectInstanceHandleNamePair().first);
          sendMessage(message);
        }
      }
      if (ReserveMultipleObjectInstanceNameResponseMessage* reserveMessage = dynamic_cast<ReserveMultipleObjectInstanceNameResponseMessage*>(message.get())) {
        if (reserveMessage->getSuccess()) {
          SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> message = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
          message->setFederationHandle(getFederationHandle());
          for (ObjectInstanceHandleNamePairVector::const_iterator k = reserveMessage->getObjectInstanceHandleNamePairVector().begin();
               k != reserveMessage->getObjectInstanceHandleNamePairVector().end(); ++k) {
            message->getObjectInstanceHandleVector().push_back(k->first);
          }
          sendMessage(message);
        }
      }
    }

    SharedPtr<ReleaseFederationHandleMessage> request3 = new ReleaseFederationHandleMessage;
    request3->setFederationHandle(getFederationHandle());
    sendMessage(request3);
  }

  virtual void registerFederationSynchronizationPoint(const std::string& label, const VariableLengthData& tag, const FederateHandleSet& syncSet)
    throw (SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    // Tell all federates about that label
    SharedPtr<RegisterFederationSynchronizationPointMessage> message;
    message = new RegisterFederationSynchronizationPointMessage;
    message->setFederationHandle(getFederationHandle());
    message->setFederateHandle(getFederateHandle());
    message->setLabel(label);
    message->setTag(tag);
    message->setFederateHandleSet(syncSet);
    sendMessage(message);
  }

  virtual void synchronizationPointAchieved(const std::string& label)
    throw (SynchronizationPointLabelNotAnnounced,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (_announcedFederationSynchonizationLabels.find(label) == _announcedFederationSynchonizationLabels.end())
      Traits::throwSynchronizationPointLabelNotAnnounced();

    // tell all federates about that label
    SharedPtr<SynchronizationPointAchievedMessage> message;
    message = new SynchronizationPointAchievedMessage;
    message->setFederationHandle(getFederationHandle());
    message->getFederateHandleSet().insert(getFederateHandle());
    message->setLabel(label);
    sendMessage(message);
  }

  virtual void requestFederationSave(const std::string& label)
    throw (SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError("Save/Restore not implemented!");
  }

  virtual void requestFederationSave(const std::string& label, const NativeLogicalTime& logicalTime)
    throw (FederateUnableToUseTime,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError("Save/Restore not implemented!");
  }

  virtual void federateSaveBegun()
    throw (SaveNotInitiated,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError("Save/Restore not implemented!");
  }

  virtual void federateSaveComplete()
    throw (FederateHasNotBegunSave,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError("Save/Restore not implemented!");
  }

  virtual void federateSaveNotComplete()
    throw (FederateHasNotBegunSave,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError("Save/Restore not implemented!");
  }

  virtual void queryFederationSaveStatus()
    throw (RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError("Save/Restore not implemented!");
  }

  virtual void requestFederationRestore(const std::string& label)
    throw (SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError("Save/Restore not implemented!");
  }

  virtual void federateRestoreComplete()
    throw (RestoreNotRequested,
           SaveInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError("Save/Restore not implemented!");
  }

  virtual void federateRestoreNotComplete()
    throw (RestoreNotRequested,
           SaveInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError("Save/Restore not implemented!");
  }

  virtual void queryFederationRestoreStatus()
    throw (SaveInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError("Save/Restore not implemented!");
  }

  virtual void publishObjectClassAttributes(ObjectClassHandle objectClassHandle, const AttributeHandleSet& attributeList)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    // At first the complete error checks
    if (!isValidObjectClass(objectClassHandle))
      Traits::throwObjectClassNotDefined(objectClassHandle.toString());
    ObjectClass& objectClass = getObjectClass(objectClassHandle);
    for (AttributeHandleSet::const_iterator i = attributeList.begin(); i != attributeList.end(); ++i)
      if (!objectClass.isValidAttribute(*i))
        Traits::throwAttributeNotDefined(i->toString());

    // now that we know not to throw, handle the request
    SharedPtr<ChangeObjectClassPublicationMessage> request = new ChangeObjectClassPublicationMessage;
    request->setFederationHandle(getFederationHandle());
    request->setObjectClassHandle(objectClassHandle);

    // Alway one more because of the implicit privilege to delete
    request->getAttributeHandles().reserve(1 + attributeList.size());
    request->setPublicationType(Published);

    // Mark the objectclass itself as published.
    // Append this to the request if this publication has changed
    if (objectClass.setPublicationType(Published))
      request->getAttributeHandles().push_back(AttributeHandle(0));
    for (AttributeHandleSet::const_iterator i = attributeList.begin(); i != attributeList.end(); ++i) {
      // returns true if there is a change in the publication state
      if (!objectClass.setAttributePublicationType(*i, Published))
        continue;
      request->getAttributeHandles().push_back(*i);
    }
    // If there has nothing changed, don't send anything.
    if (request->getAttributeHandles().empty())
      return;

    sendMessage(request);
  }

  virtual void unpublishObjectClass(ObjectClassHandle objectClassHandle)
    throw (ObjectClassNotDefined,
           OwnershipAcquisitionPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    // At first the complete error checks
    if (!isValidObjectClass(objectClassHandle))
      Traits::throwObjectClassNotDefined(objectClassHandle.toString());
    ObjectClass& objectClass = getObjectClass(objectClassHandle);

    // now that we know not to throw, handle the request
    SharedPtr<ChangeObjectClassPublicationMessage> request = new ChangeObjectClassPublicationMessage;
    request->setFederationHandle(getFederationHandle());
    request->setObjectClassHandle(objectClassHandle);

    // Alway one more because of the implicit privilege to delete
    request->getAttributeHandles().reserve(objectClass._attributeVector.size());
    request->setPublicationType(Unpublished);

    // Mark the objectclass itself as unpublished.
    // Append this to the request if this publication has changed
    if (objectClass.setPublicationType(Unpublished))
      request->getAttributeHandles().push_back(AttributeHandle(0));
    for (size_t i = 0; i < objectClass._attributeVector.size(); ++i) {
      // returns true if there is a change in the publication state
      if (!objectClass.setAttributePublicationType(AttributeHandle(i), Unpublished))
        continue;
      request->getAttributeHandles().push_back(AttributeHandle(i));
    }
    // If there has nothing changed, don't send anything.
    if (request->getAttributeHandles().empty())
      return;

    sendMessage(request);
  }

  virtual void unpublishObjectClassAttributes(ObjectClassHandle objectClassHandle, const AttributeHandleSet& attributeList)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           OwnershipAcquisitionPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    // At first the complete error checks
    if (!isValidObjectClass(objectClassHandle))
      Traits::throwObjectClassNotDefined(objectClassHandle.toString());
    ObjectClass& objectClass = getObjectClass(objectClassHandle);
    for (AttributeHandleSet::const_iterator i = attributeList.begin(); i != attributeList.end(); ++i)
      if (!objectClass.isValidAttribute(*i))
        Traits::throwAttributeNotDefined(i->toString());

    // now that we know not to throw, handle the request
    SharedPtr<ChangeObjectClassPublicationMessage> request = new ChangeObjectClassPublicationMessage;
    request->setFederationHandle(getFederationHandle());
    request->setObjectClassHandle(objectClassHandle);

    // Alway one more because of the implicit privilege to delete
    request->getAttributeHandles().reserve(attributeList.size());
    request->setPublicationType(Unpublished);

    for (AttributeHandleSet::const_iterator i = attributeList.begin(); i != attributeList.end(); ++i) {
      // returns true if there is a change in the publication state
      if (!objectClass.setAttributePublicationType(*i, Unpublished))
        continue;
      request->getAttributeHandles().push_back(*i);
    }
    // If there has nothing changed, don't send anything.
    if (request->getAttributeHandles().empty())
      return;

    sendMessage(request);
  }

  virtual void publishInteractionClass(InteractionClassHandle interactionClassHandle)
    throw (InteractionClassNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (!isValidInteractionClass(interactionClassHandle))
      Traits::throwInteractionClassNotDefined(interactionClassHandle.toString());
    InteractionClass& interactionClass = getInteractionClass(interactionClassHandle);

    if (!interactionClass.setPublicationType(Published))
      return;

    SharedPtr<ChangeInteractionClassPublicationMessage> request = new ChangeInteractionClassPublicationMessage;
    request->setFederationHandle(getFederationHandle());
    request->setInteractionClassHandle(interactionClassHandle);
    request->setPublicationType(Published);

    sendMessage(request);
  }

  virtual void unpublishInteractionClass(InteractionClassHandle interactionClassHandle)
    throw (InteractionClassNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (!isValidInteractionClass(interactionClassHandle))
      Traits::throwInteractionClassNotDefined(interactionClassHandle.toString());
    InteractionClass& interactionClass = getInteractionClass(interactionClassHandle);

    if (!interactionClass.setPublicationType(Unpublished))
      return;

    SharedPtr<ChangeInteractionClassPublicationMessage> request = new ChangeInteractionClassPublicationMessage;
    request->setFederationHandle(getFederationHandle());
    request->setInteractionClassHandle(interactionClassHandle);
    request->setPublicationType(Unpublished);

    sendMessage(request);
  }

  virtual void subscribeObjectClassAttributes(ObjectClassHandle objectClassHandle, const AttributeHandleSet& attributeList, bool active)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    // At first the complete error checks
    if (!isValidObjectClass(objectClassHandle))
      Traits::throwObjectClassNotDefined(objectClassHandle.toString());
    ObjectClass& objectClass = getObjectClass(objectClassHandle);
    for (AttributeHandleSet::const_iterator i = attributeList.begin(); i != attributeList.end(); ++i)
      if (!objectClass.isValidAttribute(*i))
        Traits::throwAttributeNotDefined(i->toString());

    // now that we know not to throw, handle the request
    SharedPtr<ChangeObjectClassSubscriptionMessage> request = new ChangeObjectClassSubscriptionMessage;
    request->setFederationHandle(getFederationHandle());
    request->setObjectClassHandle(objectClassHandle);

    // Alway one more because of the implicit privilege to delete
    request->getAttributeHandles().reserve(1 + attributeList.size());

    SubscriptionType subscriptionType;
    if (active) {
      subscriptionType = SubscribedActive;
    } else {
      subscriptionType = SubscribedPassive;
    }
    request->setSubscriptionType(subscriptionType);

    // Mark the objectclass itself as subscribed.
    // Append this to the request if this subscription has changed
    if (objectClass.setSubscriptionType(subscriptionType))
      request->getAttributeHandles().push_back(AttributeHandle(0));
    for (AttributeHandleSet::const_iterator i = attributeList.begin(); i != attributeList.end(); ++i) {
      // returns true if there is a change in the subscription state
      if (!objectClass.setAttributeSubscriptionType(*i, subscriptionType))
        continue;
      request->getAttributeHandles().push_back(*i);
    }
    // If there has nothing changed, don't send anything.
    if (request->getAttributeHandles().empty())
      return;

    sendMessage(request);
  }

  virtual void unsubscribeObjectClass(ObjectClassHandle objectClassHandle)
    throw (ObjectClassNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    // At first the complete error checks
    if (!isValidObjectClass(objectClassHandle))
      Traits::throwObjectClassNotDefined(objectClassHandle.toString());
    ObjectClass& objectClass = getObjectClass(objectClassHandle);

    // now that we know not to throw, handle the request
    SharedPtr<ChangeObjectClassSubscriptionMessage> request = new ChangeObjectClassSubscriptionMessage;
    request->setFederationHandle(getFederationHandle());
    request->setObjectClassHandle(objectClassHandle);
    request->setSubscriptionType(Unsubscribed);
    request->getAttributeHandles().reserve(objectClass._attributeVector.size());
    if (objectClass.setSubscriptionType(Unsubscribed))
      request->getAttributeHandles().push_back(AttributeHandle(0));
    for (size_t i = 0; i < objectClass._attributeVector.size(); ++i) {
      // returns true if there is a change in the subscription state
      if (!objectClass.setAttributeSubscriptionType(AttributeHandle(i), Unsubscribed))
        continue;
      request->getAttributeHandles().push_back(AttributeHandle(i));
    }
    // If there has nothing changed, don't send anything.
    if (request->getAttributeHandles().empty())
      return;

    sendMessage(request);

    OpenRTIAssert(objectClass.getEffectiveSubscriptionType() == Unsubscribed);

    for (typename ObjectInstanceHandleMap::iterator i = _objectInstanceHandleMap.begin(); i != _objectInstanceHandleMap.end();) {
      if (i->second._objectClassHandle != objectClassHandle) {
        ++i;
      } else if (i->second._instanceAttributeVector[AttributeHandle(0)]->_isOwnedByFederate) {
        ++i;
      } else {
        eraseObjectInstance((i++)->first);
      }
    }
  }

  virtual void unsubscribeObjectClassAttributes(ObjectClassHandle objectClassHandle, const AttributeHandleSet& attributeList)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    // At first the complete error checks
    if (!isValidObjectClass(objectClassHandle))
      Traits::throwObjectClassNotDefined(objectClassHandle.toString());
    ObjectClass& objectClass = getObjectClass(objectClassHandle);
    for (AttributeHandleSet::const_iterator i = attributeList.begin(); i != attributeList.end(); ++i)
      if (!objectClass.isValidAttribute(*i))
        Traits::throwAttributeNotDefined(i->toString());

    // now that we know not to throw, handle the request
    SharedPtr<ChangeObjectClassSubscriptionMessage> request = new ChangeObjectClassSubscriptionMessage;
    request->setFederationHandle(getFederationHandle());
    request->setObjectClassHandle(objectClassHandle);
    request->setSubscriptionType(Unsubscribed);

    request->getAttributeHandles().reserve(attributeList.size());
    for (AttributeHandleSet::const_iterator i = attributeList.begin(); i != attributeList.end(); ++i) {
      // returns true if there is a change in the subscription state
      if (!objectClass.setAttributeSubscriptionType(*i, Unsubscribed))
        continue;
      request->getAttributeHandles().push_back(*i);
    }
    // If there has nothing changed, don't send anything.
    if (request->getAttributeHandles().empty())
      return;

    sendMessage(request);

    if (objectClass.getEffectiveSubscriptionType() == Unsubscribed) {
      for (typename ObjectInstanceHandleMap::iterator i = _objectInstanceHandleMap.begin(); i != _objectInstanceHandleMap.end();) {
        if (i->second._objectClassHandle != objectClassHandle) {
          ++i;
        } else if (i->second._instanceAttributeVector[AttributeHandle(0)]->_isOwnedByFederate) {
          ++i;
        } else {
          eraseObjectInstance((i++)->first);
        }
      }
    }
  }

  virtual void subscribeInteractionClass(InteractionClassHandle interactionClassHandle, bool active)
    throw (InteractionClassNotDefined,
           FederateServiceInvocationsAreBeingReportedViaMOM,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (!isValidInteractionClass(interactionClassHandle))
      Traits::throwInteractionClassNotDefined(interactionClassHandle.toString());
    InteractionClass& interactionClass = getInteractionClass(interactionClassHandle);

    SubscriptionType subscriptionType;
    if (active) {
      subscriptionType = SubscribedActive;
    } else {
      subscriptionType = SubscribedPassive;
    }
    if (!interactionClass.setSubscriptionType(subscriptionType))
      return;

    SharedPtr<ChangeInteractionClassSubscriptionMessage> request = new ChangeInteractionClassSubscriptionMessage;
    request->setFederationHandle(getFederationHandle());
    request->setInteractionClassHandle(interactionClassHandle);
    request->setSubscriptionType(subscriptionType);

    sendMessage(request);
  }

  virtual void unsubscribeInteractionClass(InteractionClassHandle interactionClassHandle)
    throw (InteractionClassNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (!isValidInteractionClass(interactionClassHandle))
      Traits::throwInteractionClassNotDefined(interactionClassHandle.toString());
    InteractionClass& interactionClass = getInteractionClass(interactionClassHandle);

    if (!interactionClass.setSubscriptionType(Unsubscribed))
      return;

    SharedPtr<ChangeInteractionClassSubscriptionMessage> request = new ChangeInteractionClassSubscriptionMessage;
    request->setFederationHandle(getFederationHandle());
    request->setInteractionClassHandle(interactionClassHandle);
    request->setSubscriptionType(Unsubscribed);

    sendMessage(request);
  }

  virtual void reserveObjectInstanceName(const std::string& objectInstanceName)
    throw (IllegalName,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (objectInstanceName.empty())
      Traits::throwIllegalName("Empty object hames are not allowed!");
    if (objectInstanceName.compare(0, 3, "HLA") == 0)
      Traits::throwIllegalName("Object instance names starting with \"HLA\" are reserved for the RTI.");

    SharedPtr<ReserveObjectInstanceNameRequestMessage> request;
    request = new ReserveObjectInstanceNameRequestMessage;
    request->setFederationHandle(getFederationHandle());
    request->setFederateHandle(getFederateHandle());
    request->setName(objectInstanceName);

    sendMessage(request);
  }

  virtual void releaseObjectInstanceName(const std::string& objectInstanceName)
    throw (ObjectInstanceNameNotReserved,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    NameObjectInstanceHandleMap::iterator i = _reservedNameObjectInstanceHandlePairs.find(objectInstanceName);
    if (i == _reservedNameObjectInstanceHandlePairs.end())
      Traits::throwObjectInstanceNameNotReserved(objectInstanceName);

    // Remove that locally ...
    ObjectInstanceHandle objectInstanceHandle = i->second;
    _reservedNameObjectInstanceHandlePairs.erase(i);

    // ... and send the release message to the rti
    SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> response = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
    response->setFederationHandle(getFederationHandle());
    response->getObjectInstanceHandleVector().push_back(objectInstanceHandle);
    sendMessage(response);
  }

  virtual void reserveMultipleObjectInstanceName(const std::set<std::string>& objectInstanceNameSet)
    throw (IllegalName,
           NameSetWasEmpty,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (objectInstanceNameSet.empty())
      Traits::throwNameSetWasEmpty("Empty object name set is not allowed!");
    SharedPtr<ReserveMultipleObjectInstanceNameRequestMessage> request;
    request = new ReserveMultipleObjectInstanceNameRequestMessage;
    request->getNameList().reserve(objectInstanceNameSet.size());
    for (std::set<std::string>::const_iterator i = objectInstanceNameSet.begin(); i != objectInstanceNameSet.end(); ++i) {
      if (i->empty())
        Traits::throwIllegalName("Empty object hames are not allowed!");
      if (i->compare(0, 3, "HLA") == 0)
        Traits::throwIllegalName("Object instance names starting with \"HLA\" are reserved for the RTI.");
      request->getNameList().push_back(*i);
    }
    request->setFederationHandle(getFederationHandle());
    request->setFederateHandle(getFederateHandle());

    sendMessage(request);
  }

  virtual void releaseMultipleObjectInstanceName(const std::set<std::string>& objectInstanceNameSet)
    throw (ObjectInstanceNameNotReserved,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    for (std::set<std::string>::const_iterator i = objectInstanceNameSet.begin(); i != objectInstanceNameSet.end(); ++i) {
      if (_reservedNameObjectInstanceHandlePairs.find(*i) == _reservedNameObjectInstanceHandlePairs.end())
        Traits::throwObjectInstanceNameNotReserved(*i);
    }

    SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> response = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
    response->setFederationHandle(getFederationHandle());
    response->getObjectInstanceHandleVector().reserve(objectInstanceNameSet.size());
    for (std::set<std::string>::const_iterator i = objectInstanceNameSet.begin(); i != objectInstanceNameSet.end(); ++i) {
      NameObjectInstanceHandleMap::iterator j = _reservedNameObjectInstanceHandlePairs.find(*i);
      response->getObjectInstanceHandleVector().push_back(j->second);
      _reservedNameObjectInstanceHandlePairs.erase(j);
    }
    sendMessage(response);
  }

  virtual ObjectInstanceHandle registerObjectInstance(ObjectClassHandle objectClassHandle)
    throw (ObjectClassNotDefined,
           ObjectClassNotPublished,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    // At first the complete error checks
    if (!isValidObjectClass(objectClassHandle))
      Traits::throwObjectClassNotDefined(objectClassHandle.toString());
    ObjectClass& objectClass = getObjectClass(objectClassHandle);
    if (Published != objectClass.getEffectivePublicationType())
      Traits::throwObjectClassNotPublished(objectClass._name);

    ObjectInstanceHandleNamePair handleNamePair = getFreeObjectInstanceHandleNamePair();

    insertObjectInstance(handleNamePair.first, handleNamePair.second, objectClassHandle, true);

    SharedPtr<InsertObjectInstanceMessage> request;
    request = new InsertObjectInstanceMessage;
    request->setFederationHandle(getFederationHandle());
    request->setObjectClassHandle(objectClassHandle);
    request->setObjectInstanceHandle(handleNamePair.first);
    request->setName(handleNamePair.second);
    request->getAttributeStateVector().reserve(objectClass._attributeVector.size());
    for (size_t i = 0; i < objectClass._attributeVector.size(); ++i) {
      if (!objectClass.isAttributePublished(AttributeHandle(i)))
        continue;
      AttributeState attributeState;
      attributeState.setAttributeHandle(AttributeHandle(i));
      request->getAttributeStateVector().push_back(attributeState);
    }
    sendMessage(request);

    return handleNamePair.first;
  }

  virtual ObjectInstanceHandle registerObjectInstance(ObjectClassHandle objectClassHandle, const std::string& objectInstanceName,
                                                      bool allowUnreservedObjectNames)
    throw (ObjectClassNotDefined,
           ObjectClassNotPublished,
           ObjectInstanceNameNotReserved,
           ObjectInstanceNameInUse,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    // Short circuit to invent an own name
    if (objectInstanceName.empty())
      return registerObjectInstance(objectClassHandle);

    // At first the complete error checks
    if (!isValidObjectClass(objectClassHandle))
      Traits::throwObjectClassNotDefined(objectClassHandle.toString());
    ObjectClass& objectClass = getObjectClass(objectClassHandle);
    if (Published != objectClass.getEffectivePublicationType())
      Traits::throwObjectClassNotPublished(objectClass._name);

    // The already available objectInstanceHandle should be stored here.
    ObjectInstanceHandle objectInstanceHandle;

    // Check if we already have the object instance name reserved
    NameObjectInstanceHandleMap::iterator i = _reservedNameObjectInstanceHandlePairs.find(objectInstanceName);
    if (i != _reservedNameObjectInstanceHandlePairs.end()) {
      // if it is reserved, all is fine, get the object instance handle.
      objectInstanceHandle = i->second;
      _reservedNameObjectInstanceHandlePairs.erase(i);
    } else {
      // If not, either policy tells us to just bail out - the standard rti1516 behaviour ...
      if (!allowUnreservedObjectNames) {
        Traits::throwObjectInstanceNameNotReserved(objectInstanceName);
      } else {
        // Or, we try to reserve this object name behind the scenes.
        // Ok, if this is allowed, like for a rti13 federate or for the option
        // of allowing that to emulate certi behavior, we need to do the reservation
        // of the name now. This is the only syncronous operation in the rti.
        SharedPtr<ReserveObjectInstanceNameRequestMessage> nameRequest;
        nameRequest = new ReserveObjectInstanceNameRequestMessage;
        nameRequest->setFederationHandle(getFederationHandle());
        nameRequest->setFederateHandle(getFederateHandle());
        nameRequest->setName(objectInstanceName);
        sendMessage(nameRequest);

        Clock timeout = Clock::now() + Clock::fromSeconds(60); // FIXME???
        _syncronousReservationHandleNamePair.first = ObjectInstanceHandle();
        _syncronousReservationHandleNamePair.second = objectInstanceName;
        do {
          if (dispatchInternal(timeout))
            continue;
          if (timeout < Clock::now())
            Traits::throwRTIinternalError("Timeout while waiting for object name response handles.");
        } while (!_syncronousReservationHandleNamePair.second.empty());
        // Get the object instance handle that is delivered with the name reservation.
        objectInstanceHandle = _syncronousReservationHandleNamePair.first;
        if (!_syncronousReservationHandleNamePair.first.valid())
          Traits::throwObjectInstanceNameInUse(objectInstanceName);
      }
    }

    // Once we have survived, we know that the objectInstanceName given in the argument is unique and ours.
    // Also the object instance handle in this local scope must be valid and ours.

    insertObjectInstance(objectInstanceHandle, objectInstanceName, objectClassHandle, true);
    SharedPtr<InsertObjectInstanceMessage> request;
    request = new InsertObjectInstanceMessage;
    request->setFederationHandle(getFederationHandle());
    request->setObjectClassHandle(objectClassHandle);
    request->setObjectInstanceHandle(objectInstanceHandle);
    request->setName(objectInstanceName);
    request->getAttributeStateVector().reserve(objectClass._attributeVector.size());
    for (size_t i = 0; i < objectClass._attributeVector.size(); ++i) {
      if (!objectClass.isAttributePublished(AttributeHandle(i)))
        continue;
      AttributeState attributeState;
      attributeState.setAttributeHandle(AttributeHandle(i));
      request->getAttributeStateVector().push_back(attributeState);
    }
    sendMessage(request);

    return objectInstanceHandle;
  }

  virtual void
  updateAttributeValues(ObjectInstanceHandle objectInstanceHandle, std::vector<OpenRTI::AttributeValue>& attributeValues,
                        const VariableLengthData& tag)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    typename ObjectInstanceHandleMap::const_iterator i = _objectInstanceHandleMap.find(objectInstanceHandle);
    if (i == _objectInstanceHandleMap.end())
      Traits::throwObjectInstanceNotKnown(objectInstanceHandle.toString());
    TransportationType transportationType = BEST_EFFORT;
    for (std::vector<OpenRTI::AttributeValue>::const_iterator j = attributeValues.begin(); j != attributeValues.end(); ++j) {
      if (i->second._instanceAttributeVector.size() <= j->getAttributeHandle().getHandle())
        Traits::throwAttributeNotDefined(j->getAttributeHandle().toString());
      if (!i->second._instanceAttributeVector[j->getAttributeHandle().getHandle()].valid())
        Traits::throwAttributeNotDefined(j->getAttributeHandle().toString());
      if (!i->second._instanceAttributeVector[j->getAttributeHandle().getHandle()]->_isOwnedByFederate)
        Traits::throwAttributeNotOwned(j->getAttributeHandle().toString());
      if (i->second._instanceAttributeVector[j->getAttributeHandle().getHandle()]->_transportationType == RELIABLE)
        transportationType = RELIABLE;
    }

    SharedPtr<AttributeUpdateMessage> request;
    request = new AttributeUpdateMessage;
    request->setFederationHandle(getFederationHandle());
    request->setObjectInstanceHandle(objectInstanceHandle);
    request->getAttributeValues().swap(attributeValues);
    request->setTransportationType(transportationType);
    request->setTag(tag);
    sendMessage(request);
  }

  virtual MessageRetractionHandle
  updateAttributeValues(ObjectInstanceHandle objectInstanceHandle, std::vector<OpenRTI::AttributeValue>& attributeValues,
                        const VariableLengthData& tag, const NativeLogicalTime& nativeLogicalTime)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           InvalidLogicalTime,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (!_timeRegulationEnabled) {
      updateAttributeValues(objectInstanceHandle, attributeValues, tag);
      return MessageRetractionHandle();
    } else {
      typename ObjectInstanceHandleMap::const_iterator i = _objectInstanceHandleMap.find(objectInstanceHandle);
      if (i == _objectInstanceHandleMap.end())
        Traits::throwObjectInstanceNotKnown(objectInstanceHandle.toString());
      TransportationType transportationType = BEST_EFFORT;
      for (std::vector<OpenRTI::AttributeValue>::const_iterator j = attributeValues.begin(); j != attributeValues.end(); ++j) {
        if (i->second._instanceAttributeVector.size() <= j->getAttributeHandle().getHandle())
          Traits::throwAttributeNotDefined(j->getAttributeHandle().toString());
        if (!i->second._instanceAttributeVector[j->getAttributeHandle().getHandle()].valid())
          Traits::throwAttributeNotDefined(j->getAttributeHandle().toString());
        if (!i->second._instanceAttributeVector[j->getAttributeHandle().getHandle()]->_isOwnedByFederate)
          Traits::throwAttributeNotOwned(j->getAttributeHandle().toString());
        if (i->second._instanceAttributeVector[j->getAttributeHandle().getHandle()]->_transportationType == RELIABLE)
          transportationType = RELIABLE;
      }
      if (logicalTimeAlreadyPassed(nativeLogicalTime))
        Traits::throwInvalidLogicalTime(_logicalTimeFactory.toString(nativeLogicalTime));

      MessageRetractionHandle messageRetractionHandle = getNextMessageRetractionHandle();

      SharedPtr<TimeStampedAttributeUpdateMessage> request;
      request = new TimeStampedAttributeUpdateMessage;
      request->setFederationHandle(getFederationHandle());
      request->setObjectInstanceHandle(objectInstanceHandle);
      request->getAttributeValues().swap(attributeValues);
      request->setTimeStamp(_logicalTimeFactory.encodeLogicalTime(nativeLogicalTime));
      request->setTag(tag);
      request->setTransportationType(transportationType);
      request->setMessageRetractionHandle(messageRetractionHandle);
      sendMessage(request);

      return messageRetractionHandle;
    }
  }

  virtual void sendInteraction(InteractionClassHandle interactionClassHandle, std::vector<ParameterValue>& parameterValues,
                       const VariableLengthData& tag)
    throw (InteractionClassNotPublished,
           InteractionClassNotDefined,
           InteractionParameterNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (!isValidInteractionClass(interactionClassHandle))
      Traits::throwInteractionClassNotDefined(interactionClassHandle.toString());
    const InteractionClass& interactionClass = getInteractionClass(interactionClassHandle);
    if (!interactionClass.isPublished())
      Traits::throwInteractionClassNotPublished(interactionClassHandle.toString());
    for (std::vector<ParameterValue>::const_iterator i = parameterValues.begin(); i != parameterValues.end(); ++i)
      if (!interactionClass.isValidParameter(i->getParameterHandle()))
        Traits::throwInteractionParameterNotDefined(i->getParameterHandle().toString());

    SharedPtr<InteractionMessage> request;
    request = new InteractionMessage;
    request->setFederationHandle(getFederationHandle());
    request->setInteractionClassHandle(interactionClassHandle);
    request->setTransportationType(interactionClass._transportationType);
    request->setTag(tag);
    request->getParameterValues().swap(parameterValues);
    sendMessage(request);
  }

  virtual MessageRetractionHandle sendInteraction(InteractionClassHandle interactionClassHandle,
                                          std::vector<ParameterValue>& parameterValues,
                                          const VariableLengthData& tag,
                                          const NativeLogicalTime& nativeLogicalTime)
    throw (InteractionClassNotPublished,
           InteractionClassNotDefined,
           InteractionParameterNotDefined,
           InvalidLogicalTime,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (!_timeRegulationEnabled) {
      sendInteraction(interactionClassHandle, parameterValues, tag);
      return MessageRetractionHandle();
    } else {
      if (!isValidInteractionClass(interactionClassHandle))
        Traits::throwInteractionClassNotDefined(interactionClassHandle.toString());
      const InteractionClass& interactionClass = getInteractionClass(interactionClassHandle);
      if (!interactionClass.isPublished())
        Traits::throwInteractionClassNotPublished(interactionClassHandle.toString());
      for (std::vector<ParameterValue>::const_iterator i = parameterValues.begin(); i != parameterValues.end(); ++i)
        if (!interactionClass.isValidParameter(i->getParameterHandle()))
          Traits::throwInteractionParameterNotDefined(i->getParameterHandle().toString());
      if (logicalTimeAlreadyPassed(nativeLogicalTime))
        Traits::throwInvalidLogicalTime(_logicalTimeFactory.toString(nativeLogicalTime));

      MessageRetractionHandle messageRetractionHandle = getNextMessageRetractionHandle();

      SharedPtr<TimeStampedInteractionMessage> request;
      request = new TimeStampedInteractionMessage;
      request->setFederationHandle(getFederationHandle());
      request->setInteractionClassHandle(interactionClassHandle);
      request->setTransportationType(interactionClass._transportationType);
      request->setTag(tag);
      request->setTimeStamp(_logicalTimeFactory.encodeLogicalTime(nativeLogicalTime));
      request->setMessageRetractionHandle(messageRetractionHandle);
      request->getParameterValues().swap(parameterValues);
      sendMessage(request);

      return messageRetractionHandle;
    }
  }

  virtual void deleteObjectInstance(ObjectInstanceHandle objectInstanceHandle, const VariableLengthData& tag)
    throw (DeletePrivilegeNotHeld,
           ObjectInstanceNotKnown,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    // Ok, this is in the end three times O(log(N)), FIXME: get the objectinstance object
    if (!isValidObjectInstance(objectInstanceHandle))
      Traits::throwObjectInstanceNotKnown(objectInstanceHandle.toString());
    if (!isAttributeOwnedByFederate(objectInstanceHandle, AttributeHandle(0)))
      Traits::throwDeletePrivilegeNotHeld(objectInstanceHandle.toString());

    SharedPtr<DeleteObjectInstanceMessage> request;
    request = new DeleteObjectInstanceMessage;
    request->setFederationHandle(getFederationHandle());
    request->setObjectInstanceHandle(objectInstanceHandle);
    request->setTag(tag);

    sendMessage(request);

    // Note that this also sends the unreference message just past the delete
    eraseObjectInstance(objectInstanceHandle);
  }

  virtual MessageRetractionHandle deleteObjectInstance(ObjectInstanceHandle objectInstanceHandle, const VariableLengthData& tag,
                                                       const NativeLogicalTime& nativeLogicalTime)
    throw (DeletePrivilegeNotHeld,
           ObjectInstanceNotKnown,
           InvalidLogicalTime,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (!_timeRegulationEnabled) {
      deleteObjectInstance(objectInstanceHandle, tag);
      return MessageRetractionHandle();
    } else {
      // Ok, this is in the end three times O(log(N)), FIXME: get the objectinstance object
      if (!isValidObjectInstance(objectInstanceHandle))
        Traits::throwObjectInstanceNotKnown(objectInstanceHandle.toString());
      if (!isAttributeOwnedByFederate(objectInstanceHandle, AttributeHandle(0)))
        Traits::throwDeletePrivilegeNotHeld(objectInstanceHandle.toString());
      if (logicalTimeAlreadyPassed(nativeLogicalTime))
        Traits::throwInvalidLogicalTime(_logicalTimeFactory.toString(nativeLogicalTime));

      MessageRetractionHandle messageRetractionHandle = getNextMessageRetractionHandle();

      SharedPtr<TimeStampedDeleteObjectInstanceMessage> request;
      request = new TimeStampedDeleteObjectInstanceMessage;
      request->setFederationHandle(getFederationHandle());
      request->setObjectInstanceHandle(objectInstanceHandle);
      request->setTag(tag);
      request->setTimeStamp(_logicalTimeFactory.encodeLogicalTime(nativeLogicalTime));
      request->setMessageRetractionHandle(messageRetractionHandle);

      sendMessage(request);

      // FIXME do this once the logical time has passed
      // When implementing message retraction this needs to be delayed probably ...
      // Note that this also sends the unreference message just past the delete
      // eraseObjectInstance(objectInstanceHandle);

      return messageRetractionHandle;
    }
  }

  virtual void localDeleteObjectInstance(ObjectInstanceHandle objectInstanceHandle)
    throw (ObjectInstanceNotKnown,
           FederateOwnsAttributes,
           OwnershipAcquisitionPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    typename ObjectInstanceHandleMap::iterator i = _objectInstanceHandleMap.find(objectInstanceHandle);
    if (i == _objectInstanceHandleMap.end())
      Traits::throwObjectInstanceNotKnown(objectInstanceHandle.toString());
    for (size_t j = 0; j < i->second._instanceAttributeVector.size(); ++j) {
      if (i->second._instanceAttributeVector[j]->_isOwnedByFederate)
        Traits::throwFederateOwnsAttributes(objectInstanceHandle.toString());
    }

    // local delete is just like unreferencing the object instance handle
    eraseObjectInstance(objectInstanceHandle);
  }

  virtual void changeAttributeTransportationType(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet,
                                                 TransportationType transportationType)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    typename ObjectInstanceHandleMap::iterator i = _objectInstanceHandleMap.find(objectInstanceHandle);
    if (i == _objectInstanceHandleMap.end())
      Traits::throwObjectInstanceNotKnown(objectInstanceHandle.toString());
    for (typename AttributeHandleSet::const_iterator j = attributeHandleSet.begin(); j != attributeHandleSet.end(); ++j) {
      if (i->second._instanceAttributeVector.size() <= j->getHandle())
        Traits::throwAttributeNotDefined(j->toString());
      if (!i->second._instanceAttributeVector[j->getHandle()].valid())
        Traits::throwAttributeNotDefined(j->toString());
      if (!i->second._instanceAttributeVector[j->getHandle()]->_isOwnedByFederate)
        Traits::throwAttributeNotOwned(j->toString());
    }
    for (typename AttributeHandleSet::const_iterator j = attributeHandleSet.begin(); j != attributeHandleSet.end(); ++j) {
      i->second._instanceAttributeVector[j->getHandle()]->_transportationType = transportationType;
    }
  }

  virtual void changeInteractionTransportationType(InteractionClassHandle interactionClassHandle, TransportationType transportationType)
    throw (InteractionClassNotDefined, InteractionClassNotPublished, SaveInProgress, RestoreInProgress, RTIinternalError)
  {
    if (!isValidInteractionClass(interactionClassHandle))
      Traits::throwInteractionClassNotDefined(interactionClassHandle.toString());
    InteractionClass& interactionClass = getInteractionClass(interactionClassHandle);
    if (!interactionClass.isPublished())
      Traits::throwInteractionClassNotPublished(interactionClass._name);
    interactionClass._transportationType = transportationType;
  }

  virtual void requestAttributeValueUpdate(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet,
                                           const VariableLengthData& tag)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    typename ObjectInstanceHandleMap::iterator i = _objectInstanceHandleMap.find(objectInstanceHandle);
    if (i == _objectInstanceHandleMap.end())
      Traits::throwObjectInstanceNotKnown(objectInstanceHandle.toString());
    for (typename AttributeHandleSet::const_iterator j = attributeHandleSet.begin(); j != attributeHandleSet.end(); ++j) {
      if (i->second._instanceAttributeVector.size() <= j->getHandle())
        Traits::throwAttributeNotDefined(j->toString());
      if (!i->second._instanceAttributeVector[j->getHandle()].valid())
        Traits::throwAttributeNotDefined(j->toString());
    }

    AttributeHandleVector attributeHandleVector;
    attributeHandleVector.reserve(attributeHandleSet.size());
    for (typename AttributeHandleSet::const_iterator j = attributeHandleSet.begin(); j != attributeHandleSet.end(); ++j) {
      attributeHandleVector.push_back(*j);
    }

    SharedPtr<RequestAttributeUpdateMessage> request;
    request = new RequestAttributeUpdateMessage;
    request->setFederationHandle(getFederationHandle());
    request->setObjectInstanceHandle(objectInstanceHandle);
    request->getAttributeHandles().swap(attributeHandleVector);
    request->setTag(tag);

    sendMessage(request);
  }

  virtual void requestAttributeValueUpdate(ObjectClassHandle objectClassHandle, const AttributeHandleSet& attributeHandleSet,
                                           const VariableLengthData& tag)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (!isValidObjectClass(objectClassHandle))
      Traits::throwObjectClassNotDefined(objectClassHandle.toString());
    ObjectClass& objectClass = getObjectClass(objectClassHandle);
    for (typename AttributeHandleSet::const_iterator j = attributeHandleSet.begin(); j != attributeHandleSet.end(); ++j) {
      if (!objectClass.isValidAttribute(*j))
        Traits::throwAttributeNotDefined(j->toString());
    }

    AttributeHandleVector attributeHandleVector;
    attributeHandleVector.reserve(attributeHandleSet.size());
    for (typename AttributeHandleSet::const_iterator j = attributeHandleSet.begin(); j != attributeHandleSet.end(); ++j) {
      attributeHandleVector.push_back(*j);
    }

    SharedPtr<RequestClassAttributeUpdateMessage> request;
    request = new RequestClassAttributeUpdateMessage;
    request->setFederationHandle(getFederationHandle());
    request->setObjectClassHandle(objectClassHandle);
    request->getAttributeHandles().swap(attributeHandleVector);
    request->setTag(tag);

    sendMessage(request);
  }

  virtual void unconditionalAttributeOwnershipDivestiture(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError();
  }

  virtual void negotiatedAttributeOwnershipDivestiture(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet, const VariableLengthData& tag)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           AttributeAlreadyBeingDivested,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError();
  }

  virtual void confirmDivestiture(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet, const VariableLengthData& tag)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           AttributeDivestitureWasNotRequested,
           NoAcquisitionPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError();
  }

  virtual void attributeOwnershipAcquisition(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet, const VariableLengthData& tag)
    throw (ObjectInstanceNotKnown,
           ObjectClassNotPublished,
           AttributeNotDefined,
           AttributeNotPublished,
           FederateOwnsAttributes,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError();
  }

  virtual void attributeOwnershipAcquisitionIfAvailable(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet)
    throw (ObjectInstanceNotKnown,
           ObjectClassNotPublished,
           AttributeNotDefined,
           AttributeNotPublished,
           FederateOwnsAttributes,
           AttributeAlreadyBeingAcquired,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError();
  }

  virtual void attributeOwnershipDivestitureIfWanted(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet,
                                             AttributeHandleSet& divestedAttributes)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError();
  }

  virtual void cancelNegotiatedAttributeOwnershipDivestiture(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeNotOwned,
           AttributeDivestitureWasNotRequested,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError();
  }

  virtual void cancelAttributeOwnershipAcquisition(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           AttributeAlreadyOwned,
           AttributeAcquisitionWasNotRequested,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError();
  }

  virtual void queryAttributeOwnership(ObjectInstanceHandle objectInstanceHandle, AttributeHandle attributeHandle)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError();
  }

  virtual bool isAttributeOwnedByFederate(ObjectInstanceHandle objectInstanceHandle, AttributeHandle attributeHandle) const
    throw (ObjectInstanceNotKnown, AttributeNotDefined, SaveInProgress, RestoreInProgress, RTIinternalError)
  {
    typename ObjectInstanceHandleMap::const_iterator i = _objectInstanceHandleMap.find(objectInstanceHandle);
    if (i == _objectInstanceHandleMap.end())
      Traits::throwObjectInstanceNotKnown(objectInstanceHandle.toString());
    if (i->second._instanceAttributeVector.size() <= attributeHandle.getHandle())
      Traits::throwAttributeNotDefined(attributeHandle.toString());
    if (!i->second._instanceAttributeVector[attributeHandle.getHandle()].valid())
      Traits::throwAttributeNotDefined(attributeHandle.toString());
    return i->second._instanceAttributeVector[attributeHandle.getHandle()]->_isOwnedByFederate;
  }

  virtual void enableTimeRegulation(const NativeLogicalTimeInterval& nativeLookahead)
    throw (TimeRegulationAlreadyEnabled,
           InvalidLookahead,
           InTimeAdvancingState,
           RequestForTimeRegulationPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (_timeRegulationEnabled)
      Traits::throwTimeRegulationAlreadyEnabled();
    if (_timeRegulationEnablePending)
      Traits::throwRequestForTimeRegulationPending();
    if (_timeAdvancePending)
      Traits::throwInTimeAdvancingState();
    if (!_logicalTimeFactory.isPositiveTimeInterval(nativeLookahead))
      Traits::throwInvalidLookahead(_logicalTimeFactory.toString(nativeLookahead));
    if (!_permitTimeRegulation)
      Traits::throwRTIinternalError("Enable time regulation not permitted due to server policy!");

    _timeRegulationEnablePending = true;

    _currentLookahead = _logicalTimeFactory.getLogicalTimeInterval(nativeLookahead);
    _targetLookahead = _currentLookahead;

    _pendingLogicalTime.first = _logicalTime;
    _pendingLogicalTime.second = _logicalTimeFactory.isZeroTimeInterval(_currentLookahead);

    _localLowerBoundTimeStamp = _pendingLogicalTime;
    _localLowerBoundTimeStamp.first += _currentLookahead;

    _timeRegulationEnableFederateHandleSet = _federateHandleSet;
    // Make sure we wait for the request looping back through the root server.
    // We need to do that round trip to the root server to stay in order with newly
    // joined federates that are serialized by the root server.
    _timeRegulationEnableFederateHandleSet.insert(getFederateHandle());

    SharedPtr<EnableTimeRegulationRequestMessage> request;
    request = new EnableTimeRegulationRequestMessage;
    request->setFederationHandle(getFederationHandle());
    request->setFederateHandle(getFederateHandle());
    request->getTimeStamp().setLogicalTime(_logicalTimeFactory.encodeLogicalTime(_localLowerBoundTimeStamp.first));
    request->getTimeStamp().setZeroLookahead(_localLowerBoundTimeStamp.second);
    sendMessage(request);
  }

  // the RTI13 variant
  virtual void enableTimeRegulation(const NativeLogicalTime& nativeLogicalTime, const NativeLogicalTimeInterval& nativeLookahead)
    throw (TimeRegulationAlreadyEnabled,
           InvalidLogicalTime,
           InvalidLookahead,
           InTimeAdvancingState,
           RequestForTimeRegulationPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (_timeRegulationEnabled)
      Traits::throwTimeRegulationAlreadyEnabled();
    if (_timeRegulationEnablePending)
      Traits::throwRequestForTimeRegulationPending();
    if (_timeAdvancePending)
      Traits::throwInTimeAdvancingState();
    if (nativeLogicalTime < _logicalTimeFactory.getLogicalTime(_logicalTime))
      Traits::throwInvalidLogicalTime(_logicalTimeFactory.toString(nativeLogicalTime));
    if (!_logicalTimeFactory.isPositiveTimeInterval(nativeLookahead))
      Traits::throwInvalidLookahead(_logicalTimeFactory.toString(nativeLookahead));

    _timeRegulationEnablePending = true;

    _currentLookahead = _logicalTimeFactory.getLogicalTimeInterval(nativeLookahead);
    _targetLookahead = _currentLookahead;

    _pendingLogicalTime.first = _logicalTimeFactory.getLogicalTime(nativeLogicalTime);
    _pendingLogicalTime.second = _logicalTimeFactory.isZeroTimeInterval(_currentLookahead);

    _localLowerBoundTimeStamp = _pendingLogicalTime;
    _localLowerBoundTimeStamp.first += _currentLookahead;

    _timeRegulationEnableFederateHandleSet = _federateHandleSet;
    // Make sure we wait for the request looping back through the root server.
    // We need to do that round trip to the root server to stay in order with newly
    // joined federates that are serialized by the root server.
    _timeRegulationEnableFederateHandleSet.insert(getFederateHandle());

    SharedPtr<EnableTimeRegulationRequestMessage> request;
    request = new EnableTimeRegulationRequestMessage;
    request->setFederationHandle(getFederationHandle());
    request->setFederateHandle(getFederateHandle());
    request->getTimeStamp().setLogicalTime(_logicalTimeFactory.encodeLogicalTime(_localLowerBoundTimeStamp.first));
    request->getTimeStamp().setZeroLookahead(_localLowerBoundTimeStamp.second);
    sendMessage(request);
  }

  virtual void disableTimeRegulation()
    throw (TimeRegulationIsNotEnabled,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (!_timeRegulationEnabled)
      Traits::throwTimeRegulationIsNotEnabled();

    _timeRegulationEnabled = false;

    SharedPtr<DisableTimeRegulationRequestMessage> request;
    request = new DisableTimeRegulationRequestMessage;
    request->setFederationHandle(getFederationHandle());
    request->setFederateHandle(getFederateHandle());
    sendMessage(request);
  }

  virtual void enableTimeConstrained()
    throw (TimeConstrainedAlreadyEnabled,
           InTimeAdvancingState,
           RequestForTimeConstrainedPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (_timeConstrainedEnabled)
      Traits::throwTimeConstrainedAlreadyEnabled();
    if (_timeConstrainedEnablePending)
      Traits::throwRequestForTimeConstrainedPending();
    if (_timeAdvancePending)
      Traits::throwInTimeAdvancingState();

    _timeConstrainedEnablePending = true;
    if (_logicalTimeFederateHandleSetMap.empty()) {
      // Ok, we are getting time constrained, but nobody is holding us back
      queueTimeConstrainedEnabled(_logicalTime);
    } else if (_logicalTime <= _logicalTimeFederateHandleSetMap.begin()->first.first) {
      // Ok, we are getting time constrained, but everybody potentially holding us back is ahead of us
      queueTimeConstrainedEnabled(_logicalTime);
    } else {
      // Ok, we are getting time constrained, but we need to wait until the one behind us unleaches us
    }
  }

  virtual void disableTimeConstrained()
    throw (TimeConstrainedIsNotEnabled,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (!_timeConstrainedEnabled)
      Traits::throwTimeConstrainedIsNotEnabled();

    _timeConstrainedEnabled = false;
    // If we are in time advance ponding, we are now able to advance immediately
    if (_timeAdvancePending)
      queueTimeAdvanceGranted(_pendingLogicalTime.first);
    for (typename LogicalTimeMessageListMap::iterator i = _logicalTimeMessageListMap.begin(); i != _logicalTimeMessageListMap.end();) {
      queueCallbacks(i->second);
      _logicalTimeMessageListMap.erase(i++);
    }
  }

  virtual void timeAdvanceRequest(const NativeLogicalTime& logicalTime)
    throw (InvalidLogicalTime,
           LogicalTimeAlreadyPassed,
           InTimeAdvancingState,
           RequestForTimeRegulationPending,
           RequestForTimeConstrainedPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    _timeAdvanceRequest(logicalTime, false, false);
  }

  virtual void timeAdvanceRequestAvailable(const NativeLogicalTime& logicalTime)
    throw (InvalidLogicalTime,
           LogicalTimeAlreadyPassed,
           InTimeAdvancingState,
           RequestForTimeRegulationPending,
           RequestForTimeConstrainedPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    _timeAdvanceRequest(logicalTime, true, false);
  }

  virtual void nextMessageRequest(const NativeLogicalTime& logicalTime)
    throw (InvalidLogicalTime,
           LogicalTimeAlreadyPassed,
           InTimeAdvancingState,
           RequestForTimeRegulationPending,
           RequestForTimeConstrainedPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    _timeAdvanceRequest(logicalTime, false, true);
  }

  virtual void nextMessageRequestAvailable(const NativeLogicalTime& logicalTime)
    throw (InvalidLogicalTime,
           LogicalTimeAlreadyPassed,
           InTimeAdvancingState,
           RequestForTimeRegulationPending,
           RequestForTimeConstrainedPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    _timeAdvanceRequest(logicalTime, true, true);
  }

  // Covers all four implementations above
  void _timeAdvanceRequest(const NativeLogicalTime& nativeLogicalTime, bool availableMode, bool nextMessageMode)
    throw (InvalidLogicalTime,
           LogicalTimeAlreadyPassed,
           InTimeAdvancingState,
           RequestForTimeRegulationPending,
           RequestForTimeConstrainedPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (nativeLogicalTime <= _logicalTimeFactory.getLogicalTime(_logicalTime))
      Traits::throwLogicalTimeAlreadyPassed(_logicalTimeFactory.toString(nativeLogicalTime));
    if (_timeAdvancePending)
      Traits::throwInTimeAdvancingState();
    if (_timeRegulationEnablePending)
      Traits::throwRequestForTimeRegulationPending();
    if (_timeConstrainedEnablePending)
      Traits::throwRequestForTimeConstrainedPending();

    LogicalTime logicalTime = _logicalTimeFactory.getLogicalTime(nativeLogicalTime);
    _nextMessageMode = nextMessageMode;
    _timeAdvancePending = true;
    _pendingLogicalTime.first = logicalTime;
    _pendingLogicalTime.second = !availableMode;

    // If we need to advance to match the new requested lookahead, try to increase that one
    if (_targetLookahead < _currentLookahead) {
      // Check if we would violate a previously given promise about our lower bound time stamp
      LogicalTimePair logicalTimePair(_logicalTime, _logicalTimeFactory.isZeroTimeInterval(_targetLookahead));
      logicalTimePair.first += _targetLookahead;
      if (logicalTimePair < _localLowerBoundTimeStamp) {
        _currentLookahead = _localLowerBoundTimeStamp.first - logicalTime;
      } else {
        _currentLookahead = _targetLookahead;
      }
    }

    _localLowerBoundTimeStamp.first = logicalTime;
    if (_logicalTimeFactory.isZeroTimeInterval(_currentLookahead)) {
      _localLowerBoundTimeStamp.second = true;
    } else {
      _localLowerBoundTimeStamp.first += _currentLookahead;
      _localLowerBoundTimeStamp.second = false;
    }

    if (_timeRegulationEnabled)
      sendCommitLowerBoundTimeStamp(_localLowerBoundTimeStamp);

    if (_timeConstrainedEnabled) {
      if (_logicalTimeFederateHandleSetMap.empty()) {
        // Ok, nobody holding us back
        queueTimeAdvanceGranted(_pendingLogicalTime.first);
      } else if (_pendingLogicalTime <= _logicalTimeFederateHandleSetMap.begin()->first) {
        // Ok, nobody holding our pending time back
        queueTimeAdvanceGranted(_pendingLogicalTime.first);
      } else if (_nextMessageMode && _logicalTime < _logicalTimeFederateHandleSetMap.begin()->first.first) {
        // Ok, room for advancement
        // FIXME, what about the guarantees???
        _pendingLogicalTime = _logicalTimeFederateHandleSetMap.begin()->first;
        queueTimeAdvanceGranted(_logicalTimeFederateHandleSetMap.begin()->first.first);
      }
    } else {
      // If we are not time constrained, just schedule the time advance
      queueTimeAdvanceGranted(_pendingLogicalTime.first);
    }
  }

  virtual void flushQueueRequest(const NativeLogicalTime& nativeLogicalTime)
    throw (InvalidLogicalTime,
           LogicalTimeAlreadyPassed,
           InTimeAdvancingState,
           RequestForTimeRegulationPending,
           RequestForTimeConstrainedPending,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (nativeLogicalTime <= _logicalTimeFactory.getLogicalTime(_logicalTime))
      Traits::throwLogicalTimeAlreadyPassed(_logicalTimeFactory.toString(nativeLogicalTime));
    if (_timeAdvancePending)
      Traits::throwInTimeAdvancingState();
    if (_timeRegulationEnablePending)
      Traits::throwRequestForTimeRegulationPending();
    if (_timeConstrainedEnablePending)
      Traits::throwRequestForTimeConstrainedPending();

    Traits::throwRTIinternalError("FQR not implemented");




    LogicalTime logicalTime = _logicalTimeFactory.getLogicalTime(nativeLogicalTime);
    _timeAdvancePending = true;
    _flushQueueMode = true;

    for (typename LogicalTimeMessageListMap::iterator i = _logicalTimeMessageListMap.begin(); i != _logicalTimeMessageListMap.end();) {
      queueCallbacks(i->second);
      _logicalTimeMessageListMap.erase(i++);
    }

  }

  virtual void enableAsynchronousDelivery()
    throw (AsynchronousDeliveryAlreadyEnabled,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (_asynchronousDeliveryEnabled)
      Traits::throwAsynchronousDeliveryAlreadyEnabled();
    _asynchronousDeliveryEnabled = true;
    // FIXME: cannot do so. Can only do that in the general callback dispatcher.
    // This way, we can check after each callback if asyncronous delivery is still enabled!!!!
    queueCallbacks(_receiveOrderMessages);
  }

  virtual void disableAsynchronousDelivery()
    throw (AsynchronousDeliveryAlreadyDisabled,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (!_asynchronousDeliveryEnabled)
      Traits::throwAsynchronousDeliveryAlreadyDisabled();
    _asynchronousDeliveryEnabled = false;
  }

  virtual bool queryGALT(NativeLogicalTime& logicalTime)
    throw (SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (_logicalTimeFederateHandleSetMap.empty())
      return false;
    logicalTime = _logicalTimeFactory.getLogicalTime(_logicalTimeFederateHandleSetMap.begin()->first.first);
    return true;
  }

  virtual void queryLogicalTime(NativeLogicalTime& logicalTime)
    throw (SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    logicalTime = _logicalTimeFactory.getLogicalTime(_logicalTime);
  }

  virtual bool queryLITS(NativeLogicalTime& logicalTime)
    throw (SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (_logicalTimeMessageListMap.empty())
      return false;
    logicalTime = _logicalTimeFactory.getLogicalTime(_logicalTimeMessageListMap.begin()->first);
    return true;
  }

  virtual void modifyLookahead(const NativeLogicalTimeInterval& nativeLookahead)
    throw (TimeRegulationIsNotEnabled,
           InvalidLookahead,
           InTimeAdvancingState,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (!_timeRegulationEnabled)
      Traits::throwTimeRegulationIsNotEnabled();
    if (!_logicalTimeFactory.isPositiveTimeInterval(nativeLookahead))
      Traits::throwInvalidLookahead(_logicalTimeFactory.toString(nativeLookahead));
    if (_timeAdvancePending)
      Traits::throwInTimeAdvancingState();

    LogicalTimeInterval lookahead = _logicalTimeFactory.getLogicalTimeInterval(nativeLookahead);
    _targetLookahead = lookahead;
    if (_currentLookahead < lookahead) {
      _currentLookahead = lookahead;

      // Now tell the other federates about or now message lower bound timestamp
      _localLowerBoundTimeStamp.first = _logicalTime;
      if (_logicalTimeFactory.isZeroTimeInterval(_currentLookahead)) {
        _localLowerBoundTimeStamp.second = true;
      } else {
        _localLowerBoundTimeStamp.first += _currentLookahead;
        _localLowerBoundTimeStamp.second = false;
      }

      sendCommitLowerBoundTimeStamp(_localLowerBoundTimeStamp);
    }
  }

  virtual void queryLookahead(NativeLogicalTimeInterval& logicalTimeInterval)
    throw (TimeRegulationIsNotEnabled,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (!_timeRegulationEnabled)
      Traits::throwTimeRegulationIsNotEnabled();
    logicalTimeInterval = _logicalTimeFactory.getLogicalTimeInterval(_currentLookahead);
  }

  virtual void retract(MessageRetractionHandle messageRetractionHandle)
    throw (InvalidRetractionHandle,
           TimeRegulationIsNotEnabled,
           MessageCanNoLongerBeRetracted,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (!messageRetractionHandle.valid())
      Traits::throwInvalidRetractionHandle(messageRetractionHandle.toString());
    if (!messageRetractionHandle.getFederateHandle() != getFederateHandle())
      Traits::throwInvalidRetractionHandle(messageRetractionHandle.toString());
    if (!_timeRegulationEnabled)
      Traits::throwTimeRegulationIsNotEnabled();


    Traits::throwRTIinternalError("Message retraction is not implemented!");
  }

  virtual void changeAttributeOrderType(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleSet& attributeHandleSet, OrderType orderType)
    throw (ObjectInstanceNotKnown, AttributeNotDefined, AttributeNotOwned, SaveInProgress, RestoreInProgress)
  {
    typename ObjectInstanceHandleMap::iterator i = _objectInstanceHandleMap.find(objectInstanceHandle);
    if (i == _objectInstanceHandleMap.end())
      Traits::throwObjectInstanceNotKnown(objectInstanceHandle.toString());
    for (typename AttributeHandleSet::const_iterator j = attributeHandleSet.begin(); j != attributeHandleSet.end(); ++j) {
      if (i->second._instanceAttributeVector.size() <= j->getHandle())
        Traits::throwAttributeNotDefined(j->toString());
      if (!i->second._instanceAttributeVector[j->getHandle()].valid())
        Traits::throwAttributeNotDefined(j->toString());
      if (!i->second._instanceAttributeVector[j->getHandle()]->_isOwnedByFederate)
        Traits::throwAttributeNotOwned(j->toString());
    }
    for (typename AttributeHandleSet::const_iterator j = attributeHandleSet.begin(); j != attributeHandleSet.end(); ++j) {
      i->second._instanceAttributeVector[j->getHandle()]->_orderType = orderType;
    }
  }

  virtual void changeInteractionOrderType(InteractionClassHandle interactionClassHandle, OrderType orderType)
    throw (InteractionClassNotDefined, InteractionClassNotPublished, SaveInProgress, RestoreInProgress)
  {
    if (!isValidInteractionClass(interactionClassHandle))
      Traits::throwInteractionClassNotDefined(interactionClassHandle.toString());
    InteractionClass& interactionClass = getInteractionClass(interactionClassHandle);
    if (!interactionClass.isPublished())
      Traits::throwInteractionClassNotPublished(interactionClass._name);
    interactionClass._orderType = orderType;
  }

  virtual RegionHandle createRegion(const DimensionHandleSet& dimensionHandleSet)
    throw (InvalidDimensionHandle,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    for (DimensionHandleSet::const_iterator i = dimensionHandleSet.begin(); i != dimensionHandleSet.end(); ++i) {
      if (_dimensionVector.size() <= i->getHandle())
        Traits::throwInvalidDimensionHandle(i->toString());
    }
    RegionHandle regionHandle(getFederateHandle(), _localRegionHandleAllocator.get());
    _regionHandleRegionDataMap[regionHandle]._dimensionHandleSet = dimensionHandleSet;

    SharedPtr<InsertRegionMessage> request = new InsertRegionMessage;
    request->setFederationHandle(getFederationHandle());
    RegionHandleDimensionHandleSetPairVector value(1);
    value[0].first = regionHandle;
    value[0].second = dimensionHandleSet;
    request->getRegionHandleDimensionHandleSetPairVector().swap(value);
    sendMessage(request);

    return regionHandle;
  }

  virtual void commitRegionModifications(const RegionHandleSet& regionHandleSet)
    throw (InvalidRegion,
           RegionNotCreatedByThisFederate,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    for (RegionHandleSet::const_iterator i = regionHandleSet.begin(); i != regionHandleSet.end(); ++i) {
      if (!i->valid())
        Traits::throwInvalidRegion(i->toString());
      if (_regionHandleRegionDataMap.find(*i) == _regionHandleRegionDataMap.end())
        Traits::throwInvalidRegion(i->toString());
      if (!i->getFederateHandle() != getFederateHandle())
        Traits::throwRegionNotCreatedByThisFederate(i->toString());
    }

    Traits::throwRTIinternalError();
  }

  virtual void deleteRegion(RegionHandle regionHandle)
    throw (InvalidRegion,
           RegionNotCreatedByThisFederate,
           RegionInUseForUpdateOrSubscription,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (!regionHandle.valid())
      Traits::throwInvalidRegion(regionHandle.toString());
    typename RegionHandleRegionDataMap::iterator i = _regionHandleRegionDataMap.find(regionHandle);
    if (i == _regionHandleRegionDataMap.end())
      Traits::throwInvalidRegion(regionHandle.toString());
    if (!regionHandle.getFederateHandle() != getFederateHandle())
      Traits::throwRegionNotCreatedByThisFederate(regionHandle.toString());

    // FIXME check for in use

    _regionHandleRegionDataMap.erase(i);
    LocalRegionHandle localRegionHandle = regionHandle.getLocalRegionHandle();
    _localRegionHandleAllocator.put(localRegionHandle);

    SharedPtr<EraseRegionMessage> request = new EraseRegionMessage;
    request->setFederationHandle(getFederationHandle());
    RegionHandleVector value(1);
    value[0] = regionHandle;
    request->getRegionHandleVector().swap(value);
    sendMessage(request);
  }

  virtual ObjectInstanceHandle
  registerObjectInstanceWithRegions(ObjectClassHandle objectClassHandle,
                                    const AttributeHandleSetRegionHandleSetPairVector& attributeHandleSetRegionHandleSetPairVector)
    throw (ObjectClassNotDefined,
           ObjectClassNotPublished,
           AttributeNotDefined,
           AttributeNotPublished,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError();
    return ObjectInstanceHandle();
  }

  virtual ObjectInstanceHandle
  registerObjectInstanceWithRegions(ObjectClassHandle objectClassHandle,
                                    const AttributeHandleSetRegionHandleSetPairVector& attributeHandleSetRegionHandleSetPairVector,
                                    const std::string& objectInstanceName)
    throw (ObjectClassNotDefined,
           ObjectClassNotPublished,
           AttributeNotDefined,
           AttributeNotPublished,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           ObjectInstanceNameNotReserved,
           ObjectInstanceNameInUse,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError();
    return ObjectInstanceHandle();
  }

  virtual void associateRegionsForUpdates(ObjectInstanceHandle objectInstanceHandle,
                                  const AttributeHandleSetRegionHandleSetPairVector& attributeHandleHandleSetRegionHandleSetPairVector)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError();
  }

  virtual void unassociateRegionsForUpdates(ObjectInstanceHandle objectInstanceHandle,
                                    const AttributeHandleSetRegionHandleSetPairVector& attributeHandleHandleSetRegionHandleSetPairVector)
    throw (ObjectInstanceNotKnown,
           AttributeNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError();
  }

  virtual void subscribeObjectClassAttributesWithRegions(ObjectClassHandle objectClassHandle,
                                                 const AttributeHandleSetRegionHandleSetPairVector& attributeHandleHandleSetRegionHandleSetPairVector,
                                                 bool active)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError();
  }

  virtual void unsubscribeObjectClassAttributesWithRegions(ObjectClassHandle objectClassHandle,
                                                   const AttributeHandleSetRegionHandleSetPairVector& attributeHandleHandleSetRegionHandleSetPairVector)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError();
  }

  virtual void subscribeInteractionClassWithRegions(InteractionClassHandle interactionClassHandle, const RegionHandleSet& regionHandleSet, bool active)
    throw (InteractionClassNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           FederateServiceInvocationsAreBeingReportedViaMOM,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError();
  }

  virtual void unsubscribeInteractionClassWithRegions(InteractionClassHandle interactionClassHandle, const RegionHandleSet& regionHandleSet)
    throw (InteractionClassNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError();
  }

  virtual void sendInteractionWithRegions(InteractionClassHandle interactionClassHandle,
                                  std::vector<ParameterValue>& parameterValues,
                                  const RegionHandleSet& regionHandleSet,
                                  const VariableLengthData& tag)
    throw (InteractionClassNotDefined,
           InteractionClassNotPublished,
           InteractionParameterNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError();
  }


  virtual MessageRetractionHandle
  sendInteractionWithRegions(InteractionClassHandle interactionClassHandle,
                             std::vector<ParameterValue>& parameterValues,
                             const RegionHandleSet& regionHandleSet,
                             const VariableLengthData& tag,
                             const NativeLogicalTime& nativeLogicalTime)
    throw (InteractionClassNotDefined,
           InteractionClassNotPublished,
           InteractionParameterNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           InvalidLogicalTime,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError();
    return MessageRetractionHandle();
  }

  virtual void requestAttributeValueUpdateWithRegions(ObjectClassHandle objectClassHandle, const AttributeHandleSetRegionHandleSetPairVector& theSet,
                                              const VariableLengthData& tag)
    throw (ObjectClassNotDefined,
           AttributeNotDefined,
           InvalidRegion,
           RegionNotCreatedByThisFederate,
           InvalidRegionContext,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError();
  }

  virtual ResignAction getAutomaticResignDirective()
    throw (RTIinternalError)
  {
    return _defaultResignAction;
  }

  virtual void setAutomaticResignDirective(ResignAction resignAction)
    throw (InvalidResignAction,
           RTIinternalError)
  {
    switch (resignAction) {
    case UNCONDITIONALLY_DIVEST_ATTRIBUTES:
    case DELETE_OBJECTS:
    case CANCEL_PENDING_OWNERSHIP_ACQUISITIONS:
    case DELETE_OBJECTS_THEN_DIVEST:
    case CANCEL_THEN_DELETE_THEN_DIVEST:
    case NO_ACTION:
      break;
    default:
      Traits::throwInvalidResignAction();
    }
    _defaultResignAction = resignAction;
  }

  virtual const ObjectClassHandle& getObjectClassHandle(const std::string& name) const
    throw (NameNotFound, RTIinternalError)
  {
    typename NameObjectClassHandleMap::const_iterator i = _nameObjectClassHandleMap.find(name);
    if (i == _nameObjectClassHandleMap.end())
      Traits::throwNameNotFound(name);
    return i->second;
  }

  virtual const std::string& getObjectClassName(const ObjectClassHandle& objectClassHandle) const
    throw (InvalidObjectClassHandle,
           RTIinternalError)
  {
    if (!isValidObjectClass(objectClassHandle))
      Traits::throwInvalidObjectClassHandle(objectClassHandle.toString());
    return getObjectClass(objectClassHandle)._name;
  }

  virtual const AttributeHandle& getAttributeHandle(const ObjectClassHandle& objectClassHandle, const std::string& name) const
    throw (InvalidObjectClassHandle,
           NameNotFound,
           RTIinternalError)
  {
    if (!isValidObjectClass(objectClassHandle))
      Traits::throwInvalidObjectClassHandle(objectClassHandle.toString());
    const ObjectClass& objectClass = getObjectClass(objectClassHandle);
    typename NameAttributeHandleMap::const_iterator i = objectClass._nameAttributeHandleMap.find(name);
    if (i == objectClass._nameAttributeHandleMap.end())
      Traits::throwNameNotFound(name);
    return i->second;
  }

  virtual const std::string& getAttributeName(const ObjectClassHandle& objectClassHandle, const AttributeHandle& attributeHandle) const
    throw (InvalidObjectClassHandle,
           InvalidAttributeHandle,
           AttributeNotDefined,
           RTIinternalError)
  {
    if (!isValidObjectClass(objectClassHandle))
      Traits::throwInvalidObjectClassHandle(objectClassHandle.toString());
    const ObjectClass& objectClass = getObjectClass(objectClassHandle);
    if (!objectClass.isValidAttribute(attributeHandle))
      Traits::throwInvalidAttributeHandle(attributeHandle.toString());
    return objectClass._attributeVector[attributeHandle.getHandle()]->_name;
  }

  virtual const InteractionClassHandle& getInteractionClassHandle(const std::string& name) const
    throw (NameNotFound,
           RTIinternalError)
  {
    typename NameInteractionClassHandleMap::const_iterator i = _nameInteractionClassHandleMap.find(name);
    if (i == _nameInteractionClassHandleMap.end())
      Traits::throwNameNotFound(name);
    return i->second;
  }

  virtual const std::string& getInteractionClassName(const InteractionClassHandle& interactionClassHandle) const
    throw (InvalidInteractionClassHandle,
           RTIinternalError)
  {
    if (!isValidInteractionClass(interactionClassHandle))
      Traits::throwInvalidInteractionClassHandle(interactionClassHandle.toString());
    return getInteractionClass(interactionClassHandle)._name;
  }

  virtual const ParameterHandle& getParameterHandle(const InteractionClassHandle& interactionClassHandle, const std::string& name) const
    throw (InvalidInteractionClassHandle,
           NameNotFound,
           RTIinternalError)
  {
    if (!isValidInteractionClass(interactionClassHandle))
      Traits::throwInvalidInteractionClassHandle(interactionClassHandle.toString());
    const InteractionClass& interactionClass = getInteractionClass(interactionClassHandle);
    typename NameParameterHandleMap::const_iterator i = interactionClass._nameParameterHandleMap.find(name);
    if (i == interactionClass._nameParameterHandleMap.end())
      Traits::throwNameNotFound(name);
    return i->second;
  }

  virtual const std::string& getParameterName(const InteractionClassHandle& interactionClassHandle, const ParameterHandle& parameterHandle) const
    throw (InvalidInteractionClassHandle,
           InvalidParameterHandle,
           InteractionParameterNotDefined,
           RTIinternalError)
  {
    if (!isValidInteractionClass(interactionClassHandle))
      Traits::throwInvalidInteractionClassHandle(interactionClassHandle.toString());
    const InteractionClass& interactionClass = getInteractionClass(interactionClassHandle);
    if (!interactionClass.isValidParameter(parameterHandle))
      Traits::throwInvalidParameterHandle(parameterHandle.toString());
    return interactionClass._parameterVector[parameterHandle.getHandle()]->_name;
  }

  virtual const ObjectInstanceHandle& getObjectInstanceHandle(const std::string& name) const
    throw (ObjectInstanceNotKnown,
           RTIinternalError)
  {
    typename NameObjectInstanceHandleMap::const_iterator i = _nameObjectInstanceHandleMap.find(name);
    if (i == _nameObjectInstanceHandleMap.end())
      Traits::throwObjectInstanceNotKnown(name);
    return i->second;
  }

  virtual const std::string& getObjectInstanceName(const ObjectInstanceHandle& objectInstanceHandle) const
    throw (ObjectInstanceNotKnown,
           RTIinternalError)
  {
    typename ObjectInstanceHandleMap::const_iterator i = _objectInstanceHandleMap.find(objectInstanceHandle);
    if (i == _objectInstanceHandleMap.end())
      Traits::throwObjectInstanceNotKnown(objectInstanceHandle.toString());
    return i->second._nameObjectInstanceHandleMapIterator->first;
  }

  virtual const DimensionHandle& getDimensionHandle(const std::string& name) const
    throw (NameNotFound,
           RTIinternalError)
  {
    typename NameDimensionHandleMap::const_iterator i = _nameDimensionHandleMap.find(name);
    if (i == _nameDimensionHandleMap.end())
      Traits::throwNameNotFound(name);
    return i->second;
  }

  virtual const std::string& getDimensionName(const DimensionHandle& dimensionHandle) const
    throw (InvalidDimensionHandle,
           RTIinternalError)
  {
    if (_dimensionVector.size() <= dimensionHandle.getHandle())
      Traits::throwInvalidDimensionHandle(dimensionHandle.toString());
    if (!_dimensionVector[dimensionHandle.getHandle()].valid())
      Traits::throwInvalidDimensionHandle(dimensionHandle.toString());
    return _dimensionVector[dimensionHandle.getHandle()]->_name;
  }

  virtual unsigned long getDimensionUpperBound(const DimensionHandle& dimensionHandle) const
    throw (InvalidDimensionHandle,
           RTIinternalError)
  {
    if (_dimensionVector.size() <= dimensionHandle.getHandle())
      Traits::throwInvalidDimensionHandle(dimensionHandle.toString());
    if (!_dimensionVector[dimensionHandle.getHandle()].valid())
      Traits::throwInvalidDimensionHandle(dimensionHandle.toString());
    return _dimensionVector[dimensionHandle.getHandle()]->_upperBound;
  }

  virtual const DimensionHandleSet& getAvailableDimensionsForClassAttribute(const ObjectClassHandle& objectClassHandle,
                                                                            const AttributeHandle& attributeHandle) const
    throw (InvalidObjectClassHandle,
           InvalidAttributeHandle,
           AttributeNotDefined,
           RTIinternalError)
  {
    if (!isValidObjectClass(objectClassHandle))
      Traits::throwInvalidObjectClassHandle(objectClassHandle.toString());
    const ObjectClass& objectClass = getObjectClass(objectClassHandle);
    if (!objectClass.isValidAttribute(attributeHandle))
      Traits::throwInvalidAttributeHandle(attributeHandle.toString());
    return objectClass._attributeVector[attributeHandle.getHandle()]->_dimensionHandleSet;
  }

  virtual const ObjectClassHandle& getKnownObjectClassHandle(const ObjectInstanceHandle& objectInstanceHandle) const
    throw (ObjectInstanceNotKnown,
           RTIinternalError)
  {
    typename ObjectInstanceHandleMap::const_iterator i = _objectInstanceHandleMap.find(objectInstanceHandle);
    if (i == _objectInstanceHandleMap.end())
      Traits::throwObjectInstanceNotKnown(objectInstanceHandle.toString());
    return i->second._objectClassHandle;
  }

  virtual const DimensionHandleSet& getAvailableDimensionsForInteractionClass(const InteractionClassHandle& interactionClassHandle) const
    throw (InvalidInteractionClassHandle,
           RTIinternalError)
  {
    if (!isValidInteractionClass(interactionClassHandle))
      Traits::throwInvalidInteractionClassHandle(interactionClassHandle.toString());
    return getInteractionClass(interactionClassHandle)._dimensionHandleSet;
  }

  virtual TransportationType getTransportationType(const std::string& name) const
    throw (InvalidTransportationName,
           RTIinternalError)
  {
    typename NameTransportationTypeMap::const_iterator i = _nameTransportationTypeMap.find(name);
    if (i == _nameTransportationTypeMap.end())
      Traits::throwInvalidTransportationName(name);
    return i->second;
  }

  virtual const std::string& getTransportationName(TransportationType transportationType) const
    throw (InvalidTransportationType,
           RTIinternalError)
  {
    typename TransportationTypeNameMap::const_iterator i = _transportationTypeNameMap.find(transportationType);
    if (i == _transportationTypeNameMap.end())
      Traits::throwInvalidTransportationType();
    return i->second;
  }

  virtual OrderType getOrderType(const std::string& name) const
    throw (InvalidOrderName,
           RTIinternalError)
  {
    typename NameOrderTypeMap::const_iterator i = _nameOrderTypeMap.find(name);
    if (i == _nameOrderTypeMap.end())
      Traits::throwInvalidOrderName(name);
    return i->second;
  }

  virtual const std::string& getOrderName(OrderType orderType) const
    throw (InvalidOrderType,
           RTIinternalError)
  {
    typename OrderTypeNameMap::const_iterator i = _orderTypeNameMap.find(orderType);
    if (i == _orderTypeNameMap.end())
      Traits::throwInvalidOrderType();
    return i->second;
  }

  virtual void enableObjectClassRelevanceAdvisorySwitch()
    throw (ObjectClassRelevanceAdvisorySwitchIsOn,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (_objectClassRelevanceAdvisorySwitchEnabled)
      Traits::throwObjectClassRelevanceAdvisorySwitchIsOn();
    _objectClassRelevanceAdvisorySwitchEnabled = true;

    Traits::throwRTIinternalError();
  }

  virtual void disableObjectClassRelevanceAdvisorySwitch()
    throw (ObjectClassRelevanceAdvisorySwitchIsOff,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (!_objectClassRelevanceAdvisorySwitchEnabled)
      Traits::throwObjectClassRelevanceAdvisorySwitchIsOff();
    _objectClassRelevanceAdvisorySwitchEnabled = false;

    Traits::throwRTIinternalError();
  }

  virtual void enableAttributeRelevanceAdvisorySwitch()
    throw (AttributeRelevanceAdvisorySwitchIsOn,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (_attributeRelevanceAdvisorySwitchEnabled)
      Traits::throwAttributeRelevanceAdvisorySwitchIsOn();
    _attributeRelevanceAdvisorySwitchEnabled = true;

    Traits::throwRTIinternalError();
  }

  virtual void disableAttributeRelevanceAdvisorySwitch()
    throw (AttributeRelevanceAdvisorySwitchIsOff,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (!_attributeRelevanceAdvisorySwitchEnabled)
      Traits::throwAttributeRelevanceAdvisorySwitchIsOff();
    _attributeRelevanceAdvisorySwitchEnabled = false;

    Traits::throwRTIinternalError();
  }

  virtual void enableAttributeScopeAdvisorySwitch()
    throw (AttributeScopeAdvisorySwitchIsOn,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (_attributeScopeAdvisorySwitchEnabled)
      Traits::throwAttributeScopeAdvisorySwitchIsOn();
    _attributeScopeAdvisorySwitchEnabled = true;

    Traits::throwRTIinternalError();
  }

  virtual void disableAttributeScopeAdvisorySwitch()
    throw (AttributeScopeAdvisorySwitchIsOff,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (!_attributeScopeAdvisorySwitchEnabled)
      Traits::throwAttributeScopeAdvisorySwitchIsOff();
    _attributeScopeAdvisorySwitchEnabled = false;

    Traits::throwRTIinternalError();
  }

  virtual void enableInteractionRelevanceAdvisorySwitch()
    throw (InteractionRelevanceAdvisorySwitchIsOn,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (_interactionRelevanceAdvisorySwitchEnabled)
      Traits::throwInteractionRelevanceAdvisorySwitchIsOn();
    _interactionRelevanceAdvisorySwitchEnabled = true;

    Traits::throwRTIinternalError();
  }

  virtual void disableInteractionRelevanceAdvisorySwitch()
    throw (InteractionRelevanceAdvisorySwitchIsOff,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    if (!_interactionRelevanceAdvisorySwitchEnabled)
      Traits::throwInteractionRelevanceAdvisorySwitchIsOff();
    _interactionRelevanceAdvisorySwitchEnabled = false;

    Traits::throwRTIinternalError();
  }

  virtual DimensionHandleSet getDimensionHandleSet(RegionHandle regionHandle)
    throw (InvalidRegion,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    typename RegionHandleRegionDataMap::const_iterator i = _regionHandleRegionDataMap.find(regionHandle);
    if (i == _regionHandleRegionDataMap.end())
      Traits::throwInvalidRegion();
    return i->second._dimensionHandleSet;
  }

  virtual RangeBounds getRangeBounds(RegionHandle regionHandle, DimensionHandle dimensionHandle)
    throw (InvalidRegion,
           RegionDoesNotContainSpecifiedDimension,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    typename RegionHandleRegionDataMap::const_iterator i = _regionHandleRegionDataMap.find(regionHandle);
    if (i == _regionHandleRegionDataMap.end())
      Traits::throwInvalidRegion();
    if (i->second._dimensionHandleSet.find(dimensionHandle) == i->second._dimensionHandleSet.end())
      Traits::throwRegionDoesNotContainSpecifiedDimension();
    return i->second._region.getRangeBounds(dimensionHandle);
  }

  virtual void setRangeBounds(RegionHandle regionHandle, DimensionHandle dimensionHandle, const RangeBounds& rangeBounds)
    throw (InvalidRegion,
           RegionNotCreatedByThisFederate,
           RegionDoesNotContainSpecifiedDimension,
           InvalidRangeBound,
           SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    typename RegionHandleRegionDataMap::iterator i = _regionHandleRegionDataMap.find(regionHandle);
    if (i == _regionHandleRegionDataMap.end())
      Traits::throwInvalidRegion();
    if (i->first.getFederateHandle() != getFederateHandle())
      Traits::throwRegionNotCreatedByThisFederate(regionHandle.toString());
    if (i->second._dimensionHandleSet.find(dimensionHandle) == i->second._dimensionHandleSet.end())
      Traits::throwRegionDoesNotContainSpecifiedDimension();
    // This is a duplicate check, should not harm here ...
    if (_dimensionVector.size() <= dimensionHandle.getHandle())
      Traits::throwRegionDoesNotContainSpecifiedDimension();
    if (!_dimensionVector[dimensionHandle.getHandle()].valid())
      Traits::throwRegionDoesNotContainSpecifiedDimension();
    if (_dimensionVector[dimensionHandle]->_upperBound < rangeBounds.getUpperBound())
      Traits::throwInvalidRangeBound();

    i->second._region.setRangeBounds(dimensionHandle, rangeBounds);
  }

  virtual unsigned long normalizeFederateHandle(FederateHandle federateHandle)
    throw (InvalidFederateHandle,
           RTIinternalError)
  {
    return federateHandle.getHandle();
  }

  virtual unsigned long normalizeServiceGroup(ServiceGroupIndicator serviceGroup)
    throw (InvalidServiceGroup,
           RTIinternalError)
  {
    return serviceGroup;
  }

  virtual bool evokeCallback(double approximateMinimumTimeInSeconds)
    throw (RTIinternalError)
  {
    Clock clock = Clock::now();
    clock += Clock::fromSeconds(approximateMinimumTimeInSeconds);
    return dispatchCallback(clock);
  }

  virtual bool evokeMultipleCallbacks(double approximateMinimumTimeInSeconds, double approximateMaximumTimeInSeconds)
    throw (RTIinternalError)
  {
    Clock clock = Clock::now();
    clock += Clock::fromSeconds(approximateMinimumTimeInSeconds);
    if (!dispatchCallback(clock))
      return false;

    clock = Clock::now() + Clock::fromSeconds(approximateMaximumTimeInSeconds);
    do {
      if (!dispatchCallback(Clock::initial()))
        return false;
    } while (Clock::now() <= clock);

    return true;
  }

  virtual void enableCallbacks()
    throw (SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError();
  }

  virtual void disableCallbacks()
    throw (SaveInProgress,
           RestoreInProgress,
           RTIinternalError)
  {
    Traits::throwRTIinternalError();
  }

protected:
  void sendMessage(SharedPtr<AbstractMessage> message)
  { _federationConnect->send(message); }
  SharedPtr<AbstractMessage> receiveMessage(const Clock& clock)
  { return _federationConnect->receive(clock); }

  ObjectInstanceHandleNamePair
  getFreeObjectInstanceHandleNamePair()
  {
    // Usually we already have some handles locally available, but for each new one,
    // start requesting the next from the server, this way we should stay asyncronous for ever.
    // May be the initial amount of object handles should be configuration option ...
    requestObjectInstanceHandles(1);

    if (_privateNameObjectInstanceHandlePairs.empty()) {
      Clock timeout = Clock::now() + Clock::fromSeconds(60);
      while (_privateNameObjectInstanceHandlePairs.empty()) {
        if (dispatchInternal(timeout))
          continue;
        if (timeout < Clock::now())
          Traits::throwRTIinternalError("Timeout while waiting for free object handles.");
      }
    }

    NameObjectInstanceHandleMap::iterator i = _privateNameObjectInstanceHandlePairs.begin();
    ObjectInstanceHandleNamePair handleNamePair(i->second, i->first);
    _privateNameObjectInstanceHandlePairs.erase(i);

    return handleNamePair;
  }

  typedef std::list<SharedPtr<const AbstractMessage> > MessageList2;

  ///////////////////////////////////////////////////////////////////
  // processing of internal messages. This happens while waiting for a callback.
  class OPENRTI_LOCAL InternalDispatchFunctor {
  public:
    InternalDispatchFunctor(Federate& federate) :
      _federate(federate)
    { }
    template<typename M>
    void operator()(const M& message) const
    { _federate.acceptInternalMessage(message); }
  private:
    Federate& _federate;
  };

  virtual void dispatchInternalMessage(const AbstractMessage& message)
  {
    FunctorConstMessageDispatcher<InternalDispatchFunctor> dispatcher(InternalDispatchFunctor(*this));
    message.dispatch(dispatcher);
  }

  // This is for internal messages
  bool dispatchInternal(const Clock& clock)
  {
    SharedPtr<AbstractMessage> message = receiveMessage(clock);
    if (!message.valid())
      return false;

    dispatchInternalMessage(*message);
    return true;
  }

#if 0
  // FIXME want to have something like that, but how, does the message dispatcher know where to forward??
  // This dispatches messages at first with the given message dispatcher and what is not processed,
  // passes this along to the internal message stuff.
  template<typename T>
  bool dispatchInternal(const T& functorMessageDispatcherCallback, const Clock& clock)
  {
    SharedPtr<AbstractMessage> message = receiveMessage(clock);
    if (!message.valid())
      return false;

    /// Derive in some intelligent way??
    class OPENRTI_LOCAL MyFunctor : public T {
    public:
      MyFunctor(const T& t, Federate& federate) :
        T(t),
        _federate(federate)
      { }
      using T::operator();
      void operator()(AbstractMessage& message) const
      { _federate.dispatchInternalMessage(message); }
    private:
      Federate& _federate;
    };
    FunctorMessageDispatcher<MyFunctor> messageDispatcher(MyFunctor(functorMessageDispatcherCallback));
    message->dispatch(messageDispatcher);
    return true;
  }
#endif
protected:
  // The default fallback implementation
  virtual void acceptInternalMessage(const AbstractMessage& message)
  { Traits::throwRTIinternalError("Unexpected message in internal message processing!"); }

  virtual void acceptInternalMessage(const ConnectionLostMessage& message)
  { }
  virtual void acceptInternalMessage(const JoinFederationExecutionResponseMessage& message)
  {
    _federateHandle = message.getFederateHandle();
    _federateName = message.getFederateName();
    _federateType = message.getFederateType();
  }
  virtual void acceptInternalMessage(const JoinFederateNotifyMessage& message)
  {
    _federateHandleSet.insert(message.getFederateHandle());
    // Ok, this is tricky. As long as we are waiting for the root servers reply, we need to assume that the
    // time regulation request hits the root server when this federate is already joined, which means it will
    // get the request and respond to us with a reply we need to wait for.
    // When we have already recieved the root servers response, the new federate just gets the information that we are
    // already time regulating - in which case we do not get a response from that federate.
    if (_timeRegulationEnableFederateHandleSet.count(getFederateHandle())) {
      _timeRegulationEnableFederateHandleSet.insert(message.getFederateHandle());
    }
  }
  virtual void acceptInternalMessage(const ResignFederateNotifyMessage& message)
  {
    removeFederateFromTimeManagement(message.getFederateHandle());
    _federateHandleSet.erase(message.getFederateHandle());
  }

  virtual void acceptInternalMessage(const RegisterFederationSynchronizationPointResponseMessage& message)
  { queueCallback(message); }
  virtual void acceptInternalMessage(const AnnounceSynchronizationPointMessage& message)
  { queueCallback(message); }
  virtual void acceptInternalMessage(const FederationSynchronizedMessage& message)
  { queueCallback(message); }

  virtual void acceptInternalMessage(const InsertRegionMessage& message)
  {
    for (RegionHandleDimensionHandleSetPairVector::const_iterator i = message.getRegionHandleDimensionHandleSetPairVector().begin();
         i != message.getRegionHandleDimensionHandleSetPairVector().end(); ++i) {
      _regionHandleRegionDataMap[i->first]._dimensionHandleSet = i->second;
    }
  }
  virtual void acceptInternalMessage(const CommitRegionMessage& message)
  {
    for (RegionHandleRegionValuePairVector::const_iterator i = message.getRegionHandleRegionValuePairVector().begin();
         i != message.getRegionHandleRegionValuePairVector().end(); ++i) {
      typename RegionHandleRegionDataMap::iterator j = _regionHandleRegionDataMap.find(i->first);
      OpenRTIAssert(j != _regionHandleRegionDataMap.end());
      for (RegionValue::const_iterator k = i->second.begin(); k != i->second.end(); ++k) {
        Region region;
        region.setRangeBounds(k->first, k->second);
        j->second._region.swap(region);
      }
    }
  }
  virtual void acceptInternalMessage(const EraseRegionMessage& message)
  {
    for (RegionHandleVector::const_iterator i = message.getRegionHandleVector().begin();
         i != message.getRegionHandleVector().end(); ++i) {
      _regionHandleRegionDataMap.erase(*i);
    }
  }

  virtual void acceptInternalMessage(const ChangeInteractionClassPublicationMessage& message)
  { }
  virtual void acceptInternalMessage(const ChangeObjectClassPublicationMessage& message)
  { }

  virtual void acceptInternalMessage(const ChangeInteractionClassSubscriptionMessage& message)
  {
    // If we are not interessted in such changes, just ignore
    if (!_interactionRelevanceAdvisorySwitchEnabled)
      return;
    const InteractionClass& interactionClass = getInteractionClass(message.getInteractionClassHandle());

    // FIXME need to track this state somewhere so that we can call this callback when we start publishing

    // // Can happen when we unpublish but a subscription is already underway to us
    // if (!interactionClass.isPublished())
    //   return;
    // SharedPtr<TurnInteractionsOnMessage> callback = new TurnInteractionsOnMessage;
    // callback->setInteractionClassHandle(message.getInteractionClassHandle());
    // callback->setOn(message.getSubscriptionType() == SubscribedActive);
    // queueCallback(*callback);
  }
  virtual void acceptInternalMessage(const ChangeObjectClassSubscriptionMessage& message)
  {
  }

  virtual void acceptInternalMessage(const ObjectInstanceHandlesResponseMessage& message)
  {
    // it is not for us, should not happen???
    if (message.getFederateHandle() != getFederateHandle())
      return;
    for (ObjectInstanceHandleNamePairVector::const_iterator i = message.getObjectInstanceHandleNamePairVector().begin();
         i != message.getObjectInstanceHandleNamePairVector().end(); ++i) {
      _privateNameObjectInstanceHandlePairs[i->second] = i->first;
    }
  }
  virtual void acceptInternalMessage(const ReserveObjectInstanceNameResponseMessage& message)
  {
    // Don't bother, not for us
    if (message.getFederateHandle() != getFederateHandle())
      return;
    if (_syncronousReservationHandleNamePair.second == message.getObjectInstanceHandleNamePair().second) {
      _syncronousReservationHandleNamePair.second.clear();
      _syncronousReservationHandleNamePair.first = message.getObjectInstanceHandleNamePair().first;
    } else {
      queueCallback(message);
    }
  }
  virtual void acceptInternalMessage(const ReserveMultipleObjectInstanceNameResponseMessage& message)
  {
    // Don't bother, not for us
    if (message.getFederateHandle() != getFederateHandle())
      return;
    queueCallback(message);
  }

  virtual void acceptInternalMessage(const InsertObjectInstanceMessage& message)
  { queueCallback(message); }
  virtual void acceptInternalMessage(const DeleteObjectInstanceMessage& message)
  { queueReceiveOrderCallback(message); }
  virtual void acceptInternalMessage(const TimeStampedDeleteObjectInstanceMessage& message)
  { queueTimeStampedMessage(message.getTimeStamp(), message); }

  virtual void acceptInternalMessage(const AttributeUpdateMessage& message)
  { queueReceiveOrderCallback(message); }
  virtual void acceptInternalMessage(const TimeStampedAttributeUpdateMessage& message)
  { queueTimeStampedMessage(message.getTimeStamp(), message); }

  virtual void acceptInternalMessage(const InteractionMessage& message)
  { queueReceiveOrderCallback(message); }
  virtual void acceptInternalMessage(const TimeStampedInteractionMessage& message)
  { queueTimeStampedMessage(message.getTimeStamp(), message); }

  // This message is sent from federates that wish to get time regulating.
  // The federate sends its current logical time in the request.
  // If we are time constraind, we rely on the guarantee that we do not receive messages
  // from the past. This means if the requesting federate would be able to send messages
  // from the past, we need to tell that federate that it has to adjust its logical time
  // to match already established logical time guarantees.
  // Also this current federate needs to know that we will not advance further than this
  // new regulating federate is. So this call in effect establishes the logical time
  // entry in the map containing the time regulating federates and the timestamps.
  // Once the new federate has collected all its knowledge, it might commit a later time.
  virtual void acceptInternalMessage(const EnableTimeRegulationRequestMessage& message)
  {
    if (getFederateHandle() == message.getFederateHandle()) {
      // This is our own request looping back to ourself.
      // The root server started the broadcast. This way we are in order with newly
      // joined federates who either reply with a response if and only if they joined the
      // root server before the enable time regulation request broadcast was started from
      // the root server due to our request.
      // The responses juse take the direct route back.

      if (!_timeRegulationEnablePending) {
        Log(Network, Warning) << "Recieved own EnableTimeRegulationRequestMessage without waiting for that to happen!" << std::endl;
        return;
      }
      _timeRegulationEnableFederateHandleSet.erase(getFederateHandle());

      // This one checks if we are the last one this ambassador is waiting for
      // and if so, queues the callback and informs the other federates about our
      // new logical time
      checkTimeRegulationEnabled();

    } else {
      LogicalTime logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getTimeStamp().getLogicalTime());
      bool zeroLookahead = message.getTimeStamp().getZeroLookahead(); /* FIXME ?? */
      LogicalTimePair logicalTimePair(logicalTime, zeroLookahead);

      // If we are in the state of a fresh joined federate which is still collecting initial information
      // we should skip sending the response.
      if (getFederateHandle().valid()) {
        SharedPtr<EnableTimeRegulationResponseMessage> response;
        response = new EnableTimeRegulationResponseMessage;
        response->setFederationHandle(getFederationHandle());
        response->setFederateHandle(message.getFederateHandle());
        response->setRespondingFederateHandle(getFederateHandle());
        response->setTimeStampValid(false);

        if (_timeConstrainedEnabled || _timeConstrainedEnablePending) {
          // The originating ambassador sends an initial proposal for the logical time,
          // We respond with a corrected logical time if this proposal is not sufficient
          if (logicalTime < _logicalTime) {
            logicalTime = _logicalTime;
            response->getTimeStamp().setLogicalTime(_logicalTimeFactory.encodeLogicalTime(logicalTime));
            response->getTimeStamp().setZeroLookahead(false /*FIXME*/);
            response->setTimeStampValid(true);
          }
        }

        sendMessage(response);
      }

      // Store that in the time map, so that we cannot advance more than that until this federate
      // commits a new lbts in the rti. This will usually happen in immediate response to that sent message.
      typename LogicalTimeFederateHandleSetMap::iterator i;
      // only inserts if the entry is new
      i = _logicalTimeFederateHandleSetMap.insert(std::make_pair(logicalTimePair, FederateHandleSet())).first;
      i->second.insert(message.getFederateHandle());
      _federateHandleLogicalTimeMap.insert(std::make_pair(message.getFederateHandle(), i));
    }
  }
  virtual void acceptInternalMessage(const EnableTimeRegulationResponseMessage& message)
  {
    if (!_timeRegulationEnablePending) {
      Log(Network, Warning) << "Recieved EnableTimeRegulationResponseMessage without waiting for that to happen!" << std::endl;
      return;
    }

    if (message.getTimeStampValid()) {
      _timeRegulationEnableFederateHandleTimeStampMap.insert(std::make_pair(message.getRespondingFederateHandle(), message.getTimeStamp()));
    }

    _timeRegulationEnableFederateHandleSet.erase(message.getRespondingFederateHandle());

    // This one checks if we are the last one this ambassador is waiting for
    // and if so, queues the callback and informs the other federates about our
    // new logical time
    checkTimeRegulationEnabled();
  }
  virtual void acceptInternalMessage(const DisableTimeRegulationRequestMessage& message)
  {
    removeFederateFromTimeManagement(message.getFederateHandle());
  }
  virtual void acceptInternalMessage(const CommitLowerBoundTimeStampMessage& message)
  {
    // Once we receive these commits, the federate must have registered as some regulating, this must be here
    OpenRTIAssert(!_logicalTimeFederateHandleSetMap.empty());
    OpenRTIAssert(!_federateHandleLogicalTimeMap.empty());

    FederateHandle federateHandle = message.getFederateHandle();
    LogicalTime logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getTimeStamp().getLogicalTime());
    bool zeroLookahead = message.getTimeStamp().getZeroLookahead(); /* FIXME*/
    LogicalTimePair logicalTimePair(logicalTime, zeroLookahead);

    // Register the new logical time for this federate.
    // If this one is federate handle is already registered at this given time, just return.
    // This case where nothing new is in this commit might happen when a federate started
    // being time regulating and got a correction from this current federate. Then we
    // have already noted this corrected time in the map.
    typename LogicalTimeFederateHandleSetMap::iterator i;
    // only inserts if the entry is new
    i = _logicalTimeFederateHandleSetMap.insert(std::make_pair(logicalTimePair, FederateHandleSet())).first;
    if (!i->second.insert(federateHandle).second)
      return;

    typename FederateHandleLogicalTimeMap::iterator j = _federateHandleLogicalTimeMap.find(federateHandle);
    OpenRTIAssert(j != _federateHandleLogicalTimeMap.end());
    // The federate has a new timestamp, complete that change and remove the reference to the old timestamp now
    // store the new logical time for the federate handle
    std::swap(j->second, i);
    // and erase the old logical time
    i->second.erase(federateHandle);

    // Check if we have with this commit now enabled a time advance in some sense.
    // If this map entry has no federate handle left, then this is the case
    if (!i->second.empty())
      return;

    // This map entry could then be erased ...
    _logicalTimeFederateHandleSetMap.erase(i);
    OpenRTIAssert(!_logicalTimeFederateHandleSetMap.empty());

    // .. and see if we could safely advance to the pending logical time
    if (_timeConstrainedEnablePending) {
      if (_logicalTime <= _logicalTimeFederateHandleSetMap.begin()->first.first) {
        queueTimeConstrainedEnabled(_logicalTime);
      }
    } else if (_timeConstrainedEnabled && _timeAdvancePending) {
      if (_pendingLogicalTime <= _logicalTimeFederateHandleSetMap.begin()->first) {
        queueTimeAdvanceGranted(_pendingLogicalTime.first);
      } else if (_nextMessageMode && _logicalTime < _logicalTimeFederateHandleSetMap.begin()->first.first) {
        _pendingLogicalTime = _logicalTimeFederateHandleSetMap.begin()->first;
        queueTimeAdvanceGranted(_logicalTimeFederateHandleSetMap.begin()->first.first);
      }
    }

    OpenRTIAssert(!_logicalTimeFederateHandleSetMap.empty());
    OpenRTIAssert(!_federateHandleLogicalTimeMap.empty());
  }

  virtual void acceptInternalMessage(const RequestAttributeUpdateMessage& message)
  { queueCallback(message); }
  virtual void acceptInternalMessage(const RequestClassAttributeUpdateMessage& message)
  { /* FIXME */ }


  void removeFederateFromTimeManagement(const FederateHandle& federateHandle)
  {
    _timeRegulationEnableFederateHandleSet.erase(federateHandle);
    _timeRegulationEnableFederateHandleTimeStampMap.erase(federateHandle);

    typename FederateHandleLogicalTimeMap::iterator i = _federateHandleLogicalTimeMap.find(federateHandle);
    if (i != _federateHandleLogicalTimeMap.end()) {
      i->second->second.erase(federateHandle);
      if (i->second->second.empty())
        _logicalTimeFederateHandleSetMap.erase(i->second);
      _federateHandleLogicalTimeMap.erase(i);
    }

    if (_timeRegulationEnablePending) {
      // This one checks if we are the last one this ambassador is waiting for
      // and if so, queues the callback and informs the other federates about our
      // new logical time
      checkTimeRegulationEnabled();
    }
    if (_timeConstrainedEnablePending) {
      // Case, time constrained pending
      if (_logicalTimeFederateHandleSetMap.empty()) {
        // Ok, now nobody constrains us anymore
        queueTimeConstrainedEnabled(_logicalTime);
      } else if (_logicalTime <= _logicalTimeFederateHandleSetMap.begin()->first.first) {
        // Ok, all that might constrain us is ahead of us
        queueTimeConstrainedEnabled(_logicalTime);
      }
    }
    if (_timeAdvancePending) {
      // Case, time advance pending
      if (_logicalTimeFederateHandleSetMap.empty()) {
        queueTimeAdvanceGranted(_pendingLogicalTime.first);
      } else if (_pendingLogicalTime <= _logicalTimeFederateHandleSetMap.begin()->first) {
        queueTimeAdvanceGranted(_pendingLogicalTime.first);
      } else if (_nextMessageMode && _logicalTime < _logicalTimeFederateHandleSetMap.begin()->first.first) {
        _pendingLogicalTime = _logicalTimeFederateHandleSetMap.begin()->first;
        queueTimeAdvanceGranted(_logicalTimeFederateHandleSetMap.begin()->first.first);
      }
    }
  }

  void checkTimeRegulationEnabled()
  {
    // Check if that was the last one we were waiting for
    if (!_timeRegulationEnableFederateHandleSet.empty())
      return;

    // Now, collect all logical times that other federates sent to us
    for (FederateHandleTimeStampMap::const_iterator i = _timeRegulationEnableFederateHandleTimeStampMap.begin();
         i != _timeRegulationEnableFederateHandleTimeStampMap.end(); ++i) {
      LogicalTime logicalTime = _logicalTimeFactory.decodeLogicalTime(i->second.getLogicalTime());
      bool zeroLookahead = i->second.getZeroLookahead(); /* FIXME*/
      LogicalTimePair logicalTimePair(logicalTime, zeroLookahead);
      if (logicalTimePair <= _localLowerBoundTimeStamp)
        continue;

      _localLowerBoundTimeStamp = logicalTimePair;
      _pendingLogicalTime.first = logicalTime;
      _pendingLogicalTime.first -= _currentLookahead;
      _pendingLogicalTime.second = zeroLookahead;
    }

    _timeRegulationEnableFederateHandleTimeStampMap.clear();

    // If somebody has corrected the logical time, then there might be several
    // federates who have a too little committed time, so tell all about them
    sendCommitLowerBoundTimeStamp(_localLowerBoundTimeStamp);

    // Ok, now go on ...
    queueTimeRegulationEnabled(_pendingLogicalTime.first);
  }

  void sendCommitLowerBoundTimeStamp(const LogicalTimePair& logicalTimePair)
  {
    SharedPtr<CommitLowerBoundTimeStampMessage> request;
    request = new CommitLowerBoundTimeStampMessage;
    request->setFederationHandle(getFederationHandle());
    request->setFederateHandle(getFederateHandle());
    request->getTimeStamp().setLogicalTime(_logicalTimeFactory.encodeLogicalTime(logicalTimePair.first));
    request->getTimeStamp().setZeroLookahead(logicalTimePair.second);
    sendMessage(request);
  }
  void queueTimeConstrainedEnabled(const LogicalTime& logicalTime)
  {
    SharedPtr<TimeConstrainedEnabledMessage> message = new TimeConstrainedEnabledMessage;
    message->setLogicalTime(_logicalTimeFactory.encodeLogicalTime(logicalTime));
    queueCallback(message);
  }
  void queueTimeRegulationEnabled(const LogicalTime& logicalTime)
  {
    SharedPtr<TimeRegulationEnabledMessage> message = new TimeRegulationEnabledMessage;
    message->setLogicalTime(_logicalTimeFactory.encodeLogicalTime(logicalTime));
    queueCallback(message);
  }
  void queueTimeAdvanceGranted(const LogicalTime& logicalTime)
  {
    // Flush all timestamped messages up to the given logical time
    for (typename LogicalTimeMessageListMap::iterator i = _logicalTimeMessageListMap.begin(); i != _logicalTimeMessageListMap.end();) {
      if (logicalTime < i->first)
        break;
      queueCallbacks(i->second);
      _logicalTimeMessageListMap.erase(i++);
    }
    // Queue the time advance granted
    SharedPtr<TimeAdvanceGrantedMessage> message = new TimeAdvanceGrantedMessage;
    message->setLogicalTime(_logicalTimeFactory.encodeLogicalTime(logicalTime));
    queueCallback(message);
    // Queue the may be accumulated ro callbacks that are hold back until now
    queueCallbacks(_receiveOrderMessages);
  }

  bool logicalTimeAlreadyPassed(const NativeLogicalTime& logicalTime)
  {
    if (_localLowerBoundTimeStamp.second) {
      return _logicalTimeFactory.getLogicalTime(_localLowerBoundTimeStamp.first) < logicalTime;
    } else {
      return _logicalTimeFactory.getLogicalTime(_localLowerBoundTimeStamp.first) <= logicalTime;
    }
  }
  // The language binding needs that to see how it should really deliver messages
  bool getTimeConstrainedEnabled() const
  { return _timeConstrainedEnabled; }
  bool getFlushQueueMode() const
  { return _flushQueueMode; }
  // Get a next message retraction handle
  MessageRetractionHandle getNextMessageRetractionHandle()
  { return MessageRetractionHandle(getFederateHandle(), _messageRetractionSerial++); }

  ///////////////////////////////////////////////////////////////////
  // processing of callback messages - this is what the ambassador user sees
  void queueCallback(SharedPtr<const AbstractMessage> message)
  { _callbackMessageQueue.push_back(message); }
  void queueCallback(const AbstractMessage& message)
  { _callbackMessageQueue.push_back(&message); }
  // Append the whole list to the callback list
  void queueCallbacks(MessageList2& messageList)
  { _callbackMessageQueue.splice(_callbackMessageQueue.end(), messageList); }

  void queueReceiveOrderCallback(const AbstractMessage& message)
  {
    // If we are in time advancing state and recieve order is not allowed
    if (!_asynchronousDeliveryEnabled && _timeAdvancePending) {
      _receiveOrderMessages.push_back(&message);
    } else {
      queueCallback(message);
    }
  }
  void queueTimeStampedMessage(const VariableLengthData& timeStamp, const AbstractMessage& message)
  {
    if (_timeConstrainedEnabled) {
      LogicalTime logicalTime = _logicalTimeFactory.decodeLogicalTime(timeStamp);
      _logicalTimeMessageListMap[logicalTime].push_back(&message);
    } else {
      queueCallback(message);
    }
  }

  class OPENRTI_LOCAL CallbackDispatchFunctor {
  public:
    CallbackDispatchFunctor(Federate& federate) :
      _federate(federate)
    { }
    template<typename M>
    void operator()(const M& message) const
    { _federate.acceptCallbackMessage(message); }
  private:
    Federate& _federate;
  };

  void dispatchCallbackMessage(const AbstractMessage& message)
  {
    FunctorConstMessageDispatcher<CallbackDispatchFunctor> dispatcher(CallbackDispatchFunctor(*this));
    message.dispatch(dispatcher);
  }

  // Here we just should see messages which do callbacks in the ambassador
  bool dispatchCallback(const Clock& clock)
  {
    // FIXME
    while (_callbackMessageQueue.empty()) {
      if (!dispatchInternal(clock))
        return false;
    }
    dispatchCallbackMessage(*_callbackMessageQueue.front());
    _callbackMessageQueue.pop_front();
    while (_callbackMessageQueue.empty()) {
      if (!dispatchInternal(Clock::initial()))
        return false;
    }
    return true;
  }

  // The default fallback implementations
  void acceptCallbackMessage(const AbstractMessage& message)
  { Traits::throwRTIinternalError("Unexpected message in callback message processing!"); }

  // These are called when callback messages are processed
  virtual void acceptCallbackMessage(const RegisterFederationSynchronizationPointResponseMessage& message)
  { synchronizationPointRegistrationResponse(message.getLabel(), message.getRegisterFederationSynchronizationPointResponseType()); }
  virtual void acceptCallbackMessage(const AnnounceSynchronizationPointMessage& message)
  {
    _announcedFederationSynchonizationLabels.insert(message.getLabel());
    announceSynchronizationPoint(message.getLabel(), message.getTag());
  }
  virtual void acceptCallbackMessage(const FederationSynchronizedMessage& message)
  {
    federationSynchronized(message.getLabel());
    _announcedFederationSynchonizationLabels.erase(message.getLabel());
  }

  virtual void acceptCallbackMessage(const RegistrationForObjectClassMessage& message)
  {
    registrationForObjectClass(message.getObjectClassHandle(), message.getStart());
  }
  virtual void acceptCallbackMessage(const TurnInteractionsOnMessage& message)
  {
    turnInteractionsOn(message.getInteractionClassHandle(), message.getOn());
  }

  virtual void acceptCallbackMessage(const ReserveObjectInstanceNameResponseMessage& message)
  {
    if (message.getSuccess())
      _reservedNameObjectInstanceHandlePairs[message.getObjectInstanceHandleNamePair().second] = message.getObjectInstanceHandleNamePair().first;
    objectInstanceNameReservation(message);
  }
  virtual void acceptCallbackMessage(const ReserveMultipleObjectInstanceNameResponseMessage& message)
  {
    if (message.getSuccess()) {
      for (ObjectInstanceHandleNamePairVector::const_iterator i = message.getObjectInstanceHandleNamePairVector().begin();
           i != message.getObjectInstanceHandleNamePairVector().end(); ++i) {
        _reservedNameObjectInstanceHandlePairs[i->second] = i->first;
      }
    }
    objectInstanceNameReservation(message);
  }
  virtual void acceptCallbackMessage(const InsertObjectInstanceMessage& message)
  {
    ObjectClassHandle objectClassHandle = message.getObjectClassHandle();
    if (!isValidObjectClass(objectClassHandle))
      return;
    while (Unsubscribed == getObjectClass(objectClassHandle).getEffectiveSubscriptionType()) {
      objectClassHandle = getObjectClass(objectClassHandle)._parentObjectClassHandle;
      if (!isValidObjectClass(objectClassHandle))
        return;
    }
    insertObjectInstance(message.getObjectInstanceHandle(), message.getName(), objectClassHandle, false);
    discoverObjectInstance(message.getObjectInstanceHandle(), objectClassHandle, message.getName());
  }
  virtual void acceptCallbackMessage(const DeleteObjectInstanceMessage& message)
  {
    ObjectInstanceHandle objectInstanceHandle = message.getObjectInstanceHandle();
    typename ObjectInstanceHandleMap::iterator i = _objectInstanceHandleMap.find(objectInstanceHandle);
    if (i == _objectInstanceHandleMap.end())
      return;
    if (isValidObjectClass(i->second._objectClassHandle)) {
      if (Unsubscribed != getObjectClass(i->second._objectClassHandle).getEffectiveSubscriptionType()) {
        removeObjectInstance(message);
      }
    }
    eraseObjectInstance(message.getObjectInstanceHandle());
  }

  virtual void acceptCallbackMessage(const AttributeUpdateMessage& message)
  {
    // Look for the known object class of this object instance.
    // Is required for the right subset of attributes that are reflected
    ObjectInstanceHandle objectInstanceHandle = message.getObjectInstanceHandle();
    typename ObjectInstanceHandleMap::iterator i = _objectInstanceHandleMap.find(objectInstanceHandle);
    if (i == _objectInstanceHandleMap.end())
      return;
    if (!isValidObjectClass(i->second._objectClassHandle))
      return;
    reflectAttributeValues(getObjectClass(i->second._objectClassHandle), message);
  }
  virtual void acceptCallbackMessage(const TimeStampedAttributeUpdateMessage& message)
  {
    // Look for the known object class of this object instance.
    // Is required for the right subset of attributes that are reflected
    ObjectInstanceHandle objectInstanceHandle = message.getObjectInstanceHandle();
    typename ObjectInstanceHandleMap::iterator i = _objectInstanceHandleMap.find(objectInstanceHandle);
    if (i == _objectInstanceHandleMap.end())
      return;
    if (!isValidObjectClass(i->second._objectClassHandle))
      return;
    reflectAttributeValues(getObjectClass(i->second._objectClassHandle), message,
                           _logicalTimeFactory.getLogicalTime(_logicalTimeFactory.decodeLogicalTime(message.getTimeStamp())));
  }

  virtual void acceptCallbackMessage(const InteractionMessage& message)
  {
    InteractionClassHandle interactionClassHandle = message.getInteractionClassHandle();
    if (!isValidInteractionClass(interactionClassHandle))
      return;
    // Subscriptions can race against sending them to this federate
    // FIXME: store the effective next interaction class that is subscribed for all interaction classes.
    // This would avoid this loop
    while (Unsubscribed == getInteractionClass(interactionClassHandle).getSubscriptionType()) {
      interactionClassHandle = getInteractionClass(interactionClassHandle)._parentInteractionClassHandle;
      if (!isValidInteractionClass(interactionClassHandle))
        return;
    }
    receiveInteraction(interactionClassHandle, message);
  }
  virtual void acceptCallbackMessage(const TimeStampedInteractionMessage& message)
  {
    InteractionClassHandle interactionClassHandle = message.getInteractionClassHandle();
    if (!isValidInteractionClass(interactionClassHandle))
      return;
    // Subscriptions can race against sending them to this federate
    // FIXME: store the effective next interaction class that is subscribed for all interaction classes.
    // This would avoid this loop
    while (Unsubscribed == getInteractionClass(interactionClassHandle).getSubscriptionType()) {
      interactionClassHandle = getInteractionClass(interactionClassHandle)._parentInteractionClassHandle;
      if (!isValidInteractionClass(interactionClassHandle))
        return;
    }
    receiveInteraction(interactionClassHandle, message,
                       _logicalTimeFactory.getLogicalTime(_logicalTimeFactory.decodeLogicalTime(message.getTimeStamp())));
  }

  virtual void acceptCallbackMessage(const TimeConstrainedEnabledMessage& message)
  {
    _timeConstrainedEnablePending = false;
    _timeConstrainedEnabled = true;
    _logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getLogicalTime());
    timeConstrainedEnabled(_logicalTimeFactory.getLogicalTime(_logicalTime));
  }
  void acceptCallbackMessage(const TimeRegulationEnabledMessage& message)
  {
    _timeRegulationEnablePending = false;
    _timeRegulationEnabled = true;
    _logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getLogicalTime());
    timeRegulationEnabled(_logicalTimeFactory.getLogicalTime(_logicalTime));
  }
  void acceptCallbackMessage(const TimeAdvanceGrantedMessage& message)
  {
    _logicalTime = _logicalTimeFactory.decodeLogicalTime(message.getLogicalTime());
    _pendingLogicalTime.first = _logicalTime;
    _timeAdvancePending = false;
    _flushQueueMode = false;
    timeAdvanceGrant(_logicalTimeFactory.getLogicalTime(_logicalTime));
  }

  virtual void acceptCallbackMessage(const RequestAttributeUpdateMessage& message)
  {
    // FIXME: can fail. Check that already in the internal processing
    ObjectInstanceHandle objectInstanceHandle = message.getObjectInstanceHandle();
    typename ObjectInstanceHandleMap::iterator i = _objectInstanceHandleMap.find(objectInstanceHandle);
    if (i == _objectInstanceHandleMap.end())
      return;
    AttributeHandleVector attributeHandleSet;
    for (AttributeHandleVector::const_iterator j = message.getAttributeHandles().begin(); j != message.getAttributeHandles().end(); ++j) {
      if (i->second._instanceAttributeVector.size() <= j->getHandle())
        continue;
      if (!i->second._instanceAttributeVector[j->getHandle()].valid())
        continue;
      if (!i->second._instanceAttributeVector[j->getHandle()]->_isOwnedByFederate)
        continue;
      attributeHandleSet.reserve(message.getAttributeHandles().size());
      attributeHandleSet.push_back(*j);
    }
    if (attributeHandleSet.empty())
      return;
    provideAttributeValueUpdate(objectInstanceHandle, attributeHandleSet, message.getTag());
  }

  // The callback into the binding concrete implementation.
  virtual void synchronizationPointRegistrationResponse(const std::string& label, RegisterFederationSynchronizationPointResponseType reason)
    throw () = 0;
  virtual void announceSynchronizationPoint(const std::string& label, const VariableLengthData& tag)
    throw () = 0;
  virtual void federationSynchronized(const std::string& label)
    throw () = 0;

  virtual void registrationForObjectClass(ObjectClassHandle objectClassHandle, bool start)
    throw () = 0;

  virtual void turnInteractionsOn(InteractionClassHandle interactionClassHandle, bool on)
    throw () = 0;

  virtual void objectInstanceNameReservation(const ReserveObjectInstanceNameResponseMessage&)
    throw () = 0;
  virtual void objectInstanceNameReservation(const ReserveMultipleObjectInstanceNameResponseMessage&)
    throw () = 0;

  // 6.5
  virtual void discoverObjectInstance(ObjectInstanceHandle objectInstanceHandle, ObjectClassHandle objectClassHandle,
                                      const std::string& objectInstanceName)
    throw () = 0;


  virtual void reflectAttributeValues(const ObjectClass& objectClass, const AttributeUpdateMessage& message)
    throw () = 0;

  virtual void reflectAttributeValues(const ObjectClass& objectClass, const TimeStampedAttributeUpdateMessage& message,
                                      const NativeLogicalTime& logicalTime)
    throw () = 0;

  // 6.11
  virtual void removeObjectInstance(const DeleteObjectInstanceMessage& message)
    throw () = 0;

  virtual void removeObjectInstance(const TimeStampedDeleteObjectInstanceMessage& message, const NativeLogicalTime& logicalTime)
    throw () = 0;

  // 6.9
  virtual void receiveInteraction(const InteractionClassHandle& interactionClassHandle, const InteractionMessage& message)
    throw () = 0;

  virtual void receiveInteraction(const InteractionClassHandle& interactionClassHandle, const TimeStampedInteractionMessage& message, const NativeLogicalTime& logicalTime)
    throw () = 0;

  // 6.15
  virtual void attributesInScope(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw () = 0;

  // 6.16
  virtual void attributesOutOfScope(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw () = 0;

  // 6.18
  virtual void provideAttributeValueUpdate(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector,
                                           const VariableLengthData& tag)
    throw () = 0;

  // 6.19
  virtual void turnUpdatesOnForObjectInstance(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw () = 0;

  // 6.20
  virtual void turnUpdatesOffForObjectInstance(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw () = 0;

  // 7.4
  virtual void requestAttributeOwnershipAssumption(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector,
                                                   const VariableLengthData& tag)
    throw () = 0;

  // 7.5
  virtual void requestDivestitureConfirmation(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw () = 0;

  // 7.7
  virtual void attributeOwnershipAcquisitionNotification(ObjectInstanceHandle objectInstanceHandle,
                                                         const AttributeHandleVector& attributeHandleVector,
                                                         const VariableLengthData& tag)
    throw () = 0;

  // 7.10
  virtual void attributeOwnershipUnavailable(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw () = 0;

  // 7.11
  virtual void requestAttributeOwnershipRelease(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector,
                                                const VariableLengthData& tag)
    throw () = 0;

  // 7.15
  virtual void confirmAttributeOwnershipAcquisitionCancellation(ObjectInstanceHandle objectInstanceHandle, const AttributeHandleVector& attributeHandleVector)
    throw () = 0;

  // 7.17
  virtual void informAttributeOwnership(ObjectInstanceHandle objectInstanceHandle, AttributeHandle attributeHandle, FederateHandle theOwner)
    throw () = 0;

  virtual void attributeIsNotOwned(ObjectInstanceHandle objectInstanceHandle, AttributeHandle attributeHandle)
    throw () = 0;

  virtual void attributeIsOwnedByRTI(ObjectInstanceHandle objectInstanceHandle, AttributeHandle attributeHandle)
    throw () = 0;

  virtual void timeRegulationEnabled(const NativeLogicalTime& logicalTime)
    throw () = 0;

  virtual void timeConstrainedEnabled(const NativeLogicalTime& logicalTime)
    throw () = 0;

  virtual void timeAdvanceGrant(const NativeLogicalTime& logicalTime)
    throw () = 0;

  virtual void requestRetraction(MessageRetractionHandle messageRetractionHandle)
    throw () = 0;

  // returns true if this is a valid interaction class
  bool isValidInteractionClass(const InteractionClassHandle& interactionClassHandle) const
  { return interactionClassHandle.getHandle() < _interactionClassVector.size() && _interactionClassVector[interactionClassHandle.getHandle()].valid(); }
  // returns true if this is a valid object class
  bool isValidObjectClass(const ObjectClassHandle& objectClassHandle) const
  { return objectClassHandle.getHandle() < _objectClassVector.size() && _objectClassVector[objectClassHandle.getHandle()].valid(); }

  InteractionClass& getInteractionClass(const InteractionClassHandle& interactionClassHandle)
  { return *_interactionClassVector[interactionClassHandle.getHandle()]; }
  const InteractionClass& getInteractionClass(const InteractionClassHandle& interactionClassHandle) const
  { return *_interactionClassVector[interactionClassHandle.getHandle()]; }

  ObjectClass& getObjectClass(const ObjectClassHandle& objectClassHandle)
  { return *_objectClassVector[objectClassHandle.getHandle()]; }
  const ObjectClass& getObjectClass(const ObjectClassHandle& objectClassHandle) const
  { return *_objectClassVector[objectClassHandle.getHandle()]; }

  bool isValidObjectInstance(const ObjectInstanceHandle& objectInstanceHandle) const
  { return _objectInstanceHandleMap.find(objectInstanceHandle) != _objectInstanceHandleMap.end(); }

  void insertObjectInstance(ObjectInstanceHandle objectInstanceHandle, const std::string& name, ObjectClassHandle objectClassHandle, bool owned)
  {
    typename NameObjectInstanceHandleMap::iterator i;
    i = _nameObjectInstanceHandleMap.insert(typename NameObjectInstanceHandleMap::value_type(name, objectInstanceHandle)).first;
    typename ObjectInstanceHandleMap::iterator j;
    j = _objectInstanceHandleMap.insert(typename ObjectInstanceHandleMap::value_type(objectInstanceHandle, ObjectInstance(i))).first;
    j->second._objectClassHandle = objectClassHandle;
    const ObjectClass& objectClass = getObjectClass(objectClassHandle);
    const AttributeVector& attributeVector = objectClass._attributeVector;
    size_t numAttributes = attributeVector.size();
    j->second._instanceAttributeVector.reserve(numAttributes);
    for (size_t k = 0; k < numAttributes; ++k) {
      if (attributeVector[k].valid()) {
        if (objectClass.isAttributePublished(AttributeHandle(k)))
          j->second._instanceAttributeVector.push_back(new InstanceAttribute(owned, attributeVector[k]->_orderType, attributeVector[k]->_transportationType));
        else
          j->second._instanceAttributeVector.push_back(new InstanceAttribute(false, attributeVector[k]->_orderType, attributeVector[k]->_transportationType));
      } else {
        j->second._instanceAttributeVector.push_back(0);
      }
    }
  }
  void eraseObjectInstance(const ObjectInstanceHandle& objectInstanceHandle)
  {
    typename ObjectInstanceHandleMap::iterator i = _objectInstanceHandleMap.find(objectInstanceHandle);
    if (i == _objectInstanceHandleMap.end()) {
      Log(FederateAmbassador, Warning) << "Federate: \"" << getFederateType() << "\": Cannot remove object instance with object handle: "
                                       << objectInstanceHandle.getHandle() << std::endl;
      return;
    }
    _nameObjectInstanceHandleMap.erase(i->second._nameObjectInstanceHandleMapIterator);
    _objectInstanceHandleMap.erase(i);

    // Unreference the object instance handle resource
    SharedPtr<ReleaseMultipleObjectInstanceNameHandlePairsMessage> message;
    message = new ReleaseMultipleObjectInstanceNameHandlePairsMessage;
    message->setFederationHandle(getFederationHandle());
    message->getObjectInstanceHandleVector().push_back(objectInstanceHandle);
    sendMessage(message);
  }

  void insertOrderType(const std::string& name, OrderType orderType)
  {
    OpenRTIAssert(_nameOrderTypeMap.find(name) == _nameOrderTypeMap.end());
    OpenRTIAssert(_orderTypeNameMap.find(orderType) == _orderTypeNameMap.end());
    _nameOrderTypeMap[name] = orderType;
    _orderTypeNameMap[orderType] = name;
  }
  void insertTransportationType(const std::string& name, TransportationType transportationType)
  {
    OpenRTIAssert(_nameTransportationTypeMap.find(name) == _nameTransportationTypeMap.end());
    OpenRTIAssert(_transportationTypeNameMap.find(transportationType) == _transportationTypeNameMap.end());
    _nameTransportationTypeMap[name] = transportationType;
    _transportationTypeNameMap[transportationType] = name;
  }
  // Insert a new dimension into the local object model
  void insertDimension(const std::string& name, const DimensionHandle& dimensionHandle, Unsigned upperBound)
  {
    OpenRTIAssert(_nameDimensionHandleMap.find(name) == _nameDimensionHandleMap.end());
    size_t index = dimensionHandle.getHandle();
    if (_dimensionVector.size() <= index)
      _dimensionVector.resize(index + 1);
    _dimensionVector[index] = new Dimension(name, upperBound);
    _nameDimensionHandleMap[name] = dimensionHandle;
  }
  void insertRoutingSpace(const std::string& name, const SpaceHandle& spaceHandle, const DimensionHandleSet& dimensionHandles)
  {
    // FIXME implement for the HLA13 stuff, when the parser is there :)
  }
  void insertInteractionClass(const FOMInteractionClass& module)
  {
    size_t index = module.getInteractionClassHandle().getHandle();
    if (_interactionClassVector.size() <= index)
      _interactionClassVector.resize(index + 1);
    _interactionClassVector[index] = new InteractionClass;

    InteractionClassHandle parentHandle = module.getParentInteractionClassHandle();

    std::string parentFQN;
    if (parentHandle.valid()) {
      parentFQN = getInteractionClass(parentHandle)._fqnName + ".";
    }

    std::string fqn = parentFQN + module.getName();
    _nameInteractionClassHandleMap[module.getName()] = module.getInteractionClassHandle();
    _nameInteractionClassHandleMap[fqn] = module.getInteractionClassHandle();
    fqn.push_back('.');

    InteractionClass& interactionClass = *_interactionClassVector[index];
    interactionClass._name = module.getName();
    interactionClass._parentInteractionClassHandle = parentHandle;
    interactionClass._orderType = module.getOrderType();
    interactionClass._transportationType = module.getTransportationType();
    interactionClass._dimensionHandleSet = module.getDimensionHandleSet();

    if (parentHandle.valid()) {
      InteractionClass& parentClass = getInteractionClass(parentHandle);
      interactionClass._parameterVector = parentClass._parameterVector;
      interactionClass._nameParameterHandleMap = parentClass._nameParameterHandleMap;
    }
    for (FOMParameterList::const_iterator i = module.getParameterList().begin();
         i != module.getParameterList().end(); ++i) {
      interactionClass._nameParameterHandleMap[i->getName()] = i->getParameterHandle();
      interactionClass._nameParameterHandleMap[fqn + i->getName()] = i->getParameterHandle();
      interactionClass._parameterVector.push_back(new Parameter(i->getName()));
    }
  }
  void insertObjectClass(const FOMObjectClass& module)
  {
    size_t index = module.getObjectClassHandle().getHandle();
    if (_objectClassVector.size() <= index)
      _objectClassVector.resize(index + 1);
    _objectClassVector[index] = new ObjectClass;

    ObjectClassHandle parentHandle = module.getParentObjectClassHandle();

    std::string parentFQN;
    if (parentHandle.valid()) {
      parentFQN = getObjectClass(parentHandle)._fqnName + ".";
    }

    std::string fqn = parentFQN + module.getName();
    _nameObjectClassHandleMap[module.getName()] = module.getObjectClassHandle();
    _nameObjectClassHandleMap[fqn] = module.getObjectClassHandle();
    fqn.push_back('.');

    ObjectClass& objectClass = *_objectClassVector[index];
    objectClass._name = module.getName();
    objectClass._parentObjectClassHandle = parentHandle;

    if (parentHandle.valid()) {
      ObjectClass& parentClass = getObjectClass(parentHandle);
      objectClass._attributeVector.reserve(parentClass._attributeVector.size());
      for (typename AttributeVector::const_iterator i = parentClass._attributeVector.begin(); i != parentClass._attributeVector.end(); ++i) {
        if (i->valid()) {
          objectClass._attributeVector.push_back(new Attribute(**i));
        } else {
          objectClass._attributeVector.push_back(0);
        }
      }
      objectClass._nameAttributeHandleMap = parentClass._nameAttributeHandleMap;
    }
    for (FOMAttributeList::const_iterator i = module.getAttributeList().begin();
         i != module.getAttributeList().end(); ++i) {
      objectClass._nameAttributeHandleMap[i->getName()] = i->getAttributeHandle();
      objectClass._nameAttributeHandleMap[fqn + i->getName()] = i->getAttributeHandle();
      objectClass._attributeVector.push_back(new Attribute());
      objectClass._attributeVector.back()->_name = i->getName();
      objectClass._attributeVector.back()->_orderType = i->getOrderType();
      objectClass._attributeVector.back()->_transportationType = i->getTransportationType();
      objectClass._attributeVector.back()->_dimensionHandleSet = i->getDimensionHandleSet();
    }
  }
  // Insert a FOM module into the local object model
  void insert(const FOMModule& module)
  {
    for (FOMDimensionList::const_iterator i = module.getDimensionList().begin();
         i != module.getDimensionList().end(); ++i)
      insertDimension(i->getName(), i->getDimensionHandle(), i->getUpperBound());
    for (FOMRoutingSpaceList::const_iterator i = module.getRoutingSpaceList().begin();
         i != module.getRoutingSpaceList().end(); ++i)
      insertRoutingSpace(i->getName(), i->getSpaceHandle(), i->getDimensionHandleSet());
    for (FOMTransportationTypeList::const_iterator i = module.getTransportationTypeList().begin();
         i != module.getTransportationTypeList().end(); ++i)
      insertTransportationType(i->getName(), i->getTransportationType());
    for (FOMInteractionClassList::const_iterator i = module.getInteractionClassList().begin();
         i != module.getInteractionClassList().end(); ++i)
      insertInteractionClass(*i);
    for (FOMObjectClassList::const_iterator i = module.getObjectClassList().begin();
         i != module.getObjectClassList().end(); ++i)
      insertObjectClass(*i);
  }
private:
  // The connect to our next server
  SharedPtr<AbstractConnect> _federationConnect;

  std::string _federateType;
  std::string _federateName;
  FederateHandle _federateHandle;
  FederationHandle _federationHandle;

  DimensionVector _dimensionVector;
  NameDimensionHandleMap _nameDimensionHandleMap;

  RegionHandleRegionDataMap _regionHandleRegionDataMap;

  InteractionClassVector _interactionClassVector;
  NameInteractionClassHandleMap _nameInteractionClassHandleMap;

  ObjectClassVector _objectClassVector;
  NameObjectClassHandleMap _nameObjectClassHandleMap;

  ObjectInstanceHandleMap _objectInstanceHandleMap;
  NameObjectInstanceHandleMap _nameObjectInstanceHandleMap;

  NameTransportationTypeMap _nameTransportationTypeMap;
  TransportationTypeNameMap _transportationTypeNameMap;

  NameOrderTypeMap _nameOrderTypeMap;
  OrderTypeNameMap _orderTypeNameMap;

  // Allocator for our local defined regions
  HandleAllocator<LocalRegionHandle> _localRegionHandleAllocator;

  // The federate handles that are currently joined
  FederateHandleSet _federateHandleSet;

  // The synchronization lables that are currently announced
  StringSet _announcedFederationSynchonizationLabels;

  // The object instance names and handles that are reserved for this federate
  NameObjectInstanceHandleMap _reservedNameObjectInstanceHandlePairs;
  // Our set of already allocated object instance handles including automatically generated names
  NameObjectInstanceHandleMap _privateNameObjectInstanceHandlePairs;

  // What happens when we just delete the ambassador ...
  ResignAction _defaultResignAction;

  // The name we are waiting for in synchronous name reservation and the reserved instance handle
  ObjectInstanceHandleNamePair _syncronousReservationHandleNamePair;

  bool _objectClassRelevanceAdvisorySwitchEnabled;
  bool _attributeRelevanceAdvisorySwitchEnabled;
  bool _attributeScopeAdvisorySwitchEnabled;
  bool _interactionRelevanceAdvisorySwitchEnabled;

  // The current logical time of this federate
  LogicalTime _logicalTime;
  // If a time advance is pending, this contains the logical time of the advance call.
  LogicalTimePair _pendingLogicalTime;
  bool _nextMessageMode;
  // The smallest allowed logical time for sending messages
  LogicalTimePair _localLowerBoundTimeStamp;
  // The lookahead of this federate
  LogicalTimeInterval _currentLookahead;
  LogicalTimeInterval _targetLookahead;
  // State values
  bool _timeRegulationEnablePending;
  bool _timeRegulationEnabled;
  bool _timeConstrainedEnablePending;
  bool _timeConstrainedEnabled;
  bool _timeAdvancePending;
  bool _flushQueueMode;
  bool _permitTimeRegulation;
  bool _asynchronousDeliveryEnabled;
  // Serial number for message retraction
  uint32_t _messageRetractionSerial;

  // When we are in time regulation enable pending state, this contains the federates we need to
  // wait for to complete time regulation enabled
  FederateHandleSet _timeRegulationEnableFederateHandleSet;
  FederateHandleTimeStampMap _timeRegulationEnableFederateHandleTimeStampMap;

  // map containing all the committed logical times of all known time regulating federates
  typedef std::map<LogicalTimePair, FederateHandleSet> LogicalTimeFederateHandleSetMap;
  LogicalTimeFederateHandleSetMap _logicalTimeFederateHandleSetMap;
  typedef std::map<FederateHandle, typename LogicalTimeFederateHandleSetMap::iterator> FederateHandleLogicalTimeMap;
  FederateHandleLogicalTimeMap _federateHandleLogicalTimeMap;

  // The logical time factory required to do our job
  LogicalTimeFactory _logicalTimeFactory;

  // Here the ready to process callbacks are queued
  MessageList2 _callbackMessageQueue;

  // The timestamped queued messages
  typedef std::map<LogicalTime, MessageList2> LogicalTimeMessageListMap;
  LogicalTimeMessageListMap _logicalTimeMessageListMap;
  MessageList2 _receiveOrderMessages;
};

} // namespace OpenRTI

#endif
