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

  struct Data {
    Data() : _isBaseType(true) { }
    bool _isBaseType;
  };
  struct TransportationData : public Data {
    TransportationType _transportationType;
  };
  typedef std::map<std::string, TransportationData> NameTransportationDataMap;
  struct DimensionData : public Data {
    DimensionHandle _dimensionHandle;
  };
  typedef std::map<std::string, DimensionData> NameDimensionDataMap;
  struct UpdateRateData : public Data {
    UpdateRateData() : _updateRate(0) {}
    double _updateRate;
  };
  typedef std::map<std::string, UpdateRateData> NameUpdateRateDataMap;
  struct InteractionData : public Data {
    FOMInteractionClass _interactionClass;
    // The next free parameter handle to allocate for a derived class
    ParameterHandle _nextParameterHandle; // FIXME determine this dynamically needed for modules referencing only the class but not the parameters
  };
  typedef std::map<StringVector, InteractionData> NameInteractionDataMap;
  typedef std::map<InteractionClassHandle, StringVector> InteractionClassHandleNameMap;

  struct ObjectData : public Data {
    FOMObjectClass _objectClass;
    // The next free attribute handle to allocate for a derived class
    AttributeHandle _nextAttributeHandle; // FIXME determine this dynamically needed for modules referencing only the class but not the parameters
  };
  typedef std::map<StringVector, ObjectData> NameObjectDataMap;
  typedef std::map<ObjectClassHandle, StringVector> ObjectClassHandleNameMap;

  typedef std::map<TransportationType, FOMModuleHandleSet> TransportationTypeFOMModuleHandleSetMap;
  typedef std::map<DimensionHandle, FOMModuleHandleSet> DimensionHandleFOMModuleHandleSetMap;
  typedef std::map<InteractionClassHandle, FOMModuleHandleSet> InteractionClassHandleFOMModuleHandleSetMap;
  typedef std::map<ObjectClassHandle, FOMModuleHandleSet> ObjectClassHandleFOMModuleHandleSetMap;

  OrderType getOrderType(const std::string& name) const
  {
    if (caseCompare(name, "timestamp"))
      return TIMESTAMP;
    else if (caseCompare(name, "receive"))
      return RECEIVE;
    else
      throw ErrorReadingFDD(std::string("Unknown order type \"") + name + "\".");
  }

  void insertTransportationType(const FOMModuleHandle& fomModuleHandle, const FOMStringTransportationType& stringModule, bool isBaseType)
  {
    NameTransportationDataMap::const_iterator i = _nameTransportationDataMap.find(stringModule.getName());
    TransportationType transportationType;
    if (i != _nameTransportationDataMap.end()) {
      transportationType = i->second._transportationType;
    } else {
      if (stringModule.getName() == "HLAreliable")
        transportationType = RELIABLE;
      else /* if (stringModule.getName() == "HLAbestEffort")  and others */
        transportationType = BEST_EFFORT;
      _nameTransportationDataMap[stringModule.getName()]._transportationType = transportationType;
      _nameTransportationDataMap[stringModule.getName()]._isBaseType = isBaseType;
    }
    _transportationTypeFOMModuleHandleSetMap[transportationType].insert(fomModuleHandle);
  }
  TransportationType getTransportationType(const std::string& name) const
  {
    NameTransportationDataMap::const_iterator i = _nameTransportationDataMap.find(name);
    if (i == _nameTransportationDataMap.end())
      throw ErrorReadingFDD(std::string("Unknown transportation type \"") + name + "\".");
    return i->second._transportationType;
  }

  void insertDimension(const FOMModuleHandle& fomModuleHandle, const FOMStringDimension& stringModule, bool isBaseType)
  {
    NameDimensionDataMap::const_iterator i = _nameDimensionDataMap.find(stringModule.getName());
    DimensionHandle dimensionHandle;
    if (i != _nameDimensionDataMap.end()) {
      dimensionHandle = i->second._dimensionHandle;
    } else {
      dimensionHandle = _dimensionHandleAllocator.get();
      _nameDimensionDataMap[stringModule.getName()]._dimensionHandle = dimensionHandle;
      _nameDimensionDataMap[stringModule.getName()]._isBaseType = isBaseType;
    }
    _dimensionHandleFOMModuleHandleSetMap[dimensionHandle].insert(fomModuleHandle);
  }
  DimensionHandle getDimensionHandle(const std::string& name) const
  {
    NameDimensionDataMap::const_iterator i = _nameDimensionDataMap.find(name);
    if (i == _nameDimensionDataMap.end())
      throw ErrorReadingFDD(std::string("Unknown dimension \"") + name + "\".");
    return i->second._dimensionHandle;
  }

  void resolveDimensionHandles(DimensionHandleSet& dimensionHandles, const StringSet& stringDimensions)
  {
    for (StringSet::const_iterator i = stringDimensions.begin(); i != stringDimensions.end(); ++i) {
      dimensionHandles.insert(getDimensionHandle(*i));
    }
  }

  ParameterHandle insertParameterList(FOMParameterList& module, const FOMModuleHandle& fomModuleHandle,
                                      const FOMStringParameterList& stringModule, ParameterHandle nextParameterHandle)
  {
    for (FOMStringParameterList::const_iterator i = stringModule.begin(); i != stringModule.end(); ++i) {
      FOMParameter parameter;
      parameter.setName(i->getName());
      parameter.setParameterHandle(++nextParameterHandle);
      module.push_back(parameter);
    }
    return nextParameterHandle;
  }

  void insertInteractionClass(FOMInteractionClass& module, const FOMModuleHandle& fomModuleHandle,
                              const FOMStringInteractionClass& stringModule, bool isBaseType)
  {
    InteractionClassHandle parentInteractionClassHandle;
    ParameterHandle nextParameterHandle;
    StringVector parentName = stringModule.getName();
    parentName.pop_back();
    if (!parentName.empty()) {
      NameInteractionDataMap::iterator i = _nameInteractionDataMap.find(parentName);
      // FIXME more an internal error??
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
        insertParameterList(parameterList, fomModuleHandle, stringModule.getParameterList(), nextParameterHandle);
        if (parameterList != i->second._interactionClass.getParameterList())
          throw ErrorReadingFDD(std::string("Incompatible InteractionClass \"") + fqClassName(stringModule.getName())
                                + "\": Parameter list does not match.");
      }
    } else {
      i = _nameInteractionDataMap.insert(NameInteractionDataMap::value_type(stringModule.getName(), InteractionData())).first;

      i->second._interactionClass.setName(stringModule.getName().back());

      i->second._interactionClass.setInteractionClassHandle(_interactionClassHandleAllocator.get());
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

      i->second._nextParameterHandle = insertParameterList(i->second._interactionClass.getParameterList(), fomModuleHandle,
                                                           stringModule.getParameterList(), nextParameterHandle);
      i->second._isBaseType = isBaseType;
    }

    module = i->second._interactionClass;

    _interactionClassHandleFOMModuleHandleSetMap[module.getInteractionClassHandle()].insert(fomModuleHandle);
    if (stringModule.getParameterList().empty()) {
      module.getParameterList().clear();
    } else {
      _interactionClassHandleParametersFOMModuleHandleSetMap[module.getInteractionClassHandle()].insert(fomModuleHandle);
    }
  }

  void insertInteractionClassList(FOMInteractionClassList& module, const FOMModuleHandle& fomModuleHandle,
                                  const FOMStringInteractionClassList& stringModule, bool isBaseType)
  {
    // skip modules that introduce an empty root and nothing beyond that
    if (fomModuleHandle.getHandle() && stringModule.size() == 1)
      return;
    for (FOMStringInteractionClassList::const_iterator i = stringModule.begin(); i != stringModule.end(); ++i) {
      module.push_back(FOMInteractionClass());
      insertInteractionClass(module.back(), fomModuleHandle, *i, isBaseType);
    }
  }

  AttributeHandle insertAttributeList(FOMAttributeList& module, const FOMModuleHandle& fomModuleHandle,
                                      const FOMStringAttributeList& stringModule, AttributeHandle nextAttributeHandle)
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

  void insertObjectClass(FOMObjectClass& module, const FOMModuleHandle& fomModuleHandle, const FOMStringObjectClass& stringModule, bool isBaseType)
  {
    ObjectClassHandle parentObjectClassHandle;
    AttributeHandle nextAttributeHandle;
    StringVector parentName = stringModule.getName();
    parentName.pop_back();
    if (!parentName.empty()) {
      NameObjectDataMap::iterator i = _nameObjectDataMap.find(parentName);
      // FIXME more an internal error??
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
        insertAttributeList(attributeList, fomModuleHandle, stringModule.getAttributeList(), nextAttributeHandle);
        if (attributeList != i->second._objectClass.getAttributeList())
          throw ErrorReadingFDD(std::string("Incompatible ObjectClass \"") + fqClassName(stringModule.getName())
                                + "\": Attribute list does not match.");
      }
    } else {
      i = _nameObjectDataMap.insert(NameObjectDataMap::value_type(stringModule.getName(), ObjectData())).first;

      i->second._objectClass.setName(stringModule.getName().back());

      i->second._objectClass.setObjectClassHandle(_objectClassHandleAllocator.get());
      i->second._objectClass.setParentObjectClassHandle(parentObjectClassHandle);
      _objectClassHandleNameMap[i->second._objectClass.getObjectClassHandle()] = stringModule.getName();

      i->second._nextAttributeHandle = insertAttributeList(i->second._objectClass.getAttributeList(), fomModuleHandle,
                                                           stringModule.getAttributeList(), nextAttributeHandle);
      i->second._isBaseType = isBaseType;
    }

    module = i->second._objectClass;

    _objectClassHandleFOMModuleHandleSetMap[module.getObjectClassHandle()].insert(fomModuleHandle);
    if (stringModule.getAttributeList().empty()) {
      module.getAttributeList().clear();
    } else {
      _objectClassHandleAttributesFOMModuleHandleSetMap[module.getObjectClassHandle()].insert(fomModuleHandle);
    }
  }

  void insertObjectClassList(FOMObjectClassList& module, const FOMModuleHandle& fomModuleHandle,
                             const FOMStringObjectClassList& stringModule, bool isBaseType)
  {
    // skip modules that introduce an empty root and nothing beyond that
    if (fomModuleHandle.getHandle() && stringModule.size() == 1)
      return;
    for (FOMStringObjectClassList::const_iterator i = stringModule.begin(); i != stringModule.end(); ++i) {
      module.push_back(FOMObjectClass());
      insertObjectClass(module.back(), fomModuleHandle, *i, isBaseType);
    }
  }

  FOMModuleHandle insertModule(const FOMStringModule& stringModule, bool isBaseType)
  {
    FOMModuleHandle fomModuleHandle = _fomModuleHandleAllocator.get();

    FOMModule module;
    for (FOMStringTransportationTypeList::const_iterator k = stringModule.getTransportationTypeList().begin();
         k != stringModule.getTransportationTypeList().end(); ++k) {
      insertTransportationType(fomModuleHandle, *k, isBaseType);

      // A new transportation type, insert that into the module candidate
      FOMTransportationType transportationType;
      transportationType.setName(k->getName());
      transportationType.setTransportationType(getTransportationType(k->getName()));
      module.getTransportationTypeList().push_back(transportationType);
    }

    for (FOMStringUpdateRateList::const_iterator k = stringModule.getUpdateRateList().begin();
         k != stringModule.getUpdateRateList().end(); ++k) {
      // FIXME
      // insertUpdateRate(fomModuleHandle, *k, isBaseType);

      FOMUpdateRate updateRate;
      updateRate.setName(k->getName());
      updateRate.setRate(k->getRate());
      module.getUpdateRateList().push_back(updateRate);
    }

    for (FOMStringDimensionList::const_iterator k = stringModule.getDimensionList().begin();
         k != stringModule.getDimensionList().end(); ++k) {
      insertDimension(fomModuleHandle, *k, isBaseType);

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

    insertInteractionClassList(module.getInteractionClassList(), fomModuleHandle, stringModule.getInteractionClassList(), isBaseType);
    insertObjectClassList(module.getObjectClassList(), fomModuleHandle, stringModule.getObjectClassList(), isBaseType);

    module.setArtificialInteractionRoot(stringModule.getArtificialInteractionRoot());
    module.setArtificialObjectRoot(stringModule.getArtificialObjectRoot());

    module.setFOMModuleHandle(fomModuleHandle);
    _fomModuleHandleFOMModuleMap[fomModuleHandle] = module;
    if (isBaseType)
      _baseFOMModuleList.push_back(module);

    return fomModuleHandle;
  }


  void insertTransportationType(const FOMModuleHandle& fomModuleHandle, const FOMTransportationType& module, bool isBaseType)
  {
    _nameTransportationDataMap[module.getName()]._transportationType = module.getTransportationType();
    _nameTransportationDataMap[module.getName()]._isBaseType = isBaseType;
    _transportationTypeFOMModuleHandleSetMap[module.getTransportationType()].insert(fomModuleHandle);
  }
  void insertDimension(const FOMModuleHandle& fomModuleHandle, const FOMDimension& module, bool isBaseType)
  {
    if (_nameDimensionDataMap.find(module.getName()) == _nameDimensionDataMap.end()) {
      _nameDimensionDataMap[module.getName()]._dimensionHandle = module.getDimensionHandle();
      _nameDimensionDataMap[module.getName()]._isBaseType = isBaseType;
      _dimensionHandleAllocator.take(module.getDimensionHandle());
    }
    _dimensionHandleFOMModuleHandleSetMap[module.getDimensionHandle()].insert(fomModuleHandle);
  }
  void insertInteractionClass(const FOMModuleHandle& fomModuleHandle, const FOMInteractionClass& module, bool isBaseType)
  {
    if (_interactionClassHandleNameMap.find(module.getInteractionClassHandle()) == _interactionClassHandleNameMap.end()) {
      ParameterHandle nextParameterHandle;
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
      _nameInteractionDataMap[name]._isBaseType = isBaseType;
      _interactionClassHandleAllocator.take(module.getInteractionClassHandle());
      _nameInteractionDataMap[name]._nextParameterHandle = nextParameterHandle;
      _interactionClassHandleNameMap[module.getInteractionClassHandle()] = name;
    }
    _interactionClassHandleFOMModuleHandleSetMap[module.getInteractionClassHandle()].insert(fomModuleHandle);
    if (!module.getParameterList().empty())
      _interactionClassHandleParametersFOMModuleHandleSetMap[module.getInteractionClassHandle()].insert(fomModuleHandle);
  }
  void insertObjectClass(const FOMModuleHandle& fomModuleHandle, const FOMObjectClass& module, bool isBaseType)
  {
    if (_objectClassHandleNameMap.find(module.getObjectClassHandle()) == _objectClassHandleNameMap.end()) {
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
      _nameObjectDataMap[name]._isBaseType = isBaseType;
      _objectClassHandleAllocator.take(module.getObjectClassHandle());
      _nameObjectDataMap[name]._nextAttributeHandle = nextAttributeHandle;
      _objectClassHandleNameMap[module.getObjectClassHandle()] = name;
    }
    _objectClassHandleFOMModuleHandleSetMap[module.getObjectClassHandle()].insert(fomModuleHandle);
    if (!module.getAttributeList().empty())
      _objectClassHandleAttributesFOMModuleHandleSetMap[module.getObjectClassHandle()].insert(fomModuleHandle);
  }
  void insertModule(const FOMModule& module, bool isBaseType)
  {
    FOMModuleHandleFOMModuleMap::const_iterator i = _fomModuleHandleFOMModuleMap.find(module.getFOMModuleHandle());
    if (i != _fomModuleHandleFOMModuleMap.end())
      return;

    for (FOMTransportationTypeList::const_iterator j = module.getTransportationTypeList().begin();
         j != module.getTransportationTypeList().end(); ++j) {
      insertTransportationType(module.getFOMModuleHandle(), *j, isBaseType);
    }

    for (FOMDimensionList::const_iterator j = module.getDimensionList().begin();
         j != module.getDimensionList().end(); ++j) {
      insertDimension(module.getFOMModuleHandle(), *j, isBaseType);
    }

    // FIXME
    // for (FOMSpaceList::const_iterator j = module.getRoutingSpaceList().begin();
    //      j != module.getRoutingSpaceList().end(); ++j) {
    // }

    for (FOMInteractionClassList::const_iterator j = module.getInteractionClassList().begin();
         j != module.getInteractionClassList().end(); ++j) {
      insertInteractionClass(module.getFOMModuleHandle(), *j, isBaseType);
    }
    for (FOMObjectClassList::const_iterator j = module.getObjectClassList().begin();
         j != module.getObjectClassList().end(); ++j) {
      insertObjectClass(module.getFOMModuleHandle(), *j, isBaseType);
    }

    // FIXME
    // for (FOMUpdateRateList::const_iterator j = module.getUpdateRateList().begin();
    //      j != module.getUpdateRateList().end(); ++j) {
    // }

    _fomModuleHandleFOMModuleMap[module.getFOMModuleHandle()] = module;

    if (isBaseType)
      _baseFOMModuleList.push_back(module);
  }

  void insertModuleList(const FOMModuleList& moduleList, bool isBaseType)
  {
    for (FOMModuleList::const_iterator i = moduleList.begin(); i != moduleList.end(); ++i) {
      insertModule(*i, isBaseType);
    }
  }


  bool eraseTransportationType(const FOMModuleHandle& fomModuleHandle, const FOMTransportationType& module)
  {
    _transportationTypeFOMModuleHandleSetMap[module.getTransportationType()].erase(fomModuleHandle);
    if (!_transportationTypeFOMModuleHandleSetMap[module.getTransportationType()].empty())
      return false;
    OpenRTIAssert(!_nameTransportationDataMap[module.getName()]._isBaseType);
    _nameTransportationDataMap.erase(module.getName());
    return true;
  }
  bool eraseDimension(const FOMModuleHandle& fomModuleHandle, const FOMDimension& module)
  {
    _dimensionHandleFOMModuleHandleSetMap[module.getDimensionHandle()].erase(fomModuleHandle);
    if (!_dimensionHandleFOMModuleHandleSetMap[module.getDimensionHandle()].empty())
      return false;
    OpenRTIAssert(!_nameDimensionDataMap[module.getName()]._isBaseType);
    _nameDimensionDataMap.erase(module.getName());
    _dimensionHandleAllocator.put(module.getDimensionHandle());
    return true;
  }
  bool eraseInteractionClass(const FOMModuleHandle& fomModuleHandle, const FOMInteractionClass& module)
  {
    _interactionClassHandleFOMModuleHandleSetMap[module.getInteractionClassHandle()].erase(fomModuleHandle);
    StringVector name = _interactionClassHandleNameMap[module.getInteractionClassHandle()];
    if (!module.getParameterList().empty()) {
      _interactionClassHandleParametersFOMModuleHandleSetMap[module.getInteractionClassHandle()].erase(fomModuleHandle);
      if (_interactionClassHandleParametersFOMModuleHandleSetMap[module.getInteractionClassHandle()].empty()) {
        _nameInteractionDataMap[name]._interactionClass.getParameterList().clear();
        ParameterHandle nextParameterHandle;
        // FIXME
        for (NameInteractionDataMap::const_iterator i = _nameInteractionDataMap.begin(); i != _nameInteractionDataMap.end(); ++i) {
          if (i->second._interactionClass.getParentInteractionClassHandle() != module.getParentInteractionClassHandle())
            continue;
          nextParameterHandle = ParameterHandle(i->second._nextParameterHandle.getHandle() + module.getParameterList().size());
          break;
        }
        _nameInteractionDataMap[name]._nextParameterHandle = nextParameterHandle;
      }
    }
    if (!_interactionClassHandleFOMModuleHandleSetMap[module.getInteractionClassHandle()].empty())
      return false;
    OpenRTIAssert(!_nameInteractionDataMap[name]._isBaseType);
    OpenRTIAssert(_interactionClassHandleParametersFOMModuleHandleSetMap[module.getInteractionClassHandle()].empty());
    _interactionClassHandleNameMap.erase(module.getInteractionClassHandle());
    _nameInteractionDataMap.erase(name);
    _interactionClassHandleAllocator.put(module.getInteractionClassHandle());
    return true;
  }
  bool eraseObjectClass(const FOMModuleHandle& fomModuleHandle, const FOMObjectClass& module)
  {
    _objectClassHandleFOMModuleHandleSetMap[module.getObjectClassHandle()].erase(fomModuleHandle);
    StringVector name = _objectClassHandleNameMap[module.getObjectClassHandle()];
    if (!module.getAttributeList().empty()) {
      _objectClassHandleAttributesFOMModuleHandleSetMap[module.getObjectClassHandle()].erase(fomModuleHandle);
      if (_objectClassHandleAttributesFOMModuleHandleSetMap[module.getObjectClassHandle()].empty()) {
        _nameObjectDataMap[name]._objectClass.getAttributeList().clear();
        AttributeHandle nextAttributeHandle;
        // FIXME
        for (NameObjectDataMap::const_iterator i = _nameObjectDataMap.begin(); i != _nameObjectDataMap.end(); ++i) {
          if (i->second._objectClass.getParentObjectClassHandle() != module.getParentObjectClassHandle())
            continue;
          nextAttributeHandle = AttributeHandle(i->second._nextAttributeHandle.getHandle() + module.getAttributeList().size());
          break;
        }
        _nameObjectDataMap[name]._nextAttributeHandle = nextAttributeHandle;
      }
    }
    if (!_objectClassHandleFOMModuleHandleSetMap[module.getObjectClassHandle()].empty())
      return false;
    OpenRTIAssert(!_nameObjectDataMap[name]._isBaseType);
    OpenRTIAssert(_objectClassHandleAttributesFOMModuleHandleSetMap[module.getObjectClassHandle()].empty());
    _objectClassHandleNameMap.erase(module.getObjectClassHandle());
    _nameObjectDataMap.erase(name);
    _objectClassHandleAllocator.put(module.getObjectClassHandle());
    return true;
  }

  FOMModule eraseModule(const FOMModuleHandle& moduleHandle)
  {
    FOMModule erasedHandles;

    FOMModuleHandleFOMModuleMap::iterator i = _fomModuleHandleFOMModuleMap.find(moduleHandle);
    if (i == _fomModuleHandleFOMModuleMap.end())
      return erasedHandles;

    // Throw out the module first so that we can easier look for multiple used entities
    // No, store for every entity a set of referencing fom module handles. Once this is empty, remove
    FOMModule module = i->second;

    // FIXME
    // for (FOMUpdateRateList::const_iterator j = module.getUpdateRateList().end();
    //      j != module.getUpdateRateList().begin();) {
    //    --j;
    // }

    for (FOMObjectClassList::const_iterator j = module.getObjectClassList().end();
         j != module.getObjectClassList().begin();) {
      if (!eraseObjectClass(module.getFOMModuleHandle(), *--j))
        continue;
      erasedHandles.getObjectClassList().push_back(*j);
    }
    for (FOMInteractionClassList::const_iterator j = module.getInteractionClassList().end();
         j != module.getInteractionClassList().begin();) {
      if (!eraseInteractionClass(module.getFOMModuleHandle(), *--j))
        continue;
      erasedHandles.getInteractionClassList().push_back(*j);
    }

    // FIXME
    // for (FOMSpaceList::const_iterator j = module.getRoutingSpaceList().end();
    //      j != module.getRoutingSpaceList().begin();) {
    //    --j;
    // }

    for (FOMDimensionList::const_iterator j = module.getDimensionList().end();
         j != module.getDimensionList().begin();) {
      if (!eraseDimension(module.getFOMModuleHandle(), *--j))
        continue;
      erasedHandles.getDimensionList().push_back(*j);
    }

    for (FOMTransportationTypeList::const_iterator j = module.getTransportationTypeList().end();
         j != module.getTransportationTypeList().begin();) {
      if (!eraseTransportationType(module.getFOMModuleHandle(), *--j))
        continue;
      erasedHandles.getTransportationTypeList().push_back(*j);
    }

    _fomModuleHandleFOMModuleMap.erase(i);
    _fomModuleHandleAllocator.put(moduleHandle);

    return erasedHandles;
  }
  // returns the components from the module list that are actually erased.
  // but dont trust anyting but the handles and names.
  FOMModuleList eraseModuleList(const FOMModuleHandleVector& moduleList)
  {
    FOMModuleList erasedHandles;
    for (FOMModuleHandleVector::const_iterator i = moduleList.begin(); i != moduleList.end(); ++i) {
      erasedHandles.push_back(eraseModule(*i));
    }
    return erasedHandles;
  }

  FOMModule getFOMModule(const FOMModuleHandle& moduleHandle) const
  {
    FOMModuleHandleFOMModuleMap::const_iterator i = _fomModuleHandleFOMModuleMap.find(moduleHandle);
    OpenRTIAssert(i != _fomModuleHandleFOMModuleMap.end());
    return i->second;
  }

  FOMModuleList getModuleList(const FOMModuleHandleVector& moduleHandles) const
  {
    FOMModuleList moduleList;
    for (FOMModuleHandleVector::const_iterator i = moduleHandles.begin(); i != moduleHandles.end(); ++i) {
      FOMModuleHandleFOMModuleMap::const_iterator j = _fomModuleHandleFOMModuleMap.find(*i);
      if (j == _fomModuleHandleFOMModuleMap.end())
        continue;
      moduleList.push_back(j->second);
    }
    return moduleList;
  }

  FOMModuleList getBaseModuleList() const
  {
    return _baseFOMModuleList;
  }


  // private:
  NameTransportationDataMap _nameTransportationDataMap;
  TransportationTypeFOMModuleHandleSetMap _transportationTypeFOMModuleHandleSetMap;

  /// UpdateRate???? FIXME

  DimensionHandleAllocator _dimensionHandleAllocator;
  NameDimensionDataMap _nameDimensionDataMap;
  DimensionHandleFOMModuleHandleSetMap _dimensionHandleFOMModuleHandleSetMap;

  InteractionClassHandleAllocator _interactionClassHandleAllocator;
  NameInteractionDataMap _nameInteractionDataMap;
  InteractionClassHandleNameMap _interactionClassHandleNameMap;
  InteractionClassHandleFOMModuleHandleSetMap _interactionClassHandleFOMModuleHandleSetMap;
  InteractionClassHandleFOMModuleHandleSetMap _interactionClassHandleParametersFOMModuleHandleSetMap;

  ObjectClassHandleAllocator _objectClassHandleAllocator;
  NameObjectDataMap _nameObjectDataMap;
  ObjectClassHandleNameMap _objectClassHandleNameMap;
  ObjectClassHandleFOMModuleHandleSetMap _objectClassHandleFOMModuleHandleSetMap;
  ObjectClassHandleFOMModuleHandleSetMap _objectClassHandleAttributesFOMModuleHandleSetMap;

  FOMModuleList _baseFOMModuleList;
  FOMModuleHandleFOMModuleMap _fomModuleHandleFOMModuleMap;
  FOMModuleHandleAllocator _fomModuleHandleAllocator;
};

FOMModuleSet::FOMModuleSet() :
  _allocatorMap(new AllocatorMap)
{
}

FOMModuleSet::~FOMModuleSet()
{
}

FOMModuleHandleSet
FOMModuleSet::insertModuleList(const FOMStringModuleList& moduleList, bool isBaseType)
{
  // The candidate that is only committed when successful
  SharedPtr<AllocatorMap> allocatorMap = new AllocatorMap(*_allocatorMap);

  FOMModuleHandleSet fomModuleHandles;
  for (FOMStringModuleList::const_iterator i = moduleList.begin(); i != moduleList.end(); ++i)
    fomModuleHandles.insert(allocatorMap->insertModule(*i, isBaseType));

  // Survived, commit the change
  _allocatorMap.swap(allocatorMap);
  return fomModuleHandles;
}

bool
FOMModuleSet::testModuleList(const FOMStringModuleList& moduleList)
{
  // The candidate that is only committed when successful
  SharedPtr<AllocatorMap> allocatorMap = new AllocatorMap(*_allocatorMap);

  for (FOMStringModuleList::const_iterator i = moduleList.begin(); i != moduleList.end(); ++i)
    allocatorMap->insertModule(*i, false);

  // Survived, throw the result away ...
  return true;
}

void
FOMModuleSet::insertModuleList(const FOMModuleList& moduleList, bool isBaseType)
{
  _allocatorMap->insertModuleList(moduleList, isBaseType);
}

FOMModuleList
FOMModuleSet::eraseModuleList(const FOMModuleHandleVector& moduleList)
{
  return _allocatorMap->eraseModuleList(moduleList);
}

FOMModule
FOMModuleSet::getFOMModule(const FOMModuleHandle& moduleHandle) const
{
  return _allocatorMap->getFOMModule(moduleHandle);
}

FOMModuleList
FOMModuleSet::getModuleList(const FOMModuleHandleSet& moduleList) const
{
  FOMModuleHandleVector moduleHandles;
  for (FOMModuleHandleSet::const_iterator i = moduleList.begin(); i != moduleList.end(); ++i)
    moduleHandles.push_back(*i);
  return _allocatorMap->getModuleList(moduleHandles);
}

FOMModuleList
FOMModuleSet::getModuleList(const FOMModuleHandleVector& moduleList) const
{
  return _allocatorMap->getModuleList(moduleList);
}

FOMModuleList
FOMModuleSet::getBaseModuleList() const
{
  return _allocatorMap->getBaseModuleList();
}

} // namespace OpenRTI
