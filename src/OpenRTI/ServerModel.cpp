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

#include "ServerModel.h"

#include "ServerOptions.h"

namespace OpenRTI {
namespace ServerModel {

////////////////////////////////////////////////////////////

Region::Region()
{
}

Region::~Region()
{
}

void
Region::setRegionHandle(const LocalRegionHandle& regionHandle)
{
  HandleEntity<Region, LocalRegionHandle>::_setHandle(regionHandle);
}

////////////////////////////////////////////////////////////

InstanceAttribute::InstanceAttribute(ObjectInstance& objectInstance, ClassAttribute& classAttribute) :
  _objectInstance(objectInstance),
  _classAttribute(classAttribute)
{
  setAttributeHandle(_classAttribute.getAttributeHandle());
  /// FIXME
  _receivingConnects = _classAttribute._cumulativeSubscribedConnectHandleSet;
}

InstanceAttribute::~InstanceAttribute()
{
}

void
InstanceAttribute::setAttributeHandle(const AttributeHandle& attributeHandle)
{
  HandleEntity<InstanceAttribute, AttributeHandle>::_setHandle(attributeHandle);
}

////////////////////////////////////////////////////////////

ObjectInstanceConnect::ObjectInstanceConnect(ObjectInstance& objectInstance, FederationConnect& federationConnect) :
  _objectInstance(objectInstance),
  _federationConnect(federationConnect)
{
  setConnectHandle(federationConnect.getConnectHandle());
}

ObjectInstanceConnect::~ObjectInstanceConnect()
{
}

void
ObjectInstanceConnect::setConnectHandle(const ConnectHandle& connectHandle)
{
  HandleMap::Hook::setKey(connectHandle);
}

////////////////////////////////////////////////////////////

ObjectInstance::ObjectInstance(Federation& federation) :
  _federation(federation),
  _objectClass(0)
{
}

ObjectInstance::~ObjectInstance()
{
  _connectHandleObjectInstanceConnectMap.clear();
  _attributeHandleInstanceAttributeMap.clear();
}

void
ObjectInstance::setObjectInstanceHandle(const ObjectInstanceHandle& objectInstanceHandle)
{
  HandleStringEntity<ObjectInstance, ObjectInstanceHandle>::_setHandle(objectInstanceHandle);
}

void
ObjectInstance::setName(const std::string& name)
{
  HandleStringEntity<ObjectInstance, ObjectInstanceHandle>::_setString(name);
}

void
ObjectInstance::insert(InstanceAttribute& instanceAttribute)
{
  _attributeHandleInstanceAttributeMap.insert(instanceAttribute);
}

InstanceAttribute*
ObjectInstance::getInstanceAttribute(const AttributeHandle& attributeHandle)
{
  InstanceAttribute::HandleMap::iterator i = _attributeHandleInstanceAttributeMap.find(attributeHandle);
  if (i == _attributeHandleInstanceAttributeMap.end())
    return 0;
  return i.get();
}

InstanceAttribute*
ObjectInstance::getPrivilegeToDeleteInstanceAttribute()
{
  // InstanceAttribute::HandleMap::iterator i = _attributeHandleInstanceAttributeMap.begin();
  InstanceAttribute::HandleMap::iterator i = _attributeHandleInstanceAttributeMap.find(AttributeHandle(0));
  if (i == _attributeHandleInstanceAttributeMap.end())
    return 0;
  OpenRTIAssert(i->getAttributeHandle() == AttributeHandle(0));
  return i.get();
}

void
ObjectInstance::reference(FederationConnect& federationConnect)
{
  // OpenRTIAssert(_connectHandleObjectInstanceConnectMap.find(federationConnect.getConnectHandle()) == _connectHandleObjectInstanceConnectMap.end());
  if (_connectHandleObjectInstanceConnectMap.find(federationConnect.getConnectHandle()) != _connectHandleObjectInstanceConnectMap.end())
    return;
  ObjectInstanceConnect* objectInstanceConnect;
  objectInstanceConnect = new ObjectInstanceConnect(*this, federationConnect);
  insert(*objectInstanceConnect);
  federationConnect.insert(*objectInstanceConnect);
}

bool
ObjectInstance::unreference(const ConnectHandle& connectHandle)
{
  ObjectInstanceConnect::HandleMap::iterator i = _connectHandleObjectInstanceConnectMap.find(connectHandle);
  OpenRTIAssert(i != _connectHandleObjectInstanceConnectMap.end());
  _connectHandleObjectInstanceConnectMap.erase(i);
  return _connectHandleObjectInstanceConnectMap.empty();
}

void
ObjectInstance::removeConnect(const ConnectHandle& connectHandle)
{
  // FIXME make that an assert at some time
  // OpenRTIAssert(_connectHandleSet.find(connectHandle) == _connectHandleSet.end());
  for (InstanceAttribute::HandleMap::iterator i = _attributeHandleInstanceAttributeMap.begin();
       i != _attributeHandleInstanceAttributeMap.end(); ++i) {
    i->removeConnect(connectHandle);
  }
}

void
ObjectInstance::setObjectClass(ObjectClass* objectClass)
{
  if (objectClass == _objectClass)
    return;
  OpenRTIAssert(!_objectClass);
  _objectClass = objectClass;
  _objectClass->insert(*this);

  ClassAttribute::HandleMap& attributeHandleClassAttributeMap = objectClass->getAttributeHandleClassAttributeMap();
  for (ClassAttribute::HandleMap::iterator i = attributeHandleClassAttributeMap.begin();
       i != attributeHandleClassAttributeMap.end(); ++i) {
    ServerModel::InstanceAttribute* instanceAttribute = new ServerModel::InstanceAttribute(*this, *i);
    insert(*instanceAttribute);
  }
}

////////////////////////////////////////////////////////////

SynchronizationFederate::SynchronizationFederate(Synchronization& synchronization, Federate& federate) :
  _synchronization(synchronization),
  _federate(federate),
  _successful(false)
{
}

SynchronizationFederate::~SynchronizationFederate()
{
}

void
SynchronizationFederate::setFederateHandle(const FederateHandle& federateHandle)
{
  HandleListEntity<SynchronizationFederate, FederateHandle>::_setHandle(federateHandle);
}

void
SynchronizationFederate::setSuccessful(bool successful)
{
  _successful = successful;
}

////////////////////////////////////////////////////////////

Synchronization::Synchronization()
{
}

Synchronization::~Synchronization()
{
  _achievedFederateSyncronizationMap.clear();
  _waitingFederateSyncronizationMap.clear();
}

void
Synchronization::setLabel(const std::string& label)
{
  NameMap::Hook::setKey(label);
}

void
Synchronization::setTag(const VariableLengthData& tag)
{
  _tag = tag;
}

void
Synchronization::setAddJoiningFederates(bool addJoiningFederates)
{
  _addJoiningFederates = addJoiningFederates;
}

bool
Synchronization::getIsWaitingFor(const FederateHandle& federateHandle)
{
  return _waitingFederateSyncronizationMap.find(federateHandle) != _waitingFederateSyncronizationMap.end();
}

void
Synchronization::insert(Federate& federate)
{
  OpenRTIAssert(_waitingFederateSyncronizationMap.find(federate.getFederateHandle()) == _waitingFederateSyncronizationMap.end());
  OpenRTIAssert(_achievedFederateSyncronizationMap.find(federate.getFederateHandle()) == _achievedFederateSyncronizationMap.end());
  if (federate.getResignPending())
    return;
  SynchronizationFederate* synchronizationFederate = new SynchronizationFederate(*this, federate);
  synchronizationFederate->setFederateHandle(federate.getFederateHandle());
  federate.insert(*synchronizationFederate);
  _waitingFederateSyncronizationMap.insert(*synchronizationFederate);
}

void
Synchronization::achieved(const FederateHandle& federateHandle, bool successful)
{
  ServerModel::SynchronizationFederate::HandleMap::iterator i;
  i = _waitingFederateSyncronizationMap.find(federateHandle);
  if (i == _waitingFederateSyncronizationMap.end())
    return;
  i->setSuccessful(successful);
  // OpenRTIAssert(_achievedFederateSyncronizationMap.find(i->getFederateHandle()) == _achievedFederateSyncronizationMap.end());
  // Note that no matter where we are currently linked,
  // this removes the entry from one of the maps
  SynchronizationFederate::HandleMap::unlink(*i);
  _achievedFederateSyncronizationMap.insert(*i);
}

////////////////////////////////////////////////////////////

Federate::Federate(Federation& federation) :
  _federation(federation),
  _resignAction(CANCEL_THEN_DELETE_THEN_DIVEST),
  _resignPending(false),
  _federationConnect(0),
  _commitId()
{
}

Federate::~Federate()
{
  _synchronizationFederateList.clear();
  _federationConnect = 0;
}

void
Federate::setName(const std::string& name)
{
  HandleStringEntity<Federate, FederateHandle>::_setString(name);
}

void
Federate::setFederateHandle(const FederateHandle& federateHandle)
{
  HandleStringEntity<Federate, FederateHandle>::_setHandle(federateHandle);
}

void
Federate::setFederateType(const std::string& federateType)
{
  _federateType = federateType;
}

void
Federate::setResignAction(ResignAction resignAction)
{
  _resignAction = resignAction;
}

void
Federate::insert(SynchronizationFederate& synchronizationFederate)
{
  _synchronizationFederateList.push_back(synchronizationFederate);
}

void
Federate::setResignPending(bool resignPending)
{
  _resignPending = resignPending;
}

Region*
Federate::getRegion(const LocalRegionHandle& regionHandle)
{
  Region::HandleMap::iterator i = _regionHandleRegionMap.find(regionHandle);
  if (i == _regionHandleRegionMap.end())
    return 0;
  return i.get();
}

bool
Federate::getIsTimeRegulating() const
{
  // OpenRTIAssert(!SecondList::Hook::is_linked() || _federationConnect->_permitTimeRegulation);
  return SecondList::Hook::is_linked();
}

void
Federate::setTimeAdvanceTimeStamp(const VariableLengthData& timeAdvanceTimeStamp)
{
  _timeAdvanceTimeStamp = timeAdvanceTimeStamp;
}

void
Federate::setNextMessageTimeStamp(const VariableLengthData& nextMessageTimeStamp)
{
  _nextMessageTimeStamp = nextMessageTimeStamp;
}

void
Federate::setCommitId(Unsigned commitId)
{
  _commitId = commitId;
}

ConnectHandle
Federate::getConnectHandle() const
{
  if (!_federationConnect)
    return ConnectHandle();
  return _federationConnect->getConnectHandle();
}

void
Federate::send(const SharedPtr<const AbstractMessage>& message)
{
  if (!_federationConnect)
    return;
  _federationConnect->send(message);
}

////////////////////////////////////////////////////////////

DimensionModule::DimensionModule(Dimension& dimension, Module& module) :
  _dimension(dimension),
  _module(module)
{
}

DimensionModule::~DimensionModule()
{
}

////////////////////////////////////////////////////////////

Dimension::Dimension(Federation& federation) :
  _federation(federation),
  _upperBound(~Unsigned(0))
{
}

Dimension::~Dimension()
{
  OpenRTIAssert(_dimensionModuleList.empty());
}

void
Dimension::setName(const std::string& name)
{
  ModuleEntity<Dimension, DimensionHandle>::_setString(name);
}

void
Dimension::setDimensionHandle(const DimensionHandle& dimensionHandle)
{
  ModuleEntity<Dimension, DimensionHandle>::_setHandle(dimensionHandle);
}

void
Dimension::setUpperBound(const Unsigned& upperBound)
{
  _upperBound = upperBound;
}

bool
Dimension::getIsReferencedByAnyModule() const
{
  return !_dimensionModuleList.empty();
}

////////////////////////////////////////////////////////////

UpdateRateModule::UpdateRateModule(UpdateRate& updateRate, Module& module) :
  _updateRate(updateRate), _module(module)
{
}

UpdateRateModule::~UpdateRateModule()
{
}

////////////////////////////////////////////////////////////

UpdateRate::UpdateRate(Federation& federation) :
  _federation(federation),
  _rate(0)
{
}

UpdateRate::~UpdateRate()
{
  OpenRTIAssert(_updateRateModuleList.empty());
}

void
UpdateRate::setName(const std::string& name)
{
  ModuleEntity<UpdateRate, UpdateRateHandle>::_setString(name);
}

void
UpdateRate::setUpdateRateHandle(const UpdateRateHandle& updateRateHandle)
{
  ModuleEntity<UpdateRate, UpdateRateHandle>::_setHandle(updateRateHandle);
}

void
UpdateRate::setRate(const double& rate)
{
  _rate = rate;
}

bool
UpdateRate::getIsReferencedByAnyModule() const
{
  return !_updateRateModuleList.empty();
}

////////////////////////////////////////////////////////////

InteractionClassModule::InteractionClassModule(InteractionClass& interactionClass, Module& module) :
  _interactionClass(interactionClass), _module(module)
{
}

InteractionClassModule::~InteractionClassModule()
{
}

////////////////////////////////////////////////////////////

ParameterDefinitionModule::ParameterDefinitionModule(InteractionClass& interactionClass, Module& module) :
  _interactionClass(interactionClass), _module(module)
{
}

ParameterDefinitionModule::~ParameterDefinitionModule()
{
}

////////////////////////////////////////////////////////////

ClassParameter::ClassParameter(InteractionClass& interactionClass, ParameterDefinition& parameterDefinition) :
  _interactionClass(interactionClass),
  _parameterDefinition(parameterDefinition)
{
}

ClassParameter::~ClassParameter()
{
}

void
ClassParameter::setParameterHandle(const ParameterHandle& parameterHandle)
{
  HandleListEntity<ClassParameter, ParameterHandle>::_setHandle(parameterHandle);
}

////////////////////////////////////////////////////////////

ParameterDefinition::ParameterDefinition(InteractionClass& interactionClass) :
  _interactionClass(interactionClass)
{
}

ParameterDefinition::~ParameterDefinition()
{
  _classParameterList.clear();
}

void
ParameterDefinition::setName(const std::string& name)
{
  HandleStringEntity<ParameterDefinition, ParameterHandle>::_setString(name);
}

void
ParameterDefinition::setParameterHandle(const ParameterHandle& parameterHandle)
{
  HandleStringEntity<ParameterDefinition, ParameterHandle>::_setHandle(parameterHandle);
}

////////////////////////////////////////////////////////////

InteractionClass::InteractionClass(Federation& federation, InteractionClass* parentInteractionClass) :
  _federation(federation),
  _parentInteractionClass(parentInteractionClass),
  _orderType(),
  _transportationType()
{
  if (_parentInteractionClass) {
    _parentInteractionClass->_childInteractionClassList.push_back(*this);

    for (ClassParameter::HandleMap::iterator i = _parentInteractionClass->_parameterHandleClassParameterMap.begin();
         i != _parentInteractionClass->_parameterHandleClassParameterMap.end(); ++i) {
      insertClassParameterFor(i->getParameterDefinition());
    }
  }
}

InteractionClass::~InteractionClass()
{
  _parameterHandleClassParameterMap.clear();
  eraseParameterDefinitions();
  OpenRTIAssert(_parameterHandleParameterMap.empty());
  OpenRTIAssert(_parameterNameParameterMap.empty());

  OpenRTIAssert(_childInteractionClassList.empty());
  OpenRTIAssert(_interactionClassModuleList.empty());
  OpenRTIAssert(_parameterDefinitionModuleList.empty());
  _parentInteractionClass = 0;
}

void
InteractionClass::setName(const StringVector& name)
{
  ModuleClassEntity<InteractionClass, InteractionClassHandle>::_setString(name);
}

void
InteractionClass::setInteractionClassHandle(const InteractionClassHandle& interactionClassHandle)
{
  ModuleClassEntity<InteractionClass, InteractionClassHandle>::_setHandle(interactionClassHandle);
}

void
InteractionClass::setOrderType(OrderType orderType)
{
  _orderType = orderType;
}

void
InteractionClass::setTransportationType(TransportationType transportationType)
{
  _transportationType = transportationType;
}

InteractionClass*
InteractionClass::getParentInteractionClass()
{
  return _parentInteractionClass;
}

InteractionClassHandle
InteractionClass::getParentInteractionClassHandle() const
{
  if (!_parentInteractionClass)
    return InteractionClassHandle();
  return _parentInteractionClass->getInteractionClassHandle();
}

bool
InteractionClass::getIsReferencedByAnyModule() const
{
  return !_interactionClassModuleList.empty() || !_parameterDefinitionModuleList.empty();
}

bool
InteractionClass::getAreParametersReferencedByAnyModule() const
{
  return !_parameterDefinitionModuleList.empty();
}

void
InteractionClass::eraseParameterDefinitions()
{
  _parameterNameParameterMap.clear();
  OpenRTIAssert(_parameterNameParameterMap.empty());
}

std::size_t
InteractionClass::getNumParameterDefinitions() const
{
  // FIXME O(N)
  return _parameterHandleParameterMap.size();
}

ParameterHandle
InteractionClass::getFirstUnusedParameterHandle()
{
  // FIXME this is O(N)
  std::size_t numParameters = 0;
  if (_parentInteractionClass)
    numParameters += _parentInteractionClass->getFirstUnusedParameterHandle().getHandle();
  numParameters += getNumParameterDefinitions();
  return ParameterHandle(numParameters);
}

void
InteractionClass::insert(ParameterDefinition& parameterDefinition)
{
  _parameterHandleParameterMap.insert(parameterDefinition);
  _parameterNameParameterMap.insert(parameterDefinition);
  insertClassParameterFor(parameterDefinition);
}

ParameterDefinition*
InteractionClass::getParameterDefinition(const std::string& name)
{
  ParameterDefinition::NameMap::iterator i = _parameterNameParameterMap.find(name);
  if (i == _parameterNameParameterMap.end())
    return 0;
  return i.get();
}

ParameterDefinition*
InteractionClass::getParameterDefinition(const ParameterHandle& parameterHandle)
{
  ParameterDefinition::HandleMap::iterator i = _parameterHandleParameterMap.find(parameterHandle);
  if (i == _parameterHandleParameterMap.end())
    return 0;
  return i.get();
}

void
InteractionClass::insertClassParameterFor(ParameterDefinition& parameterDefinition)
{
  ClassParameter* classParameter = new ClassParameter(*this, parameterDefinition);
  classParameter->setParameterHandle(parameterDefinition.getParameterHandle());
  _parameterHandleClassParameterMap.insert(*classParameter);
  parameterDefinition.insert(*classParameter);

  for (ChildList::iterator i = _childInteractionClassList.begin(); i != _childInteractionClassList.end(); ++i)
    i->insertClassParameterFor(parameterDefinition);
}

ClassParameter*
InteractionClass::getClassParameter(const ParameterHandle& parameterHandle)
{
  ClassParameter::HandleMap::iterator i = _parameterHandleClassParameterMap.find(parameterHandle);
  if (i == _parameterHandleClassParameterMap.end())
    return 0;
  return i.get();
}

////////////////////////////////////////////////////////////

ObjectClassModule::ObjectClassModule(ObjectClass& objectClass, Module& module) :
  _objectClass(objectClass), _module(module)
{
}

ObjectClassModule::~ObjectClassModule()
{
}

////////////////////////////////////////////////////////////

AttributeDefinitionModule::AttributeDefinitionModule(ObjectClass& objectClass, Module& module) :
  _objectClass(objectClass), _module(module)
{
}

AttributeDefinitionModule::~AttributeDefinitionModule()
{
}

////////////////////////////////////////////////////////////

ClassAttribute::ClassAttribute(ObjectClass& objectClass, AttributeDefinition& attributeDefinition) :
  _objectClass(objectClass),
  _attributeDefinition(attributeDefinition)
{
}

ClassAttribute::~ClassAttribute()
{
}

void
ClassAttribute::setAttributeHandle(const AttributeHandle& attributeHandle)
{
  HandleListEntity<ClassAttribute, AttributeHandle>::_setHandle(attributeHandle);
}

////////////////////////////////////////////////////////////

AttributeDefinition::AttributeDefinition(ObjectClass& objectClass) :
  _objectClass(objectClass),
  _orderType(),
  _transportationType()
{
}

AttributeDefinition::~AttributeDefinition()
{
  _classAttributeList.clear();
}

void
AttributeDefinition::setName(const std::string& name)
{
  HandleStringEntity<AttributeDefinition, AttributeHandle>::_setString(name);
}

void
AttributeDefinition::setAttributeHandle(const AttributeHandle& attributeHandle)
{
  HandleStringEntity<AttributeDefinition, AttributeHandle>::_setHandle(attributeHandle);
}

void
AttributeDefinition::setOrderType(OrderType orderType)
{
  _orderType = orderType;
}

void
AttributeDefinition::setTransportationType(TransportationType transportationType)
{
  _transportationType = transportationType;
}

////////////////////////////////////////////////////////////

ObjectClass::ObjectClass(Federation& federation, ObjectClass* parentObjectClass) :
  _federation(federation),
  _parentObjectClass(parentObjectClass)
{
  if (_parentObjectClass) {
    _parentObjectClass->_childObjectClassList.push_back(*this);

    for (ClassAttribute::HandleMap::iterator i = _parentObjectClass->_attributeHandleClassAttributeMap.begin();
         i != _parentObjectClass->_attributeHandleClassAttributeMap.end(); ++i) {
      insertClassAttributeFor(i->getAttributeDefinition());
    }
  }
}

ObjectClass::~ObjectClass()
{
  _objectInstanceList.unlink();
  _attributeHandleClassAttributeMap.clear();
  eraseAttributeDefinitions();
  OpenRTIAssert(_childObjectClassList.empty());
  OpenRTIAssert(_objectClassModuleList.empty());
  OpenRTIAssert(_attributeDefinitionModuleList.empty());
  _parentObjectClass = 0;
}

void
ObjectClass::setName(const StringVector& name)
{
  ModuleClassEntity<ObjectClass, ObjectClassHandle>::_setString(name);
}

void
ObjectClass::setObjectClassHandle(const ObjectClassHandle& objectClassHandle)
{
  ModuleClassEntity<ObjectClass, ObjectClassHandle>::_setHandle(objectClassHandle);
}

ObjectClass*
ObjectClass::getParentObjectClass()
{
  return _parentObjectClass;
}

ObjectClassHandle
ObjectClass::getParentObjectClassHandle() const
{
  if (!_parentObjectClass)
    return ObjectClassHandle();
  return _parentObjectClass->getObjectClassHandle();
}

bool
ObjectClass::getIsReferencedByAnyModule() const
{
  return !_objectClassModuleList.empty() || !_attributeDefinitionModuleList.empty();
}

bool
ObjectClass::getAreAttributesReferencedByAnyModule() const
{
  return !_attributeDefinitionModuleList.empty();
}

void
ObjectClass::eraseAttributeDefinitions()
{
  _attributeHandleAttributeDefinitionMap.clear();
  OpenRTIAssert(_attributeNameAttributeDefinitionMap.empty());
}

std::size_t
ObjectClass::getNumAttributeDefinitions() const
{
  // FIXME O(N)
  return _attributeHandleAttributeDefinitionMap.size();
}

AttributeHandle
ObjectClass::getFirstUnusedAttributeHandle()
{
  // FIXME this is O(N)
  std::size_t numAttributes = 0;
  if (_parentObjectClass)
    numAttributes += _parentObjectClass->getFirstUnusedAttributeHandle().getHandle();
  numAttributes += getNumAttributeDefinitions();
  return AttributeHandle(numAttributes);
}

void
ObjectClass::insert(AttributeDefinition& attributeDefinition)
{
  _attributeHandleAttributeDefinitionMap.insert(attributeDefinition);
  _attributeNameAttributeDefinitionMap.insert(attributeDefinition);
  insertClassAttributeFor(attributeDefinition);
}

AttributeDefinition*
ObjectClass::getAttributeDefinition(const std::string& name)
{
  AttributeDefinition::NameMap::iterator i = _attributeNameAttributeDefinitionMap.find(name);
  if (i == _attributeNameAttributeDefinitionMap.end())
    return 0;
  return i.get();
}

AttributeDefinition*
ObjectClass::getAttributeDefinition(const AttributeHandle& attributeHandle)
{
  AttributeDefinition::HandleMap::iterator i = _attributeHandleAttributeDefinitionMap.find(attributeHandle);
  if (i == _attributeHandleAttributeDefinitionMap.end())
    return 0;
  return i.get();
}

void
ObjectClass::insertClassAttributeFor(AttributeDefinition& attributeDefinition)
{
  ClassAttribute* classAttribute = new ClassAttribute(*this, attributeDefinition);
  classAttribute->setAttributeHandle(attributeDefinition.getAttributeHandle());
  _attributeHandleClassAttributeMap.insert(*classAttribute);
  attributeDefinition.insert(*classAttribute);

  for (ChildList::iterator i = _childObjectClassList.begin(); i != _childObjectClassList.end(); ++i)
    i->insertClassAttributeFor(attributeDefinition);
}

ClassAttribute*
ObjectClass::getClassAttribute(const AttributeHandle& attributeHandle)
{
  ClassAttribute::HandleMap::iterator i = _attributeHandleClassAttributeMap.find(attributeHandle);
  if (i == _attributeHandleClassAttributeMap.end())
    return 0;
  return i.get();
}

ClassAttribute*
ObjectClass::getPrivilegeToDeleteClassAttribute()
{
  // ClassAttribute::HandleMap::iterator i = _attributeHandleClassAttributeMap.begin();
  ClassAttribute::HandleMap::iterator i = _attributeHandleClassAttributeMap.find(AttributeHandle(0));
  if (i == _attributeHandleClassAttributeMap.end())
    return 0;
  OpenRTIAssert(i->getAttributeHandle() == AttributeHandle(0));
  return i.get();
}

void
ObjectClass::insert(ObjectClassModule& objectClassModule)
{
  _objectClassModuleList.push_back(objectClassModule);
}

void
ObjectClass::insert(AttributeDefinitionModule& attributeDefinitionModule)
{
  _attributeDefinitionModuleList.push_back(attributeDefinitionModule);
}

void
ObjectClass::insert(ObjectInstance& objectInstance)
{
  _objectInstanceList.push_back(objectInstance);
}

void
ObjectClass::removeConnect(const ConnectHandle& connectHandle)
{
  for (ClassAttribute::HandleMap::iterator i = _attributeHandleClassAttributeMap.begin();
       i != _attributeHandleClassAttributeMap.end(); ++i)
    i->removeConnect(connectHandle);
}

////////////////////////////////////////////////////////////

Module::Module(Federation& federation) :
  _federation(federation),
  _artificialInteractionRoot(false),
  _artificialObjectRoot(false)
{
}

Module::~Module()
{
  OpenRTIAssert(_dimensionModuleList.empty());
  OpenRTIAssert(_updateRateModuleList.empty());
  OpenRTIAssert(_interactionClassModuleList.empty());
  OpenRTIAssert(_parameterDefinitionModuleList.empty());
  OpenRTIAssert(_objectClassModuleList.empty());
  OpenRTIAssert(_attributeDefinitionModuleList.empty());
}

void
Module::setModuleHandle(const ModuleHandle& moduleHandle)
{
  HandleEntity<Module, ModuleHandle>::_setHandle(moduleHandle);
}

void
Module::setContent(const std::string& content)
{
  _content = content;
}

void
Module::setArtificialInteractionRoot(bool artificialInteractionRoot)
{
  _artificialInteractionRoot = artificialInteractionRoot;
}

void Module::setArtificialObjectRoot(bool artificialObjectRoot)
{
  _artificialObjectRoot = artificialObjectRoot;
}

void
Module::getModule(FOMModule& module) const
{
  module.setModuleHandle(getModuleHandle());
  /// FIXME
  // FOMSwitchList _switchList;
  module.setArtificialInteractionRoot(getArtificialInteractionRoot());
  module.setArtificialObjectRoot(getArtificialObjectRoot());
  module.setContent(getContent());

  module.getDimensionList().reserve(_dimensionModuleList.size());
  for (DimensionModule::FirstList::const_iterator i = _dimensionModuleList.begin();
       i != _dimensionModuleList.end(); ++i) {
    const Dimension& dimension = i->getDimension();
    module.getDimensionList().push_back(FOMDimension());
    FOMDimension& fomDimension = module.getDimensionList().back();
    fomDimension.setName(dimension.getName());
    fomDimension.setDimensionHandle(dimension.getDimensionHandle());
    fomDimension.setUpperBound(dimension.getUpperBound());
  }

  module.getUpdateRateList().reserve(_updateRateModuleList.size());
  for (UpdateRateModule::FirstList::const_iterator i = _updateRateModuleList.begin();
       i != _updateRateModuleList.end(); ++i) {
    const UpdateRate& updateRate = i->getUpdateRate();
    module.getUpdateRateList().push_back(FOMUpdateRate());
    FOMUpdateRate& fomUpdateRate = module.getUpdateRateList().back();
    fomUpdateRate.setName(updateRate.getName());
    fomUpdateRate.setUpdateRateHandle(updateRate.getUpdateRateHandle());
    fomUpdateRate.setRate(updateRate.getRate());
  }

  {
    module.getInteractionClassList().reserve(_interactionClassModuleList.size());
    ParameterDefinitionModule::FirstList::const_iterator j = _parameterDefinitionModuleList.begin();
    for (InteractionClassModule::FirstList::const_iterator i = _interactionClassModuleList.begin();
         i != _interactionClassModuleList.end(); ++i) {
      const InteractionClass& interactionClass = i->getInteractionClass();
      module.getInteractionClassList().push_back(FOMInteractionClass());
      FOMInteractionClass& fomInteractionClass = module.getInteractionClassList().back();
      fomInteractionClass.setName(interactionClass.getName().back());
      fomInteractionClass.setInteractionClassHandle(interactionClass.getInteractionClassHandle());
      fomInteractionClass.setParentInteractionClassHandle(interactionClass.getParentInteractionClassHandle());
      fomInteractionClass.setOrderType(interactionClass.getOrderType());
      fomInteractionClass.setTransportationType(interactionClass.getTransportationType());
      fomInteractionClass.setDimensionHandleSet(interactionClass._dimensionHandleSet);

      // Check if we also reference the parameters with this module
      if (j == _parameterDefinitionModuleList.end())
        continue;
      if (interactionClass.getInteractionClassHandle() != j->getInteractionClass().getInteractionClassHandle())
        continue;
      // If so, add them too
      ++j;
      fomInteractionClass.getParameterList().reserve(interactionClass.getNumParameterDefinitions());
      for (ParameterDefinition::HandleMap::const_iterator k = interactionClass.getParameterHandleParameterMap().begin();
           k != interactionClass.getParameterHandleParameterMap().end(); ++k) {
        fomInteractionClass.getParameterList().push_back(FOMParameter());
        FOMParameter& fomParameter = fomInteractionClass.getParameterList().back();
        fomParameter.setName(k->getName());
        fomParameter.setParameterHandle(k->getParameterHandle());
      }
    }
  }

  {
    module.getObjectClassList().reserve(_objectClassModuleList.size());
    AttributeDefinitionModule::FirstList::const_iterator j = _attributeDefinitionModuleList.begin();
    for (ObjectClassModule::FirstList::const_iterator i = _objectClassModuleList.begin();
         i != _objectClassModuleList.end(); ++i) {
      const ObjectClass& objectClass = i->getObjectClass();
      module.getObjectClassList().push_back(FOMObjectClass());
      FOMObjectClass& fomObjectClass = module.getObjectClassList().back();
      fomObjectClass.setName(objectClass.getName().back());
      fomObjectClass.setObjectClassHandle(objectClass.getObjectClassHandle());
      fomObjectClass.setParentObjectClassHandle(objectClass.getParentObjectClassHandle());

      // Check if we also reference the attributes with this module
      if (j == _attributeDefinitionModuleList.end())
        continue;
      if (objectClass.getObjectClassHandle() != j->getObjectClass().getObjectClassHandle())
        continue;
      // If so, add them too
      ++j;
      fomObjectClass.getAttributeList().reserve(objectClass.getNumAttributeDefinitions());
      for (AttributeDefinition::HandleMap::const_iterator k = objectClass.getAttributeHandleAttributeDefinitionMap().begin();
           k != objectClass.getAttributeHandleAttributeDefinitionMap().end(); ++k) {
        fomObjectClass.getAttributeList().push_back(FOMAttribute());
        FOMAttribute& fomAttribute = fomObjectClass.getAttributeList().back();
        fomAttribute.setName(k->getName());
        fomAttribute.setAttributeHandle(k->getAttributeHandle());
        fomAttribute.setOrderType(k->getOrderType());
        fomAttribute.setTransportationType(k->getTransportationType());
        fomAttribute.setDimensionHandleSet(k->_dimensionHandleSet);
      }
    }
  }
}

DimensionModule*
Module::insert(Dimension& dimension)
{
  DimensionModule* dimensionModule;
  dimensionModule = new DimensionModule(dimension, *this);
  dimension.insert(*dimensionModule);
  insert(*dimensionModule);
  return dimensionModule;
}

UpdateRateModule*
Module::insert(UpdateRate& updateRate)
{
  UpdateRateModule* updateRateModule;
  updateRateModule = new UpdateRateModule(updateRate, *this);
  updateRate.insert(*updateRateModule);
  insert(*updateRateModule);
  return updateRateModule;
}

InteractionClassModule*
Module::insert(InteractionClass& interactionClass)
{
  InteractionClassModule* interactionClassModule;
  interactionClassModule = new InteractionClassModule(interactionClass, *this);
  interactionClass.insert(*interactionClassModule);
  insert(*interactionClassModule);
  return interactionClassModule;
}

ParameterDefinitionModule*
Module::insertParameters(InteractionClass& interactionClass)
{
  ParameterDefinitionModule* parameterDefinitionModule;
  parameterDefinitionModule = new ParameterDefinitionModule(interactionClass, *this);
  interactionClass.insert(*parameterDefinitionModule);
  insert(*parameterDefinitionModule);
  return parameterDefinitionModule;
}

ObjectClassModule*
Module::insert(ObjectClass& objectClass)
{
  ObjectClassModule* objectClassModule;
  objectClassModule = new ObjectClassModule(objectClass, *this);
  objectClass.insert(*objectClassModule);
  insert(*objectClassModule);
  return objectClassModule;
}

AttributeDefinitionModule*
Module::insertAttributes(ObjectClass& objectClass)
{
  AttributeDefinitionModule* attributeDefinitionModule;
  attributeDefinitionModule = new AttributeDefinitionModule(objectClass, *this);
  objectClass.insert(*attributeDefinitionModule);
  insert(*attributeDefinitionModule);
  return attributeDefinitionModule;
}

////////////////////////////////////////////////////////////

FederationConnect::FederationConnect(Federation& federation, NodeConnect& nodeConnect) :
  _federation(federation),
  _nodeConnect(nodeConnect),
  _active(false),
  _permitTimeRegulation(true)
{
  HandleMap::Hook::setKey(nodeConnect.getConnectHandle());
}

FederationConnect::~FederationConnect()
{
  /// FIXME
  _objectInstanceConnectList.clear();
  _timeRegulatingFederateList.unlink();
}

const FederationHandle&
FederationConnect::getFederationHandle() const
{
  return _federation.getFederationHandle();
}

const ConnectHandle&
FederationConnect::getConnectHandle() const
{
  // return _nodeConnect.getConnectHandle();
  return HandleMap::Hook::getKey();
}

bool
FederationConnect::getIsParentConnect() const
{
  return _nodeConnect.getIsParentConnect();
}

bool
FederationConnect::getHasFederates() const
{
  return !_federateList.empty();
}

bool
FederationConnect::getActive() const
{
  return _active;
}

void
FederationConnect::setActive(bool active)
{
  _active = active;
}

bool
FederationConnect::getPermitTimeRegulation() const
{
  if (_permitTimeRegulation)
    return true;
  return getIsParentConnect();
}

void
FederationConnect::setPermitTimeRegulation(bool permitTimeRegulation)
{
  _permitTimeRegulation = permitTimeRegulation;
}

bool
FederationConnect::getIsTimeRegulating() const
{
  OpenRTIAssert(_timeRegulatingFederateList.empty() != SecondList::Hook::is_linked());
  OpenRTIAssert(!SecondList::Hook::is_linked() || _permitTimeRegulation);
  return SecondList::Hook::is_linked();
}

void
FederationConnect::insertTimeRegulating(Federate& federate)
{
  _timeRegulatingFederateList.push_back(federate);
}

void
FederationConnect::eraseTimeRegulating(Federate& federate)
{
  _timeRegulatingFederateList.unlink(federate);
}

void
FederationConnect::send(const SharedPtr<const AbstractMessage>& message)
{
  if (!_active)
    return;
  _nodeConnect.send(message);
}

////////////////////////////////////////////////////////////

Federation::Federation(Node& serverNode) :
  _serverNode(serverNode),
  _objectInstanceHandleObjectInstanceMap(16384/*hash size*/)
{
}

Federation::~Federation()
{
  // In case of an exception tis can be non empty
  _objectInstanceHandleObjectInstanceMap.clear();

  // There is no need that this gets cleaned up. We can have open synchronizations ...
  // OpenRTIAssert(_synchronizationNameSynchronizationMap.empty());
  _synchronizationNameSynchronizationMap.clear();

  _federateHandleFederateMap.clear();
  OpenRTIAssert(_federateHandleFederateMap.empty());

  OpenRTIAssert(_timeRegulatingFederationConnectList.empty());

  // Throw away the base modules
  while (!_moduleHandleModuleMap.empty()) {
    Federation::erase(_moduleHandleModuleMap.back());
  }

  OpenRTIAssert(!hasChildConnects());
  OpenRTIAssert(_connectHandleFederationConnectMap.size() <= 1);
  _connectHandleFederationConnectMap.clear();
  OpenRTIAssert(_moduleHandleModuleMap.empty());
  OpenRTIAssert(_dimensionNameDimensionMap.empty());
  OpenRTIAssert(_dimensionHandleDimensionMap.empty());
  OpenRTIAssert(_updateRateNameUpdateRateMap.empty());
  OpenRTIAssert(_updateRateHandleUpdateRateMap.empty());
  OpenRTIAssert(_interactionClassNameInteractionClassMap.empty());
  OpenRTIAssert(_interactionClassHandleInteractionClassMap.empty());
  OpenRTIAssert(_objectClassNameObjectClassMap.empty());
  OpenRTIAssert(_objectClassHandleObjectClassMap.empty());
}

const std::string&
Federation::getServerName() const
{
  return getServerNode().getServerName();
}

const std::string&
Federation::getServerPath() const
{
  return getServerNode().getServerPath();
}

bool
Federation::isRootServer() const
{
  return getServerNode().isRootServer();
}

bool
Federation::isParentConnect(const ConnectHandle& connectHandle) const
{
  return getServerNode().isParentConnect(connectHandle);
}

bool
Federation::hasChildConnects() const
{
  for (FederationConnect::HandleMap::const_reverse_iterator i = _connectHandleFederationConnectMap.rbegin();
       i != _connectHandleFederationConnectMap.rend(); ++i) {
    if (i->getIsParentConnect())
      continue;
    return true;
  }
  return false;
}

bool
Federation::hasChildConnect(const ConnectHandle& connectHandle)
{
  FederationConnect::HandleMap::iterator i = _connectHandleFederationConnectMap.find(connectHandle);
  if (i == _connectHandleFederationConnectMap.end())
    return false;
  return !i->getIsParentConnect();
}

bool
Federation::hasJoinedFederates() const
{
  OpenRTIAssert(_federateHandleFederateMap.empty() == (!_federateHandleAllocator.used()));
  return !_federateHandleFederateMap.empty();
}

bool
Federation::hasJoinedChildren()
{
  for (ServerModel::Federate::HandleMap::const_iterator i = getFederateHandleFederateMap().begin();
       i != getFederateHandleFederateMap().end(); ++i) {
    FederationConnect* federationConnect = i->getFederationConnect();
    if (!federationConnect)
      continue;
    if (federationConnect->getIsParentConnect())
      continue;
    // Ok, not even the still pending resign request for invalid connect handles is treated as valid child.
    if (i->getResignPending())
      continue;
    return true;
  }
  return false;
}

void
Federation::setName(const std::string& name)
{
  HandleStringEntity<Federation, FederationHandle>::_setString(name);
}

void
Federation::setFederationHandle(const FederationHandle& federationHandle)
{
  HandleStringEntity<Federation, FederationHandle>::_setHandle(federationHandle);
}

void
Federation::setLogicalTimeFactoryName(const std::string& logicalTimeFactoryName)
{
  _logicalTimeFactoryName = logicalTimeFactoryName;
}

FederationConnect*
Federation::getFederationConnect(const ConnectHandle& connectHandle)
{
  FederationConnect::HandleMap::iterator i = _connectHandleFederationConnectMap.find(connectHandle);
  if (i == _connectHandleFederationConnectMap.end())
    return 0;
  return i.get();
}

void
Federation::removeConnect(const ConnectHandle& connectHandle)
{
  for (ObjectInstance::HandleMap::iterator i = _objectInstanceHandleObjectInstanceMap.begin();
       i != _objectInstanceHandleObjectInstanceMap.end(); ++i) {
    i->removeConnect(connectHandle);
  }
  for (ObjectClass::HandleMap::iterator i = _objectClassHandleObjectClassMap.begin();
       i != _objectClassHandleObjectClassMap.end(); ++i) {
    i->removeConnect(connectHandle);
  }
  for (InteractionClass::HandleMap::iterator i = _interactionClassHandleInteractionClassMap.begin();
       i != _interactionClassHandleInteractionClassMap.end(); ++i) {
    i->removeConnect(connectHandle);
  }
}

void
Federation::send(const ConnectHandle& connectHandle, const SharedPtr<const AbstractMessage>& message)
{
  FederationConnect::HandleMap::iterator i = _connectHandleFederationConnectMap.find(connectHandle);
  if (i == _connectHandleFederationConnectMap.end())
    return;
  i->send(message);
}

void
Federation::send(const FederateHandle& federateHandle, const SharedPtr<const AbstractMessage>& message)
{
  Federate::HandleMap::iterator i = _federateHandleFederateMap.find(federateHandle);
  if (i == _federateHandleFederateMap.end())
    return;
  i->send(message);
}

void
Federation::sendToParent(const SharedPtr<const AbstractMessage>& message)
{
  _serverNode.sendToParent(message);
}

void
Federation::broadcast(const SharedPtr<const AbstractMessage>& message)
{
  for (FederationConnect::HandleMap::iterator i = _connectHandleFederationConnectMap.begin();
       i != _connectHandleFederationConnectMap.end(); ++i) {
    i->send(message);
  }
}

void
Federation::broadcast(const ConnectHandle& connectHandle, const SharedPtr<const AbstractMessage>& message)
{
  for (FederationConnect::HandleMap::iterator i = _connectHandleFederationConnectMap.begin();
       i != _connectHandleFederationConnectMap.end(); ++i) {
    if (i->getConnectHandle() == connectHandle)
      continue;
    i->send(message);
  }
}

void
Federation::broadcastToChildren(const SharedPtr<const AbstractMessage>& message)
{
  for (FederationConnect::HandleMap::iterator i = _connectHandleFederationConnectMap.begin();
       i != _connectHandleFederationConnectMap.end(); ++i) {
    if (i->getIsParentConnect())
      continue;
    i->send(message);
  }
}

void
Federation::broadcastToChildren(const ConnectHandle& connectHandle, const SharedPtr<const AbstractMessage>& message)
{
  for (FederationConnect::HandleMap::iterator i = _connectHandleFederationConnectMap.begin();
       i != _connectHandleFederationConnectMap.end(); ++i) {
    if (i->getIsParentConnect())
      continue;
    if (i->getConnectHandle() == connectHandle)
      continue;
    i->send(message);
  }
}

void
Federation::insert(Dimension& dimension)
{
  _dimensionNameDimensionMap.insert(dimension);
  _dimensionHandleDimensionMap.insert(dimension);
}

void
Federation::insert(UpdateRate& updateRate)
{
  _updateRateNameUpdateRateMap.insert(updateRate);
  _updateRateHandleUpdateRateMap.insert(updateRate);
}

void
Federation::insert(InteractionClass& interactionClass)
{
  _interactionClassNameInteractionClassMap.insert(interactionClass);
  _interactionClassHandleInteractionClassMap.insert(interactionClass);
}

void
Federation::insert(ObjectClass& objectClass)
{
  _objectClassNameObjectClassMap.insert(objectClass);
  _objectClassHandleObjectClassMap.insert(objectClass);
}

void
Federation::insert(Module& module)
{
  OpenRTIAssert(_moduleHandleModuleMap.find(module.getModuleHandle()) == _moduleHandleModuleMap.end());
  _moduleHandleModuleMap.insert(module);
}

bool
Federation::insertOrCheck(Module& module, const FOMStringDimension& stringDimension)
{
  Dimension::NameMap::iterator i;
  i = _dimensionNameDimensionMap.find(stringDimension.getName());
  if (i != _dimensionNameDimensionMap.end()) {
    if (stringDimension.getUpperBound() != i->getUpperBound()) {
      std::stringstream ss;
      ss << "Inconsistent Dimension \"" << stringDimension.getName() << "\": Upper bound "
         << stringDimension.getUpperBound() << " does not match the already established value of "
         << i->getUpperBound() << "!";
      throw InconsistentFDD(ss.str());
    }
    module.insert(*i);
    return false;
  } else {
    Dimension* dimension = new Dimension(*this);
    dimension->setName(stringDimension.getName());
    dimension->setDimensionHandle(_dimensionHandleAllocator.get());
    dimension->setUpperBound(stringDimension.getUpperBound());
    insert(*dimension);
    module.insert(*dimension);
    return true;
  }
}

bool
Federation::insertOrCheck(Module& module, const FOMStringUpdateRate& stringUpdateRate)
{
  UpdateRate::NameMap::iterator i;
  i = _updateRateNameUpdateRateMap.find(stringUpdateRate.getName());
  if (i != _updateRateNameUpdateRateMap.end()) {
    if (stringUpdateRate.getRate() != i->getRate()) {
      std::stringstream ss;
      ss << "Inconsistent UpdateRate \"" << stringUpdateRate.getName() << "\": Rate "
         << stringUpdateRate.getRate() << " does not match the already established value of "
         << i->getRate() << "!";
      throw InconsistentFDD(ss.str());
    }
    module.insert(*i);
    return false;
  } else {
    UpdateRate* updateRate = new UpdateRate(*this);
    updateRate->setName(stringUpdateRate.getName());
    updateRate->setUpdateRateHandle(_updateRateHandleAllocator.get());
    updateRate->setRate(stringUpdateRate.getRate());
    insert(*updateRate);
    module.insert(*updateRate);
    return true;
  }
}

bool
Federation::insertOrCheck(Module& module, const FOMStringInteractionClass& stringInteractionClass)
{
  InteractionClass::NameMap::iterator i;
  i = _interactionClassNameInteractionClassMap.find(stringInteractionClass.getName());
  if (i != _interactionClassNameInteractionClassMap.end()) {
    OpenRTIAssert(!i->getName().empty());

    if (i->getOrderType() != resolveOrderType(stringInteractionClass.getOrderType())) {
      std::stringstream ss;
      ss << "Inconsistent InteractionClass \"" << stringInteractionClass.getName() << "\": OrderType "
         << stringInteractionClass.getOrderType() << " does not match the already established value of "
         << i->getOrderType() << "!";
      throw InconsistentFDD(ss.str());
    }
    if (i->getTransportationType() != resolveTransportationType(stringInteractionClass.getTransportationType())) {
      std::stringstream ss;
      ss << "Inconsistent InteractionClass \"" << stringInteractionClass.getName() << "\": TransportationType "
         << stringInteractionClass.getTransportationType() << " does not match the already established value of "
         << i->getTransportationType() << "!";
      throw InconsistentFDD(ss.str());
    }

    DimensionHandleSet dimensionHandleSet;
    for (StringSet::const_iterator j = stringInteractionClass.getDimensionSet().begin();
         j != stringInteractionClass.getDimensionSet().end(); ++j) {
      Dimension* dimension = resolveDimension(*j);
      if (!dimension)
        throw InconsistentFDD("Cannot resolve dimension \"" + *j + "\"!");
      /// FIXME avoid using the handle sets ...
      dimensionHandleSet.insert(dimension->getDimensionHandle());
    }
    if (i->_dimensionHandleSet != dimensionHandleSet) {
      std::stringstream ss;
      ss << "Inconsistent InteractionClass \"" << stringInteractionClass.getName()
         << "\": Dimensions do not match the already established value!";
      throw InconsistentFDD(ss.str());
    }

    // In this case we need to check for the parameter list being the same on both ends.
    if (!stringInteractionClass.getParameterList().empty()) {
      std::vector<ParameterHandle> parameterHandles;
      parameterHandles.reserve(stringInteractionClass.getParameterList().size());
      for (FOMStringParameterList::const_iterator j = stringInteractionClass.getParameterList().begin();
           j != stringInteractionClass.getParameterList().end(); ++j) {
        ParameterDefinition* parameterDefinition;
        parameterDefinition = i->getParameterDefinition(j->getName());
        if (!parameterDefinition) {
          std::stringstream ss;
          ss << "Inconsistent InteractionClass \"" << stringInteractionClass.getName() << "\": Parameter \""
             << j->getName() << "\" not present in already established interaction class!";
          throw InconsistentFDD(ss.str());
        }
        parameterHandles.push_back(parameterDefinition->getParameterHandle());
      }
      std::sort(parameterHandles.begin(), parameterHandles.end());
      if (std::unique(parameterHandles.begin(), parameterHandles.end()) != parameterHandles.end()) {
        std::stringstream ss;
        ss << "Inconsistent InteractionClass \"" << stringInteractionClass.getName()
           << "\": Parameters do not match the already established value!";
        throw InconsistentFDD(ss.str());
      }
    }

    module.insert(*i);
    if (!stringInteractionClass.getParameterList().empty())
      module.insertParameters(*i);

    return false;
  } else {
    InteractionClass* parentInteractionClass;
    parentInteractionClass = resolveParentInteractionClass(stringInteractionClass.getName());
    // This signals an error in message preparation
    if (!parentInteractionClass && 1 < stringInteractionClass.getName().size())
      throw MessageError("Cannot resolve parent interaction class name!");
    InteractionClass* interactionClass = new InteractionClass(*this, parentInteractionClass);
    interactionClass->setName(stringInteractionClass.getName());
    interactionClass->setInteractionClassHandle(_interactionClassHandleAllocator.get());
    insert(*interactionClass);
    module.insert(*interactionClass);

    interactionClass->setOrderType(resolveOrderType(stringInteractionClass.getOrderType()));
    interactionClass->setTransportationType(resolveTransportationType(stringInteractionClass.getTransportationType()));
    for (StringSet::const_iterator i = stringInteractionClass.getDimensionSet().begin();
         i != stringInteractionClass.getDimensionSet().end(); ++i) {
      Dimension* dimension = resolveDimension(*i);
      if (!dimension)
        throw InconsistentFDD("Cannot resolve dimension \"" + *i + "\"!");
      /// FIXME avoid using the handle sets ...
      interactionClass->_dimensionHandleSet.insert(dimension->getDimensionHandle());
    }

    if (!stringInteractionClass.getParameterList().empty())
      module.insertParameters(*interactionClass);

    ParameterHandle nextParameterHandle = interactionClass->getFirstUnusedParameterHandle();
    for (FOMStringParameterList::const_iterator i = stringInteractionClass.getParameterList().begin();
         i != stringInteractionClass.getParameterList().end(); ++i) {
      if (interactionClass->getParameterDefinition(i->getName())) {
        std::stringstream ss;
        ss << "Duplicate parameter name \"" << i->getName() << "\" in InteractionClass \""
           << interactionClass->getName() << "\"!";
        throw InconsistentFDD(ss.str());
      }

      ParameterDefinition* parameterDefinition;
      parameterDefinition = new ParameterDefinition(*interactionClass);
      parameterDefinition->setName(i->getName());
      parameterDefinition->setParameterHandle(nextParameterHandle);
      interactionClass->insert(*parameterDefinition);

      nextParameterHandle = ParameterHandle(nextParameterHandle.getHandle() + 1);
    }

    return true;
  }
}

bool
Federation::insertOrCheck(Module& module, const FOMStringObjectClass& stringObjectClass)
{
  ObjectClass::NameMap::iterator i;
  i = _objectClassNameObjectClassMap.find(stringObjectClass.getName());
  if (i != _objectClassNameObjectClassMap.end()) {
    OpenRTIAssert(!i->getName().empty());

    // In this case we need to check for the attribute list being the same on both ends.
    if (!stringObjectClass.getAttributeList().empty()) {
      std::vector<AttributeHandle> attributeHandles;
      attributeHandles.reserve(stringObjectClass.getAttributeList().size());
      for (FOMStringAttributeList::const_iterator j = stringObjectClass.getAttributeList().begin();
           j != stringObjectClass.getAttributeList().end(); ++j) {
        AttributeDefinition* attributeDefinition;
        attributeDefinition = i->getAttributeDefinition(j->getName());
        if (!attributeDefinition) {
          std::stringstream ss;
          ss << "Inconsistent ObjectClass \"" << stringObjectClass.getName() << "\": Attribute \""
             << j->getName() << "\" not present in already established object class!";
          throw InconsistentFDD(ss.str());
        }
        if (attributeDefinition->getOrderType() != resolveOrderType(j->getOrderType())) {
          std::stringstream ss;
          ss << "Inconsistent ObjectClass  \"" << stringObjectClass.getName() << "\": Attribute \""
             << j->getName() << "\": OrderType " << j->getOrderType()
             << " does not match the already established value of "
             << attributeDefinition->getOrderType() << "!";
          throw InconsistentFDD(ss.str());
        }
        if (attributeDefinition->getTransportationType() != resolveTransportationType(j->getTransportationType())) {
          std::stringstream ss;
          ss << "Inconsistent ObjectClass  \"" << stringObjectClass.getName() << "\": Attribute \""
             << j->getName() << "\": TransportationType " << j->getTransportationType()
             << " does not match the already established value of "
             << attributeDefinition->getTransportationType() << "!";
          throw InconsistentFDD(ss.str());
        }

        DimensionHandleSet dimensionHandleSet;
        for (StringSet::const_iterator k = j->getDimensionSet().begin();
             k != j->getDimensionSet().end(); ++k) {
          Dimension* dimension = resolveDimension(*k);
          if (!dimension)
            throw InconsistentFDD("Cannot resolve dimension \"" + *k + "\"!");
          /// FIXME avoid using the handle sets ...
          dimensionHandleSet.insert(dimension->getDimensionHandle());
        }
        if (attributeDefinition->_dimensionHandleSet != dimensionHandleSet) {
          std::stringstream ss;
          ss << "Inconsistent ObjectClass  \"" << stringObjectClass.getName() << "\": Attribute \""
             << j->getName() << "\": Dimensions do not match the already established value!";
          throw InconsistentFDD(ss.str());
        }

        attributeHandles.push_back(attributeDefinition->getAttributeHandle());
      }
      std::sort(attributeHandles.begin(), attributeHandles.end());
      if (std::unique(attributeHandles.begin(), attributeHandles.end()) != attributeHandles.end()) {
        std::stringstream ss;
        ss << "Inconsistent ObjectClass \"" << stringObjectClass.getName()
           << "\": Attributes do not match the already established value!";
        throw InconsistentFDD(ss.str());
      }
    }

    module.insert(*i);
    if (!stringObjectClass.getAttributeList().empty())
      module.insertAttributes(*i);

    return false;
  } else {
    ObjectClass* parentObjectClass;
    parentObjectClass = resolveParentObjectClass(stringObjectClass.getName());
    // This signals an error in message preparation
    if (!parentObjectClass && 1 < stringObjectClass.getName().size())
      throw MessageError("Cannot resolve parent object class name!");
    ObjectClass* objectClass = new ObjectClass(*this, parentObjectClass);
    objectClass->setName(stringObjectClass.getName());
    objectClass->setObjectClassHandle(_objectClassHandleAllocator.get());
    insert(*objectClass);
    module.insert(*objectClass);

    if (!stringObjectClass.getAttributeList().empty())
      module.insertAttributes(*objectClass);

    AttributeHandle nextAttributeHandle = objectClass->getFirstUnusedAttributeHandle();
    for (FOMStringAttributeList::const_iterator i = stringObjectClass.getAttributeList().begin();
         i != stringObjectClass.getAttributeList().end(); ++i) {
      if (objectClass->getAttributeDefinition(i->getName())) {
        std::stringstream ss;
        ss << "Duplicate attribute name \"" << i->getName() << "\" in ObjectClass \""
           << objectClass->getName() << "\"!";
        throw InconsistentFDD(ss.str());
      }

      AttributeDefinition* attributeDefinition;
      attributeDefinition = new AttributeDefinition(*objectClass);
      attributeDefinition->setName(i->getName());
      attributeDefinition->setAttributeHandle(nextAttributeHandle);
      objectClass->insert(*attributeDefinition);

      attributeDefinition->setOrderType(resolveOrderType(i->getOrderType()));
      attributeDefinition->setTransportationType(resolveTransportationType(i->getTransportationType()));
      for (StringSet::const_iterator j = i->getDimensionSet().begin();
           j != i->getDimensionSet().end(); ++j) {
        Dimension* dimension = resolveDimension(*j);
        if (!dimension)
          throw InconsistentFDD("Cannot resolve dimension \"" + *j + "\"!");
        /// FIXME avoid using the handle sets ...
        attributeDefinition->_dimensionHandleSet.insert(dimension->getDimensionHandle());
      }

      nextAttributeHandle = AttributeHandle(nextAttributeHandle.getHandle() + 1);
    }

    return true;
  }
}

ModuleHandle
Federation::insert(const FOMStringModule& stringModule)
{
  Module* module = new Module(*this);
  module->setModuleHandle(_moduleHandleAllocator.get());
  module->setContent(stringModule.getContent());
  module->setArtificialInteractionRoot(stringModule.getArtificialInteractionRoot());
  module->setArtificialObjectRoot(stringModule.getArtificialObjectRoot());
  insert(*module);

  bool created = false;
  try {

    for (FOMStringDimensionList::const_iterator j = stringModule.getDimensionList().begin();
         j != stringModule.getDimensionList().end(); ++j) {
      if (insertOrCheck(*module, *j))
        created = true;
    }

    for (FOMStringUpdateRateList::const_iterator j = stringModule.getUpdateRateList().begin();
         j != stringModule.getUpdateRateList().end(); ++j) {
      if (insertOrCheck(*module, *j))
        created = true;
    }

    for (FOMStringInteractionClassList::const_iterator j = stringModule.getInteractionClassList().begin();
         j != stringModule.getInteractionClassList().end(); ++j) {
      if (insertOrCheck(*module, *j))
        created = true;
    }

    for (FOMStringObjectClassList::const_iterator j = stringModule.getObjectClassList().begin();
         j != stringModule.getObjectClassList().end(); ++j) {
      if (insertOrCheck(*module, *j))
        created = true;
    }

    if (!created) {
      erase(*module);
      return ModuleHandle();
    }

    return module->getModuleHandle();
  } catch (...) {
    erase(*module);
    throw;
  }

  return ModuleHandle();
}

void
Federation::insert(const FOMStringModuleList& stringModuleList)
{
  ModuleHandleVector moduleHandleVector;
  insert(moduleHandleVector, stringModuleList);
}

void
Federation::insert(ModuleHandleVector& moduleHandleVector, const FOMStringModuleList& stringModuleList)
{
  moduleHandleVector.reserve(stringModuleList.size());
  try {
    for (FOMStringModuleList::const_iterator i = stringModuleList.begin(); i != stringModuleList.end(); ++i) {
      ModuleHandle moduleHandle = insert(*i);
      if (moduleHandle.valid())
        moduleHandleVector.push_back(moduleHandle);
    }
  } catch (...) {
    for (ModuleHandleVector::iterator i = moduleHandleVector.begin(); i != moduleHandleVector.end(); ++i)
      erase(*i);
    moduleHandleVector.clear();
    throw;
  }
}

void
Federation::insert(Module& module, const FOMDimension& fomDimension)
{
  Dimension::HandleMap::iterator i = _dimensionHandleDimensionMap.find(fomDimension.getDimensionHandle());
  if (i != _dimensionHandleDimensionMap.end()) {
    if (fomDimension.getName() != i->getName())
      throw MessageError("Dimension name does not match.");
    if (fomDimension.getUpperBound() != i->getUpperBound())
      throw MessageError("Dimension upper bound does not match.");
    module.insert(*i);
  } else {
    _dimensionHandleAllocator.take(fomDimension.getDimensionHandle());
    Dimension* dimension = new Dimension(*this);
    dimension->setName(fomDimension.getName());
    dimension->setDimensionHandle(fomDimension.getDimensionHandle());
    dimension->setUpperBound(fomDimension.getUpperBound());
    insert(*dimension);
    module.insert(*dimension);
  }
}

void
Federation::insert(Module& module, const FOMUpdateRate& fomUpdateRate)
{
  UpdateRate::HandleMap::iterator i = _updateRateHandleUpdateRateMap.find(fomUpdateRate.getUpdateRateHandle());
  if (i != _updateRateHandleUpdateRateMap.end()) {
    if (fomUpdateRate.getName() != i->getName())
      throw MessageError("UpdateRate name does not match.");
    if (fomUpdateRate.getRate() != i->getRate())
      throw MessageError("UpdateRate rate does not match.");
    module.insert(*i);
  } else {
    _updateRateHandleAllocator.take(fomUpdateRate.getUpdateRateHandle());
    UpdateRate* updateRate = new UpdateRate(*this);
    updateRate->setName(fomUpdateRate.getName());
    updateRate->setUpdateRateHandle(fomUpdateRate.getUpdateRateHandle());
    updateRate->setRate(fomUpdateRate.getRate());
    insert(*updateRate);
    module.insert(*updateRate);
  }
}

void
Federation::insert(Module& module, const FOMInteractionClass& fomInteractionClass)
{
  InteractionClass::HandleMap::iterator i;
  i = _interactionClassHandleInteractionClassMap.find(fomInteractionClass.getInteractionClassHandle());
  if (i != _interactionClassHandleInteractionClassMap.end()) {
    OpenRTIAssert(!i->getName().empty());
    if (fomInteractionClass.getName() != i->getName().back())
      throw MessageError("InteractionClass name does not match.");
    if (fomInteractionClass.getParentInteractionClassHandle() != i->getParentInteractionClassHandle())
      throw MessageError("Parent InteractionClass does not match.");
    if (fomInteractionClass.getOrderType() != i->getOrderType())
      throw MessageError("InteractionClass order type does not match.");
    if (fomInteractionClass.getTransportationType() != i->getTransportationType())
      throw MessageError("InteractionClass transportation type does not match.");
    if (fomInteractionClass.getDimensionHandleSet() != i->_dimensionHandleSet)
      throw MessageError("InteractionClass dimension handle set does not match.");
    // In this case we want to check for the parameter list being the same on both ends.
    if (!fomInteractionClass.getParameterList().empty()) {
      module.insertParameters(*i);
      if (i->getParameterHandleParameterMap().empty()) {
        for (FOMParameterList::const_iterator j = fomInteractionClass.getParameterList().begin();
             j != fomInteractionClass.getParameterList().end(); ++j) {
          if (i->getParameterDefinition(j->getName()))
            throw MessageError("Duplicate InteractionClass parameter names.");
          if (i->getParameterDefinition(j->getParameterHandle()))
            throw MessageError("Duplicate InteractionClass parameter handles.");
          ParameterDefinition* parameterDefinition;
          parameterDefinition = new ParameterDefinition(*i);
          parameterDefinition->setName(j->getName());
          parameterDefinition->setParameterHandle(j->getParameterHandle());
          i->insert(*parameterDefinition);
        }
      } else {
        std::vector<ParameterHandle> parameterHandles;
        parameterHandles.reserve(fomInteractionClass.getParameterList().size());
        for (FOMParameterList::const_iterator j = fomInteractionClass.getParameterList().begin();
             j != fomInteractionClass.getParameterList().end(); ++j) {
          ParameterDefinition* parameterDefinition;
          parameterDefinition = i->getParameterDefinition(j->getParameterHandle());
          if (!parameterDefinition)
            throw MessageError("InteractionClass parameter lists do not match.");
          if (parameterDefinition->getName() != j->getName())
            throw MessageError("InteractionClass parameter lists do not match.");
          parameterHandles.push_back(j->getParameterHandle());
        }
        std::sort(parameterHandles.begin(), parameterHandles.end());
        if (std::unique(parameterHandles.begin(), parameterHandles.end()) != parameterHandles.end())
          throw MessageError("InteractionClass parameter lists do not match.");
      }
    }
    module.insert(*i);
  } else {
    InteractionClass* parentInteractionClass;
    parentInteractionClass = getInteractionClass(fomInteractionClass.getParentInteractionClassHandle());
   if (fomInteractionClass.getParentInteractionClassHandle().valid() && !parentInteractionClass)
      throw MessageError("Invalid parent InteractionClass!");
    StringVector name;
    if (parentInteractionClass)
      name = parentInteractionClass->getName();
    name.push_back(fomInteractionClass.getName());
    _interactionClassHandleAllocator.take(fomInteractionClass.getInteractionClassHandle());
    InteractionClass* interactionClass = new InteractionClass(*this, parentInteractionClass);
    interactionClass->setName(name);
    interactionClass->setInteractionClassHandle(fomInteractionClass.getInteractionClassHandle());
    insert(*interactionClass);
    module.insert(*interactionClass);
    interactionClass->setOrderType(fomInteractionClass.getOrderType());
    interactionClass->setTransportationType(fomInteractionClass.getTransportationType());
    interactionClass->_dimensionHandleSet = fomInteractionClass.getDimensionHandleSet();
    if (!fomInteractionClass.getParameterList().empty())
      module.insertParameters(*interactionClass);
    for (FOMParameterList::const_iterator j = fomInteractionClass.getParameterList().begin();
         j != fomInteractionClass.getParameterList().end(); ++j) {
      if (interactionClass->getParameterDefinition(j->getName()))
        throw MessageError("Duplicate InteractionClass parameter names.");
      if (interactionClass->getParameterDefinition(j->getParameterHandle()))
        throw MessageError("Duplicate InteractionClass parameter handles.");
      ParameterDefinition* parameterDefinition;
      parameterDefinition = new ParameterDefinition(*interactionClass);
      parameterDefinition->setName(j->getName());
      parameterDefinition->setParameterHandle(j->getParameterHandle());
      interactionClass->insert(*parameterDefinition);
    }
  }
}

void
Federation::insert(Module& module, const FOMObjectClass& fomObjectClass)
{
  ObjectClass::HandleMap::iterator i;
  i = _objectClassHandleObjectClassMap.find(fomObjectClass.getObjectClassHandle());
  if (i != _objectClassHandleObjectClassMap.end()) {
    OpenRTIAssert(!i->getName().empty());
    if (fomObjectClass.getName() != i->getName().back())
      throw MessageError("ObjectClass name does not match.");
    if (fomObjectClass.getParentObjectClassHandle() != i->getParentObjectClassHandle())
      throw MessageError("Parent ObjectClass does not match.");
    // In this case we want to check for the attribute list being the same on both ends.
    if (!fomObjectClass.getAttributeList().empty()) {
      module.insertAttributes(*i);
      if (i->getAttributeHandleAttributeDefinitionMap().empty()) {
        for (FOMAttributeList::const_iterator j = fomObjectClass.getAttributeList().begin();
             j != fomObjectClass.getAttributeList().end(); ++j) {
          if (i->getAttributeDefinition(j->getName()))
            throw MessageError("Duplicate ObjectClass attribute names.");
          if (i->getAttributeDefinition(j->getAttributeHandle()))
            throw MessageError("Duplicate ObjectClass attribute handles.");
          AttributeDefinition* attributeDefinition;
          attributeDefinition = new AttributeDefinition(*i);
          attributeDefinition->setName(j->getName());
          attributeDefinition->setAttributeHandle(j->getAttributeHandle());
          i->insert(*attributeDefinition);
          attributeDefinition->setOrderType(j->getOrderType());
          attributeDefinition->setTransportationType(j->getTransportationType());
          attributeDefinition->_dimensionHandleSet = j->getDimensionHandleSet();
        }
      } else {
        std::vector<AttributeHandle> attributeHandles;
        attributeHandles.reserve(fomObjectClass.getAttributeList().size());
        for (FOMAttributeList::const_iterator j = fomObjectClass.getAttributeList().begin();
             j != fomObjectClass.getAttributeList().end(); ++j) {
          AttributeDefinition* attributeDefinition;
          attributeDefinition = i->getAttributeDefinition(j->getAttributeHandle());
          if (!attributeDefinition)
            throw MessageError("ObjectClass attribute lists do not match.");
          if (attributeDefinition->getName() != j->getName())
            throw MessageError("ObjectClass attribute lists do not match.");
          if (attributeDefinition->getOrderType() != j->getOrderType())
            throw MessageError("ObjectClass order type does not match.");
          if (attributeDefinition->getTransportationType() != j->getTransportationType())
            throw MessageError("ObjectClass transportation type does not match.");
          if (attributeDefinition->_dimensionHandleSet != j->getDimensionHandleSet())
            throw MessageError("ObjectClass dimension handle set does not match.");
          attributeHandles.push_back(j->getAttributeHandle());
        }
        std::sort(attributeHandles.begin(), attributeHandles.end());
        if (std::unique(attributeHandles.begin(), attributeHandles.end()) != attributeHandles.end())
          throw MessageError("ObjectClass attribute lists do not match.");
      }
    }
    module.insert(*i);
  } else {
    ObjectClass* parentObjectClass;
    parentObjectClass = getObjectClass(fomObjectClass.getParentObjectClassHandle());
    if (fomObjectClass.getParentObjectClassHandle().valid() && !parentObjectClass)
      throw MessageError("Invalid parent ObjectClass!");
    StringVector name;
    if (parentObjectClass)
      name = parentObjectClass->getName();
    name.push_back(fomObjectClass.getName());
    _objectClassHandleAllocator.take(fomObjectClass.getObjectClassHandle());
    ObjectClass* objectClass = new ObjectClass(*this, parentObjectClass);
    objectClass->setName(name);
    objectClass->setObjectClassHandle(fomObjectClass.getObjectClassHandle());
    insert(*objectClass);
    module.insert(*objectClass);
    if (!fomObjectClass.getAttributeList().empty())
      module.insertAttributes(*objectClass);
    for (FOMAttributeList::const_iterator j = fomObjectClass.getAttributeList().begin();
         j != fomObjectClass.getAttributeList().end(); ++j) {
      if (objectClass->getAttributeDefinition(j->getName()))
        throw MessageError("Duplicate ObjectClass attribute names.");
      if (objectClass->getAttributeDefinition(j->getAttributeHandle()))
        throw MessageError("Duplicate ObjectClass attribute handles.");
      AttributeDefinition* attributeDefinition;
      attributeDefinition = new AttributeDefinition(*objectClass);
      attributeDefinition->setName(j->getName());
      attributeDefinition->setAttributeHandle(j->getAttributeHandle());
      objectClass->insert(*attributeDefinition);
      attributeDefinition->setOrderType(j->getOrderType());
      attributeDefinition->setTransportationType(j->getTransportationType());
      attributeDefinition->_dimensionHandleSet = j->getDimensionHandleSet();
    }
  }
}

void
Federation::insert(const FOMModule& fomModule)
{
  Module::HandleMap::iterator i = _moduleHandleModuleMap.find(fomModule.getModuleHandle());
  if (i == _moduleHandleModuleMap.end()) {
    _moduleHandleAllocator.take(fomModule.getModuleHandle());
    Module* module = new Module(*this);
    module->setModuleHandle(fomModule.getModuleHandle());
    module->setContent(fomModule.getContent());
    module->setArtificialInteractionRoot(fomModule.getArtificialInteractionRoot());
    module->setArtificialObjectRoot(fomModule.getArtificialObjectRoot());
    insert(*module);

    for (FOMDimensionList::const_iterator j = fomModule.getDimensionList().begin();
         j != fomModule.getDimensionList().end(); ++j) {
      insert(*module, *j);
    }

    for (FOMUpdateRateList::const_iterator j = fomModule.getUpdateRateList().begin();
         j != fomModule.getUpdateRateList().end(); ++j) {
      insert(*module, *j);
    }

    for (FOMInteractionClassList::const_iterator j = fomModule.getInteractionClassList().begin();
         j != fomModule.getInteractionClassList().end(); ++j) {
      insert(*module, *j);
    }

    for (FOMObjectClassList::const_iterator j = fomModule.getObjectClassList().begin();
         j != fomModule.getObjectClassList().end(); ++j) {
      insert(*module, *j);
    }
  }
}

void
Federation::insert(const FOMModuleList& fomModuleList)
{
  for (FOMModuleList::const_iterator i = fomModuleList.begin(); i != fomModuleList.end(); ++i) {
    insert(*i);
  }
}

void
Federation::erase(const ModuleHandle& moduleHandle)
{
  Module::HandleMap::iterator i = _moduleHandleModuleMap.find(moduleHandle);
  OpenRTIAssert(i != _moduleHandleModuleMap.end());
  erase(*i);
}

void
Federation::erase(Module& module)
{
  while (!module.getParameterDefinitionModuleList().empty()) {
    InteractionClass& interactionClass = module.getParameterDefinitionModuleList().back().getInteractionClass();
    module.getParameterDefinitionModuleList().pop_back();
    if (interactionClass.getAreParametersReferencedByAnyModule())
      continue;
    interactionClass.eraseParameterDefinitions();
  }
  while (!module.getInteractionClassModuleList().empty()) {
    InteractionClass& interactionClass = module.getInteractionClassModuleList().back().getInteractionClass();
    module.getInteractionClassModuleList().pop_back();
    if (interactionClass.getIsReferencedByAnyModule())
      continue;
    _interactionClassHandleAllocator.put(interactionClass.getInteractionClassHandle());
    InteractionClass::HandleMap::erase(interactionClass);
  }

  while (!module.getAttributeDefinitionModuleList().empty()) {
    ObjectClass& objectClass = module.getAttributeDefinitionModuleList().back().getObjectClass();
    module.getAttributeDefinitionModuleList().pop_back();
    if (objectClass.getAreAttributesReferencedByAnyModule())
      continue;
    objectClass.eraseAttributeDefinitions();
  }
  while (!module.getObjectClassModuleList().empty()) {
    ObjectClass& objectClass = module.getObjectClassModuleList().back().getObjectClass();
    module.getObjectClassModuleList().pop_back();
    if (objectClass.getIsReferencedByAnyModule())
      continue;
    _objectClassHandleAllocator.put(objectClass.getObjectClassHandle());
    ObjectClass::HandleMap::erase(objectClass);
  }

  while (!module.getUpdateRateModuleList().empty()) {
    UpdateRate& updateRate = module.getUpdateRateModuleList().back().getUpdateRate();
    module.getUpdateRateModuleList().pop_back();
    if (updateRate.getIsReferencedByAnyModule())
      continue;
    _updateRateHandleAllocator.put(updateRate.getUpdateRateHandle());
    UpdateRate::HandleMap::erase(updateRate);
  }

  while (!module.getDimensionModuleList().empty()) {
    Dimension& dimension = module.getDimensionModuleList().back().getDimension();
    module.getDimensionModuleList().pop_back();
    if (dimension.getIsReferencedByAnyModule())
      continue;
    _dimensionHandleAllocator.put(dimension.getDimensionHandle());
    Dimension::HandleMap::erase(dimension);
  }

  _moduleHandleAllocator.put(module.getModuleHandle());
  Module::HandleMap::erase(module);
}

void
Federation::getModuleList(FOMModuleList& moduleList) const
{
  moduleList.reserve(_moduleHandleModuleMap.size());
  for (Module::HandleMap::const_iterator i = _moduleHandleModuleMap.begin();
       i != _moduleHandleModuleMap.end(); ++i) {
    moduleList.push_back(FOMModule());
    i->getModule(moduleList.back());
  }
}

void
Federation::getModuleList(FOMModuleList& moduleList, const ModuleHandleVector& moduleHandleVector) const
{
  for (ModuleHandleVector::const_iterator i = moduleHandleVector.begin(); i != moduleHandleVector.end(); ++i) {
    Module::HandleMap::const_iterator j = _moduleHandleModuleMap.find(*i);
    OpenRTIAssert(j != _moduleHandleModuleMap.end());
    moduleList.push_back(FOMModule());
    j->getModule(moduleList.back());
  }
}

Module*
Federation::getModule(const ModuleHandle& moduleHandle)
{
  Module::HandleMap::iterator i = _moduleHandleModuleMap.find(moduleHandle);
  if (i == _moduleHandleModuleMap.end())
    return 0;
  return i.get();
}

Dimension*
Federation::getDimension(const DimensionHandle& dimensionHandle)
{
  Dimension::HandleMap::iterator i = _dimensionHandleDimensionMap.find(dimensionHandle);
  if (i == _dimensionHandleDimensionMap.end())
    return 0;
  return i.get();
}

UpdateRate*
Federation::getUpdateRate(const UpdateRateHandle& updateRateHandle)
{
  UpdateRate::HandleMap::iterator i = _updateRateHandleUpdateRateMap.find(updateRateHandle);
  if (i == _updateRateHandleUpdateRateMap.end())
    return 0;
  return i.get();
}

InteractionClass*
Federation::getInteractionClass(const InteractionClassHandle& interactionClassHandle)
{
  InteractionClass::HandleMap::iterator i = _interactionClassHandleInteractionClassMap.find(interactionClassHandle);
  if (i == _interactionClassHandleInteractionClassMap.end())
    return 0;
  return i.get();
}

ObjectClass*
Federation::getObjectClass(const ObjectClassHandle& objectClassHandle)
{
  ObjectClass::HandleMap::iterator i = _objectClassHandleObjectClassMap.find(objectClassHandle);
  if (i == _objectClassHandleObjectClassMap.end())
    return 0;
  return i.get();
}

OrderType
Federation::resolveOrderType(const std::string& orderType)
{
  if (caseCompare(orderType, "timestamp"))
    return TIMESTAMP;
  else if (caseCompare(orderType, "receive"))
    return RECEIVE;
  else if (orderType.empty())
    return TIMESTAMP;
  else
    throw InconsistentFDD(std::string("Unknown order type \"") + orderType + "\".");
}

TransportationType
Federation::resolveTransportationType(const std::string& transportationType)
{
  if (transportationType == "HLAreliable")
    return RELIABLE;
  else if (transportationType == "HLAbestEffort")
    return BEST_EFFORT;
  else if (transportationType.empty())
    return RELIABLE;
  else
    throw InconsistentFDD(std::string("Unknown transportation type \"") + transportationType + "\".");
}

Dimension*
Federation::resolveDimension(const std::string& dimensionName)
{
  Dimension::NameMap::iterator i = _dimensionNameDimensionMap.find(dimensionName);
  if (i == _dimensionNameDimensionMap.end())
    return 0;
  return i.get();
}

UpdateRate*
Federation::resolveUpdateRate(const std::string& updateRateName)
{
  UpdateRate::NameMap::iterator i = _updateRateNameUpdateRateMap.find(updateRateName);
  if (i == _updateRateNameUpdateRateMap.end())
    return 0;
  return i.get();
}

InteractionClass*
Federation::resolveParentInteractionClass(const StringVector& interactionClassName)
{
  StringVector parentInteractionClassName = interactionClassName;
  parentInteractionClassName.pop_back();
  InteractionClass::NameMap::iterator i = _interactionClassNameInteractionClassMap.find(parentInteractionClassName);
  if (i == _interactionClassNameInteractionClassMap.end())
    return 0;
  return i.get();
}

ObjectClass*
Federation::resolveParentObjectClass(const StringVector& objectClassName)
{
  StringVector parentObjectClassName = objectClassName;
  parentObjectClassName.pop_back();
  ObjectClass::NameMap::iterator i = _objectClassNameObjectClassMap.find(parentObjectClassName);
  if (i == _objectClassNameObjectClassMap.end())
    return 0;
  return i.get();
}

bool
Federation::isFederateNameInUse(const std::string& name) const
{
  return _federateNameFederateMap.find(name) != _federateNameFederateMap.end();
}

void
Federation::insert(Federate& federate)
{
  federate.setFederateHandle(_federateHandleAllocator.getOrTake(federate.getFederateHandle()));
  if (federate.getName().empty())
    federate.setName(federate.getFederateHandle().getReservedName("HLAfederate"));
  OpenRTIAssert(!isFederateNameInUse(federate.getName()));
  _federateNameFederateMap.insert(federate);
  _federateHandleFederateMap.insert(federate);
}

void
Federation::erase(const FederateHandle& federateHandle)
{
  Federate::HandleMap::iterator i = _federateHandleFederateMap.find(federateHandle);
  OpenRTIAssert(i != _federateHandleFederateMap.end());
  erase(*i);
}

void
Federation::erase(Federate& federate)
{
  _federateHandleAllocator.put(federate.getFederateHandle());
  Federate::HandleMap::erase(federate);
}

Federate*
Federation::getFederate(const FederateHandle& federateHandle)
{
  Federate::HandleMap::iterator i = _federateHandleFederateMap.find(federateHandle);
  if (i == _federateHandleFederateMap.end())
    return 0;
  return i.get();
}

void
Federation::insertTimeRegulating(Federate& federate)
{
  OpenRTIAssert(!federate.getIsTimeRegulating());
  FederationConnect* federationConnect = federate.getFederationConnect();
  OpenRTIAssert(federationConnect);
  if (!federationConnect->getIsTimeRegulating())
    _timeRegulatingFederationConnectList.push_back(*federationConnect);
  federationConnect->insertTimeRegulating(federate);
}

void
Federation::eraseTimeRegulating(Federate& federate)
{
  OpenRTIAssert(federate.getIsTimeRegulating());
  FederationConnect* federationConnect = federate.getFederationConnect();
  // We can only be time regulating if this exists and provides us with a link
  OpenRTIAssert(federationConnect);
  OpenRTIAssert(federationConnect->getIsTimeRegulating());
  federationConnect->eraseTimeRegulating(federate);
  if (!federationConnect->getTimeRegulatingFederateList().empty())
    return;
  _timeRegulatingFederationConnectList.unlink(*federationConnect);
}

Region*
Federation::getOrCreateRegion(const RegionHandle& regionHandle)
{
  Federate::HandleMap::iterator i = _federateHandleFederateMap.find(regionHandle.getFederateHandle());
  if (i == _federateHandleFederateMap.end())
    return 0;
  Region* region = i->getRegion(regionHandle.getLocalRegionHandle());
  if (region)
    return region;
  region = new Region;
  region->setRegionHandle(regionHandle.getLocalRegionHandle());
  i->insert(*region);
  return region;
}

Region*
Federation::getRegion(const RegionHandle& regionHandle)
{
  Federate::HandleMap::iterator i = _federateHandleFederateMap.find(regionHandle.getFederateHandle());
  if (i == _federateHandleFederateMap.end())
    return 0;
  return i->getRegion(regionHandle.getLocalRegionHandle());
}

void
Federation::insert(ObjectInstance& objectInstance)
{
  objectInstance.setObjectInstanceHandle(_objectInstanceHandleAllocator.getOrTake(objectInstance.getObjectInstanceHandle()));
  if (objectInstance.getName().empty())
    objectInstance.setName(objectInstance.getObjectInstanceHandle().getReservedName("HLAobjectInstance"));
  OpenRTIAssert(_objectInstanceHandleObjectInstanceMap.find(objectInstance.getObjectInstanceHandle()) == _objectInstanceHandleObjectInstanceMap.end());
  OpenRTIAssert(!isObjectInstanceNameInUse(objectInstance.getName()));
  _objectInstanceHandleObjectInstanceMap.insert(objectInstance);
  _objectInstanceNameObjectInstanceMap.insert(objectInstance);
}

ObjectInstance*
Federation::getObjectInstance(const ObjectInstanceHandle& objectInstanceHandle)
{
  ObjectInstance::HandleMap::iterator i = _objectInstanceHandleObjectInstanceMap.find(objectInstanceHandle);
  if (i == _objectInstanceHandleObjectInstanceMap.end())
    return 0;
  return i.get();
}

void
Federation::erase(ObjectInstance& objectInstance)
{
  _objectInstanceHandleAllocator.put(objectInstance.getObjectInstanceHandle());
  ObjectInstance::HandleMap::erase(objectInstance);
}

bool
Federation::isObjectInstanceNameInUse(const std::string& name) const
{
  return _objectInstanceNameObjectInstanceMap.find(name) != _objectInstanceNameObjectInstanceMap.end();
}

ObjectInstance*
Federation::insertObjectInstance(const ObjectInstanceHandle& objectInstanceHandle, const std::string& objectInstanceName)
{
  ObjectInstance* objectInstance = new ObjectInstance(*this);
  objectInstance->setObjectInstanceHandle(objectInstanceHandle);
  objectInstance->setName(objectInstanceName);
  insert(*objectInstance);
  return objectInstance;
}

////////////////////////////////////////////////////////////

NodeConnect::NodeConnect() :
  _isParentConnect(false)
{
}

NodeConnect::~NodeConnect()
{
  // We need to make sure that this list got processed before deletion.
  // FIXME
  // OpenRTIAssert(_federationConnectList.empty());
  _federationConnectList.clear();
}

void
NodeConnect::setConnectHandle(const ConnectHandle& connectHandle)
{
  HandleMap::Hook::setKey(connectHandle);
}

void
NodeConnect::setIsParentConnect(const bool& isParentConnect)
{
  _isParentConnect = isParentConnect;
}

void
NodeConnect::setName(const std::string& name)
{
  _name = name;
}

const SharedPtr<AbstractMessageSender>&
NodeConnect::getMessageSender() const
{
  return _messageSender;
}

void
NodeConnect::setMessageSender(const SharedPtr<AbstractMessageSender>& messageSender)
{
  _messageSender = messageSender;
}

const StringStringListMap&
NodeConnect::getOptions() const
{
  return _options;
}

void
NodeConnect::setOptions(const StringStringListMap& options)
{
  _options = options;

  StringStringListMap::const_iterator i = options.find("serverName");
  if (i != options.end() && !i->second.empty())
    _name = i->second.front();
  else
    _name.clear();
}

void
NodeConnect::send(const SharedPtr<const AbstractMessage>& message)
{
  if (!_messageSender.valid())
    return;
  _messageSender->send(message);
}

////////////////////////////////////////////////////////////

Node::Node() :
  _serverOptions(new ServerOptions)
{
}

Node::~Node()
{
  _federationHandleFederationMap.clear();
  OpenRTIAssert(_federationHandleFederationMap.empty());
  OpenRTIAssert(_federationNameFederationMap.empty());

  _connectHandleNodeConnectMap.clear();
  OpenRTIAssert(_connectHandleNodeConnectMap.empty());
}

bool
Node::hasChildConnects() const
{
  for (NodeConnect::HandleMap::const_reverse_iterator i = _connectHandleNodeConnectMap.rbegin();
       i != _connectHandleNodeConnectMap.rend(); ++i) {
    if (i->getIsParentConnect())
      continue;
    return true;
  }
  return false;
}

bool
Node::isRootServer() const
{
  return !_parentConnectHandle.valid();
}

bool
Node::isIdle() const
{
  return !isRootServer() && !hasChildConnects();
}

NodeConnect*
Node::getNodeConnect(const ConnectHandle& connectHandle)
{
  NodeConnect::HandleMap::iterator i = _connectHandleNodeConnectMap.find(connectHandle);
  if (i == _connectHandleNodeConnectMap.end())
    return 0;
  return i.get();
}

NodeConnect*
Node::insertNodeConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& options)
{
  NodeConnect* nodeConnect = new NodeConnect;
  insert(*nodeConnect);
  nodeConnect->setMessageSender(messageSender);
  nodeConnect->setOptions(options);
  return nodeConnect;
}

NodeConnect*
Node::insertParentNodeConnect(const SharedPtr<AbstractMessageSender>& messageSender, const StringStringListMap& options)
{
  OpenRTIAssert(!_parentConnectHandle.valid());
  NodeConnect* nodeConnect = insertNodeConnect(messageSender, options);
  nodeConnect->setIsParentConnect(true);
  _parentConnectHandle = nodeConnect->getConnectHandle();
  return nodeConnect;
}

void
Node::insert(NodeConnect& nodeConnect)
{
  nodeConnect.setConnectHandle(_connectHandleAllocator.getOrTake(nodeConnect.getConnectHandle()));
  _connectHandleNodeConnectMap.insert(nodeConnect);
}

void
Node::erase(const ConnectHandle& connectHandle)
{
  NodeConnect::HandleMap::iterator i = _connectHandleNodeConnectMap.find(connectHandle);
  OpenRTIAssert(i != _connectHandleNodeConnectMap.end());
  erase(*i);
}

void
Node::erase(NodeConnect& nodeConnect)
{
  if (_parentConnectHandle == nodeConnect.getConnectHandle())
    _parentConnectHandle = ConnectHandle();
  _connectHandleAllocator.put(nodeConnect.getConnectHandle());
  NodeConnect::HandleMap::erase(nodeConnect);
}

bool
Node::getFederationExecutionAlreadyExists(const std::string& federationName) const
{
  return _federationNameFederationMap.find(federationName) != _federationNameFederationMap.end();
}

Federation*
Node::getFederation(const std::string& federationName)
{
  Federation::NameMap::iterator i = _federationNameFederationMap.find(federationName);
  if (i == _federationNameFederationMap.end())
    return 0;
  return i.get();
}

Federation*
Node::getFederation(const FederationHandle& federationHandle)
{
  Federation::HandleMap::iterator i = _federationHandleFederationMap.find(federationHandle);
  if (i == _federationHandleFederationMap.end())
    return 0;
  return i.get();
}

void
Node::insert(Federation& federation)
{
  federation.setFederationHandle(_federationHandleAllocator.getOrTake(federation.getFederationHandle()));
  _federationNameFederationMap.insert(federation);
  _federationHandleFederationMap.insert(federation);
}

void
Node::erase(Federation& federation)
{
  OpenRTIAssert(!federation.hasJoinedChildren());
  Log(ServerFederation, Info) << getServerPath() << ": Released FederationHandle in child server for \""
                              << federation.getName() << "\"!" << std::endl;
  _federationHandleAllocator.put(federation.getFederationHandle());
  Federation::HandleMap::erase(federation);
}

void
Node::insertName(Federation& federation)
{
  _federationNameFederationMap.insert(federation);
}

void
Node::eraseName(Federation& federation)
{
  OpenRTIAssert(!federation.hasJoinedChildren());
  Log(ServerFederation, Info) << getServerPath() << ": Destroyed federation execution in child server for \""
                              << federation.getName() << "\"!" << std::endl;
  _federationNameFederationMap.unlink(federation);
}

void
Node::send(const ConnectHandle& connectHandle, const SharedPtr<const AbstractMessage>& message)
{
  NodeConnect::HandleMap::iterator i = _connectHandleNodeConnectMap.find(connectHandle);
  if (i == _connectHandleNodeConnectMap.end())
    return;
  i->send(message);
}

void
Node::sendToParent(const SharedPtr<const AbstractMessage>& message)
{
  send(_parentConnectHandle, message);
}

void
Node::broadcast(const SharedPtr<const AbstractMessage>& message)
{
  for (NodeConnect::HandleMap::iterator i = _connectHandleNodeConnectMap.begin();
       i != _connectHandleNodeConnectMap.end(); ++i) {
    i->send(message);
  }
}

void
Node::broadcast(const ConnectHandle& connectHandle, const SharedPtr<const AbstractMessage>& message)
{
  for (NodeConnect::HandleMap::iterator i = _connectHandleNodeConnectMap.begin();
       i != _connectHandleNodeConnectMap.end(); ++i) {
    if (i->getConnectHandle() == connectHandle)
      continue;
    i->send(message);
  }
}

void
Node::broadcastToChildren(const SharedPtr<const AbstractMessage>& message)
{
  broadcast(_parentConnectHandle, message);
}

} // namespace ServerModel
} // namespace OpenRTI
