/* -*-c++-*- OpenRTI - Copyright (C) 2009-2012 Mathias Froehlich
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

#include "ServerObjectModel.h"

namespace OpenRTI {

void
ServerObjectModel::ObjectClass::insertObjectInstance(ObjectInstance* objectInstance)
{
  OpenRTIAssert(objectInstance);
  objectInstance->insertToObjectClassList(_objectInstanceList);
}

void
ServerObjectModel::ObjectClass::eraseObjectInstance(ObjectInstance* objectInstance)
{
  OpenRTIAssert(objectInstance);
  objectInstance->eraseFromObjectClassList(_objectInstanceList);
}

ServerObjectModel::ObjectInstanceConnect*
ServerObjectModel::ObjectInstance::referenceObjectInstance(ConnectData* connect)
{
  OpenRTIAssert(connect);
  ConnectHandle connectHandle = connect->getHandle();
  OpenRTIAssert(connectHandle.valid());

  typedef ConnectHandleObjectInstanceConnectMap::value_type value_type;
  ConnectHandleObjectInstanceConnectMap::iterator i;
  // FIXME make that assert at some time
  // OpenRTIAssert(_connectHandleObjectInstanceConnectMap.find(connectHandle) == _connectHandleObjectInstanceConnectMap.end());
  i = _connectHandleObjectInstanceConnectMap.insert(value_type(connectHandle, SharedPtr<ObjectInstanceConnect>())).first;
  if (i->second.get())
    return i->second.get();
  i->second = new ObjectInstanceConnect(i, this, connect);
  connect->insertObjectInstance(*(i->second));

  return i->second.get();
}

bool
ServerObjectModel::ObjectInstance::unreferenceObjectInstance(ConnectData* connect)
{
  OpenRTIAssert(connect);
  ConnectHandle connectHandle = connect->getHandle();
  OpenRTIAssert(connectHandle.valid());

  ConnectHandleObjectInstanceConnectMap::iterator i;
  i = _connectHandleObjectInstanceConnectMap.find(connectHandle);
  OpenRTIAssert(i != _connectHandleObjectInstanceConnectMap.end());

  return unreferenceObjectInstance(i->second.get());
}

bool
ServerObjectModel::ObjectInstance::unreferenceObjectInstance(ObjectInstanceConnect* objectInstanceConnect)
{
  OpenRTIAssert(objectInstanceConnect);
  ConnectData* connect = objectInstanceConnect->getConnect();
  OpenRTIAssert(connect);

  ConnectHandleObjectInstanceConnectMap::iterator connectHandleObjectInstanceConnectMapIterator;
  connectHandleObjectInstanceConnectMapIterator = objectInstanceConnect->getConnectHandleObjectInstanceConnectMapIterator();
  OpenRTIAssert(connectHandleObjectInstanceConnectMapIterator != _connectHandleObjectInstanceConnectMap.end());

  connect->eraseObjectInstance(*objectInstanceConnect);
  _connectHandleObjectInstanceConnectMap.erase(connectHandleObjectInstanceConnectMapIterator);

  return _connectHandleObjectInstanceConnectMap.empty();
}

void
ServerObjectModel::insert(const FOMModuleList& moduleList, bool isBaseType)
{
  for (FOMModuleList::const_iterator i = moduleList.begin(); i != moduleList.end(); ++i) {
    insertFomModule(*i);
  }

  _fomModuleSet.insertModuleList(moduleList, isBaseType);
}

void
ServerObjectModel::erase(const FOMModuleList& moduleList)
{
  for (FOMModuleList::const_iterator i = moduleList.begin(); i != moduleList.end(); ++i) {
    eraseFomModule(*i);
  }
}

bool
ServerObjectModel::insertFomModule(const FOMModule& fomModule)
{
  /// FIXME: make sure this one does not collide, and if so roll back ...
  for (FOMInteractionClassList::const_iterator i = fomModule.getInteractionClassList().begin();
       i != fomModule.getInteractionClassList().end(); ++i) {
    insertInteractionClass(*i);
  }
  for (FOMObjectClassList::const_iterator i = fomModule.getObjectClassList().begin();
       i != fomModule.getObjectClassList().end(); ++i) {
    insertObjectClass(*i);
  }
  return true;
}

void
ServerObjectModel::eraseFomModule(const FOMModule& fomModule)
{
  for (FOMInteractionClassList::const_iterator i = fomModule.getInteractionClassList().begin();
       i != fomModule.getInteractionClassList().end(); ++i) {
    eraseInteractionClass(*i);
  }
  for (FOMObjectClassList::const_iterator i = fomModule.getObjectClassList().begin();
       i != fomModule.getObjectClassList().end(); ++i) {
    eraseObjectClass(*i);
  }
}

void
ServerObjectModel::insertInteractionClass(const FOMInteractionClass& module)
{
  if (getInteractionClass(module.getInteractionClassHandle())) {
    // FIXME: currently jus trust the FOMModuleSet that this cannot happen.
    // Also, since this  kind of stuff can arrive from the network, this is unsafe
    // OpenRTIAssert(module.getParameterList().empty());
  } else {
    InteractionClass* parentInteractionClass = getInteractionClass(module.getParentInteractionClassHandle());

    InteractionClassHandle interactionClassHandle = module.getInteractionClassHandle();
    OpenRTIAssert(interactionClassHandle.valid());

    SharedPtr<InteractionClass> interactionClass;
    interactionClass = new InteractionClass(module.getName(), interactionClassHandle, parentInteractionClass);
    // FIXME, this???
    // <field name="DimensionHandleSet" type="DimensionHandleSet"/>

    if (_interactionClassVector.size() <= interactionClassHandle.getHandle())
      _interactionClassVector.resize(interactionClassHandle.getHandle() + 1);

    _interactionClassVector[interactionClassHandle.getHandle()] = interactionClass;
  }
}

void
ServerObjectModel::eraseInteractionClass(const FOMInteractionClass& module)
{
  // FIXME move this into the destructors, but then we need to reliably unfold these things even in exception paths
#ifndef _NDEBUG
  InteractionClass* interactionClass = _interactionClassVector[module.getInteractionClassHandle().getHandle()].get();
  if (interactionClass) {
    OpenRTIAssert(interactionClass->getSubscriptionType() == Unsubscribed);
    OpenRTIAssert(interactionClass->getPublicationType() == Unpublished);
  }
#endif
  _interactionClassVector[module.getInteractionClassHandle().getHandle()].clear();
}

void
ServerObjectModel::insertObjectClass(const FOMObjectClass& module)
{
  if (ObjectClass* existingObjectClass = getObjectClass(module.getObjectClassHandle())) {
    for (FOMAttributeList::const_iterator i = module.getAttributeList().begin();
         i != module.getAttributeList().end(); ++i) {
      // FIXME share these among object classes???
      SharedPtr<ObjectClassAttribute> attribute;
      attribute = new ObjectClassAttribute(i->getName(), i->getAttributeHandle());
      // FIXME, this???
      // <field name="DimensionHandleSet" type="DimensionHandleSet"/>
      existingObjectClass->insertObjectClassAttribute(attribute);
    }
  } else {
    ObjectClass* parentObjectClass = getObjectClass(module.getParentObjectClassHandle());

    ObjectClassHandle objectClassHandle = module.getObjectClassHandle();
    OpenRTIAssert(objectClassHandle.valid());

    SharedPtr<ObjectClass> objectClass;
    objectClass = new ObjectClass(module.getName(), objectClassHandle, parentObjectClass);

    if (parentObjectClass) {
      for (ObjectClassAttributeVector::const_iterator i = parentObjectClass->getObjectClassAttributeVector().begin();
           i != parentObjectClass->getObjectClassAttributeVector().end(); ++i) {
        // FIXME share these among object classes???
        SharedPtr<ObjectClassAttribute> attribute;
        attribute = new ObjectClassAttribute((*i)->getName(), (*i)->getHandle());
        // FIXME, this???
        // <field name="DimensionHandleSet" type="DimensionHandleSet"/>
        objectClass->insertObjectClassAttribute(attribute);
      }
    }

    for (FOMAttributeList::const_iterator i = module.getAttributeList().begin();
         i != module.getAttributeList().end(); ++i) {
      // FIXME share these among object classes???
      SharedPtr<ObjectClassAttribute> attribute;
      attribute = new ObjectClassAttribute(i->getName(), i->getAttributeHandle());
      // FIXME, this???
      // <field name="DimensionHandleSet" type="DimensionHandleSet"/>
      objectClass->insertObjectClassAttribute(attribute);
    }

    if (_objectClassVector.size() <= objectClassHandle.getHandle())
      _objectClassVector.resize(objectClassHandle.getHandle() + 1);

    _objectClassVector[objectClassHandle.getHandle()] = objectClass;
  }
}

void
ServerObjectModel::eraseObjectClass(const FOMObjectClass& module)
{
  // FIXME move this into the destructors, but then we need to reliably unfold these things even in exception paths
#ifndef _NDEBUG
  ObjectClass* objectClass = _objectClassVector[module.getObjectClassHandle().getHandle()].get();
  if (objectClass) {
    OpenRTIAssert(objectClass->getChildObjectClassList().empty());
    OpenRTIAssert(objectClass->getObjectInstanceList().empty());
    const ObjectClassAttributeVector& objectClassAttributeVector = objectClass->getObjectClassAttributeVector();
    for (ObjectClassAttributeVector::const_iterator i = objectClassAttributeVector.begin();
         i != objectClassAttributeVector.end(); ++i) {
      if (!i->valid())
        continue;
      OpenRTIAssert((*i)->getSubscriptionType() == Unsubscribed);
      OpenRTIAssert((*i)->getPublicationType() == Unpublished);
    }
  }
#endif
  _objectClassVector[module.getObjectClassHandle().getHandle()].clear();
}

ServerObjectModel::Region*
ServerObjectModel::getRegion(const RegionHandle& regionHandle)
{
  RegionHandleRegionMap::iterator i;
  i = _regionHandleRegionMap.find(regionHandle);
  if (i == _regionHandleRegionMap.end())
    return 0;
  return i->second.get();
}

ServerObjectModel::Region*
ServerObjectModel::insertRegion(const RegionHandle& regionHandle)
{
  typedef RegionHandleRegionMap::value_type value_type;
  RegionHandleRegionMap::iterator i;
  i = _regionHandleRegionMap.insert(value_type(regionHandle, SharedPtr<Region>())).first;
  OpenRTIAssert(!i->second.valid());
  i->second = new Region(i);
  return i->second.get();
}

void
ServerObjectModel::eraseRegion(Region* region)
{
  OpenRTIAssert(region);
  OpenRTIAssert(!region->getConnect());
  _regionHandleRegionMap.erase(region->getRegionHandleRegionMapIterator());
}

void
ServerObjectModel::eraseRegion(const RegionHandle& regionHandle)
{
  Region* region = getRegion(regionHandle);
  if (!region)
    return;

  ConnectData* connect = region->getConnect();
  if (connect)
    connect->eraseRegion(*region);

  eraseRegion(region);
}

ServerObjectModel::ObjectInstance*
ServerObjectModel::getObjectInstance(const ObjectInstanceHandle& objectInstanceHandle)
{
  ObjectInstanceHandleObjectInstanceMap::iterator i = _objectInstanceHandleObjectInstanceMap.find(objectInstanceHandle);
  if (i == _objectInstanceHandleObjectInstanceMap.end())
    return 0;
  return i->second.get();
}

ServerObjectModel::ObjectInstance*
ServerObjectModel::insertObjectInstanceHandle()
{
  ObjectInstanceHandle objectInstanceHandle = _objectInstanceHandleAllocator.get();
  std::string reservedName = objectInstanceHandle.getReservedName("HLAobjectInstance");
  return _insertObjectInstanceHandle(objectInstanceHandle, reservedName);
}

ServerObjectModel::ObjectInstance*
ServerObjectModel::insertObjectInstanceHandle(const std::string& objectInstanceName)
{
  ObjectInstanceHandle objectInstanceHandle = _objectInstanceHandleAllocator.get();
  return _insertObjectInstanceHandle(objectInstanceHandle, objectInstanceName);
}

ServerObjectModel::ObjectInstance*
ServerObjectModel::insertObjectInstanceHandle(const ObjectInstanceHandle& objectInstanceHandle, const std::string& objectInstanceName)
{
  _objectInstanceHandleAllocator.take(objectInstanceHandle);
  return _insertObjectInstanceHandle(objectInstanceHandle, objectInstanceName);
}

ServerObjectModel::ObjectInstance*
ServerObjectModel::_insertObjectInstanceHandle(const ObjectInstanceHandle& objectInstanceHandle, const std::string& objectInstanceName)
{
  Log(ServerObjectInstance, Debug) << getServerPath() << ": Insert Object Instance \""
                                   << objectInstanceHandle << "\"!" << std::endl;
  OpenRTIAssert(objectInstanceHandle.valid());
  OpenRTIAssert(!isObjectNameInUse(objectInstanceName));
  OpenRTIAssert(_objectInstanceHandleObjectInstanceMap.find(objectInstanceHandle) == _objectInstanceHandleObjectInstanceMap.end());
  StringSet::iterator stringSetIterator = _objectInstanceNameSet.insert(objectInstanceName).first;
  ObjectInstanceHandleObjectInstanceMap::iterator i;
  typedef ObjectInstanceHandleObjectInstanceMap::value_type value_type;
  i = _objectInstanceHandleObjectInstanceMap.insert(value_type(objectInstanceHandle, SharedPtr<ObjectInstance>())).first;
  i->second = new ObjectInstance(i, stringSetIterator);
  return i->second.get();
}

void
ServerObjectModel::eraseObjectInstanceHandle(ObjectInstance* objectInstance)
{
  OpenRTIAssert(objectInstance);
  ObjectInstanceHandleObjectInstanceMap::iterator i;
  i = objectInstance->_objectInstanceHandleObjectInstanceMapIterator;
  OpenRTIAssert(i != _objectInstanceHandleObjectInstanceMap.end());
  objectInstance->_objectInstanceHandleObjectInstanceMapIterator = _objectInstanceHandleObjectInstanceMap.end();

  /// FIXME may be move into ObjectInstance' destructor??
  ObjectClass* objectClass = objectInstance->getObjectClass();
  if (objectClass)
    objectClass->eraseObjectInstance(objectInstance);

  _objectInstanceHandleAllocator.put(objectInstance->getHandle());
  _objectInstanceNameSet.erase(objectInstance->_stringSetIterator);
  _objectInstanceHandleObjectInstanceMap.erase(i);
}

ServerObjectModel::Federate*
ServerObjectModel::getFederate(const FederateHandle& federateHandle)
{
  FederateHandleFederateMap::iterator i = _federateHandleFederateMap.find(federateHandle);
  if (i == _federateHandleFederateMap.end())
    return 0;
  return i->second.get();
}

ServerObjectModel::Federate*
ServerObjectModel::insertFederate(const ConnectHandle& connectHandle, const std::string& federateName,
                                  const FederateHandle& federateHandle)
{
  // Either we allocate a new federate, then the connect must still be alive
  // or we insert an already died federate and just keep everything in order for a currect the resing request sequence
  OpenRTIAssert(federateHandle.valid());
  OpenRTIAssert(!federateName.empty());
  OpenRTIAssert(!isFederateNameInUse(federateName));
  OpenRTIAssert(_federateHandleFederateMap.find(federateHandle) == _federateHandleFederateMap.end());

  // Register that we reach this federate through this connect
  StringSet::iterator stringSetIterator;
  stringSetIterator = _federateNameSet.insert(federateName).first;
  FederateHandleFederateMap::iterator i;
  typedef FederateHandleFederateMap::value_type value_type;
  i = _federateHandleFederateMap.insert(value_type(federateHandle, SharedPtr<Federate>())).first;
  i->second = new Federate(i, stringSetIterator);
  // i->second->_connectHandle = connectHandle;
  ConnectHandleConnectDataMap::iterator j = _connectHandleConnectDataMap.find(connectHandle);
  if (j != _connectHandleConnectDataMap.end()) {
    i->second->_connect = j->second.get();
    i->second->_federateListIterator = j->second->_federateList.insert(j->second->_federateList.begin(), i->second.get());
  } else {
    i->second->_resignPending = true;
  }

  return i->second.get();
}

ServerObjectModel::Federate*
ServerObjectModel::insertFederate(const ConnectHandle& connectHandle, const std::string& federateName)
{
  FederateHandle federateHandle = _federateHandleAllocator.get();
  if (federateName.empty()) {
    std::string reservedName = federateHandle.getReservedName("HLAfederate");
    OpenRTIAssert(!isFederateNameInUse(reservedName));
    return insertFederate(connectHandle, reservedName, federateHandle);
  } else {
    OpenRTIAssert(!isFederateNameInUse(federateName));
    return insertFederate(connectHandle, federateName, federateHandle);
  }
}

void
ServerObjectModel::eraseFederate(ServerObjectModel::FederateHandleFederateMap::iterator i)
{
  OpenRTIAssert(i != _federateHandleFederateMap.end());

  // drop fom modules in use just by this
  erase(_fomModuleSet.eraseModuleList(i->second->_fomModuleHandleVector));

  // The time management stuff
  _federateHandleCommitMap.erase(i->first);

  // Remove from syncronization state
  // FIXME: complete syncronization states if this is the last they wait for.
  // FIXME: have a list of labels that this federate participates - so avoid traversing all
  for (SyncronizationLabelStateMap::iterator j = _syncronizationLabelStateMap.begin();
       j != _syncronizationLabelStateMap.end(); ++j) {
    j->second.removeFederate(i->first);
  }

  // Remove from connects
  ConnectData* connect = i->second->_connect;
  if (connect)
    connect->eraseFederate(i->second.get());

  // Give back the handle to the allocator
  _federateHandleAllocator.put(i->first);
  // Erase from the indices
  _federateNameSet.erase(i->second->_stringSetIterator);
  _federateHandleFederateMap.erase(i);
}

void
ServerObjectModel::eraseFederate(const FederateHandle& federateHandle)
{
  eraseFederate(_federateHandleFederateMap.find(federateHandle));
}

void
ServerObjectModel::eraseFederate(Federate* federate)
{
  OpenRTIAssert(federate);
  eraseFederate(federate->_federateHandleFederateMapIterator);
}

ServerObjectModel::ConnectData*
ServerObjectModel::getConnect(const ConnectHandle& connectHandle)
{
  ConnectHandleConnectDataMap::iterator i = _connectHandleConnectDataMap.find(connectHandle);
  if (i == _connectHandleConnectDataMap.end())
    return 0;
  return i->second.get();
}

ServerObjectModel::ConnectData*
ServerObjectModel::getOrInsertParentConnect(const ConnectHandle& connectHandle)
{
  OpenRTIAssert(!_parentServerConnectHandle.valid());
  _parentServerConnectHandle = connectHandle;
  return getOrInsertConnect(connectHandle);
}

ServerObjectModel::ConnectData*
ServerObjectModel::getOrInsertConnect(const ConnectHandle& connectHandle)
{
  OpenRTIAssert(connectHandle.valid());
  std::pair<ConnectHandleConnectDataMap::iterator, bool> iteratorBoolPair;
  typedef ConnectHandleConnectDataMap::value_type value_type;
  iteratorBoolPair = _connectHandleConnectDataMap.insert(value_type(connectHandle, SharedPtr<ConnectData>()));
  if (!iteratorBoolPair.second) {
    OpenRTIAssert(iteratorBoolPair.first->second.valid());
    return iteratorBoolPair.first->second.get();
  }
  iteratorBoolPair.first->second = new ConnectData(iteratorBoolPair.first);
  return iteratorBoolPair.first->second.get();
}

void
ServerObjectModel::eraseConnect(ServerObjectModel::ConnectHandleConnectDataMap::iterator i)
{
  OpenRTIAssert(i != _connectHandleConnectDataMap.end());
  OpenRTIAssert(i->first == _parentServerConnectHandle || i->second->_federateList.empty());

  if (_parentServerConnectHandle == i->first) {
    Log(ServerConnect, Error) << getServerPath() << ": Removing parent connect!" << std::endl;
    _parentServerConnectHandle = ConnectHandle();
  }

  // Finally remove what is referencing the old connect handle
  _connectHandleConnectDataMap.erase(i);
}

void
ServerObjectModel::eraseConnect(const ConnectHandle& connectHandle)
{
  eraseConnect(_connectHandleConnectDataMap.find(connectHandle));
}

void
ServerObjectModel::eraseConnect(ConnectData* connectData)
{
  OpenRTIAssert(connectData);
  eraseConnect(connectData->_connectHandleConnectDataMapIterator);
}

}
