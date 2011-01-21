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

#ifndef OpenRTI_ObjectModel_h
#define OpenRTI_ObjectModel_h

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <sstream>

#include "Exception.h"
#include "FOMModuleSet.h"
#include "Handle.h"
#include "HandleAllocator.h"
#include "Message.h"
#include "NameHandleMap.h"
#include "NameHandlePair.h"
#include "NameHandleVector.h"
#include "Referenced.h"
#include "RegionSet.h"
#include "SharedPtr.h"
#include "StringUtils.h"
#include "Types.h"
#include "VariableLengthData.h"
#include "LogStream.h"

namespace OpenRTI {

class Federation;
class ObjectClass;
class ObjectClassAttribute;
class ObjectInstance;
class ObjectAttribute;
class InteractionClass;

typedef std::list<ObjectInstance*> ObjectInstanceList;
typedef std::vector<SharedPtr<ObjectAttribute> > HandleObjectAttributeVector;

typedef NameHandleVector<ObjectClassHandle, ObjectClass> ObjectClassVector;
typedef NameHandleVector<AttributeHandle, ObjectClassAttribute> ObjectClassAttributeVector;
typedef NameHandleVector<InteractionClassHandle, InteractionClass> InteractionClassVector;

typedef NameHandleMap<ObjectInstanceHandle, ObjectInstance> ObjectInstanceMap;

// Helper return type for some publish/subscribe stuff.
// Define outside the template class to avoid having different types of this per template
enum PropagationType {
  PropagateBroadcast,
  PropagateSend,
  PropagateNone
};
typedef std::pair<PropagationType, ConnectHandle> PropagationTypeConnectHandlePair;

class OPENRTI_LOCAL BroadcastConnectHandleSet {
public:
  // Returns what to do with the other connects in the server node.
  // If .first is PropagateBroadcast, tell all but this one that sends this publication about the publication change.
  // If .first is PropagateSend, tell the connect at .second about the change.
  // If .first is PropagateNone, nothing has changed for the connects at this server node.
  PropagationTypeConnectHandlePair insert(const ConnectHandle& connectHandle)
  {
    // Mark this connect as publishing
    size_t initialConnectHandleSetSize = _connectHandleSet.size();
    ConnectHandle previousExclusiveConnectHandle;
    if (initialConnectHandleSetSize == 1)
      previousExclusiveConnectHandle = *_connectHandleSet.begin();

    // Did that change something? If not, propagate nothing
    if (!_connectHandleSet.insert(connectHandle).second)
      return PropagationTypeConnectHandlePair(PropagateNone, ConnectHandle());

    switch (initialConnectHandleSetSize) {
    case 0:
      // Nobody was publishing this so far. Tell all *others* about that
      return PropagationTypeConnectHandlePair(PropagateBroadcast, ConnectHandle());
    case 1:
      // One was publishing this so far. Due to not getting the loopback message in the 0 case just send that
      // remaining connect that does not yet know that this server node is already publishing this the publication
      // notification
      return PropagationTypeConnectHandlePair(PropagateSend, previousExclusiveConnectHandle);
    default:
      // Nothing changes, nothing todo
      return PropagationTypeConnectHandlePair(PropagateNone, ConnectHandle());
    }
  }

  // Returns what to do with the other connects in the server node.
  // If .first is PropagateBroadcast, tell all but this one that sends this publication about the publication change.
  // If .first is PropagateSend, tell the connect at .second about the change.
  // If .first is PropagateNone, nothing has changed for the connects at this server node.
  PropagationTypeConnectHandlePair erase(const ConnectHandle& connectHandle)
  {
    // Did that change something? If not, propagate nothing
    if (_connectHandleSet.erase(connectHandle) == 0)
      return PropagationTypeConnectHandlePair(PropagateNone, ConnectHandle());

    switch (_connectHandleSet.size()) {
    case 0:
      return PropagationTypeConnectHandlePair(PropagateBroadcast, ConnectHandle());
    case 1:
      return PropagationTypeConnectHandlePair(PropagateSend, *_connectHandleSet.begin());
    default:
      // Nothing changes, nothing todo
      return PropagationTypeConnectHandlePair(PropagateNone, ConnectHandle());
    }
  }

  // Returns if any existing connect in this server node publishes this
  bool empty() const
  { return _connectHandleSet.empty(); }

  // Returns if the given connect handle publishes this
  bool contains(const ConnectHandle& connectHandle) const
  { return _connectHandleSet.find(connectHandle) != _connectHandleSet.end(); }
  // Returns the subscription type of all connects except the given one
  bool containsMoreThan(const ConnectHandle& connectHandle) const
  {
    size_t size = _connectHandleSet.size();
    if (1 < size)
      return true;
    if (size == 1 && *_connectHandleSet.begin() != connectHandle)
      return true;
    return false;
  }

  const ConnectHandleSet& getConnectHandleSet() const
  { return _connectHandleSet; }

private:
  ConnectHandleSet _connectHandleSet;
};

template<typename H>
class OPENRTI_LOCAL PublishSubscribe : public NameHandlePair<H> {
public:
  typedef H Handle;

  PublishSubscribe(const std::wstring& name, const Handle& handle) :
    NameHandlePair<H>(name, handle)
  { }

  // Change publication state of the given connect.
  // Returns what to do with the other connects in the server node.
  // If .first is PropagateBroadcast, tell all but this one that sends this publication about the publication change.
  // If .first is PropagateSend, tell the connect at .second about the change.
  // If .first is PropagateNone, nothing has changed for the connects at this server node.
  PropagationTypeConnectHandlePair setPublicationType(const ConnectHandle& connectHandle, PublicationType publicationType)
  {
    if (publicationType == Published) {
      return _publishedConnects.insert(connectHandle);
    } else {
      return _publishedConnects.erase(connectHandle);
    }
  }
  // Returns if the given connect handle publishes this
  PublicationType getPublicationType(const ConnectHandle& connectHandle) const
  {
    if (_publishedConnects.contains(connectHandle))
      return Published;
    else
      return Unpublished;
  }
  // Returns if any existing connect in this server node publishes this
  PublicationType getPublicationType() const
  {
    if (_publishedConnects.empty())
      return Unpublished;
    else
      return Published;
  }
  const ConnectHandleSet& getPublishingConnectHandleSet() const
  { return _publishedConnects.getConnectHandleSet(); }

  // FIXME currently only passive subscriptions are propagated

  // Change publication state of the given connect.
  // Returns what to do with the other connects in the server node.
  // If .first is PropagateBroadcast, tell all but this one that sends this publication about the publication change.
  // If .first is PropagateSend, tell the connect at .second about the change.
  // If .first is PropagateNone, nothing has changed for the connects at this server node.
  PropagationTypeConnectHandlePair setSubscriptionType(const ConnectHandle& connectHandle, SubscriptionType subscriptionType)
  {
    // if (subscriptionType == Subscribed) {
    if (subscriptionType != Unsubscribed) {
      return _subscribedConnects.insert(connectHandle);
    } else {
      return _subscribedConnects.erase(connectHandle);
    }
  }

  SubscriptionType getSubscriptionType() const
  {
    if (!_activeSubscribedConnects.empty())
      return SubscribedActive;
    else if (!_subscribedConnects.empty())
      return SubscribedPassive;
    else
      return Unsubscribed;
  }

  SubscriptionType getSubscriptionType(const ConnectHandle& connectHandle) const
  {
    if (_activeSubscribedConnects.contains(connectHandle))
      return SubscribedActive;
    else if (_subscribedConnects.contains(connectHandle))
      return SubscribedPassive;
    else
      return Unsubscribed;
  }
  // Returns the subscription type of all connects except the given one
  SubscriptionType getSubscriptionTypeToConnect(const ConnectHandle& connectHandle) const
  {
    if (_activeSubscribedConnects.containsMoreThan(connectHandle))
      return SubscribedActive;
    if (_subscribedConnects.containsMoreThan(connectHandle))
      return SubscribedPassive;
    return Unsubscribed;
  }

  const ConnectHandleSet& getSubscribedConnectHandleSet() const
  { return _subscribedConnects.getConnectHandleSet(); }
  void getSubscribedAndIntersectingConnectHandleSet(ConnectHandleSet& connectHandleSet, const RegionSet& regionSet) const
  { connectHandleSet = _subscribedConnects.getConnectHandleSet(); }

  void removeConnect(const ConnectHandle& connectHandle)
  {
    // FIXME, should not use this
    setSubscriptionType(connectHandle, Unsubscribed);

    // The the unpublish must have happened before, so just make sure that it is empty
    OpenRTIAssert(!_publishedConnects.contains(connectHandle));
    // The the unsubscribe must have happened before, so just make sure that it is empty
    OpenRTIAssert(!_activeSubscribedConnects.contains(connectHandle));
    OpenRTIAssert(!_subscribedConnects.contains(connectHandle));
  }


  // Same idea like we have now:
  // std::map<Region, BroadcastConnectHandleSet>
  // ... may be little more data like
  // struct RegionData {
  //   BroadcastConnectHandleSet _broadcastConnectHandleSet;
  //   RegionHandleSet _regionHandleSet;
  // };

private:
  // All the connects that publish this
  BroadcastConnectHandleSet _publishedConnects;

  // All the connects that are regionless subscribed at this
  // Since region subscriptions should not interfere with normal subscriptions, have these two and something for
  // regions set subscriptions without any active/passive flag
  BroadcastConnectHandleSet _subscribedConnects;
  BroadcastConnectHandleSet _activeSubscribedConnects;
};

/// FIXME share them among derived object classes ??
class OPENRTI_LOCAL ObjectClassAttribute : public PublishSubscribe<AttributeHandle> {
public:
  ObjectClassAttribute(const std::wstring& name, const AttributeHandle& handle) :
    PublishSubscribe<AttributeHandle>(name, handle)
  { }

};

class OPENRTI_LOCAL ObjectClass : public NameHandlePair<ObjectClassHandle> {
public:
  ObjectClass(const std::wstring& name, const ObjectClassHandle& handle, ObjectClass* parentObjectClass) :
    NameHandlePair<ObjectClassHandle>(name, handle),
    _parentObjectClass(parentObjectClass)
  { }

  /// Get the parent ObjectClass
  ObjectClass* getParentObjectClass() const
  { return _parentObjectClass; }

  /// Get the set of object classe attributes
  const ObjectClassAttributeVector& getObjectClassAttributeVector() const
  { return _objectClassAttributeVector; }

  /// Manipulation routines
  void insertObjectClassAttribute(const SharedPtr<ObjectClassAttribute>& objectClassAttribute)
  {
    OpenRTIAssert(objectClassAttribute.valid());
    AttributeHandle attributeHandle = objectClassAttribute->getHandle();
    OpenRTIAssert(attributeHandle.valid());
    OpenRTIAssert(!_objectClassAttributeVector.exists(attributeHandle));
    _objectClassAttributeVector.insert(objectClassAttribute);
  }
  ObjectClassAttribute* getAttribute(const AttributeHandle& attributeHandle) const
  { return _objectClassAttributeVector.getObject(attributeHandle); }
  ObjectClassAttribute* getPrivilegeToDeleteAttribute() const
  { return _objectClassAttributeVector.getObject(AttributeHandle(0)); }

  /// Get the set of object instances of this object class type
  const ObjectInstanceList& getObjectInstanceList() const
  { return _objectInstanceList; }

  /// Manipulation routines
  void insertObjectInstance(ObjectInstance* objectInstance);
  void eraseObjectInstance(ObjectInstance* objectInstance);

  void removeConnect(const ConnectHandle& connectHandle)
  {
    ObjectClassAttributeVector::iterator i;
    for (i = _objectClassAttributeVector.begin(); i != _objectClassAttributeVector.end(); ++i)
      (*i)->removeConnect(connectHandle);
  }

private:
  /// The pointer to the parent object class, may be 0
  ObjectClass* const _parentObjectClass;

  /// The set of class attributes of this object class type
  ObjectClassAttributeVector _objectClassAttributeVector;

  /// The set of objects of this object class type
  /// Note that the object class does not 'own' the instances, thus
  /// this ObjectInstanceList only stores plain pointers instead of references
  /// The federation owns the objects and consequently carries references
  ObjectInstanceList _objectInstanceList;
};

class OPENRTI_LOCAL ObjectAttribute : public Referenced {
public:
  ObjectAttribute(ObjectClassAttribute* objectClassAttribute) :
    _objectClassAttribute(objectClassAttribute)
  {
    // FIXME exclude the owner connect handle
    _recieveingConnects = _objectClassAttribute->getSubscribedConnectHandleSet();
  }
  ObjectAttribute(ObjectClassAttribute* objectClassAttribute, const RegionSet& regionSet) :
    _objectClassAttribute(objectClassAttribute)
  {
    // FIXME exclude the owner connect handle
    _objectClassAttribute->getSubscribedAndIntersectingConnectHandleSet(_recieveingConnects, regionSet);
  }

  /// Get the attributes handle
  AttributeHandle getHandle() const
  { return _objectClassAttribute->getHandle(); }

  /// Get the attributes name
  const std::wstring& getName() const
  { return _objectClassAttribute->getName(); }

  /// Get the parent ObjectClassAttribute
  ObjectClassAttribute* getObjectClassAttribute()
  { return _objectClassAttribute; }

  /// Get the federate this attribute is owned
  const FederateHandle& getOwnerFederateHandle() const
  { return _ownerFederateHandle; }
  void setOwnerFederateHandle(const FederateHandle& federateHandle)
  { _ownerFederateHandle = federateHandle; }

  /// Get the ConnectHandle this attribute is owned
  const ConnectHandle& getOwnerConnectHandle() const
  { return _ownerConnectHandle; }
  void setOwnerConnectHandle(const ConnectHandle& connectHandle)
  { _ownerConnectHandle = connectHandle; }

  void removeConnect(const ConnectHandle& connectHandle)
  {
    _recieveingConnects.erase(connectHandle);
  }
  void insertConnect(const ConnectHandle& connectHandle)
  {
  }

  // Because of attribute ownership, it is clear for an object attribute where the update
  // stems from, so just have a set of connect handles that want to recieve the updates
  ConnectHandleSet _recieveingConnects;

private:
  /// The pointer to the parent object class attribute
  ObjectClassAttribute* const _objectClassAttribute;

  /// The federate this attribute is owned by
  FederateHandle _ownerFederateHandle;
  /// The connect this attribute is owned by
  ConnectHandle _ownerConnectHandle;
};

class OPENRTI_LOCAL ObjectInstance : public NameHandlePair<ObjectInstanceHandle> {
public:
  ObjectInstance(const std::wstring& name, const ObjectInstanceHandle& handle, ObjectClass* objectClass) :
    NameHandlePair<ObjectInstanceHandle>(name, handle),
    _objectClass(objectClass)
  {
    const ObjectClassAttributeVector& objectClassAttributeVector = objectClass->getObjectClassAttributeVector();
    for (ObjectClassAttributeVector::const_iterator i = objectClassAttributeVector.begin();
         i != objectClassAttributeVector.end(); ++i) {
      SharedPtr<ObjectAttribute> objectAttribute = new ObjectAttribute(i->get());
      OpenRTIAssert(_handleObjectAttributeVector.size() == objectAttribute->getHandle());
      _handleObjectAttributeVector.push_back(objectAttribute);
    }
  }

  /// Get the parent ObjectClass
  ObjectClass* getObjectClass()
  { return _objectClass; }

  /// Get the set of ObjectAttribute pointers
  const HandleObjectAttributeVector& getHandleObjectAttributeVector() const
  { return _handleObjectAttributeVector; }

  ObjectAttribute* getAttribute(const AttributeHandle& attributeHandle) const
  {
    HandleObjectAttributeVector::size_type index = attributeHandle;
    if (_handleObjectAttributeVector.size() <= index)
      return 0;
    return _handleObjectAttributeVector[index].get();
  }
  ObjectAttribute* getPrivilegeToDeleteAttribute() const
  { return getAttribute(AttributeHandle(0)); }

  /// Return the federate that owns this object
  FederateHandle getOwnerFederateHandle() const
  {
    ObjectAttribute* attribute = getPrivilegeToDeleteAttribute();
    if (!attribute)
      return FederateHandle();
    return attribute->getOwnerFederateHandle();
  }
  void setOwnerFederateHandle(const FederateHandle& federateHandle)
  {
    ObjectAttribute* attribute = getPrivilegeToDeleteAttribute();
    if (!attribute)
      return;
    attribute->setOwnerFederateHandle(federateHandle);
  }
  /// Return the connect that owns this object
  ConnectHandle getOwnerConnectHandle() const
  {
    ObjectAttribute* attribute = getPrivilegeToDeleteAttribute();
    if (!attribute)
      return ConnectHandle();
    return attribute->getOwnerConnectHandle();
  }
  void setOwnerConnectHandle(const ConnectHandle& connectHandle)
  {
    ObjectAttribute* attribute = getPrivilegeToDeleteAttribute();
    if (!attribute)
      return;
    attribute->setOwnerConnectHandle(connectHandle);
  }

  // Inserts itself into the list that is usually held in the object class knowing all the object instances
  void insertToObjectClassList(ObjectInstanceList& objectInstanceList)
  {
    _objectInstanceListIterator = objectInstanceList.insert(objectInstanceList.begin(), this);
  }
  // Removes itself from the list that is usually held in the object class knowing all the object instances
  void eraseFromObjectClassList(ObjectInstanceList& objectInstanceList)
  {
    objectInstanceList.erase(_objectInstanceListIterator);
    _objectInstanceListIterator = objectInstanceList.end();
  }

  void removeConnect(const ConnectHandle& connectHandle)
  {
    HandleObjectAttributeVector::const_iterator i;
    for (i = _handleObjectAttributeVector.begin(); i != _handleObjectAttributeVector.end(); ++i) {
      (*i)->removeConnect(connectHandle);
    }
  }

  void insertConnect(const ConnectHandle& connectHandle)
  {
    HandleObjectAttributeVector::const_iterator i;
    for (i = _handleObjectAttributeVector.begin(); i != _handleObjectAttributeVector.end(); ++i) {
      (*i)->insertConnect(connectHandle);
    }
  }

private:
  /// The pointer to the object class this object is an instance of, cannot be zero
  ObjectClass* const _objectClass;
  // We store the position in the list of objects of this type in the object class,
  // this way erasing an object is also O(1).
  ObjectInstanceList::iterator _objectInstanceListIterator;
  // FIXME: also store the federation wide object by handle map iterator
  // ObjectInstanceList::iterator _objectInstanceListIterator;

  /// The set of instance attributes of this object
  HandleObjectAttributeVector _handleObjectAttributeVector;
};

class OPENRTI_LOCAL InteractionClass : public PublishSubscribe<InteractionClassHandle> {
public:
  InteractionClass(const std::wstring& name, const InteractionClassHandle& handle, InteractionClass* parentInteractionClass) :
    PublishSubscribe<InteractionClassHandle>(name, handle),
    _parentInteractionClass(parentInteractionClass)
  { }

  /// Get the parent InteractionClass
  InteractionClass* getParentInteractionClass() const
  { return _parentInteractionClass; }

private:
  /// The pointer to the parent interaction class, may be 0
  InteractionClass* const _parentInteractionClass;
};


class OPENRTI_LOCAL Federation : public NameHandlePair<FederationHandle> {
public:
  Federation(const std::wstring& name, const FederationHandle& handle) :
    NameHandlePair<FederationHandle>(name, handle)
  { }
  ~Federation()
  {
    // The members would be cleaned up in any case, this is to enforce some order in destruction
    _objectInstanceMap.clear();
    _objectClassVector.clear();
    _interactionClassVector.clear();
  }

  // The logical time factorys name.
  // Should vanish ... FIXME
  const std::wstring& getLogicalTimeFactoryName() const
  { return _logicalTimeFactoryName; }
  void setLogicalTimeFactoryName(const std::wstring& logicalTimeFactoryName)
  { _logicalTimeFactoryName = logicalTimeFactoryName; }
  std::wstring _logicalTimeFactoryName;

  /// Object class handling methods
  const ObjectClassVector& getObjectClassVector() const
  { return _objectClassVector; }
  ObjectClassVector& getObjectClassVector()
  { return _objectClassVector; }
  void insertObjectClass(const SharedPtr<ObjectClass>& objectClass)
  {
    OpenRTIAssert(objectClass.valid());
    ObjectClassHandle objectClassHandle = objectClass->getHandle();
    OpenRTIAssert(objectClassHandle.valid());
    OpenRTIAssert(!_objectClassVector.exists(objectClassHandle));
    OpenRTIAssert(!_objectClassVector.exists(objectClass->getName()));
    _objectClassVector.insert(objectClass);
  }
  ObjectClass* getObjectClass(const ObjectClassHandle& objectClassHandle)
  { return _objectClassVector.getObject(objectClassHandle); }

  /// Interaction class handling methods
  const InteractionClassVector& getInteractionClassVector() const
  { return _interactionClassVector; }
  InteractionClassVector& getInteractionClassVector()
  { return _interactionClassVector; }
  void insertInteractionClass(const SharedPtr<InteractionClass>& interactionClass)
  {
    OpenRTIAssert(interactionClass.valid());
    InteractionClassHandle interactionClassHandle = interactionClass->getHandle();
    OpenRTIAssert(interactionClassHandle.valid());
    OpenRTIAssert(!_interactionClassVector.exists(interactionClassHandle));
    OpenRTIAssert(!_interactionClassVector.exists(interactionClass->getName()));
    _interactionClassVector.insert(interactionClass);
  }
  InteractionClass* getInteractionClass(const InteractionClassHandle& interactionClassHandle)
  { return _interactionClassVector.getObject(interactionClassHandle); }


  /// ObjectInstance handling classes
  const ObjectInstanceMap& getObjectInstanceMap() const
  { return _objectInstanceMap; }
  ObjectInstanceMap& getObjectInstanceMap()
  { return _objectInstanceMap; }
  void insertObjectInstance(const SharedPtr<ObjectInstance>& objectInstance)
  {
    OpenRTIAssert(objectInstance.valid());

    ObjectInstanceHandle objectInstanceHandle = objectInstance->getHandle();
    OpenRTIAssert(objectInstanceHandle.valid());

    OpenRTIAssert(!_objectInstanceMap.exists(objectInstanceHandle));
    OpenRTIAssert(!_objectInstanceMap.exists(objectInstance->getName()));

    ObjectClass* objectClass = objectInstance->getObjectClass();
    OpenRTIAssert(objectClass);

    _objectInstanceMap.insert(ObjectInstanceMap::value_type(objectInstanceHandle, objectInstance));
    objectClass->insertObjectInstance(objectInstance.get());
  }
  void eraseObjectInstance(const SharedPtr<ObjectInstance>& objectInstance)
  {
    OpenRTIAssert(objectInstance.valid());

    ObjectClass* objectClass = objectInstance->getObjectClass();
    objectClass->eraseObjectInstance(objectInstance.get());

    ObjectInstanceHandle objectInstanceHandle = objectInstance->getHandle();
    ObjectInstanceMap::iterator i = _objectInstanceMap.find(objectInstanceHandle);

    OpenRTIAssert(i != _objectInstanceMap.end());

    _objectInstanceMap.erase(i);
  }
  ObjectInstance* getObjectInstance(const ObjectInstanceHandle& objectInstanceHandle)
  {
    ObjectInstanceMap::const_iterator i = _objectInstanceMap.find(objectInstanceHandle);
    if (i == _objectInstanceMap.end())
      return 0;
    return i->second.get();
  }

  void removeConnect(const ConnectHandle& connectHandle)
  {
    for (ObjectInstanceMap::const_iterator i = _objectInstanceMap.begin();
         i != _objectInstanceMap.end(); ++i) {
      i->second->removeConnect(connectHandle);
    }
    for (ObjectClassVector::const_iterator i = _objectClassVector.begin();
         i != _objectClassVector.end(); ++i) {
      (*i)->removeConnect(connectHandle);
    }
    for (InteractionClassVector::const_iterator i = _interactionClassVector.begin();
         i != _interactionClassVector.end(); ++i) {
      (*i)->removeConnect(connectHandle);
    }
  }

  void insert(const FOMModuleList& moduleList);
  FOMModuleList getModuleList() const
  { return _fomModuleSet.getModuleList(); }

  FOMModuleSet _fomModuleSet;

protected:
  void insert(const FOMModule& module);
  void insertInteractionClass(const FOMInteractionClass& interactionClass, const InteractionClassHandle& parentHandle);
  void insertObjectClass(const FOMObjectClass& objectClass, const ObjectClassHandle& parentHandle);

  /// The object classes in the current object model
  ObjectClassVector _objectClassVector;

  /// The interactions in the current object model
  InteractionClassVector _interactionClassVector;

  /// The object instances
  ObjectInstanceMap _objectInstanceMap;
};

} // namespace OpenRTI

#endif
