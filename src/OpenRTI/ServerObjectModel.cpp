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

void
ServerObjectModel::insert(const FOMModuleList& moduleList)
{
  for (FOMModuleList::const_iterator i = moduleList.begin(); i != moduleList.end(); ++i) {
    insert(*i);
  }

  _fomModuleSet.insertModuleList(moduleList);
}

void
ServerObjectModel::insert(const FOMModule& module)
{
  for (FOMInteractionClassList::const_iterator i = module.getInteractionClassList().begin();
       i != module.getInteractionClassList().end(); ++i) {
    insertInteractionClass(*i, i->getParentInteractionClassHandle());
  }
  for (FOMObjectClassList::const_iterator i = module.getObjectClassList().begin();
       i != module.getObjectClassList().end(); ++i) {
    insertObjectClass(*i, i->getParentObjectClassHandle());
  }
}

void
ServerObjectModel::insertInteractionClass(const FOMInteractionClass& module, const InteractionClassHandle& parentHandle)
{
  if (getInteractionClass(module.getInteractionClassHandle())) {
    OpenRTIAssert(module.getParameterList().empty());
  } else {
    InteractionClass* parentInteractionClass = getInteractionClass(parentHandle);
    SharedPtr<InteractionClass> interactionClass;
    interactionClass = new InteractionClass(module.getName(), module.getInteractionClassHandle(), parentInteractionClass);
    // FIXME, this???
    // <field name="DimensionHandleSet" type="DimensionHandleSet"/>
    insertInteractionClass(interactionClass);
  }
}

void
ServerObjectModel::insertObjectClass(const FOMObjectClass& module, const ObjectClassHandle& parentHandle)
{
  if (getObjectClass(module.getObjectClassHandle())) {
    OpenRTIAssert(module.getAttributeList().empty());
  } else {
    ObjectClass* parentObjectClass = getObjectClass(parentHandle);
    SharedPtr<ObjectClass> objectClass;
    objectClass = new ObjectClass(module.getName(), module.getObjectClassHandle(), parentObjectClass);

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
    insertObjectClass(objectClass);
  }
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
  i = _objectInstanceHandleObjectInstanceMap.insert(value_type(objectInstanceHandle, 0)).first;
  i->second = new ObjectInstance(i, stringSetIterator);
  return i->second.get();
}

bool
ServerObjectModel::unreferenceObjectInstanceHandle(ObjectInstanceHandleObjectInstanceMap::iterator i, const ConnectHandle& connectHandle)
{
  OpenRTIAssert(i != _objectInstanceHandleObjectInstanceMap.end());
  Log(ServerObjectInstance, Debug) << getServerPath() << ": Unreference Object Instance \""
                                   << i->first << "\" referenced by connect \""
                                   << connectHandle <<  "\"!" << std::endl;
  OpenRTIAssert(connectHandle != _parentServerConnectHandle);
  if (!i->second->unreferenceObjectInstanceHandle(connectHandle))
    return false;

  Log(ServerObjectInstance, Debug) << getServerPath() << ": Dropped last reference to Object Instance \""
                                   << i->first << "\"!" << std::endl;

  _objectInstanceHandleAllocator.put(i->first);
  _objectInstanceNameSet.erase(i->second->_stringSetIterator);
  _objectInstanceHandleObjectInstanceMap.erase(i);

  return true;
}

bool
ServerObjectModel::unreferenceObjectInstanceHandle(const ObjectInstanceHandle& objectInstanceHandle, const ConnectHandle& connectHandle)
{
  return unreferenceObjectInstanceHandle(_objectInstanceHandleObjectInstanceMap.find(objectInstanceHandle), connectHandle);
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
  i = _federateHandleFederateMap.insert(value_type(federateHandle, 0)).first;
  i->second = new Federate(i, stringSetIterator);
  i->second->_connectHandle = connectHandle;
  ConnectHandleConnectDataMap::iterator j = _connectHandleConnectDataMap.find(connectHandle);
  if (j != _connectHandleConnectDataMap.end()) {
    j->second->_federateHandleSet.insert(federateHandle);
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

  // The time management stuff
  _federateHandleTimeStampMap.erase(i->first);

  // Remove from syncronization state
  // FIXME: complete syncronization states if this is the last they wait for.
  // FIXME: have a list of labels that this federate participates - so avoid traversing all
  for (SyncronizationLabelStateMap::iterator j = _syncronizationLabelStateMap.begin();
       j != _syncronizationLabelStateMap.end(); ++j) {
    j->second.removeFederate(i->first);
  }

  // Remove from connects
  ConnectHandleConnectDataMap::iterator k = _connectHandleConnectDataMap.find(i->second->_connectHandle);
  if (k != _connectHandleConnectDataMap.end())
    k->second->_federateHandleSet.erase(i->first);

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
  iteratorBoolPair = _connectHandleConnectDataMap.insert(value_type(connectHandle, 0));
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
  OpenRTIAssert(i->first == _parentServerConnectHandle || i->second->_federateHandleSet.empty());

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
