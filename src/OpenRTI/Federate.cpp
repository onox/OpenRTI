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

#include "Federate.h"

#include "InternalTimeManagement.h"
#include "InternalAmbassador.h"

namespace OpenRTI {

Federate::Dimension::Dimension(const std::string& name, unsigned long upperBound) :
  _name(name),
  _upperBound(upperBound)
{
}

Federate::Dimension::~Dimension()
{
}

Federate::RegionData::RegionData(const DimensionHandleSet& dimensionHandleSet) :
  _dimensionHandleSet(dimensionHandleSet)
{
}

Federate::RegionData::~RegionData()
{
}

bool
Federate::RegionData::containsDimensionHandle(const DimensionHandle& dimensionHandle) const
{
  return _dimensionHandleSet.find(dimensionHandle) != _dimensionHandleSet.end();
}

void
Federate::RegionData::setRegion(const Region& region)
{
  _region = region;
}

Federate::PublishSubscribe::PublishSubscribe() :
  _orderType(RECEIVE),
  _transportationType(RELIABLE),
  _subscriptionType(Unsubscribed),
  _publicationType(Unpublished)
{
}

Federate::PublishSubscribe::~PublishSubscribe()
{
}

void
Federate::PublishSubscribe::setName(const std::string& name)
{
  _name = name;
}

void
Federate::PublishSubscribe::setOrderType(OrderType orderType)
{
  _orderType = orderType;
}

void
Federate::PublishSubscribe::setTransportationType(TransportationType transportationType)
{
  _transportationType = transportationType;
}

bool
Federate::PublishSubscribe::setSubscriptionType(SubscriptionType subscriptionType)
{
  std::swap(_subscriptionType, subscriptionType);
  return _subscriptionType != subscriptionType;
}

bool
Federate::PublishSubscribe::setPublicationType(PublicationType publicationType)
{
  std::swap(_publicationType, publicationType);
  return _publicationType != publicationType;
}

void
Federate::PublishSubscribe::setDimensionHandleSet(const DimensionHandleSet& dimensionHandleSet)
{
  _dimensionHandleSet = dimensionHandleSet;
}

Federate::Parameter::Parameter()
{
}

Federate::Parameter::~Parameter()
{
}

void
Federate::Parameter::setName(const std::string& name)
{
  _name = name;
}

Federate::InteractionClass::InteractionClass()
{
}

Federate::InteractionClass::~InteractionClass()
{
  _childInteractionClassList.unlink();
}

void
Federate::InteractionClass::setFQName(const std::string& fqName)
{
  _fqName = fqName;
}

void
Federate::InteractionClass::setParentInteractionClassHandle(const InteractionClassHandle& parentInteractionClassHandle)
{
  _parentInteractionClassHandle = parentInteractionClassHandle;
}

const Federate::Parameter*
Federate::InteractionClass::getParameter(const ParameterHandle& parameterHandle) const
{
  if (_parameterVector.size() <= parameterHandle.getHandle())
    return 0;
  return _parameterVector[parameterHandle.getHandle()].get();
}

Federate::Parameter*
Federate::InteractionClass::getParameter(const ParameterHandle& parameterHandle)
{
  if (_parameterVector.size() <= parameterHandle.getHandle())
    return 0;
  return _parameterVector[parameterHandle.getHandle()].get();
}

ParameterHandle
Federate::InteractionClass::getParameterHandle(const std::string& name) const
{
  NameParameterHandleMap::const_iterator i = _nameParameterHandleMap.find(name);
  if (i == _nameParameterHandleMap.end())
    return ParameterHandle();
  return i->second;
}

void
Federate::InteractionClass::insertParameter(const FOMParameter& fomParameter)
{
  ParameterHandle parameterHandle = fomParameter.getParameterHandle();
  size_t index = parameterHandle.getHandle();
  if (_parameterVector.size() <= index)
    _parameterVector.resize(index + 1);
  if (_parameterVector[index].valid())
    return;
  _parameterVector[index] = new Parameter;
  Parameter* parameter = _parameterVector[index].get();

  _nameParameterHandleMap[fomParameter.getName()] = parameterHandle;
  _nameParameterHandleMap[_fqName + "." + fomParameter.getName()] = parameterHandle;

  parameter->setName(fomParameter.getName());

  for (IntrusiveList<InteractionClass>::iterator i = _childInteractionClassList.begin();
       i != _childInteractionClassList.end(); ++i) {
    i->insertParameter(fomParameter);
  }
}

void
Federate::InteractionClass::insertChildInteractionClass(InteractionClass& interactionClass)
{
  // Since a parameter only contains a name we can share the parameters of the base interaction class
  interactionClass._parameterVector = _parameterVector;
  interactionClass._nameParameterHandleMap = _nameParameterHandleMap;

  _childInteractionClassList.push_back(interactionClass);
}

Federate::ObjectClass::ObjectClass() :
  _subscriptionType(Unsubscribed),
  _publicationType(Unpublished)
{
}

Federate::ObjectClass::~ObjectClass()
{
  _childObjectClassList.unlink();
}

void
Federate::ObjectClass::setName(const std::string& name)
{
  _name = name;
}

void
Federate::ObjectClass::setFQName(const std::string& fqName)
{
  _fqName = fqName;
}

void
Federate::ObjectClass::setParentObjectClassHandle(const ObjectClassHandle& parentObjectClassHandle)
{
  _parentObjectClassHandle = parentObjectClassHandle;
}

const Federate::Attribute*
Federate::ObjectClass::getAttribute(const AttributeHandle& attributeHandle) const
{
  if (_attributeVector.size() <= attributeHandle.getHandle())
    return 0;
  return _attributeVector[attributeHandle.getHandle()].get();
}

Federate::Attribute*
Federate::ObjectClass::getAttribute(const AttributeHandle& attributeHandle)
{
  if (_attributeVector.size() <= attributeHandle.getHandle())
    return 0;
  return _attributeVector[attributeHandle.getHandle()].get();
}

AttributeHandle
Federate::ObjectClass::getAttributeHandle(const std::string& name) const
{
  NameAttributeHandleMap::const_iterator i = _nameAttributeHandleMap.find(name);
  if (i == _nameAttributeHandleMap.end())
    return AttributeHandle();
  return i->second;
}

void
Federate::ObjectClass::insertAttribute(const FOMAttribute& fomAttribute)
{
  AttributeHandle attributeHandle = fomAttribute.getAttributeHandle();

  size_t index = attributeHandle.getHandle();
  if (_attributeVector.size() <= index)
    _attributeVector.resize(index + 1);
  if (_attributeVector[index].valid())
    return;
  _attributeVector[index] = new Attribute;
  Attribute* attribute = _attributeVector[index].get();

  _nameAttributeHandleMap[fomAttribute.getName()] = attributeHandle;
  _nameAttributeHandleMap[_fqName + "." + fomAttribute.getName()] = attributeHandle;

  attribute->setName(fomAttribute.getName());
  attribute->setOrderType(fomAttribute.getOrderType());
  attribute->setTransportationType(fomAttribute.getTransportationType());
  attribute->setDimensionHandleSet(fomAttribute.getDimensionHandleSet());

  for (IntrusiveList<ObjectClass>::iterator i = _childObjectClassList.begin();
       i != _childObjectClassList.end(); ++i) {
    i->insertAttribute(fomAttribute);
  }
}

void
Federate::ObjectClass::insertChildObjectClass(ObjectClass& objectClass)
{
  objectClass._attributeVector.reserve(_attributeVector.size());
  for (AttributeVector::const_iterator i = _attributeVector.begin();
       i != _attributeVector.end(); ++i) {
    if (i->valid()) {
      objectClass._attributeVector.push_back(new Attribute(**i));
    } else {
      objectClass._attributeVector.push_back(0);
    }
  }
  objectClass._nameAttributeHandleMap = _nameAttributeHandleMap;

  _childObjectClassList.push_back(objectClass);
}

bool
Federate::ObjectClass::isAttributeSubscribed(const AttributeHandle& attributeHandle) const
{
  return Unsubscribed != getEffectiveAttributeSubscriptionType(attributeHandle);
}

SubscriptionType
Federate::ObjectClass::getEffectiveAttributeSubscriptionType(const AttributeHandle& attributeHandle) const
{
  SubscriptionType subscriptionType = _attributeVector[attributeHandle.getHandle()]->getSubscriptionType();
  if (attributeHandle == AttributeHandle(0))
    subscriptionType = std::max(subscriptionType, _subscriptionType);
  return subscriptionType;
}

SubscriptionType
Federate::ObjectClass::getAttributeSubscriptionType(const AttributeHandle& attributeHandle) const
{
  return _attributeVector[attributeHandle.getHandle()]->getSubscriptionType();
}

bool
Federate::ObjectClass::setAttributeSubscriptionType(const AttributeHandle& attributeHandle, SubscriptionType subscriptionType)
{
  SubscriptionType oldSubscriptionType = getEffectiveAttributeSubscriptionType(attributeHandle);
  _attributeVector[attributeHandle.getHandle()]->setSubscriptionType(subscriptionType);
  return oldSubscriptionType != getEffectiveAttributeSubscriptionType(attributeHandle);
}

SubscriptionType
Federate::ObjectClass::getEffectiveSubscriptionType() const
{
  return std::max(_subscriptionType, _attributeVector[0]->getSubscriptionType());
}

bool
Federate::ObjectClass::setSubscriptionType(SubscriptionType subscriptionType)
{
  SubscriptionType oldSubscriptionType = getEffectiveSubscriptionType();
  _subscriptionType = subscriptionType;
  return oldSubscriptionType != getEffectiveSubscriptionType();
}

bool
Federate::ObjectClass::isAttributePublished(const AttributeHandle& attributeHandle) const
{
  return Unpublished != getEffectiveAttributePublicationType(attributeHandle);
}

PublicationType
Federate::ObjectClass::getEffectiveAttributePublicationType(const AttributeHandle& attributeHandle) const
{
  PublicationType publicationType = _attributeVector[attributeHandle.getHandle()]->getPublicationType();
  if (attributeHandle == AttributeHandle(0))
    publicationType = std::max(publicationType, _publicationType);
  return publicationType;
}

PublicationType
Federate::ObjectClass::getAttributePublicationType(const AttributeHandle& attributeHandle) const
{
  return _attributeVector[attributeHandle.getHandle()]->getPublicationType();
}

bool
Federate::ObjectClass::setAttributePublicationType(const AttributeHandle& attributeHandle, PublicationType publicationType)
{
  PublicationType oldPublicationType = getEffectiveAttributePublicationType(attributeHandle);
  _attributeVector[attributeHandle.getHandle()]->setPublicationType(publicationType);
  return oldPublicationType != getEffectiveAttributePublicationType(attributeHandle);
}

bool
Federate::ObjectClass::isPublished() const
{
  return Unpublished != getEffectivePublicationType();
}

PublicationType
Federate::ObjectClass::getEffectivePublicationType() const
{
  return std::max(_publicationType, _attributeVector[0]->getPublicationType());
}

bool
Federate::ObjectClass::setPublicationType(PublicationType publicationType)
{
  PublicationType oldPublicationType = getEffectivePublicationType();
  _publicationType = publicationType;
  return oldPublicationType != getEffectivePublicationType();
}

Federate::InstanceAttribute::InstanceAttribute(const Federate::Attribute& attribute, bool isOwnedByFederate) :
  _orderType(attribute.getOrderType()),
  _transportationType(attribute.getTransportationType()),
  _isOwnedByFederate(isOwnedByFederate),
  _updateRate(0)
{
}

Federate::InstanceAttribute::~InstanceAttribute()
{
}

void
Federate::InstanceAttribute::setOrderType(OrderType orderType)
{
  _orderType = orderType;
}

void
Federate::InstanceAttribute::setTransportationType(TransportationType transportationType)
{
  _transportationType = transportationType;
}

void
Federate::InstanceAttribute::setIsOwnedByFederate(bool isOwnedByFederate)
{
  _isOwnedByFederate = isOwnedByFederate;
}

void
Federate::InstanceAttribute::setUpdateRate(double updateRate)
{
  _updateRate = updateRate;
}

Federate::ObjectInstance::ObjectInstance(NameObjectInstanceHandleMap::iterator nameObjectInstanceHandleMapIterator,
                                         const ObjectClass& objectClass, bool owned) :
  _nameObjectInstanceHandleMapIterator(nameObjectInstanceHandleMapIterator)
{
  size_t numAttributes = objectClass.getNumAttributes();
  _instanceAttributeVector.reserve(numAttributes);
  for (size_t k = 0; k < numAttributes; ++k) {
    const Attribute* attribute = objectClass.getAttribute(k);
    if (attribute) {
      if (objectClass.isAttributePublished(AttributeHandle(k)))
        _instanceAttributeVector.push_back(new InstanceAttribute(*attribute, owned));
      else
        _instanceAttributeVector.push_back(new InstanceAttribute(*attribute, false));
    } else {
      _instanceAttributeVector.push_back(0);
    }
  }
}

Federate::ObjectInstance::~ObjectInstance()
{
}

const std::string&
Federate::ObjectInstance::getName() const
{
  return _nameObjectInstanceHandleMapIterator->first;
}

void
Federate::ObjectInstance::setObjectClassHandle(const ObjectClassHandle& objectClassHandle)
{
  _objectClassHandle = objectClassHandle;
}

const Federate::InstanceAttribute*
Federate::ObjectInstance::getInstanceAttribute(const AttributeHandle& attributeHandle) const
{
  if (_instanceAttributeVector.size() <= attributeHandle.getHandle())
    return 0;
  return _instanceAttributeVector[attributeHandle.getHandle()].get();
}

Federate::InstanceAttribute*
Federate::ObjectInstance::getInstanceAttribute(const AttributeHandle& attributeHandle)
{
  if (_instanceAttributeVector.size() <= attributeHandle.getHandle())
    return 0;
  return _instanceAttributeVector[attributeHandle.getHandle()].get();
}

bool
Federate::ObjectInstance::isOwnedByFederate() const
{
  const InstanceAttribute* instanceAttribute = getInstanceAttribute(AttributeHandle(0));
  if (!instanceAttribute)
    return false;
  return instanceAttribute->getIsOwnedByFederate();
}

bool
Federate::ObjectInstance::ownsAnyAttribute() const
{
  for (size_t i = 0; i < _instanceAttributeVector.size(); ++i) {
    const InstanceAttribute* instanceAttribute = getInstanceAttribute(AttributeHandle(i));
    if (!instanceAttribute)
      continue;
    if (instanceAttribute->getIsOwnedByFederate())
      return true;
  }
  return false;
}

Federate::_Federate::_Federate(NameFederateHandleMap::iterator nameFederateHandleMapIterator) :
  _nameFederateHandleMapIterator(nameFederateHandleMapIterator)
{
}

Federate::_Federate::~_Federate()
{
}

const std::string&
Federate::_Federate::getName() const
{
  return _nameFederateHandleMapIterator->first;
}

Federate::Federate() :
  _automaticResignDirective(CANCEL_THEN_DELETE_THEN_DIVEST),
  _objectClassRelevanceAdvisorySwitchEnabled(false),
  _attributeRelevanceAdvisorySwitchEnabled(false),
  _attributeScopeAdvisorySwitchEnabled(false),
  _interactionRelevanceAdvisorySwitchEnabled(false),
  _permitTimeRegulation(true)
{
}

Federate::~Federate()
{
}

void
Federate::setFederateHandle(const FederateHandle& federateHandle)
{
  _federateHandle = federateHandle;
}

void
Federate::setFederateType(const std::string& federateType)
{
  _federateType = federateType;
}

void
Federate::setFederateName(const std::string& federateName)
{
  _federateName = federateName;
}

void
Federate::setFederationHandle(const FederationHandle& federationHandle)
{
  _federationHandle = federationHandle;
}

void
Federate::setLogicalTimeFactoryName(const std::string& logicalTimeFactoryName)
{
  _logicalTimeFactoryName = logicalTimeFactoryName;
}

void
Federate::setAutomaticResignDirective(ResignAction resignAction)
{
  _automaticResignDirective = resignAction;
}

void
Federate::setObjectClassRelevanceAdvisorySwitchEnabled(bool objectClassRelevanceAdvisorySwitchEnabled)
{
  _objectClassRelevanceAdvisorySwitchEnabled = objectClassRelevanceAdvisorySwitchEnabled;
}

void
Federate::setAttributeRelevanceAdvisorySwitchEnabled(bool attributeRelevanceAdvisorySwitchEnabled)
{
  _attributeRelevanceAdvisorySwitchEnabled = attributeRelevanceAdvisorySwitchEnabled;
}

void
Federate::setAttributeScopeAdvisorySwitchEnabled(bool attributeScopeAdvisorySwitchEnabled)
{
  _attributeScopeAdvisorySwitchEnabled = attributeScopeAdvisorySwitchEnabled;
}

void
Federate::setInteractionRelevanceAdvisorySwitchEnabled(bool interactionRelevanceAdvisorySwitchEnabled)
{
  _interactionRelevanceAdvisorySwitchEnabled = interactionRelevanceAdvisorySwitchEnabled;
}

void
Federate::setPermitTimeRegulation(bool permitTimeRegulation)
{
  _permitTimeRegulation = permitTimeRegulation;
}

void
Federate::applySwitch(const FOMSwitch& switchValue)
{
  switch (switchValue.getSwitchesType()) {
  case InteractionRelevanceAdvisorySwitchesType:
    _interactionRelevanceAdvisorySwitchEnabled = switchValue.getEnabled();
    break;
  case ObjectClassRelevanceAdvisorySwitchesType:
    _objectClassRelevanceAdvisorySwitchEnabled = switchValue.getEnabled();
    break;
  case AttributeRelevanceAdvisorySwitchesType:
    _attributeRelevanceAdvisorySwitchEnabled = switchValue.getEnabled();
    break;
  case AttributeScopeAdvisorySwitchesType:
    _attributeScopeAdvisorySwitchEnabled = switchValue.getEnabled();
    break;
  case AutoProvideSwitchesType:
    // _autoProvideSwitchEnabled = switchValue.getEnabled();
    break;
  case ConveyRegionDesignatorSetsSwitchesType:
    // _conveyRegionDesignatorSetsSwitchEnabled = switchValue.getEnabled();
    break;
  case ServiceReportingSwitchesType:
    // _serviceReportingSwitchesEnabled = switchValue.getEnabled();
    break;
  }
}

double
Federate::getUpdateRateValue(const std::string& name) const
{
  NameUpdateRateMap::const_iterator i = _nameUpdateRateMap.find(name);
  if (i == _nameUpdateRateMap.end())
    return -1;
  return i->second;
}

void
Federate::insertUpdateRate(const std::string& name, double value)
{
  // OpenRTIAssert(_nameUpdateRateMap.find(name) == _nameUpdateRateMap.end());
  _nameUpdateRateMap[name] = value;
}

const TransportationType*
Federate::getTransportationType(const std::string& name) const
{
  NameTransportationTypeMap::const_iterator i = _nameTransportationTypeMap.find(name);
  if (i == _nameTransportationTypeMap.end())
    return 0;
  return &i->second;
}

const std::string*
Federate::getTransportationName(TransportationType transportationType) const
{
  TransportationTypeNameMap::const_iterator i = _transportationTypeNameMap.find(transportationType);
  if (i == _transportationTypeNameMap.end())
    return 0;
  return &i->second;
}

void
Federate::insertTransportationType(const std::string& name, TransportationType transportationType)
{
  OpenRTIAssert(_nameTransportationTypeMap.find(name) == _nameTransportationTypeMap.end());
  _nameTransportationTypeMap[name] = transportationType;
  // Only insert the first occurance of transportationType to the type to name map
  // This way we should get the standard name back even if we map a custom transportation
  // type name to the same resulting transportation.
  _transportationTypeNameMap.insert(TransportationTypeNameMap::value_type(transportationType, name));
}

const OrderType*
Federate::getOrderType(const std::string& name) const
{
  NameOrderTypeMap::const_iterator i = _nameOrderTypeMap.find(name);
  if (i == _nameOrderTypeMap.end())
    return 0;
  return &i->second;
}

const std::string*
Federate::getOrderName(OrderType orderType) const
{
  OrderTypeNameMap::const_iterator i = _orderTypeNameMap.find(orderType);
  if (i == _orderTypeNameMap.end())
    return 0;
  return &i->second;
}

void
Federate::insertOrderType(const std::string& name, OrderType orderType)
{
  OpenRTIAssert(_nameOrderTypeMap.find(name) == _nameOrderTypeMap.end());
  OpenRTIAssert(_orderTypeNameMap.find(orderType) == _orderTypeNameMap.end());
  _nameOrderTypeMap[name] = orderType;
  _orderTypeNameMap[orderType] = name;
}

const Federate::Dimension*
Federate::getDimension(const DimensionHandle& dimensionHandle) const
{
  size_t index = dimensionHandle.getHandle();
  if (_dimensionVector.size() <= index)
    return 0;
  return _dimensionVector[index].get();
}

DimensionHandle
Federate::getDimensionHandle(const std::string& name) const
{
  NameDimensionHandleMap::const_iterator i = _nameDimensionHandleMap.find(name);
  if (i == _nameDimensionHandleMap.end())
    return DimensionHandle();
  return i->second;
}

void
Federate::insertDimension(const std::string& name, const DimensionHandle& dimensionHandle, Unsigned upperBound)
{
  // OpenRTIAssert(_nameDimensionHandleMap.find(name) == _nameDimensionHandleMap.end());
  size_t index = dimensionHandle.getHandle();
  if (_dimensionVector.size() <= index)
    _dimensionVector.resize(index + 1);
  _dimensionVector[index] = new Dimension(name, upperBound);
  _nameDimensionHandleMap[name] = dimensionHandle;
}

void
Federate::insertRoutingSpace(const std::string& name, const SpaceHandle& spaceHandle, const DimensionHandleSet& dimensionHandles)
{
  // FIXME implement for the HLA13 stuff, when the parser is there :)
}

Federate::RegionData*
Federate::getRegion(const RegionHandle& regionHandle)
{
  RegionHandleRegionDataMap::const_iterator i = _regionHandleRegionDataMap.find(regionHandle);
  if (i == _regionHandleRegionDataMap.end())
    return 0;
  return i->second.get();
}

const Federate::RegionData*
Federate::getRegion(const RegionHandle& regionHandle) const
{
  RegionHandleRegionDataMap::const_iterator i = _regionHandleRegionDataMap.find(regionHandle);
  if (i == _regionHandleRegionDataMap.end())
    return 0;
  return i->second.get();
}

void
Federate::insertRegion(const RegionHandle& regionHandle, const DimensionHandleSet& dimensionHandleSet)
{
  OpenRTIAssert(_regionHandleRegionDataMap.find(regionHandle) == _regionHandleRegionDataMap.end());
  _regionHandleRegionDataMap[regionHandle] = new RegionData(dimensionHandleSet);
}

void
Federate::insertRegions(const RegionHandleDimensionHandleSetPairVector& regionHandleDimensionHandleSetPairVector)
{
  for (RegionHandleDimensionHandleSetPairVector::const_iterator i = regionHandleDimensionHandleSetPairVector.begin();
       i != regionHandleDimensionHandleSetPairVector.end(); ++i)
    insertRegion(i->first, i->second);
}

void
Federate::eraseRegion(const RegionHandle& regionHandle)
{
  OpenRTIAssert(_regionHandleRegionDataMap.find(regionHandle) != _regionHandleRegionDataMap.end());
  _regionHandleRegionDataMap.erase(regionHandle);
}

void
Federate::eraseRegions(const RegionHandleVector& regionHandleVector)
{
  for (RegionHandleVector::const_iterator i = regionHandleVector.begin(); i != regionHandleVector.end(); ++i)
    eraseRegion(*i);
}

RegionHandle
Federate::insertLocalRegion(const DimensionHandleSet& dimensionHandleSet)
{
  RegionHandle regionHandle(getFederateHandle(), _localRegionHandleAllocator.get());
  insertRegion(regionHandle, dimensionHandleSet);
  return regionHandle;
}

void
Federate::eraseLocalRegion(const RegionHandle& regionHandle)
{
  eraseRegion(regionHandle);
  LocalRegionHandle localRegionHandle = regionHandle.getLocalRegionHandle();
  _localRegionHandleAllocator.put(localRegionHandle);
}

RegionHandleVector
Federate::getLocalRegionHandles() const
{
  /// FIXME equally well ask the local handle allocator for the used ones?!
  RegionHandleVector regionHandleVector;
  regionHandleVector.reserve(_regionHandleRegionDataMap.size());
  for (RegionHandleRegionDataMap::const_iterator i = _regionHandleRegionDataMap.begin();
       i != _regionHandleRegionDataMap.end(); ++i) {
    if (i->first.getFederateHandle() != getFederateHandle())
      continue;
    regionHandleVector.push_back(i->first);
  }
  return regionHandleVector;
}

void
Federate::commitRegion(const RegionHandle& regionHandle, const RegionValue& regionValue)
{
  RegionHandleRegionDataMap::iterator i = _regionHandleRegionDataMap.find(regionHandle);
  OpenRTIAssert(i != _regionHandleRegionDataMap.end());
  Region(regionValue).swap(i->second->getRegion());
}

void
Federate::commitRegions(const RegionHandleRegionValuePairVector& regionHandleRegionValuePairVector)
{
  for (RegionHandleRegionValuePairVector::const_iterator i = regionHandleRegionValuePairVector.begin();
       i != regionHandleRegionValuePairVector.end(); ++i)
    commitRegion(i->first, i->second);
}

Federate::InteractionClass*
Federate::getInteractionClass(const InteractionClassHandle& interactionClassHandle)
{
  if (_interactionClassVector.size() <= interactionClassHandle.getHandle())
    return 0;
  return _interactionClassVector[interactionClassHandle.getHandle()].get();
}

const Federate::InteractionClass*
Federate::getInteractionClass(const InteractionClassHandle& interactionClassHandle) const
{
  if (_interactionClassVector.size() <= interactionClassHandle.getHandle())
    return 0;
  return _interactionClassVector[interactionClassHandle.getHandle()].get();
}

InteractionClassHandle
Federate::getInteractionClassHandle(const std::string& name) const
{
  NameInteractionClassHandleMap::const_iterator i = _nameInteractionClassHandleMap.find(name);
  if (i == _nameInteractionClassHandleMap.end())
    return InteractionClassHandle();
  return i->second;
}

void
Federate::insertInteractionClass(const FOMInteractionClass& module, bool artificialInteractionRoot)
{
  size_t index = module.getInteractionClassHandle().getHandle();
  if (_interactionClassVector.size() <= index)
    _interactionClassVector.resize(index + 1);
  if (!_interactionClassVector[index].valid()) {
    _interactionClassVector[index] = new InteractionClass;

    InteractionClassHandle parentHandle = module.getParentInteractionClassHandle();

    std::string fqName;
    if (parentHandle.valid()) {
      fqName = getInteractionClass(parentHandle)->getFQName();
      if (!fqName.empty())
        fqName.append(".");
    }
    fqName.append(module.getName());
    if (artificialInteractionRoot && fqName.compare(0, 19, "HLAinteractionRoot.") == 0) {
      fqName = fqName.substr(19, std::string::npos);
    }

    _nameInteractionClassHandleMap[module.getName()] = module.getInteractionClassHandle();
    if (!fqName.empty() && fqName != module.getName()) {
      _nameInteractionClassHandleMap[fqName] = module.getInteractionClassHandle();
    }

    InteractionClass* interactionClass = _interactionClassVector[index].get();
    interactionClass->setName(module.getName());
    interactionClass->setFQName(fqName);
    interactionClass->setParentInteractionClassHandle(parentHandle);
    interactionClass->setOrderType(module.getOrderType());
    interactionClass->setTransportationType(module.getTransportationType());
    interactionClass->setDimensionHandleSet(module.getDimensionHandleSet());

    if (parentHandle.valid()) {
      InteractionClass* parentClass = getInteractionClass(parentHandle);
      OpenRTIAssert(parentClass);
      parentClass->insertChildInteractionClass(*interactionClass);
    }
  }
  for (FOMParameterList::const_iterator i = module.getParameterList().begin();
       i != module.getParameterList().end(); ++i) {
    _interactionClassVector[index]->insertParameter(*i);
  }
}

Federate::ObjectClass*
Federate::getObjectClass(const ObjectClassHandle& objectClassHandle)
{
  if (_objectClassVector.size() <= objectClassHandle.getHandle())
    return 0;
  return _objectClassVector[objectClassHandle.getHandle()].get();
}

const Federate::ObjectClass*
Federate::getObjectClass(const ObjectClassHandle& objectClassHandle) const
{
  if (_objectClassVector.size() <= objectClassHandle.getHandle())
    return 0;
  return _objectClassVector[objectClassHandle.getHandle()].get();
}

ObjectClassHandle
Federate::getObjectClassHandle(const std::string& name) const
{
  NameObjectClassHandleMap::const_iterator i = _nameObjectClassHandleMap.find(name);
  if (i == _nameObjectClassHandleMap.end())
    return ObjectClassHandle();
  return i->second;
}

void
Federate::insertObjectClass(const FOMObjectClass& module, bool artificialObjectRoot)
{
  size_t index = module.getObjectClassHandle().getHandle();
  if (_objectClassVector.size() <= index)
    _objectClassVector.resize(index + 1);
  if (!_objectClassVector[index].valid()) {
    _objectClassVector[index] = new ObjectClass;
    ObjectClass* objectClass = _objectClassVector[index].get();

    ObjectClassHandle parentHandle = module.getParentObjectClassHandle();

    std::string fqName;
    if (parentHandle.valid()) {
      fqName = getObjectClass(parentHandle)->getFQName();
      if (!fqName.empty())
        fqName.append(".");
    }
    fqName.append(module.getName());
    if (artificialObjectRoot && fqName.compare(0, 14, "HLAobjectRoot.") == 0) {
      fqName = fqName.substr(14, std::string::npos);
    }

    _nameObjectClassHandleMap[module.getName()] = module.getObjectClassHandle();
    if (!fqName.empty() && fqName != module.getName()) {
      _nameObjectClassHandleMap[fqName] = module.getObjectClassHandle();
    }

    objectClass->setName(module.getName());
    objectClass->setFQName(fqName);
    objectClass->setParentObjectClassHandle(parentHandle);

    if (parentHandle.valid()) {
      ObjectClass* parentClass = getObjectClass(parentHandle);
      OpenRTIAssert(parentClass);
      parentClass->insertChildObjectClass(*objectClass);
    }
  }
  for (FOMAttributeList::const_iterator i = module.getAttributeList().begin();
       i != module.getAttributeList().end(); ++i) {
    _objectClassVector[index]->insertAttribute(*i);
  }
}

Federate::ObjectInstance*
Federate::getObjectInstance(const ObjectInstanceHandle& objectInstanceHandle)
{
  ObjectInstanceHandleMap::const_iterator i = _objectInstanceHandleMap.find(objectInstanceHandle);
  if (i == _objectInstanceHandleMap.end())
    return 0;
  return i->second.get();
}

const Federate::ObjectInstance*
Federate::getObjectInstance(const ObjectInstanceHandle& objectInstanceHandle) const
{
  ObjectInstanceHandleMap::const_iterator i = _objectInstanceHandleMap.find(objectInstanceHandle);
  if (i == _objectInstanceHandleMap.end())
    return 0;
  return i->second.get();
}

ObjectInstanceHandle
Federate::getObjectInstanceHandle(const std::string& name) const
{
  NameObjectInstanceHandleMap::const_iterator i = _nameObjectInstanceHandleMap.find(name);
  if (i == _nameObjectInstanceHandleMap.end())
    return ObjectInstanceHandle();
  return i->second;
}

void
Federate::insertObjectInstance(ObjectInstanceHandle objectInstanceHandle, const std::string& name, ObjectClassHandle objectClassHandle, bool owned)
{
  OpenRTIAssert(_nameObjectInstanceHandleMap.find(name) == _nameObjectInstanceHandleMap.end());
  OpenRTIAssert(_objectInstanceHandleMap.find(objectInstanceHandle) == _objectInstanceHandleMap.end());
  const ObjectClass* objectClass = getObjectClass(objectClassHandle);
  OpenRTIAssert(objectClass);
  NameObjectInstanceHandleMap::iterator i;
  i = _nameObjectInstanceHandleMap.insert(NameObjectInstanceHandleMap::value_type(name, objectInstanceHandle)).first;
  SharedPtr<ObjectInstance> objectInstance = new ObjectInstance(i, *objectClass, owned);
  objectInstance->setObjectClassHandle(objectClassHandle);
  _objectInstanceHandleMap.insert(ObjectInstanceHandleMap::value_type(objectInstanceHandle, objectInstance));
}

void
Federate::eraseObjectInstance(const ObjectInstanceHandle& objectInstanceHandle)
{
  ObjectInstanceHandleMap::iterator i = _objectInstanceHandleMap.find(objectInstanceHandle);
  if (i == _objectInstanceHandleMap.end()) {
    Log(FederateAmbassador, Warning) << "Federate: \"" << getFederateType() << "\": Cannot remove object instance with object handle: "
                                     << objectInstanceHandle.getHandle() << std::endl;
    return;
  }
  _nameObjectInstanceHandleMap.erase(i->second->getNameObjectInstanceHandleMapIterator());
  _objectInstanceHandleMap.erase(i);
}

void
Federate::insertObjectInstanceHandleNamePairs(const ObjectInstanceHandleNamePairVector& objectInstanceHandleNamePairVector)
{
  for (ObjectInstanceHandleNamePairVector::const_iterator i = objectInstanceHandleNamePairVector.begin();
       i != objectInstanceHandleNamePairVector.end(); ++i) {
    _privateNameObjectInstanceHandlePairs[i->second] = i->first;
  }
}

bool
Federate::haveFreeObjectInstanceHandleNamePair() const
{
  return !_privateNameObjectInstanceHandlePairs.empty();
}

ObjectInstanceHandleNamePair
Federate::takeFreeObjectInstanceHandleNamePair()
{
  if (_privateNameObjectInstanceHandlePairs.empty())
    return ObjectInstanceHandleNamePair();

  NameObjectInstanceHandleMap::iterator i = _privateNameObjectInstanceHandlePairs.begin();
  ObjectInstanceHandleNamePair handleNamePair(i->second, i->first);
  _privateNameObjectInstanceHandlePairs.erase(i);

  return handleNamePair;
}

void
Federate::insertReservedObjectInstanceHandleNamePair(const ObjectInstanceHandleNamePair& objectInstanceHandleNamePair)
{
  _reservedNameObjectInstanceHandlePairs[objectInstanceHandleNamePair.second] = objectInstanceHandleNamePair.first;
}

bool
Federate::objectInstanceNameReserved(const std::string& objectInstanceName) const
{
  return _reservedNameObjectInstanceHandlePairs.find(objectInstanceName) != _reservedNameObjectInstanceHandlePairs.end();
}

ObjectInstanceHandle
Federate::takeReservedObjectInstanceName(const std::string& objectInstanceName)
{
  NameObjectInstanceHandleMap::iterator i = _reservedNameObjectInstanceHandlePairs.find(objectInstanceName);
  if (i == _reservedNameObjectInstanceHandlePairs.end())
    return ObjectInstanceHandle();

  // Remove that locally ...
  ObjectInstanceHandle objectInstanceHandle = i->second;
  _reservedNameObjectInstanceHandlePairs.erase(i);
  return objectInstanceHandle;
}

Federate::_Federate*
Federate::getFederate(const FederateHandle& federateHandle)
{
  FederateHandleMap::const_iterator i = _federateHandleMap.find(federateHandle);
  if (i == _federateHandleMap.end())
    return 0;
  return i->second.get();
}

const Federate::_Federate*
Federate::getFederate(const FederateHandle& federateHandle) const
{
  FederateHandleMap::const_iterator i = _federateHandleMap.find(federateHandle);
  if (i == _federateHandleMap.end())
    return 0;
  return i->second.get();
}

FederateHandle
Federate::getFederateHandle(const std::string& name) const
{
  NameFederateHandleMap::const_iterator i = _nameFederateHandleMap.find(name);
  if (i == _nameFederateHandleMap.end())
    return FederateHandle();
  return i->second;
}

void
Federate::insertFederate(const FederateHandle& federateHandle, const std::string& name)
{
  OpenRTIAssert(_nameFederateHandleMap.find(name) == _nameFederateHandleMap.end());
  OpenRTIAssert(_federateHandleMap.find(federateHandle) == _federateHandleMap.end());
  NameFederateHandleMap::iterator i;
  i = _nameFederateHandleMap.insert(NameFederateHandleMap::value_type(name, federateHandle)).first;
  SharedPtr<_Federate> federate = new _Federate(i);
  _federateHandleMap.insert(FederateHandleMap::value_type(federateHandle, federate));
}

void
Federate::eraseFederate(const FederateHandle& federateHandle)
{
  FederateHandleMap::iterator i = _federateHandleMap.find(federateHandle);
  if (i == _federateHandleMap.end()) {
    Log(FederateAmbassador, Warning) << "Federate: \"" << getFederateType() << "\": Cannot remove object instance with object handle: "
                                     << federateHandle.getHandle() << std::endl;
    return;
  }
  _nameFederateHandleMap.erase(i->second->getNameFederateHandleMapIterator());
  _federateHandleMap.erase(i);
}

void
Federate::insertAnnouncedFederationSynchonizationLabel(const std::string& label)
{
  _announcedFederationSynchonizationLabels.insert(label);
}

void
Federate::eraseAnnouncedFederationSynchonizationLabel(const std::string& label)
{
  _announcedFederationSynchonizationLabels.erase(label);
}

bool
Federate::synchronizationLabelAnnounced(const std::string& label) const
{
  return _announcedFederationSynchonizationLabels.find(label) != _announcedFederationSynchonizationLabels.end();
}

void
Federate::insertFOMModule(const FOMModule& module)
{
  for (FOMUpdateRateList::const_iterator i = module.getUpdateRateList().begin();
       i != module.getUpdateRateList().end(); ++i)
    insertUpdateRate(i->getName(), i->getRate());
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
    insertInteractionClass(*i, module.getArtificialInteractionRoot());
  for (FOMObjectClassList::const_iterator i = module.getObjectClassList().begin();
       i != module.getObjectClassList().end(); ++i)
    insertObjectClass(*i, module.getArtificialObjectRoot());
  for (FOMSwitchList::const_iterator i = module.getSwitchList().begin();
       i != module.getSwitchList().end(); ++i)
    applySwitch(*i);
}

void
Federate::insertFOMModuleList(const FOMModuleList& moduleList)
{
  for (FOMModuleList::const_iterator i = moduleList.begin(); i != moduleList.end(); ++i)
    insertFOMModule(*i);
}

}
