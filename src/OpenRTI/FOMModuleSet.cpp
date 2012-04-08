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

#include "FOMModuleSet.h"

#include "Exception.h"
#include "Handle.h"
#include "HandleAllocator.h"
#include "Message.h"
#include "Referenced.h"
#include "SharedPtr.h"
#include "StringUtils.h"
#include "Types.h"
#include "LogStream.h"

namespace OpenRTI {

struct OPENRTI_LOCAL FOMModuleSet::AllocatorMap : public Referenced {
  typedef std::map<FOMStringModule, FOMModuleHandle> FOMStringModuleFOMModuleHandleMap;
  typedef std::map<FOMModuleHandle, FOMModule> FOMModuleHandleFOMModuleMap;

  typedef std::map<std::string, TransportationType> NameTransportationTypeMap;
  typedef std::map<std::string, OrderType> NameOrderTypeMap;
  typedef std::map<std::string, DimensionHandle> NameDimensionHandleMap;
  struct InteractionData {
    FOMInteractionClass _interactionClass;
    // The next free parameter handle to allocate for a derived class
    ParameterHandle _nextParameterHandle;
  };
  typedef std::map<StringVector, InteractionData> NameInteractionDataMap;
  typedef std::map<InteractionClassHandle, StringVector> InteractionClassHandleNameMap;

  struct ObjectData {
    FOMObjectClass _objectClass;
    // The next free attribute handle to allocate for a derived class
    AttributeHandle _nextAttributeHandle;
  };
  typedef std::map<StringVector, ObjectData> NameObjectDataMap;
  typedef std::map<ObjectClassHandle, StringVector> ObjectClassHandleNameMap;

  OrderType getOrderType(const std::string& name) const
  {
    if (caseCompare(name, "timestamp"))
      return TIMESTAMP;
    else if (caseCompare(name, "receive"))
      return RECEIVE;
    else
      throw ErrorReadingFDD(std::string("Unknown order type \"") + name + "\".");
  }

  bool insertTransportationType(const FOMStringTransportationType& stringModule)
  {
    if (_nameTransportationTypeMap.find(stringModule.getName()) != _nameTransportationTypeMap.end())
      return false;
    if (stringModule.getName() == "HLAreliable")
      _nameTransportationTypeMap[stringModule.getName()] = RELIABLE;
    else if (caseCompare(stringModule.getName(), "reliable"))
      _nameTransportationTypeMap[stringModule.getName()] = RELIABLE;
    else if (stringModule.getName() == "HLAbestEffort")
      _nameTransportationTypeMap[stringModule.getName()] = BEST_EFFORT;
    else if (caseCompare(stringModule.getName(), "best_effort"))
      _nameTransportationTypeMap[stringModule.getName()] = BEST_EFFORT;
    else
      throw ErrorReadingFDD(std::string("Unsupported transportation type \"") + stringModule.getName() + "\".");
    return true;
  }
  TransportationType getTransportationType(const std::string& name) const
  {
    NameTransportationTypeMap::const_iterator i = _nameTransportationTypeMap.find(name);
    if (i == _nameTransportationTypeMap.end())
      throw ErrorReadingFDD(std::string("Unknown transportation type \"") + name + "\".");
    return i->second;
  }

  bool insertDimension(const FOMStringDimension& stringModule)
  {
    NameDimensionHandleMap::const_iterator i = _nameDimensionHandleMap.find(stringModule.getName());
    if (i != _nameDimensionHandleMap.end())
      return false;
    _nameDimensionHandleMap[stringModule.getName()] = ++_nextDimensionHandle;
    return true;
  }
  DimensionHandle getDimensionHandle(const std::string& name) const
  {
    NameDimensionHandleMap::const_iterator i = _nameDimensionHandleMap.find(name);
    if (i == _nameDimensionHandleMap.end())
      throw ErrorReadingFDD(std::string("Unknown dimension \"") + name + "\".");
    return i->second;
  }

  void resolveDimensionHandles(DimensionHandleSet& dimensionHandles, const StringSet& stringDimensions)
  {
    for (StringSet::const_iterator i = stringDimensions.begin(); i != stringDimensions.end(); ++i) {
      dimensionHandles.insert(getDimensionHandle(*i));
    }
  }

  ParameterHandle insertParameterList(FOMParameterList& module, const FOMStringParameterList& stringModule,
                                      ParameterHandle nextParameterHandle)
  {
    for (FOMStringParameterList::const_iterator i = stringModule.begin(); i != stringModule.end(); ++i) {
      FOMParameter parameter;
      parameter.setName(i->getName());
      parameter.setParameterHandle(++nextParameterHandle);
      module.push_back(parameter);
    }
    return nextParameterHandle;
  }

  void insertInteractionClass(FOMInteractionClass& module, const FOMStringInteractionClass& stringModule)
  {
    InteractionClassHandle parentInteractionClassHandle;
    ParameterHandle nextParameterHandle;
    StringVector parentName = stringModule.getName();
    parentName.pop_back();
    if (!parentName.empty()) {
      NameInteractionDataMap::iterator i = _nameInteractionDataMap.find(parentName);
      // FIXME nore an internal error??
      if (i == _nameInteractionDataMap.end())
        throw ErrorReadingFDD(std::string("Unknown parent object class \"") + fqClassName(parentName) + "\".");
      nextParameterHandle = i->second._nextParameterHandle;
      parentInteractionClassHandle = i->second._interactionClass.getInteractionClassHandle();
    }

    // Check if the new string module is compatible with the existing
    NameInteractionDataMap::iterator i = _nameInteractionDataMap.find(stringModule.getName());
    // Ok, if it is not there, we are compatible by default...
    if (i != _nameInteractionDataMap.end()) {
      // ... but in this case, we already have this class.
      // Check for some stuff if this is the same FIXME

      OpenRTIAssert(!stringModule.getName().empty());
      OpenRTIAssert(i->second._interactionClass.getName() == stringModule.getName().back());

      if (!stringModule.getOrderType().empty()) {
        OrderType orderType = getOrderType(stringModule.getOrderType());
        if (orderType != i->second._interactionClass.getOrderType())
          throw ErrorReadingFDD(std::string("Incompatible InteractionClass \"") + fqClassName(stringModule.getName())
                                + "\": OrderType does not match.");
      }

      if (!stringModule.getTransportationType().empty()) {
        TransportationType transportationType = getTransportationType(stringModule.getTransportationType());
        if (transportationType != i->second._interactionClass.getTransportationType())
          throw ErrorReadingFDD(std::string("Incompatible InteractionClass \"") + fqClassName(stringModule.getName())
                                + "\": TransportationType does not match.");
      }

      if (!stringModule.getDimensionSet().empty()) {
        DimensionHandleSet dimensionHandles;
        resolveDimensionHandles(dimensionHandles, stringModule.getDimensionSet());
        if (i->second._interactionClass.getDimensionHandleSet() != dimensionHandles)
          throw ErrorReadingFDD(std::string("Incompatible InteractionClass \"") + fqClassName(stringModule.getName())
                                + "\": Dimension handle set does not match.");
      }

      // Check if either the parameter list is empty or identical
      if (!stringModule.getParameterList().empty()) {
        FOMParameterList parameterList;
        insertParameterList(parameterList, stringModule.getParameterList(), nextParameterHandle);
        if (parameterList != i->second._interactionClass.getParameterList())
          throw ErrorReadingFDD(std::string("Incompatible InteractionClass \"") + fqClassName(stringModule.getName())
                                + "\": Parameter list does not match.");
      }
    } else {
      i = _nameInteractionDataMap.insert(NameInteractionDataMap::value_type(stringModule.getName(), InteractionData())).first;

      i->second._interactionClass.setName(stringModule.getName().back());

      i->second._interactionClass.setInteractionClassHandle(++_nextInteractionClassHandle);
      i->second._interactionClass.setParentInteractionClassHandle(parentInteractionClassHandle);
      _interactionClassHandleNameMap[i->second._interactionClass.getInteractionClassHandle()] = stringModule.getName();

      if (stringModule.getOrderType().empty())
        i->second._interactionClass.setOrderType(TIMESTAMP);
      else
        i->second._interactionClass.setOrderType(getOrderType(stringModule.getOrderType()));

      if (stringModule.getTransportationType().empty())
        i->second._interactionClass.setTransportationType(RELIABLE);
      else
        i->second._interactionClass.setTransportationType(getTransportationType(stringModule.getTransportationType()));

      resolveDimensionHandles(i->second._interactionClass.getDimensionHandleSet(), stringModule.getDimensionSet());

      i->second._nextParameterHandle = insertParameterList(i->second._interactionClass.getParameterList(), stringModule.getParameterList(), nextParameterHandle);
    }

    // Ok, this contains all but the derived classes.
    module = i->second._interactionClass;
  }

  void insertInteractionClassList(FOMInteractionClassList& module, const FOMStringInteractionClassList& stringModule)
  {
    for (FOMStringInteractionClassList::const_iterator i = stringModule.begin(); i != stringModule.end(); ++i) {
      module.push_back(FOMInteractionClass());
      insertInteractionClass(module.back(), *i);
    }
  }

  AttributeHandle insertAttributeList(FOMAttributeList& module, const FOMStringAttributeList& stringModule,
                                      AttributeHandle nextAttributeHandle)
  {
    for (FOMStringAttributeList::const_iterator i = stringModule.begin(); i != stringModule.end(); ++i) {
      FOMAttribute attribute;
      attribute.setName(i->getName());
      attribute.setAttributeHandle(++nextAttributeHandle);

      if (i->getOrderType().empty())
        attribute.setOrderType(TIMESTAMP);
      else
        attribute.setOrderType(getOrderType(i->getOrderType()));

      if (i->getTransportationType().empty())
        attribute.setTransportationType(RELIABLE);
      else
        attribute.setTransportationType(getTransportationType(i->getTransportationType()));

      resolveDimensionHandles(attribute.getDimensionHandleSet(), i->getDimensionSet());

      module.push_back(attribute);
    }
    return nextAttributeHandle;
  }

  void insertObjectClass(FOMObjectClass& module, const FOMStringObjectClass& stringModule)
  {
    ObjectClassHandle parentObjectClassHandle;
    AttributeHandle nextAttributeHandle;
    StringVector parentName = stringModule.getName();
    parentName.pop_back();
    if (!parentName.empty()) {
      NameObjectDataMap::iterator i = _nameObjectDataMap.find(parentName);
      // FIXME nore an internal error??
      if (i == _nameObjectDataMap.end())
        throw ErrorReadingFDD(std::string("Unknown parent object class \"") + fqClassName(parentName) + "\".");
      nextAttributeHandle = i->second._nextAttributeHandle;
      parentObjectClassHandle = i->second._objectClass.getObjectClassHandle();
    }

    // Check if the new string module is compatible with the existing
    NameObjectDataMap::iterator i = _nameObjectDataMap.find(stringModule.getName());
    // Ok, if it is not there, we are compatible by default...
    if (i != _nameObjectDataMap.end()) {
      // ... but in this case, we already have this class.
      // Check for some stuff if this is the same FIXME

      OpenRTIAssert(!stringModule.getName().empty());
      OpenRTIAssert(i->second._objectClass.getName() == stringModule.getName().back());

      // Check if either the attribute list is empty or identical
      if (!stringModule.getAttributeList().empty()) {
        FOMAttributeList attributeList;
        insertAttributeList(attributeList, stringModule.getAttributeList(), nextAttributeHandle);
        if (attributeList != i->second._objectClass.getAttributeList())
          throw ErrorReadingFDD(std::string("Incompatible ObjectClass \"") + fqClassName(stringModule.getName())
                                + "\": Attribute list does not match.");
      }
    } else {
      i = _nameObjectDataMap.insert(NameObjectDataMap::value_type(stringModule.getName(), ObjectData())).first;

      i->second._objectClass.setName(stringModule.getName().back());

      i->second._objectClass.setObjectClassHandle(++_nextObjectClassHandle);
      i->second._objectClass.setParentObjectClassHandle(parentObjectClassHandle);
      _objectClassHandleNameMap[i->second._objectClass.getObjectClassHandle()] = stringModule.getName();

      i->second._nextAttributeHandle = insertAttributeList(i->second._objectClass.getAttributeList(), stringModule.getAttributeList(), nextAttributeHandle);
    }

    // Ok, this contains all but the derived classes.
    module = i->second._objectClass;
  }

  void insertObjectClassList(FOMObjectClassList& module, const FOMStringObjectClassList& stringModule)
  {
    for (FOMStringObjectClassList::const_iterator i = stringModule.begin(); i != stringModule.end(); ++i) {
      module.push_back(FOMObjectClass());
      insertObjectClass(module.back(), *i);
    }
  }

  FOMModuleHandle insertModule(const FOMStringModule& stringModule)
  {
    // Move that into the AllocatorMap
    // FOMStringModuleFOMModuleHandleMap::const_iterator j = _fomStringModuleFOMModuleHandleMap.find(stringModule);
    // if (j != _fomStringModuleFOMModuleHandleMap.end()) {
    //   // Is already there, so no problem with compatibility
    //   return j->second;
    // } else {
    FOMModule module;
    for (FOMStringTransportationTypeList::const_iterator k = stringModule.getTransportationTypeList().begin();
         k != stringModule.getTransportationTypeList().end(); ++k) {
      if (!insertTransportationType(*k))
        continue;
      // A new transportation type, insert that into the module candidate
      FOMTransportationType transportationType;
      transportationType.setName(k->getName());
      transportationType.setTransportationType(getTransportationType(k->getName()));
      module.getTransportationTypeList().push_back(transportationType);
    }

    for (FOMStringDimensionList::const_iterator k = stringModule.getDimensionList().begin();
         k != stringModule.getDimensionList().end(); ++k) {
      if (!insertDimension(*k))
        continue;
      // A new dimension, insert that into the module candidate
      FOMDimension dimension;
      dimension.setName(k->getName());
      dimension.setDimensionHandle(getDimensionHandle(k->getName()));
      dimension.setUpperBound(k->getUpperBound());
      module.getDimensionList().push_back(dimension);
    }

    for (FOMStringRoutingSpaceList::const_iterator k = stringModule.getRoutingSpaceList().begin();
         k != stringModule.getRoutingSpaceList().end(); ++k) {
      // FIXME
    }

    insertInteractionClassList(module.getInteractionClassList(), stringModule.getInteractionClassList());
    insertObjectClassList(module.getObjectClassList(), stringModule.getObjectClassList());

    FOMModuleHandle fomModuleHandle = ++_nextFOMModuleHandle;
    module.setFOMModuleHandle(fomModuleHandle);
    // _fomStringModuleFOMModuleHandleMap[stringModule] = fomModuleHandle;
    _fomModuleHandleFOMModuleMap[fomModuleHandle] = module;
    return fomModuleHandle;
    // }
  }


  void insertTransportationType(const FOMTransportationType& module)
  {
    _nameTransportationTypeMap[module.getName()] = module.getTransportationType();
  }
  void insertDimension(const FOMDimension& module)
  {
    _nameDimensionHandleMap[module.getName()] = module.getDimensionHandle();
    if (_nextDimensionHandle < module.getDimensionHandle())
      _nextDimensionHandle = module.getDimensionHandle();
  }
  void insertInteractionClass(const FOMInteractionClass& module)
  {
    InteractionClassHandle nextParameterHandle;
    // FIXME
    for (NameInteractionDataMap::const_iterator i = _nameInteractionDataMap.begin(); i != _nameInteractionDataMap.end(); ++i) {
      if (i->second._interactionClass.getParentInteractionClassHandle() == module.getParentInteractionClassHandle())
        nextParameterHandle = ParameterHandle(i->second._nextParameterHandle.getHandle() + module.getParameterList().size());
    }
    StringVector name;
    InteractionClassHandleNameMap::const_iterator i = _interactionClassHandleNameMap.find(module.getParentInteractionClassHandle());
    if (i != _interactionClassHandleNameMap.end())
      name = i->second;
    name.push_back(module.getName());
    _nameInteractionDataMap[name]._interactionClass = module;
    if (_nextInteractionClassHandle < module.getInteractionClassHandle())
      _nextInteractionClassHandle = module.getInteractionClassHandle();
    _nameInteractionDataMap[name]._nextParameterHandle = nextParameterHandle;
    _interactionClassHandleNameMap[module.getInteractionClassHandle()] = name;
  }
  void insertObjectClass(const FOMObjectClass& module)
  {
    AttributeHandle nextAttributeHandle;
    // FIXME
    for (NameObjectDataMap::const_iterator i = _nameObjectDataMap.begin(); i != _nameObjectDataMap.end(); ++i) {
      if (i->second._objectClass.getParentObjectClassHandle() == module.getParentObjectClassHandle())
        nextAttributeHandle = AttributeHandle(i->second._nextAttributeHandle.getHandle() + module.getAttributeList().size());
    }
    StringVector name;
    ObjectClassHandleNameMap::const_iterator i = _objectClassHandleNameMap.find(module.getParentObjectClassHandle());
    if (i != _objectClassHandleNameMap.end())
      name = i->second;
    name.push_back(module.getName());
    _nameObjectDataMap[name]._objectClass = module;
    if (_nextObjectClassHandle < module.getObjectClassHandle())
      _nextObjectClassHandle = module.getObjectClassHandle();
    _nameObjectDataMap[name]._nextAttributeHandle = nextAttributeHandle;
    _objectClassHandleNameMap[module.getObjectClassHandle()] = name;
  }
  void insertModuleList(const FOMModuleList& moduleList)
  {
    for (FOMModuleList::const_iterator i = moduleList.begin(); i != moduleList.end(); ++i) {
      FOMModuleHandleFOMModuleMap::const_iterator j = _fomModuleHandleFOMModuleMap.find(i->getFOMModuleHandle());
      if (j != _fomModuleHandleFOMModuleMap.end())
        continue;

      for (FOMTransportationTypeList::const_iterator k = i->getTransportationTypeList().begin();
           k != i->getTransportationTypeList().end(); ++k) {
        insertTransportationType(*k);
      }

      for (FOMDimensionList::const_iterator k = i->getDimensionList().begin();
           k != i->getDimensionList().end(); ++k) {
        insertDimension(*k);
      }

      // FIXME
      // for (FOMSpaceList::const_iterator k = i->getRoutingSpaceList().begin();
      //      k != i->getRoutingSpaceList().end(); ++k) {
      // }

      for (FOMInteractionClassList::const_iterator k = i->getInteractionClassList().begin();
           k != i->getInteractionClassList().end(); ++k) {
        insertInteractionClass(*k);
      }
      for (FOMObjectClassList::const_iterator k = i->getObjectClassList().begin();
           k != i->getObjectClassList().end(); ++k) {
        insertObjectClass(*k);
      }

      _fomModuleHandleFOMModuleMap[i->getFOMModuleHandle()] = *i;
    }
  }

  FOMModule getFOMModule(const FOMModuleHandle& moduleHandle) const
  {
    FOMModuleHandleFOMModuleMap::const_iterator i = _fomModuleHandleFOMModuleMap.find(moduleHandle);
    OpenRTIAssert(i != _fomModuleHandleFOMModuleMap.end());
    return i->second;
  }

  FOMModuleList getModuleList() const
  {
    FOMModuleList moduleList;
    for (FOMModuleHandleFOMModuleMap::const_iterator i = _fomModuleHandleFOMModuleMap.begin();
         i != _fomModuleHandleFOMModuleMap.end(); ++i) {
      moduleList.push_back(i->second);
    }
    return moduleList;
  }

  // private:
  NameTransportationTypeMap _nameTransportationTypeMap;
  NameOrderTypeMap _nameOrderTypeMap;
  NameDimensionHandleMap _nameDimensionHandleMap;
  DimensionHandle _nextDimensionHandle;
  NameInteractionDataMap _nameInteractionDataMap;
  InteractionClassHandleNameMap _interactionClassHandleNameMap;
  InteractionClassHandle _nextInteractionClassHandle;
  NameObjectDataMap _nameObjectDataMap;
  ObjectClassHandleNameMap _objectClassHandleNameMap;
  ObjectClassHandle _nextObjectClassHandle;
  // FOMStringModuleFOMModuleHandleMap _fomStringModuleFOMModuleHandleMap;
  FOMModuleHandleFOMModuleMap _fomModuleHandleFOMModuleMap;
  FOMModuleHandle _nextFOMModuleHandle;
};

FOMModuleSet::FOMModuleSet() :
  _allocatorMap(new AllocatorMap)
{
}

FOMModuleSet::~FOMModuleSet()
{
}

FOMModuleHandleSet
FOMModuleSet::insertModuleList(const FOMStringModuleList& moduleList)
{
  // The candidate that is only committed when successful
  SharedPtr<AllocatorMap> allocatorMap = new AllocatorMap(*_allocatorMap);

  FOMModuleHandleSet fomModuleHandles;
  for (FOMStringModuleList::const_iterator i = moduleList.begin(); i != moduleList.end(); ++i)
    fomModuleHandles.insert(allocatorMap->insertModule(*i));

  // Survived, commit the change
  _allocatorMap.swap(allocatorMap);
  return fomModuleHandles;
}

void
FOMModuleSet::insertModuleList(const FOMModuleList& moduleList)
{
  _allocatorMap->insertModuleList(moduleList);
}

FOMModule
FOMModuleSet::getFOMModule(const FOMModuleHandle& moduleHandle) const
{
  return _allocatorMap->getFOMModule(moduleHandle);
}

FOMModuleList
FOMModuleSet::getModuleList() const
{
  return _allocatorMap->getModuleList();
}

} // namespace OpenRTI
