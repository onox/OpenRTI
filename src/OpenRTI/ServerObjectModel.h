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

#ifndef OpenRTI_ServerObjectModel_h
#define OpenRTI_ServerObjectModel_h

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <sstream>

#include "AbstractMessageSender.h"
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

  PublishSubscribe(const std::string& name, const Handle& handle) :
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
  ObjectClassAttribute(const std::string& name, const AttributeHandle& handle) :
    PublishSubscribe<AttributeHandle>(name, handle)
  { }

};

class OPENRTI_LOCAL ObjectClass : public NameHandlePair<ObjectClassHandle> {
public:
  ObjectClass(const std::string& name, const ObjectClassHandle& handle, ObjectClass* parentObjectClass) :
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
  const std::string& getName() const
  { return _objectClassAttribute->getName(); }

  /// Get the parent ObjectClassAttribute
  ObjectClassAttribute* getObjectClassAttribute()
  { return _objectClassAttribute; }

  /// Get the ConnectHandle this attribute is owned
  const ConnectHandle& getOwnerConnectHandle() const
  { return _ownerConnectHandle; }
  void setOwnerConnectHandle(const ConnectHandle& connectHandle)
  { _ownerConnectHandle = connectHandle; }

  void removeConnect(const ConnectHandle& connectHandle)
  {
    _recieveingConnects.erase(connectHandle);
    if (_ownerConnectHandle == connectHandle)
      _ownerConnectHandle = ConnectHandle();
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

  /// The connect this attribute is owned by
  ConnectHandle _ownerConnectHandle;
};

class OPENRTI_LOCAL ObjectInstance : public NameHandlePair<ObjectInstanceHandle> {
public:
  ObjectInstance(const std::string& name, const ObjectInstanceHandle& handle, ObjectClass* objectClass) :
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

  bool getIsPrivilegeToDeleteAttributeSubscribed() const
  {
    ObjectAttribute* attribute = getPrivilegeToDeleteAttribute();
    if (!attribute)
      return false;
    return !attribute->_recieveingConnects.empty();
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
  InteractionClass(const std::string& name, const InteractionClassHandle& handle, InteractionClass* parentInteractionClass) :
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
  Federation(const std::string& name, const FederationHandle& handle) :
    NameHandlePair<FederationHandle>(name, handle)
  { }

  // The logical time factorys name.
  // Should vanish ... FIXME
  const std::string& getLogicalTimeFactoryName() const
  { return _logicalTimeFactoryName; }
  void setLogicalTimeFactoryName(const std::string& logicalTimeFactoryName)
  { _logicalTimeFactoryName = logicalTimeFactoryName; }
  std::string _logicalTimeFactoryName;
};

/// AttributeUpdateBroadcastList !???!

// class AttributeUpdateMessageFilter : public Referenced {
// public:
//   virtual ~AttributeUpdateMessageFilter() {}
//   AttributeUpdateMessage* filter(AttributeUpdateMessage* message) const = 0;
//   TimeStampedAttributeUpdateMessage* filter(TimeStampedAttributeUpdateMessage* message) const = 0;
// };

// class FullAttributeUpdateMessageFilter : public AttributeUpdateMessageFilter {
// public:
//   virtual ~FullAttributeUpdateMessageFilter() {}
//   AttributeUpdateMessage* filter(AttributeUpdateMessage* message) const
//   { return message; }
//   TimeStampedAttributeUpdateMessage* filter(TimeStampedAttributeUpdateMessage* message) const
//   { return message; }
// };

// class PartialAttributeUpdateMessageFilter : public AttributeUpdateMessageFilter {
// public:
//   virtual ~PartialAttributeUpdateMessageFilter() {}
//   AttributeUpdateMessage* filter(AttributeUpdateMessage* message) const
//   {
//     AttributeUpdateMessage* filteredMessage = new AttributeUpdateMessage;
//     filteredMessage->setFederationhandle(message->getFederationHandle());
//     filteredMessage->setObjectInstanceHandle(message->getObjectInstanceHandle());
//     filteredMessage->setTag(message->getTag());
//     filteredMessage->setTransportationType(message->getTransportationType());
//     filteredMessage->getAttributeValues().reserve(_attributeHandleSet.size());
//     for (AttributeValueVector::const_iterator i = message->getAttributeValues().begin();
//          i != message->getAttributeValues().end(); ++i) {
//       if (_attributeHandleSet.find(i->getAttributeHandle()) == _attributeHandleSet.end())
//         continue;
//       filteredMessage->getAttributeValues().push_back(*i);
//     }
//     return filteredMessage;
//   }
//   TimeStampedAttributeUpdateMessage* filter(TimeStampedAttributeUpdateMessage* message) const
//   {
//     AttributeUpdateMessage* filteredMessage = new AttributeUpdateMessage;
//     filteredMessage->setFederationhandle(message->getFederationHandle());
//     filteredMessage->setObjectInstanceHandle(message->getObjectInstanceHandle());
//     filteredMessage->setTag(message->getTag());
//     filteredMessage->setTransportationType(message->getTransportationType());
//     filteredMessage->setTimeStamp(message->getTimeStamp());
//     filteredMessage->setMessageRetractionHandle(message->getMessageRetractionHandle());
//     filteredMessage->getAttributeValues().reserve(_attributeHandleSet.size());
//     // FIXME? O(n*log(n)), but could be O(n), if messages had a guaranteed sorted attribute vector
//     for (AttributeValueVector::const_iterator i = message->getAttributeValues().begin();
//          i != message->getAttributeValues().end(); ++i) {
//       if (_attributeHandleSet.find(i->getAttributeHandle()) == _attributeHandleSet.end())
//         continue;
//       filteredMessage->getAttributeValues().push_back(*i);
//     }
//     return filteredMessage;
//   }
//   const AttributeHandleSet& getAttributehandleSet() const
//   { return _attributeHandleSet; }
//   AttributeHandleSet& getAttributehandleSet()
//   { return _attributeHandleSet; }
// private:
//   AttributeHandleSet _attributeHandleSet;
// };

// typedef std::pair<ConnectHandleSet, SharedPtr<AttributeUpdateMessageFilter> > ConnectHandleSetAttributeUpdateMessageFilterPair;
// typedef std::vector<ConnectHandleSetAttributeUpdateMessageFilterPair> AttributeUpdateBroadcastList;

/// AttributeUpdateBroadcastList !???!

class OPENRTI_LOCAL SynchronizationState {
public:
  SynchronizationState() :
    _addJoiningFederates(true)
  { }

  void set(const FederateHandleSet& participatingFederates, const FederateHandleSet& joinedFederates)
  {
    if (participatingFederates.empty()) {
      _participatingFederates = joinedFederates;
      _waitFederates = joinedFederates;
      _addJoiningFederates = true;
    } else {
      _participatingFederates = participatingFederates;
      _waitFederates = participatingFederates;
      _addJoiningFederates = false;
    }
  }
  // Use this to build up the initial connect handle set
  void insert(const FederateHandle& federateHandle)
  {
    _participatingFederates.insert(federateHandle);
    _waitFederates.insert(federateHandle);
  }
  void insert(const FederateHandleSet& federateHandleSet)
  {
    _participatingFederates.insert(federateHandleSet.begin(), federateHandleSet.end());
    _waitFederates.insert(federateHandleSet.begin(), federateHandleSet.end());
  }
  // Call when a federate resigns
  void removeFederate(const FederateHandle& federateHandle)
  {
    _participatingFederates.erase(federateHandle);
    _waitFederates.erase(federateHandle);
  }

  VariableLengthData& getTag()
  { return _tag; }
  const VariableLengthData& getTag() const
  { return _tag; }

// private:
  bool _addJoiningFederates;
  VariableLengthData _tag;
  // The set of federates that was given in the RegisterFederationSynchronizationPoint.
  // If this is empty all joined federates should be taken instead.
  FederateHandleSet _participatingFederates;
  // The set of federates we need to wait for until the synchronization point could be announced to the parent.
  // This one must be filled with the the active federate handles in any case
  FederateHandleSet _waitFederates;
};


// Encapsulates the object model used in the server node
/// FederationServerObjectModel ???
class OPENRTI_LOCAL ServerObjectModel : public Federation {
public:
  struct ConnectData;
  typedef std::map<ConnectHandle, ConnectData> ConnectHandleConnectDataMap;

  struct FederateData;
  typedef std::map<FederateHandle, FederateData> FederateHandleFederateDataMap;

  struct ObjectInstanceData;
  typedef std::map<ObjectInstanceHandle, ObjectInstanceData> ObjectInstanceHandleDataMap;

  ServerObjectModel(const std::string& name, const FederationHandle& handle) :
    Federation(name, handle)
  {
  }
  ~ServerObjectModel()
  {
    // The members would be cleaned up in any case, this is to enforce some order in destruction
    _objectClassVector.clear();
    _interactionClassVector.clear();
  }

  /// Get the rti servers name
  virtual const std::string& getServerName() const = 0;
  /// Get the rti servers path
  virtual const std::string& getServerPath() const = 0;
  // /// Get the rti servers name
  // const std::string& getServerName() const
  // { return _serverOptions->getServerName(); }
  // /// Get the rti servers path
  // const std::string& getServerPath() const
  // { return _serverOptions->getServerPath(); }
  /// Get the servers name lurking behind this connect
  const std::string& getConnectName(const ConnectHandle& connectHandle) const
  {
    ConnectHandleConnectDataMap::const_iterator i = _connectHandleConnectDataMap.find(connectHandle);
    OpenRTIAssert(i != _connectHandleConnectDataMap.end());
    return i->second._name;
  }

  /// Synchronizatoin labels are tracked here
  typedef std::map<std::string, SynchronizationState> SyncronizationLabelStateMap;
  SyncronizationLabelStateMap _syncronizationLabelStateMap;

  /// Contains the committed timestamps, we will need them for a join response
  FederateHandleTimeStampMap _federateHandleTimeStampMap;





  // utilities for connecthandle/federatehandle handling

  bool hasJoinedFederatesForConnect(const ConnectHandle& connectHandle) const
  {
    ConnectHandleConnectDataMap::const_iterator i = _connectHandleConnectDataMap.find(connectHandle);
    if (i == _connectHandleConnectDataMap.end())
      return false;
    return !i->second._federateHandleSet.empty();
  }

  bool getActive(const ConnectHandle& connectHandle) const
  {
    ConnectHandleConnectDataMap::const_iterator i = _connectHandleConnectDataMap.find(connectHandle);
    if (i == _connectHandleConnectDataMap.end())
      return false;
    return i->second._messageSender.valid();
  }
  void setInActive(const ConnectHandle& connectHandle)
  {
    ConnectHandleConnectDataMap::iterator i = _connectHandleConnectDataMap.find(connectHandle);
    OpenRTIAssert(i != _connectHandleConnectDataMap.end());
    if (i->first == _parentServerConnectHandle)
      return;
    // FIXME which one of the following
    OpenRTIAssert(i->second._federateHandleSet.empty());
    // OpenRTIAssert(!hasJoinedFederatesForConnect(connectHandle));
    i->second._messageSender.clear();
  }

  bool hasJoinedFederates() const
  {
    OpenRTIAssert(_federateHandleFederateDataMap.empty() == (!_federateHandleAllocator.used()));
    return !_federateHandleFederateDataMap.empty();
  }

  /// Returns true if there is any federate joined that belongs to a child connect
  bool hasJoinedChildren() const
  {
    for (FederateHandleFederateDataMap::const_iterator i = _federateHandleFederateDataMap.begin();
         i != _federateHandleFederateDataMap.end(); ++i) {
      if (i->second._connectHandle == _parentServerConnectHandle)
        continue;
      // Ok, not even the still pending resign request for invalid connect handles is treated as valid child.
      if (i->second._resignPending)
        continue;
      return true;
    }
    return false;
  }

  bool hasChildConnects() const
  {
    if (_parentServerConnectHandle.valid())
      return 1 < _connectHandleConnectDataMap.size();
    else
      return !_connectHandleConnectDataMap.empty();
  }
  bool hasChildConnect(const ConnectHandle& connectHandle)
  {
    return _connectHandleConnectDataMap.find(connectHandle) != _connectHandleConnectDataMap.end();
  }

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


  void insertObjectInstance(const SharedPtr<ObjectInstance>& objectInstance)
  {
    OpenRTIAssert(objectInstance.valid());

    ObjectInstanceHandle objectInstanceHandle = objectInstance->getHandle();
    OpenRTIAssert(objectInstanceHandle.valid());

    ObjectClass* objectClass = objectInstance->getObjectClass();
    OpenRTIAssert(objectClass);
    objectClass->insertObjectInstance(objectInstance.get());

    ObjectInstanceHandleDataMap::iterator j = _objectInstanceHandleDataMap.find(objectInstanceHandle);
    OpenRTIAssert(j != _objectInstanceHandleDataMap.end());
    j->second._objectInstance = objectInstance;
  }
  void eraseObjectInstance(const SharedPtr<ObjectInstance>& objectInstance)
  {
    OpenRTIAssert(objectInstance.valid());

    ObjectClass* objectClass = objectInstance->getObjectClass();
    objectClass->eraseObjectInstance(objectInstance.get());

    ObjectInstanceHandle objectInstanceHandle = objectInstance->getHandle();
    ObjectInstanceHandleDataMap::iterator j = _objectInstanceHandleDataMap.find(objectInstanceHandle);
    OpenRTIAssert(j != _objectInstanceHandleDataMap.end());
    j->second._objectInstance = 0;
  }
  ObjectInstance* getObjectInstance(const ObjectInstanceHandle& objectInstanceHandle)
  {
    ObjectInstanceHandleDataMap::iterator i = _objectInstanceHandleDataMap.find(objectInstanceHandle);
    if (i == _objectInstanceHandleDataMap.end())
      return 0;
    return i->second._objectInstance.get();
  }

  void removeConnect(const ConnectHandle& connectHandle)
  {
    for (ObjectInstanceHandleDataMap::iterator i = _objectInstanceHandleDataMap.begin();
         i != _objectInstanceHandleDataMap.end(); ++i) {
      if (!i->second._objectInstance.valid())
        continue;
      i->second._objectInstance->removeConnect(connectHandle);
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

  void insert(const FOMModule& module);
  void insertInteractionClass(const FOMInteractionClass& interactionClass, const InteractionClassHandle& parentHandle);
  void insertObjectClass(const FOMObjectClass& objectClass, const ObjectClassHandle& parentHandle);

  /// The object classes in the current object model
  ObjectClassVector _objectClassVector;

  /// The interactions in the current object model
  InteractionClassVector _interactionClassVector;




  bool isObjectNameInUse(const std::string& name) const
  { return _objectInstanceNameSet.find(name) != _objectInstanceNameSet.end(); }

  ObjectInstanceHandleDataMap::iterator
  insertObjectInstanceHandle();
  ObjectInstanceHandleDataMap::iterator
  insertObjectInstanceHandle(const std::string& objectInstanceName);
  ObjectInstanceHandleDataMap::iterator
  insertObjectInstanceHandle(const ObjectInstanceHandle& objectInstanceHandle, const std::string& objectInstanceName);
  ObjectInstanceHandleDataMap::iterator
  _insertObjectInstanceHandle(const ObjectInstanceHandle& objectInstanceHandle, const std::string& objectInstanceName);

  void referenceObjectInstanceHandle(ObjectInstanceHandleDataMap::iterator i, const ConnectHandle& connectHandle);
  void referenceObjectInstanceHandle(const ObjectInstanceHandle& objectInstanceHandle, const ConnectHandle& connectHandle);

  bool unreferenceObjectInstanceHandle(ObjectInstanceHandleDataMap::iterator i, const ConnectHandle& connectHandle);
  bool unreferenceObjectInstanceHandle(const ObjectInstanceHandle& objectInstanceHandle, const ConnectHandle& connectHandle);


  // Ok, this is not held in the ObjectInstance ??!! the problem is that this is just the
  // knowledge about the object handle and name. Not yet any object class is known.
  // Hmm, may be we should be able to create object instances without object class instead??
  struct ObjectInstanceData {
    ObjectInstanceData(const StringSet::iterator& stringSetIterator) : _stringSetIterator(stringSetIterator) {}
    // Points to the string set name entry for this object instance
    StringSet::iterator _stringSetIterator;
    // Returns the object instance's name
    const std::string& getName() const { return *_stringSetIterator; }
    // The child connect handles that reference this object instance handle
    ConnectHandleSet _connectHandleSet;
    // FIXME Temporary ...
    SharedPtr<ObjectInstance> _objectInstance;
  };
  /// track object instance handles
  ObjectInstanceHandleAllocator _objectInstanceHandleAllocator;
  ObjectInstanceHandleDataMap _objectInstanceHandleDataMap;
  StringSet _objectInstanceNameSet;



  bool isFederateNameInUse(const std::string& name) const
  { return _federateNameSet.find(name) != _federateNameSet.end(); }

  /// Insert a new federate, return an iterator to the struct FederateData
  FederateHandleFederateDataMap::iterator
  insertFederate(const ConnectHandle& connectHandle, const std::string& federateName,
                 const FederateHandle& federateHandle);
  FederateHandleFederateDataMap::iterator
  insertFederate(const ConnectHandle& connectHandle, const std::string& federateName);
  /// Erase the given federate
  void eraseFederate(FederateHandleFederateDataMap::iterator i);
  void eraseFederate(const FederateHandle& federateHandle);


  // The FederateHandle <-> federate data mappings
  struct OPENRTI_LOCAL FederateData {
    FederateData(const ConnectHandle& connectHandle, const StringSet::iterator& stringSetIterator) :
      _connectHandle(connectHandle),
      _stringSetIterator(stringSetIterator),
      _resignPending(false)
    { }
    ConnectHandle _connectHandle;
    std::string _federateType;
    StringSet::iterator _stringSetIterator;
    // Returns the federates's name
    const std::string& getName() const { return *_stringSetIterator; }
    bool _resignPending;
  };
  FederateHandleAllocator _federateHandleAllocator;
  FederateHandleFederateDataMap _federateHandleFederateDataMap;
  StringSet _federateNameSet;


  /// Returns true if this is a root server
  bool isRootServer() const
  { return !_parentServerConnectHandle.valid(); }
  /// Returns true if this is the parents server connect
  bool isParentConnect(const ConnectHandle& connectHandle) const
  { return connectHandle.valid() && connectHandle == _parentServerConnectHandle; }
  /// Returns true if this is a valid child server connect
  bool isChildConnect(const ConnectHandle& connectHandle) const
  { return connectHandle.valid() && connectHandle != _parentServerConnectHandle; }

  /// Insert the parent connect into the object model
  std::pair<ConnectHandleConnectDataMap::iterator, bool>
  insertParentConnect(const ConnectHandle& connectHandle, const SharedPtr<AbstractMessageSender>& messageSender, const std::string& name);
  /// Insert a new connect into the object model
  std::pair<ConnectHandleConnectDataMap::iterator, bool>
  insertConnect(const ConnectHandle& connectHandle, const SharedPtr<AbstractMessageSender>& messageSender, const std::string& name);
  /// Erase a connect from the object model.
  /// Precondition is that the connect is already idle
  void eraseConnect(ConnectHandleConnectDataMap::iterator i);
  void eraseConnect(const ConnectHandle& connectHandle);

  // The ConnectHandle <-> connect data mappings
  struct OPENRTI_LOCAL ConnectData {
    SharedPtr<AbstractMessageSender> _messageSender;
    std::string _name;
    // FIXME make this an iterator to the FederateData above
    FederateHandleSet _federateHandleSet;
  };
  ConnectHandle _parentServerConnectHandle;
  ConnectHandleConnectDataMap _connectHandleConnectDataMap;
};

} // namespace OpenRTI

#endif
