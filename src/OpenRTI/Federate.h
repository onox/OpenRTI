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

#ifndef OpenRTI_Federate_h
#define OpenRTI_Federate_h

#include "Export.h"
#include "IntrusiveList.h"
#include "HandleAllocator.h"
#include "LogStream.h"
#include "Message.h"
#include "Referenced.h"
#include "Region.h"

namespace OpenRTI {

class InternalAmbassador;

class OPENRTI_API Federate : public Referenced {
public:
  Federate();
  virtual ~Federate();

  /// Federate specific members
  const FederateHandle& getFederateHandle() const
  { return _federateHandle; }
  void setFederateHandle(const FederateHandle& federateHandle);

  const std::string& getFederateType() const
  { return _federateType; }
  void setFederateType(const std::string& federateType);

  const std::string& getFederateName() const
  { return _federateName; }
  void setFederateName(const std::string& federateName);

  /// Federation specific members
  const FederationHandle& getFederationHandle() const
  { return _federationHandle; }
  void setFederationHandle(const FederationHandle& federationHandle);

  const std::string& getLogicalTimeFactoryName() const
  { return _logicalTimeFactoryName; }
  void setLogicalTimeFactoryName(const std::string& logicalTimeFactoryName);

  ResignAction getAutomaticResignDirective() const
  { return _automaticResignDirective; }
  void setAutomaticResignDirective(ResignAction resignAction);

  bool getObjectClassRelevanceAdvisorySwitchEnabled() const
  { return _objectClassRelevanceAdvisorySwitchEnabled; }
  void setObjectClassRelevanceAdvisorySwitchEnabled(bool objectClassRelevanceAdvisorySwitchEnabled);

  bool getAttributeRelevanceAdvisorySwitchEnabled() const
  { return _attributeRelevanceAdvisorySwitchEnabled; }
  void setAttributeRelevanceAdvisorySwitchEnabled(bool attributeRelevanceAdvisorySwitchEnabled);

  bool getAttributeScopeAdvisorySwitchEnabled() const
  { return _attributeScopeAdvisorySwitchEnabled; }
  void setAttributeScopeAdvisorySwitchEnabled(bool attributeScopeAdvisorySwitchEnabled);

  bool getInteractionRelevanceAdvisorySwitchEnabled() const
  { return _interactionRelevanceAdvisorySwitchEnabled; }
  void setInteractionRelevanceAdvisorySwitchEnabled(bool interactionRelevanceAdvisorySwitchEnabled);

  bool getPermitTimeRegulation() const
  { return _permitTimeRegulation; }
  void setPermitTimeRegulation(bool permitTimeRegulation);

  void applySwitch(const FOMSwitch& switchValue);

  /// rti1516e update rates.
  double getUpdateRateValue(const std::string& name) const;
  void insertUpdateRate(const std::string& name, double updateRate);

  /// TransportationTypes
  const TransportationType* getTransportationType(const std::string& name) const;
  const std::string* getTransportationName(TransportationType transportationType) const;
  void insertTransportationType(const std::string& name, TransportationType transportationType);

  /// OrderTypes
  const OrderType* getOrderType(const std::string& name) const;
  const std::string* getOrderName(OrderType orderType) const;
  void insertOrderType(const std::string& name, OrderType orderType);

  /// Dimensions
  class OPENRTI_API Dimension : public Referenced {
  public:
    Dimension(const std::string& name, unsigned long upperBound);
    ~Dimension();

    const std::string& getName() const
    { return _name; }
    unsigned long getUpperBound() const
    { return _upperBound; }

  private:
    Dimension(const Dimension&);
    Dimension& operator=(const Dimension&);

    std::string _name;
    unsigned long _upperBound;
  };
  typedef std::vector<SharedPtr<Dimension> > DimensionVector;

  const Dimension* getDimension(const DimensionHandle& dimensionHandle) const;
  DimensionHandle getDimensionHandle(const std::string& name) const;
  void insertDimension(const std::string& name, const DimensionHandle& dimensionHandle, Unsigned upperBound);


  /// Routing space??! RTI13 thing?!
  void insertRoutingSpace(const std::string& name, const SpaceHandle& spaceHandle, const DimensionHandleSet& dimensionHandles);


  /// Regions
  class OPENRTI_API RegionData : public Referenced {
  public:
    RegionData(const DimensionHandleSet& dimensionHandleSet);
    ~RegionData();

    const DimensionHandleSet& getDimensionHandleSet() const
    { return _dimensionHandleSet; }
    bool containsDimensionHandle(const DimensionHandle& dimensionHandle) const;

    const Region& getRegion() const
    { return _region; }
    Region& getRegion()
    { return _region; }
    void setRegion(const Region& region);

  private:
    RegionData(const RegionData&);
    RegionData& operator=(const RegionData&);

    DimensionHandleSet _dimensionHandleSet;
    Region _region;
  };

  RegionData* getRegion(const RegionHandle& regionHandle);
  const RegionData* getRegion(const RegionHandle& regionHandle) const;
  void insertRegion(const RegionHandle& regionHandle, const DimensionHandleSet& dimensionHandleSet);
  void insertRegions(const RegionHandleDimensionHandleSetPairVector& regionHandleDimensionHandleSetPairVector);
  void eraseRegion(const RegionHandle& regionHandle);
  void eraseRegions(const RegionHandleVector& regionHandleVector);
  RegionHandle insertLocalRegion(const DimensionHandleSet& dimensionHandleSet);
  void eraseLocalRegion(const RegionHandle& regionHandle);
  RegionHandleVector getLocalRegionHandles() const;
  void commitRegion(const RegionHandle& regionHandle, const RegionValue& regionValue);
  void commitRegions(const RegionHandleRegionValuePairVector& regionHandleRegionValuePairVector);


  /// The object and interaction classes

  struct OPENRTI_API PublishSubscribe : public Referenced {
    PublishSubscribe();
    ~PublishSubscribe();

    const std::string& getName() const
    { return _name; }
    void setName(const std::string& name);

    OrderType getOrderType() const
    { return _orderType; }
    void setOrderType(OrderType orderType);

    TransportationType getTransportationType() const
    { return _transportationType; }
    void setTransportationType(TransportationType transportationType);

    SubscriptionType getSubscriptionType() const
    { return _subscriptionType; }
    bool setSubscriptionType(SubscriptionType subscriptionType);
    bool isSubscribed() const
    { return Unsubscribed != _subscriptionType; }

    PublicationType getPublicationType() const
    { return _publicationType; }
    bool setPublicationType(PublicationType publicationType);
    bool isPublished() const
    { return Unpublished != _publicationType; }

    const DimensionHandleSet& getDimensionHandleSet() const
    { return _dimensionHandleSet; }
    void setDimensionHandleSet(const DimensionHandleSet& dimensionHandleSet);

  private:
    // PublishSubscribe(const PublishSubscribe&);
    // PublishSubscribe& operator=(const PublishSubscribe&);

    std::string _name;

    OrderType _orderType;
    TransportationType _transportationType;

    SubscriptionType _subscriptionType;
    PublicationType _publicationType;

    DimensionHandleSet _dimensionHandleSet;
  };


  /// Interaction Classes
  struct OPENRTI_API Parameter : public Referenced {
    Parameter();
    ~Parameter();

    const std::string& getName() const
    { return _name; }
    void setName(const std::string& name);

  private:
    Parameter(const Parameter&);
    Parameter& operator=(const Parameter&);

    std::string _name;
  };
  typedef std::vector<SharedPtr<Parameter> > ParameterVector;

  struct InteractionClass;
  typedef IntrusiveList<InteractionClass> ChildInteractionClassList;

  struct OPENRTI_API InteractionClass : public PublishSubscribe, public ChildInteractionClassList::Hook {
    InteractionClass();
    ~InteractionClass();

    const std::string& getFQName() const
    { return _fqName; }
    void setFQName(const std::string& fqName);

    const InteractionClassHandle& getParentInteractionClassHandle() const
    { return _parentInteractionClassHandle; }
    void setParentInteractionClassHandle(const InteractionClassHandle& parentInteractionClassHandle);

    const Parameter* getParameter(const ParameterHandle& parameterHandle) const;
    Parameter* getParameter(const ParameterHandle& parameterHandle);
    ParameterHandle getParameterHandle(const std::string& name) const;
    void insertParameter(const FOMParameter& fomParameter);
    void insertChildInteractionClass(InteractionClass& interactionClass);

  private:
    InteractionClass(const InteractionClass&);
    InteractionClass& operator=(const InteractionClass&);

    std::string _fqName;
    InteractionClassHandle _parentInteractionClassHandle;

    ParameterVector _parameterVector;
    typedef std::map<std::string, ParameterHandle> NameParameterHandleMap;
    NameParameterHandleMap _nameParameterHandleMap;

    ChildInteractionClassList _childInteractionClassList;
  };
  typedef std::vector<SharedPtr<InteractionClass> > InteractionClassVector;

  InteractionClass* getInteractionClass(const InteractionClassHandle& interactionClassHandle);
  const InteractionClass* getInteractionClass(const InteractionClassHandle& interactionClassHandle) const;
  InteractionClassHandle getInteractionClassHandle(const std::string& name) const;
  void insertInteractionClass(const FOMInteractionClass& module, bool artificialObjectRoot);
  size_t getNumInteractionClasses() const
  { return _interactionClassVector.size(); }

  /// Object Classes
  struct OPENRTI_API Attribute : public PublishSubscribe {
  // private:
  //   Attribute(const Attribute&);
  //   Attribute& operator=(const Attribute&);
  };
  typedef std::vector<SharedPtr<Attribute> > AttributeVector;

  struct ObjectClass;
  typedef IntrusiveList<ObjectClass> ChildObjectClassList;

  struct OPENRTI_API ObjectClass : public Referenced, public ChildObjectClassList::Hook {
    ObjectClass();
    ~ObjectClass();

    const std::string& getName() const
    { return _name; }
    void setName(const std::string& name);

    const std::string& getFQName() const
    { return _fqName; }
    void setFQName(const std::string& fqName);

    const ObjectClassHandle& getParentObjectClassHandle() const
    { return _parentObjectClassHandle; }
    void setParentObjectClassHandle(const ObjectClassHandle& parentObjectClassHandle);

    const Attribute* getAttribute(const AttributeHandle& attributeHandle) const;
    Attribute* getAttribute(const AttributeHandle& attributeHandle);
    AttributeHandle getAttributeHandle(const std::string& name) const;
    size_t getNumAttributes() const
    { return _attributeVector.size(); }
    void insertAttribute(const FOMAttribute& fomAttribute);
    void insertChildObjectClass(ObjectClass& objectClass);

    // Returns true if the attribute is subscribed.
    bool isAttributeSubscribed(const AttributeHandle& attributeHandle) const;
    SubscriptionType getEffectiveAttributeSubscriptionType(const AttributeHandle& attributeHandle) const;
    SubscriptionType getAttributeSubscriptionType(const AttributeHandle& attributeHandle) const;
    // Sets the local subscription status.
    bool setAttributeSubscriptionType(const AttributeHandle& attributeHandle, SubscriptionType subscriptionType);

    SubscriptionType getEffectiveSubscriptionType() const;
    SubscriptionType getSubscriptionType() const
    { return _subscriptionType; }
    bool setSubscriptionType(SubscriptionType subscriptionType);

    // Returns true if the attribute is published.
    bool isAttributePublished(const AttributeHandle& attributeHandle) const;
    PublicationType getEffectiveAttributePublicationType(const AttributeHandle& attributeHandle) const;
    PublicationType getAttributePublicationType(const AttributeHandle& attributeHandle) const;
    // Sets the local publish status. Returns true if the publish type changed with this call.
    bool setAttributePublicationType(const AttributeHandle& attributeHandle, PublicationType publicationType);

    bool isPublished() const;
    PublicationType getEffectivePublicationType() const;
    PublicationType getPublicationType() const
    { return _publicationType; }
    bool setPublicationType(PublicationType publicationType);

  private:
    ObjectClass(const ObjectClass&);
    ObjectClass& operator=(const ObjectClass&);

    std::string _name;
    std::string _fqName;
    ObjectClassHandle _parentObjectClassHandle;

    AttributeVector _attributeVector;
    typedef std::map<std::string, AttributeHandle> NameAttributeHandleMap;
    NameAttributeHandleMap _nameAttributeHandleMap;

    // Because of implicit publication/subscription
    SubscriptionType _subscriptionType;
    PublicationType _publicationType;

    ChildObjectClassList _childObjectClassList;
  };
  typedef std::vector<SharedPtr<ObjectClass> > ObjectClassVector;

  ObjectClass* getObjectClass(const ObjectClassHandle& objectClassHandle);
  const ObjectClass* getObjectClass(const ObjectClassHandle& objectClassHandle) const;
  ObjectClassHandle getObjectClassHandle(const std::string& name) const;
  void insertObjectClass(const FOMObjectClass& module, bool artificialObjectRoot);
  size_t getNumObjectClasses() const
  { return _objectClassVector.size(); }


  /// Object Instances
  struct OPENRTI_API InstanceAttribute : public Referenced {
    InstanceAttribute(const Federate::Attribute& attribute, bool isOwnedByFederate);
    ~InstanceAttribute();

    OrderType getOrderType() const
    { return _orderType; }
    void setOrderType(OrderType orderType);

    TransportationType getTransportationType() const
    { return _transportationType; }
    void setTransportationType(TransportationType transportationType);

    bool getIsOwnedByFederate() const
    { return _isOwnedByFederate; }
    void setIsOwnedByFederate(bool isOwnedByFederate);

    double getUpdateRate() const
    { return _updateRate; }
    void setUpdateRate(double updateRate);

  private:
    InstanceAttribute(const InstanceAttribute&);
    InstanceAttribute& operator=(const InstanceAttribute&);

    OrderType _orderType;
    TransportationType _transportationType;
    bool _isOwnedByFederate;
    double _updateRate;
  };
  typedef std::vector<SharedPtr<InstanceAttribute> > InstanceAttributeVector;

  typedef std::map<std::string, ObjectInstanceHandle> NameObjectInstanceHandleMap;
  struct OPENRTI_API ObjectInstance : public Referenced {
    ObjectInstance(NameObjectInstanceHandleMap::iterator nameObjectInstanceHandleMapIterator, const ObjectClass& objectClass, bool owned);
    ~ObjectInstance();

    const std::string& getName() const;

    // This is the known object class of this instance, which might be something less than the original one
    const ObjectClassHandle& getObjectClassHandle() const
    { return _objectClassHandle; }
    void setObjectClassHandle(const ObjectClassHandle& objectClassHandle);

    const NameObjectInstanceHandleMap::iterator& getNameObjectInstanceHandleMapIterator() const
    { return _nameObjectInstanceHandleMapIterator; }

    const InstanceAttribute* getInstanceAttribute(const AttributeHandle& attributeHandle) const;
    InstanceAttribute* getInstanceAttribute(const AttributeHandle& attributeHandle);

    bool isOwnedByFederate() const;
    bool ownsAnyAttribute() const;

  private:
    ObjectInstance(const ObjectInstance&);
    ObjectInstance& operator=(const ObjectInstance&);

    ObjectClassHandle _objectClassHandle;
    NameObjectInstanceHandleMap::iterator _nameObjectInstanceHandleMapIterator;
    InstanceAttributeVector _instanceAttributeVector;
  };
  typedef std::map<ObjectInstanceHandle, SharedPtr<ObjectInstance> > ObjectInstanceHandleMap;

  typedef std::map<std::string, FederateHandle> NameFederateHandleMap;
  struct OPENRTI_API _Federate : public Referenced {
    _Federate(NameFederateHandleMap::iterator nameFederateHandleMapIterator);
    ~_Federate();

    const std::string& getName() const;

    const NameFederateHandleMap::iterator& getNameFederateHandleMapIterator() const
    { return _nameFederateHandleMapIterator; }

  private:
    _Federate(const _Federate&);
    _Federate& operator=(const _Federate&);

    NameFederateHandleMap::iterator _nameFederateHandleMapIterator;
  };
  typedef std::map<FederateHandle, SharedPtr<_Federate> > FederateHandleMap;

  ObjectInstance* getObjectInstance(const ObjectInstanceHandle& objectInstanceHandle);
  const ObjectInstance* getObjectInstance(const ObjectInstanceHandle& objectInstanceHandle) const;
  ObjectInstanceHandle getObjectInstanceHandle(const std::string& name) const;
  void insertObjectInstance(ObjectInstanceHandle objectInstanceHandle, const std::string& name, ObjectClassHandle objectClassHandle, bool owned);
  void eraseObjectInstance(const ObjectInstanceHandle& objectInstanceHandle);
  const ObjectInstanceHandleMap& getObjectInstanceHandleMap() const
  { return _objectInstanceHandleMap; }

  /// The pool of preallocated ObjectInstanceHandle - object names
  void insertObjectInstanceHandleNamePairs(const ObjectInstanceHandleNamePairVector& objectInstanceHandleNamePairVector);
  bool haveFreeObjectInstanceHandleNamePair() const;
  ObjectInstanceHandleNamePair takeFreeObjectInstanceHandleNamePair();

  /// Explicitly reserved object names, the instance handles for them are allocated already
  void insertReservedObjectInstanceHandleNamePair(const ObjectInstanceHandleNamePair& objectInstanceHandleNamePair);
  bool objectInstanceNameReserved(const std::string& objectInstanceName) const;
  ObjectInstanceHandle takeReservedObjectInstanceName(const std::string& objectInstanceName);

  /// The federates in the federation we know about
  _Federate* getFederate(const FederateHandle& federateHandle);
  const _Federate* getFederate(const FederateHandle& federateHandle) const;
  FederateHandle getFederateHandle(const std::string& name) const;
  void insertFederate(const FederateHandle& federateHandle, const std::string& name);
  void eraseFederate(const FederateHandle& federateHandle);
  void getFederateHandleSet(FederateHandleSet& federateHandleSet) const
  {
    for (FederateHandleMap::const_iterator i = _federateHandleMap.begin(); i != _federateHandleMap.end(); ++i)
      federateHandleSet.insert(i->first);
  }

  /// The Synchronization Labels
  void insertAnnouncedFederationSynchonizationLabel(const std::string& label);
  void eraseAnnouncedFederationSynchonizationLabel(const std::string& label);
  bool synchronizationLabelAnnounced(const std::string& label) const;

  /// On initialization we need to process object model data.
  /// We need only write access to this.
  void insertFOMModule(const FOMModule& module);
  void insertFOMModuleList(const FOMModuleList& moduleList);

private:
  Federate(const Federate&);
  Federate& operator=(const Federate&);

  std::string _federateType;
  std::string _federateName;
  FederateHandle _federateHandle;

  FederationHandle _federationHandle;

  std::string _logicalTimeFactoryName;

  // What happens when we just delete the ambassador ...
  ResignAction _automaticResignDirective;

  bool _objectClassRelevanceAdvisorySwitchEnabled;
  bool _attributeRelevanceAdvisorySwitchEnabled;
  bool _attributeScopeAdvisorySwitchEnabled;
  bool _interactionRelevanceAdvisorySwitchEnabled;

  bool _permitTimeRegulation;

  typedef std::map<std::string, double> NameUpdateRateMap;
  NameUpdateRateMap _nameUpdateRateMap;

  typedef std::map<std::string, TransportationType> NameTransportationTypeMap;
  NameTransportationTypeMap _nameTransportationTypeMap;
  typedef std::map<TransportationType, std::string> TransportationTypeNameMap;
  TransportationTypeNameMap _transportationTypeNameMap;

  typedef std::map<std::string, OrderType> NameOrderTypeMap;
  NameOrderTypeMap _nameOrderTypeMap;
  typedef std::map<OrderType, std::string> OrderTypeNameMap;
  OrderTypeNameMap _orderTypeNameMap;

  DimensionVector _dimensionVector;
  typedef std::map<std::string, DimensionHandle> NameDimensionHandleMap;
  NameDimensionHandleMap _nameDimensionHandleMap;

  typedef std::map<RegionHandle, SharedPtr<RegionData> > RegionHandleRegionDataMap;
  RegionHandleRegionDataMap _regionHandleRegionDataMap;
  HandleAllocator<LocalRegionHandle> _localRegionHandleAllocator;

  InteractionClassVector _interactionClassVector;
  typedef std::map<std::string, InteractionClassHandle> NameInteractionClassHandleMap;
  NameInteractionClassHandleMap _nameInteractionClassHandleMap;

  ObjectClassVector _objectClassVector;
  typedef std::map<std::string, ObjectClassHandle> NameObjectClassHandleMap;
  NameObjectClassHandleMap _nameObjectClassHandleMap;

  ObjectInstanceHandleMap _objectInstanceHandleMap;
  NameObjectInstanceHandleMap _nameObjectInstanceHandleMap;

  // Our set of already allocated object instance handles including automatically generated names
  NameObjectInstanceHandleMap _privateNameObjectInstanceHandlePairs;

  // The object instance names and handles that are reserved for this federate
  NameObjectInstanceHandleMap _reservedNameObjectInstanceHandlePairs;

  // The federate handles that are currently joined
  NameFederateHandleMap _nameFederateHandleMap;
  FederateHandleMap _federateHandleMap;

  // The synchronization lables that are currently announced
  StringSet _announcedFederationSynchonizationLabels;
};

} // namespace OpenRTI

#endif
