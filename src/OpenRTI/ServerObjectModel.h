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
#include "IntrusiveList.h"
#include "Message.h"
#include "NameHandlePair.h"
#include "Referenced.h"
#include "RegionSet.h"
#include "SharedPtr.h"
#include "StringUtils.h"
#include "Types.h"
#include "VariableLengthData.h"
#include "LogStream.h"

namespace OpenRTI {

// Encapsulates the object model used in the server node
/// FederationServerObjectModel ???
class OPENRTI_LOCAL ServerObjectModel : public NameHandlePair<FederationHandle> {
public:

  struct ObjectClass;
  struct ObjectClassAttribute;
  struct ObjectInstance;
  struct ObjectAttribute;
  struct InteractionClass;
  struct Federate;
  struct ConnectData;

  typedef std::vector<SharedPtr<ObjectClass> > ObjectClassVector;
  typedef std::vector<SharedPtr<ObjectClassAttribute> > ObjectClassAttributeVector;
  typedef std::vector<SharedPtr<InteractionClass> > InteractionClassVector;
  typedef std::vector<SharedPtr<ObjectAttribute> > HandleObjectAttributeVector;
  typedef std::map<ConnectHandle, SharedPtr<ConnectData> > ConnectHandleConnectDataMap;
  typedef std::map<FederateHandle, SharedPtr<Federate> > FederateHandleFederateMap;
  typedef std::map<ObjectInstanceHandle, SharedPtr<ObjectInstance> > ObjectInstanceHandleObjectInstanceMap;

  typedef IntrusiveList<ObjectClass, 0> ChildObjectClassList;
  typedef IntrusiveList<ObjectInstance, 0> ObjectClassInstanceList;
  typedef std::list<ObjectInstance*> ObjectInstanceList;
  typedef IntrusiveList<InteractionClass, 0> ChildInteractionClassList;
  typedef IntrusiveList<Federate, 0> ConnectFederateList;


  ServerObjectModel(const std::string& name, const FederationHandle& handle) :
    NameHandlePair<FederationHandle>(name, handle)
  {
  }
  virtual ~ServerObjectModel()
  {
    // FIXME assert that this is empty
    _objectInstanceHandleObjectInstanceMap.clear();
    // The members would be cleaned up in any case, this is to enforce some order in destruction
    while (!_objectClassVector.empty())
      _objectClassVector.pop_back();
    while (!_interactionClassVector.empty())
      _interactionClassVector.pop_back();
  }

  // /// Get the rti servers name
  // virtual const std::string& getServerName() const = 0;
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
    return i->second->_name;
  }

  // The logical time factorys name.
  const std::string& getLogicalTimeFactoryName() const
  { return _logicalTimeFactoryName; }
  void setLogicalTimeFactoryName(const std::string& logicalTimeFactoryName)
  { _logicalTimeFactoryName = logicalTimeFactoryName; }
  std::string _logicalTimeFactoryName;

  // Contains the committed timestamps, we will need them for a join response
  struct CommitTimeStamps {
    CommitTimeStamps(const Unsigned& commitId) : _commitId(commitId) { }
    VariableLengthData _timeAdvanceTimeStamp;
    VariableLengthData _nextMessageTimeStamp;
    Unsigned _commitId;
  };
  typedef std::map<FederateHandle, CommitTimeStamps> FederateHandleCommitMap;
  FederateHandleCommitMap _federateHandleCommitMap;


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

    // returns true if something changed
    bool updateCumulativeSubscribedConnectHandleSet(const ConnectHandle& connectHandle, bool subscribe)
    {
      if (subscribe) {
        return _cumulativeSubscribedConnectHandleSet.insert(connectHandle).second;
      } else {
        return 0 != _cumulativeSubscribedConnectHandleSet.erase(connectHandle);
      }
    }

    /// The cumulative set of connects that subscribe to this.
    /// Cumulative in the sense that also subscriptions to base classes are included here.
    ConnectHandleSet _cumulativeSubscribedConnectHandleSet;

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
  struct OPENRTI_LOCAL ObjectClassAttribute : public PublishSubscribe<AttributeHandle> {
    ObjectClassAttribute(const std::string& name, const AttributeHandle& handle) :
      PublishSubscribe<AttributeHandle>(name, handle)
    { }

  };

  struct OPENRTI_LOCAL ObjectClass : public NameHandlePair<ObjectClassHandle>, public ChildObjectClassList::Hook {
    ObjectClass(const std::string& name, const ObjectClassHandle& handle, ObjectClass* parentObjectClass) :
      NameHandlePair<ObjectClassHandle>(name, handle),
      _parentObjectClass(parentObjectClass)
    {
      if (_parentObjectClass)
        _parentObjectClass->_childObjectClassList.push_front(*this);
    }
    ~ObjectClass()
    {
      if (_parentObjectClass)
        _parentObjectClass->_childObjectClassList.unlink(*this);
      _childObjectClassList.unlink();
    }

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
      if (_objectClassAttributeVector.size() <= attributeHandle.getHandle())
        _objectClassAttributeVector.resize(attributeHandle.getHandle() + 1);
      if (_objectClassAttributeVector[attributeHandle.getHandle()].valid())
        return;
      _objectClassAttributeVector[attributeHandle.getHandle()] = objectClassAttribute;

      for (ChildObjectClassList::iterator i = _childObjectClassList.begin(); i != _childObjectClassList.end(); ++i) {
        // FIXME share these among object classes???
        SharedPtr<ObjectClassAttribute> attribute;
        attribute = new ObjectClassAttribute(objectClassAttribute->getName(), objectClassAttribute->getHandle());
        // FIXME, this???
        // <field name="DimensionHandleSet" type="DimensionHandleSet"/>
        i->insertObjectClassAttribute(attribute);
      }
    }
    ObjectClassAttribute* getAttribute(const AttributeHandle& attributeHandle) const
    {
      if (_objectClassAttributeVector.size() <= attributeHandle.getHandle())
        return 0;
      return _objectClassAttributeVector[attributeHandle.getHandle()].get();
    }
    ObjectClassAttribute* getPrivilegeToDeleteAttribute() const
    { return getAttribute(AttributeHandle(0)); }

    /// Get the set of object instances of this object class type
    const ObjectClassInstanceList& getObjectInstanceList() const
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

    const ChildObjectClassList& getChildObjectClassList() const
    { return _childObjectClassList; }

    /// since we might end in different depths for different attributes, this is done per attribute
    void updateCumulativeSubscription(const ConnectHandle& connectHandle, const AttributeHandle& attributeHandle,
                                      ObjectInstanceList& objectInstanceList)
    {
      bool parentSubscribed = false;
      if (_parentObjectClass &&
          attributeHandle.getHandle() < _parentObjectClass->_objectClassAttributeVector.size() &&
          0 != _parentObjectClass->_objectClassAttributeVector[attributeHandle.getHandle()]->_cumulativeSubscribedConnectHandleSet.count(connectHandle))
        parentSubscribed = true;

      _updateCumulativeSubscription(connectHandle, attributeHandle, parentSubscribed, objectInstanceList);
    }
    void _updateCumulativeSubscription(const ConnectHandle& connectHandle, const AttributeHandle& attributeHandle,
                                       bool subscribe /*Replace with regionset or something*/, ObjectInstanceList& objectInstanceList)
    {
      ObjectClassAttribute*  objectClassAttribute = getAttribute(attributeHandle);
      subscribe |= (Unsubscribed != objectClassAttribute->getSubscriptionType(connectHandle));
      if (!objectClassAttribute->updateCumulativeSubscribedConnectHandleSet(connectHandle, subscribe))
        return;
      // Update the receiving connect handle set
      for (ChildObjectClassList::iterator i = _childObjectClassList.begin(); i != _childObjectClassList.end(); ++i) {
        i->_updateCumulativeSubscription(connectHandle, attributeHandle, subscribe, objectInstanceList);
      }
      /// FIXME: need to walk the objects and see how the routing for the object changes
      /// FIXME: store the object instances that are yet unknown to a connect and store these to propagate them into the connect
      /// Hmm, here is the first good use case for a visitor
      for (ObjectClassInstanceList::iterator i = _objectInstanceList.begin(); i != _objectInstanceList.end(); ++i) {
        ObjectAttribute* objectAttribute = i->getAttribute(attributeHandle);
        if (!objectAttribute) // FIXME: is this an error??
          continue;

        // Don't add the owner to the list of connect handles that recieve this attribute
        if (objectAttribute->getOwnerConnectHandle() == connectHandle)
          continue;

        if (subscribe) {
          // Insert the connect handle into the recieving connects
          if (!objectAttribute->_recieveingConnects.insert(connectHandle).second)
            continue;

          // Note that we need to insert this object instance into this connect
          if (attributeHandle == AttributeHandle(0))
            objectInstanceList.push_back(&*i);

        } else {
          // Erase the connect handle from the recieving connects
          if (objectAttribute->_recieveingConnects.erase(connectHandle) == 0)
            continue;
        }
      }
    }

  private:
    /// The pointer to the parent object class, may be 0
    ObjectClass* const _parentObjectClass;

    /// The list of child object classes
    ChildObjectClassList _childObjectClassList;

    /// The set of class attributes of this object class type
    ObjectClassAttributeVector _objectClassAttributeVector;

    /// The set of objects of this object class type
    /// Note that the object class does not 'own' the instances, thus
    /// this ObjectInstanceList only stores plain pointers instead of references
    /// The federation owns the objects and consequently carries references
    ObjectClassInstanceList _objectInstanceList;
  };

  struct OPENRTI_LOCAL ObjectAttribute : public Referenced {
    ObjectAttribute(ObjectClassAttribute* objectClassAttribute) :
      _objectClassAttribute(objectClassAttribute)
    {
      // FIXME exclude the owner connect handle
      _recieveingConnects = _objectClassAttribute->_cumulativeSubscribedConnectHandleSet;
    }
    // ObjectAttribute(ObjectClassAttribute* objectClassAttribute, const RegionSet& regionSet) :
    //   _objectClassAttribute(objectClassAttribute)
    // {
    //   // FIXME exclude the owner connect handle
    //   _objectClassAttribute->getSubscribedAndIntersectingConnectHandleSet(_recieveingConnects, regionSet);
    // }

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
    {
      _recieveingConnects.erase(connectHandle);
      _ownerConnectHandle = connectHandle;
    }

    void removeConnect(const ConnectHandle& connectHandle)
    {
      _recieveingConnects.erase(connectHandle);
      if (_ownerConnectHandle == connectHandle)
        _ownerConnectHandle = ConnectHandle();
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

  struct OPENRTI_LOCAL InteractionClass : public PublishSubscribe<InteractionClassHandle>, public ChildInteractionClassList::Hook {
    InteractionClass(const std::string& name, const InteractionClassHandle& handle, InteractionClass* parentInteractionClass) :
      PublishSubscribe<InteractionClassHandle>(name, handle),
      _parentInteractionClass(parentInteractionClass)
    {
      if (_parentInteractionClass)
        _parentInteractionClass->_childInteractionClassList.push_front(*this);
    }
    ~InteractionClass()
    {
      if (_parentInteractionClass)
        _parentInteractionClass->_childInteractionClassList.unlink(*this);
      _childInteractionClassList.unlink();
    }

    /// Get the parent InteractionClass
    InteractionClass* getParentInteractionClass() const
    { return _parentInteractionClass; }

    const ChildInteractionClassList& getChildInteractionClassList() const
    { return _childInteractionClassList; }

    void updateCumulativeSubscription(const ConnectHandle& connectHandle)
    {
      bool parentSubscribed = false;
      if (_parentInteractionClass &&
          0 != _parentInteractionClass->_cumulativeSubscribedConnectHandleSet.count(connectHandle))
        parentSubscribed = true;

      _updateCumulativeSubscription(connectHandle, parentSubscribed);
    }
    void _updateCumulativeSubscription(const ConnectHandle& connectHandle, bool subscribe /*Replace with regionset or something*/)
    {
      subscribe |= (Unsubscribed != getSubscriptionType(connectHandle));
      if (!updateCumulativeSubscribedConnectHandleSet(connectHandle, subscribe))
        return;
      // Update the receiving connect handle set
      for (ChildInteractionClassList::iterator i = _childInteractionClassList.begin(); i != _childInteractionClassList.end(); ++i) {
        i->_updateCumulativeSubscription(connectHandle, subscribe);
      }
    }

  private:
    /// The pointer to the parent interaction class, may be 0
    InteractionClass* const _parentInteractionClass;

    /// The list of child interaction classes
    ChildInteractionClassList _childInteractionClassList;
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
// typedef std::list<ConnectHandleSetAttributeUpdateMessageFilterPair> AttributeUpdateBroadcastList;

/// AttributeUpdateBroadcastList !???!

  class OPENRTI_LOCAL SynchronizationState {
  public:
    SynchronizationState() :
      _addJoiningFederates(true)
    { }

    void set(const FederateHandleVector& participatingFederates, const FederateHandleFederateMap& joinedFederates)
    {
      if (participatingFederates.empty()) {
        for (FederateHandleFederateMap::const_iterator i = joinedFederates.begin(); i != joinedFederates.end(); ++i) {
          _participatingFederates.insert(i->first);
          _waitFederates.insert(i->first);
        }
        _addJoiningFederates = true;
      } else {
        for (FederateHandleVector::const_iterator i = participatingFederates.begin(); i != participatingFederates.end(); ++i) {
          _participatingFederates.insert(*i);
          _waitFederates.insert(*i);
        }
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
      _successfulFederates.erase(federateHandle);
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
    // The set of federates that have set the successfully flag on synchronizationPointAchieved
    FederateHandleSet _successfulFederates;
  };

  /// Synchronizatoin labels are tracked here
  typedef std::map<std::string, SynchronizationState> SyncronizationLabelStateMap;
  SyncronizationLabelStateMap _syncronizationLabelStateMap;



  // utilities for connecthandle/federatehandle handling

  bool hasJoinedFederatesForConnect(const ConnectHandle& connectHandle) const
  {
    ConnectHandleConnectDataMap::const_iterator i = _connectHandleConnectDataMap.find(connectHandle);
    if (i == _connectHandleConnectDataMap.end())
      return false;
    return !i->second->_federateList.empty();
  }

  bool getActive(const ConnectHandle& connectHandle) const
  {
    ConnectHandleConnectDataMap::const_iterator i = _connectHandleConnectDataMap.find(connectHandle);
    if (i == _connectHandleConnectDataMap.end())
      return false;
    return i->second->_messageSender.valid();
  }
  void setInActive(const ConnectHandle& connectHandle)
  {
    ConnectHandleConnectDataMap::iterator i = _connectHandleConnectDataMap.find(connectHandle);
    OpenRTIAssert(i != _connectHandleConnectDataMap.end());
    if (i->first == _parentServerConnectHandle)
      return;
    // FIXME which one of the following
    OpenRTIAssert(i->second->_federateList.empty());
    // OpenRTIAssert(!hasJoinedFederatesForConnect(connectHandle));
    i->second->_messageSender.clear();
  }

  bool hasJoinedFederates() const
  {
    OpenRTIAssert(_federateHandleFederateMap.empty() == (!_federateHandleAllocator.used()));
    return !_federateHandleFederateMap.empty();
  }

  /// Returns true if there is any federate joined that belongs to a child connect
  bool hasJoinedChildren() const
  {
    for (FederateHandleFederateMap::const_iterator i = _federateHandleFederateMap.begin();
         i != _federateHandleFederateMap.end(); ++i) {
      if (i->second->getConnectHandle() == _parentServerConnectHandle)
        continue;
      // Ok, not even the still pending resign request for invalid connect handles is treated as valid child.
      if (i->second->_resignPending)
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
  ObjectClass* getObjectClass(const ObjectClassHandle& objectClassHandle)
  {
    if (_objectClassVector.size() <= objectClassHandle.getHandle())
      return 0;
    return _objectClassVector[objectClassHandle.getHandle()].get();
  }

  /// Interaction class handling methods
  const InteractionClassVector& getInteractionClassVector() const
  { return _interactionClassVector; }
  InteractionClass* getInteractionClass(const InteractionClassHandle& interactionClassHandle)
  {
    if (_interactionClassVector.size() <= interactionClassHandle.getHandle())
      return 0;
    return _interactionClassVector[interactionClassHandle.getHandle()].get();
  }

  void removeConnect(const ConnectHandle& connectHandle)
  {
    for (ObjectInstanceHandleObjectInstanceMap::iterator i = _objectInstanceHandleObjectInstanceMap.begin();
         i != _objectInstanceHandleObjectInstanceMap.end(); ++i) {
      i->second->removeConnect(connectHandle);
    }
    for (ObjectClassVector::const_iterator i = _objectClassVector.begin();
         i != _objectClassVector.end(); ++i) {
      if (!i->valid())
        continue;
      (*i)->removeConnect(connectHandle);
    }
    for (InteractionClassVector::const_iterator i = _interactionClassVector.begin();
         i != _interactionClassVector.end(); ++i) {
      if (!i->valid())
        continue;
      (*i)->removeConnect(connectHandle);
    }
  }

  void insert(const FOMModuleList& moduleList, bool isBaseType);
  void erase(const FOMModuleList& moduleList);

  FOMModuleSet _fomModuleSet;

  // Insert a given fom module, must be compatible
  bool insertFomModule(const FOMModule& fomModule);
  void eraseFomModule(const FOMModule& fomModule);
  void insertInteractionClass(const FOMInteractionClass& interactionClass);
  void eraseInteractionClass(const FOMInteractionClass& interactionClass);
  void insertObjectClass(const FOMObjectClass& objectClass);
  void eraseObjectClass(const FOMObjectClass& objectClass);

  /// The object classes in the current object model
  ObjectClassVector _objectClassVector;

  /// The interactions in the current object model
  InteractionClassVector _interactionClassVector;



  struct Region;
  typedef std::map<RegionHandle, SharedPtr<Region> > RegionHandleRegionMap;
  typedef IntrusiveList<Region, 0> ConnectOwnedRegionList;

  Region* getRegion(const RegionHandle& regionHandle);
  Region* insertRegion(const RegionHandle& regionHandle);
  void eraseRegion(Region* region);
  void eraseRegion(const RegionHandle& regionHandle);

  struct OPENRTI_LOCAL Region : public Referenced, public ConnectOwnedRegionList::Hook {
    Region(const RegionHandleRegionMap::iterator& regionHandleRegionMapIterator) :
      _regionHandleRegionMapIterator(regionHandleRegionMapIterator),
      _connect(0)
    { }
    const RegionHandle& getHandle() const
    { return _regionHandleRegionMapIterator->first; }
    const RegionHandleRegionMap::iterator& getRegionHandleRegionMapIterator() const
    { return _regionHandleRegionMapIterator; }

    ConnectData* getConnect()
    { return _connect; }
    void setConnect(ConnectData* connect)
    { _connect = connect; }

    void insertToRegionList(ConnectOwnedRegionList& regionList)
    {
      regionList.push_front(*this);
    }
    void eraseFromRegionList(ConnectOwnedRegionList& regionList)
    {
      regionList.unlink(*this);
    }

    RegionValue _regionValue;
    DimensionHandleSet _dimensionHandleSet;

  private:
    const RegionHandleRegionMap::iterator _regionHandleRegionMapIterator;
    ConnectData* _connect;

    // originating connect???
    // recieving connectset???
  };

  RegionHandleRegionMap _regionHandleRegionMap;



  struct ObjectInstanceConnect;
  typedef std::map<ConnectHandle, SharedPtr<ObjectInstanceConnect> > ConnectHandleObjectInstanceConnectMap;
  typedef IntrusiveList<ObjectInstanceConnect, 1> ObjectInstanceConnectList;

  // Per Connect object instance data.
  struct OPENRTI_LOCAL ObjectInstanceConnect : public Referenced, public ObjectInstanceConnectList::Hook {
    ObjectInstanceConnect(const ConnectHandleObjectInstanceConnectMap::iterator& iterator,
                          ObjectInstance* objectInstance, ConnectData* connect) :
      _connectHandleObjectInstanceConnectMapIterator(iterator),
      _objectInstance(objectInstance),
      _connect(connect)
    { }

    ObjectInstance* getObjectInstance()
    { return _objectInstance; }
    ConnectData* getConnect()
    { return _connect; }

    const ConnectHandleObjectInstanceConnectMap::iterator& getConnectHandleObjectInstanceConnectMapIterator() const
    { return _connectHandleObjectInstanceConnectMapIterator; }

    void insertToObjectInstanceConnectList(ObjectInstanceConnectList& objectInstanceConnectList)
    {
      objectInstanceConnectList.push_front(*this);
    }
    void eraseFromObjectInstanceConnectList(ObjectInstanceConnectList& objectInstanceConnectList)
    {
      objectInstanceConnectList.unlink(*this);
    }

  private:
    /// iterator into the primary index in the ObjectInstance, here for O(1) removal
    ConnectHandleObjectInstanceConnectMap::iterator _connectHandleObjectInstanceConnectMapIterator;
    /// The original ObjectInstance this belongs to
    ObjectInstance* _objectInstance;
    /// The original Connect this belongs to
    ConnectData* _connect;
  };




  bool isObjectNameInUse(const std::string& name) const
  { return _objectInstanceNameSet.find(name) != _objectInstanceNameSet.end(); }

  ObjectInstance* getObjectInstance(const ObjectInstanceHandle& objectInstanceHandle);

  ObjectInstance*
  insertObjectInstanceHandle();
  ObjectInstance*
  insertObjectInstanceHandle(const std::string& objectInstanceName);
  ObjectInstance*
  insertObjectInstanceHandle(const ObjectInstanceHandle& objectInstanceHandle, const std::string& objectInstanceName);
  ObjectInstance*
  _insertObjectInstanceHandle(const ObjectInstanceHandle& objectInstanceHandle, const std::string& objectInstanceName);

  void eraseObjectInstanceHandle(ObjectInstance* objectInstance);

  struct OPENRTI_LOCAL ObjectInstance : public NameHandlePair<ObjectInstanceHandle>, public ObjectClassInstanceList::Hook {
    ObjectInstance(const ObjectInstanceHandleObjectInstanceMap::iterator& objectInstanceHandleObjectInstanceMapIterator,
                   const StringSet::iterator& stringSetIterator) :
      NameHandlePair<ObjectInstanceHandle>(*stringSetIterator, objectInstanceHandleObjectInstanceMapIterator->first),
      _objectInstanceHandleObjectInstanceMapIterator(objectInstanceHandleObjectInstanceMapIterator),
      _stringSetIterator(stringSetIterator),
      _objectClass(0)
    {
    }

    void setObjectClass(ObjectClass* objectClass)
    {
      if (_objectClass) {
        OpenRTIAssert(objectClass == _objectClass);
        return;
      }
      _objectClass = objectClass;
      _objectClass->insertObjectInstance(this);
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

    void removeConnect(const ConnectHandle& connectHandle)
    {
      // FIXME make that an assert at some time
      // OpenRTIAssert(_connectHandleSet.find(connectHandle) == _connectHandleSet.end());
      HandleObjectAttributeVector::const_iterator i;
      for (i = _handleObjectAttributeVector.begin(); i != _handleObjectAttributeVector.end(); ++i) {
        (*i)->removeConnect(connectHandle);
      }
    }

    // Inserts itself into the list that is usually held in the object class knowing all the object instances
    void insertToObjectClassList(ObjectClassInstanceList& objectInstanceList)
    {
      objectInstanceList.push_front(*this);
    }
    // Removes itself from the list that is usually held in the object class knowing all the object instances
    void eraseFromObjectClassList(ObjectClassInstanceList& objectInstanceList)
    {
      objectInstanceList.unlink(*this);
    }

    ObjectInstanceConnect* referenceObjectInstance(ConnectData* connect);
    bool unreferenceObjectInstance(ConnectData* connect);
    bool unreferenceObjectInstance(ObjectInstanceConnect* objectInstanceConnect);

    // Points back to the index object instance by handle map
    ObjectInstanceHandleObjectInstanceMap::iterator _objectInstanceHandleObjectInstanceMapIterator;
    // Points to the string set name entry for this object instance
    StringSet::iterator _stringSetIterator;

    ConnectHandleObjectInstanceConnectMap _connectHandleObjectInstanceConnectMap;

  private:
    /// The pointer to the object class this object is an instance of, can be zero
    ObjectClass* _objectClass;

    /// The set of instance attributes of this object
    HandleObjectAttributeVector _handleObjectAttributeVector;
  };
  /// track object instance handles
  ObjectInstanceHandleAllocator _objectInstanceHandleAllocator;
  ObjectInstanceHandleObjectInstanceMap _objectInstanceHandleObjectInstanceMap;
  StringSet _objectInstanceNameSet;



  bool isFederateNameInUse(const std::string& name) const
  { return _federateNameSet.find(name) != _federateNameSet.end(); }

  /// get the federate for the given federateHandle
  Federate* getFederate(const FederateHandle& federateHandle);
  /// Insert a new federate, return an iterator to the struct Federate
  Federate*
  insertFederate(const ConnectHandle& connectHandle, const std::string& federateName,
                 const FederateHandle& federateHandle);
  Federate*
  insertFederate(const ConnectHandle& connectHandle, const std::string& federateName);
  /// Erase the given federate
  void eraseFederate(FederateHandleFederateMap::iterator i);
  void eraseFederate(const FederateHandle& federateHandle);
  void eraseFederate(Federate* federate);


  // The FederateHandle <-> federate data mappings
  struct OPENRTI_LOCAL Federate : public Referenced, public ConnectFederateList::Hook {
    Federate(const FederateHandleFederateMap::iterator& federateHandleFederateMapIterator,
             const StringSet::iterator& stringSetIterator) :
      _federateHandleFederateMapIterator(federateHandleFederateMapIterator),
      _stringSetIterator(stringSetIterator),
      _connect(0),
      _resignPending(false),
      _automaticResignDirective(CANCEL_THEN_DELETE_THEN_DIVEST)
    { }
    FederateHandleFederateMap::iterator _federateHandleFederateMapIterator;
    StringSet::iterator _stringSetIterator;
    const FederateHandle& getHandle() const { return _federateHandleFederateMapIterator->first; }
    // Returns the federates's name
    const std::string& getName() const { return *_stringSetIterator; }
    std::string _federateType;
    ConnectData* _connect;
    ConnectHandle getConnectHandle() const
    {
      if (!_connect)
        return ConnectHandle();
      return _connect->getHandle();
    }
    bool _resignPending;
    ResignAction _automaticResignDirective;
    FOMModuleHandleVector _fomModuleHandleVector;
  };
  FederateHandleAllocator _federateHandleAllocator;
  FederateHandleFederateMap _federateHandleFederateMap;
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

  /// Get the connect data for the given connectHandle
  ConnectData* getConnect(const ConnectHandle& connectHandle);
  /// Insert the parent connect into the object model
  ConnectData* getOrInsertParentConnect(const ConnectHandle& connectHandle);
  /// Insert a new connect into the object model
  ConnectData* getOrInsertConnect(const ConnectHandle& connectHandle);
  /// Erase a connect from the object model.
  /// Precondition is that the connect is already idle
  void eraseConnect(ConnectHandleConnectDataMap::iterator i);
  void eraseConnect(const ConnectHandle& connectHandle);
  void eraseConnect(ConnectData* connectData);

  // The ConnectHandle <-> connect data mappings
  struct OPENRTI_LOCAL ConnectData : public Referenced {
    ConnectData(const ConnectHandleConnectDataMap::iterator& connectHandleConnectDataMapIterator) :
      _connectHandleConnectDataMapIterator(connectHandleConnectDataMapIterator),
      _permitTimeRegulation(true)
    { }
    ~ConnectData()
    {
      _federateList.unlink();
      _objectInstanceConnectList.unlink();
      _ownedRegions.unlink();
    }
    ConnectHandleConnectDataMap::iterator _connectHandleConnectDataMapIterator;
    SharedPtr<AbstractMessageSender> _messageSender;
    const ConnectHandle& getHandle() const { return _connectHandleConnectDataMapIterator->first; }
    std::string _name;
    ConnectFederateList _federateList;
    bool _permitTimeRegulation;
    void send(const SharedPtr<const AbstractMessage>& message)
    {
      if (!_messageSender.valid())
        return;
      _messageSender->send(message);
    }

    void eraseFederate(Federate* federate)
    {
      OpenRTIAssert(federate);
      ConnectData* connect = federate->_connect;
      if (!connect)
        return;
      OpenRTIAssert(connect == this);
      // Remove from connects
      connect->_federateList.unlink(*federate);
      federate->_connect = 0;
    }

    void insertObjectInstance(ObjectInstanceConnect& objectInstanceConnect)
    { objectInstanceConnect.insertToObjectInstanceConnectList(_objectInstanceConnectList); }
    void eraseObjectInstance(ObjectInstanceConnect& objectInstanceConnect)
    { objectInstanceConnect.eraseFromObjectInstanceConnectList(_objectInstanceConnectList); }

    void insertRegion(Region& region)
    {
      OpenRTIAssert(!region.getConnect());
      region.insertToRegionList(_ownedRegions);
      region.setConnect(this);
    }
    void eraseRegion(Region& region)
    {
      OpenRTIAssert(this == region.getConnect());
      region.setConnect(0);
      region.eraseFromRegionList(_ownedRegions);
    }

  // private:
    /// ObjectInstances that are known/referenced by this connect
    ObjectInstanceConnectList _objectInstanceConnectList;

    /// List of regions that are owned by this connect
    ConnectOwnedRegionList _ownedRegions;
  };
  ConnectHandle _parentServerConnectHandle;
  ConnectHandleConnectDataMap _connectHandleConnectDataMap;
};

} // namespace OpenRTI

#endif
