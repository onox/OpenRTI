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

#ifndef OpenRTI_FederateHandleLowerBoundTimeStampMap_h
#define OpenRTI_FederateHandleLowerBoundTimeStampMap_h

#include <map>

#include "Handle.h"
#include "LogStream.h"

namespace OpenRTI {

template<typename T>
class OPENRTI_LOCAL FederateHandleLowerBoundTimeStampMap {
public:
  typedef T LogicalTime;
  typedef std::pair<LogicalTime, int> LogicalTimePair;

  /// Insert a new federate handle with the initial logical time pair
  void insert(const FederateHandle& federateHandle, const LogicalTime& logicalTime, const LogicalTime& nextMessageTime, const Unsigned& commitId, const Unsigned& beforeOwnCommitId)
  {
    // Store that in the time map, so that we cannot advance more than that until this federate
    // commits a new lbts in the rti. This will usually happen in immediate response to that sent message.
    typename LogicalTimeFederateCountMap::iterator i = _timeAdvanceLogicalTimeFederateCountMap.insert(logicalTime);
    typename LogicalTimeFederateCountMap::iterator j = _nextMessageLogicalTimeFederateCountMap.insert(nextMessageTime);
    OpenRTIAssert(_federateHandleCommitMap.find(federateHandle) == _federateHandleCommitMap.end());
    _federateHandleCommitMap.insert(typename FederateHandleCommitMap::value_type(federateHandle, Commit(i, j, commitId, beforeOwnCommitId)));
  }

  /// Erase a federate handle including its logical time bounds
  /// Returns true if this unleaches a new logical time
  bool erase(const FederateHandle& federateHandle)
  {
    typename FederateHandleCommitMap::iterator i;
    i = _federateHandleCommitMap.find(federateHandle);
    // OpenRTIAssert(i != _federateHandleCommitMap.end());
    if (i == _federateHandleCommitMap.end())
      return false;

    typename LogicalTimeFederateCountMap::iterator j = i->second._timeAdvanceCommit;
    typename LogicalTimeFederateCountMap::iterator k = i->second._nextMessageCommit;
    _federateHandleCommitMap.erase(i);
    bool isFirstLogicalTime;
    isFirstLogicalTime = _timeAdvanceLogicalTimeFederateCountMap.erase(j);
    isFirstLogicalTime = _nextMessageLogicalTimeFederateCountMap.erase(k) || isFirstLogicalTime;
    return isFirstLogicalTime;
  }

  /// Returns true if this unleaches a new logical time
  std::pair<bool, bool> commit(const FederateHandle& federateHandle, const LogicalTime& logicalTime, const LowerBoundTimeStampCommitType& commitType, const Unsigned& commitId)
  {
    // Once we receive these commits, the federate must have registered as some regulating, this must be here
    OpenRTIAssert(!_timeAdvanceLogicalTimeFederateCountMap.empty());
    OpenRTIAssert(!_nextMessageLogicalTimeFederateCountMap.empty());
    OpenRTIAssert(!_federateHandleCommitMap.empty());

    typename FederateHandleCommitMap::iterator i;
    i = _federateHandleCommitMap.find(federateHandle);
    OpenRTIAssert(i != _federateHandleCommitMap.end());

    bool isFirstLogicalTime;
    if (commitType & TimeAdvanceCommit) {
      std::pair<typename LogicalTimeFederateCountMap::iterator, bool> iteratorPair = _timeAdvanceLogicalTimeFederateCountMap.move(i->second._timeAdvanceCommit, logicalTime);
      i->second._timeAdvanceCommit = iteratorPair.first;
      isFirstLogicalTime = iteratorPair.second;
    } else {
      isFirstLogicalTime = false;
    }

    if (commitType & NextMessageCommit) {
      i->second._nextMessageCommit = _nextMessageLogicalTimeFederateCountMap.move(i->second._nextMessageCommit, logicalTime).first;
    }

    // Forcefully clear this
    bool nextMessageMode = i->second.isInNextMessageMode();
    if (!nextMessageMode)
      i->second._federateIsLocked = false;

    bool commmitIdChangedAndNextMessageMode;
    if (i->second._commitId != commitId) {
      OpenRTIAssert(nextMessageMode);
      i->second._commitId = commitId;
      commmitIdChangedAndNextMessageMode = true;
    } else {
      commmitIdChangedAndNextMessageMode = false;
    }

    OpenRTIAssert(!_timeAdvanceLogicalTimeFederateCountMap.empty());
    OpenRTIAssert(!_nextMessageLogicalTimeFederateCountMap.empty());
    OpenRTIAssert(!_federateHandleCommitMap.empty());
    OpenRTIAssert(i->second._timeAdvanceCommit->first <= i->second._nextMessageCommit->first);
    OpenRTIAssert(_timeAdvanceLogicalTimeFederateCountMap.begin()->first <= _nextMessageLogicalTimeFederateCountMap.begin()->first);

    return std::pair<bool, bool>(isFirstLogicalTime, commmitIdChangedAndNextMessageMode);
  }

  // O(1)
  bool canAdvanceTo(const LogicalTimePair& logicalTimePair) const
  {
    if (empty())
      return true;
    if (0 < logicalTimePair.second)
      return logicalTimePair.first < _timeAdvanceLogicalTimeFederateCountMap.begin()->first;
    else
      return logicalTimePair.first <= _timeAdvanceLogicalTimeFederateCountMap.begin()->first;
  }

  // O(1)
  bool canAdvanceToNextMessage(const LogicalTimePair& logicalTimePair) const
  {
    if (empty())
      return true;
    if (0 < logicalTimePair.second)
      return logicalTimePair.first < _nextMessageLogicalTimeFederateCountMap.begin()->first;
    else
      return logicalTimePair.first <= _nextMessageLogicalTimeFederateCountMap.begin()->first;
  }

  // O(1)
  bool empty() const
  {
    OpenRTIAssert(_timeAdvanceLogicalTimeFederateCountMap.empty() == _federateHandleCommitMap.empty());
    OpenRTIAssert(_nextMessageLogicalTimeFederateCountMap.empty() == _federateHandleCommitMap.empty());
    return _timeAdvanceLogicalTimeFederateCountMap.empty();
  }

  // O(1)
  const LogicalTime& getGALT() const
  {
    OpenRTIAssert(!_timeAdvanceLogicalTimeFederateCountMap.empty());
    OpenRTIAssert(!_nextMessageLogicalTimeFederateCountMap.empty());
    OpenRTIAssert(!_federateHandleCommitMap.empty());
    return _timeAdvanceLogicalTimeFederateCountMap.begin()->first;
  }

  // O(1)
  const LogicalTime& getNextMessageGALT() const
  {
    OpenRTIAssert(!_timeAdvanceLogicalTimeFederateCountMap.empty());
    OpenRTIAssert(!_nextMessageLogicalTimeFederateCountMap.empty());
    OpenRTIAssert(!_federateHandleCommitMap.empty());
    return _nextMessageLogicalTimeFederateCountMap.begin()->first;
  }

  // O(1)
  bool getConstrainedByNextMessage() const
  {
    if (empty())
      return false;
    return _timeAdvanceLogicalTimeFederateCountMap.begin()->first < _nextMessageLogicalTimeFederateCountMap.begin()->first;
  }

  // O(log(n))
  bool getFederateIsInNextMessageModeForAssert(const FederateHandle& federateHandle) const
  {
    typename FederateHandleCommitMap::const_iterator i = _federateHandleCommitMap.find(federateHandle);
    if (i == _federateHandleCommitMap.end())
      return false;
    return i->second.isInNextMessageMode();
  }

  // O(log(n))
  void setFederateWaitCommitId(const FederateHandle& federateHandle, const Unsigned& commitId)
  {
    typename FederateHandleCommitMap::iterator i = _federateHandleCommitMap.find(federateHandle);
    if (i == _federateHandleCommitMap.end())
      return;
    i->second._federateIsWaitingForCommitId = commitId;
  }

  // O(log(n))
  void setFederateLockedByNextMessage(const FederateHandle& federateHandle, bool locked)
  {
    typename FederateHandleCommitMap::iterator i = _federateHandleCommitMap.find(federateHandle);
    if (i == _federateHandleCommitMap.end())
      return;
    i->second._federateIsLocked = locked;
  }

  // O(n)
  bool getLockedByNextMessage(const Unsigned& commitId) const
  {
    if (!getConstrainedByNextMessage())
      return false;
    // Hmm, can we work on counts to summarize that
    for (typename FederateHandleCommitMap::const_iterator i = _federateHandleCommitMap.begin();
         i != _federateHandleCommitMap.end(); ++i) {
      if (_nextMessageLogicalTimeFederateCountMap.begin()->first <= i->second._timeAdvanceCommit->first)
        continue;
      if (!i->second.isInNextMessageMode())
        continue;
      if (i->second._federateIsWaitingForCommitId != commitId)
        return false;
    }
    return true;
  }

  // O(n)
  bool getIsSaveToAdvanceToNextMessage(const Unsigned& commitId) const
  {
    if (!getConstrainedByNextMessage())
      return false;
    // Hmm, can we work on counts to summarize that
    for (typename FederateHandleCommitMap::const_iterator i = _federateHandleCommitMap.begin();
         i != _federateHandleCommitMap.end(); ++i) {
      if (_nextMessageLogicalTimeFederateCountMap.begin()->first <= i->second._timeAdvanceCommit->first)
        continue;
      if (!i->second.isInNextMessageMode())
        continue;
      if (i->second._federateIsWaitingForCommitId != commitId)
        return false;
      if (!i->second._federateIsLocked)
        return false;
    }
    return true;
  }

  // O(n)
  void getNextMessageFederateHandleList(std::list<std::pair<FederateHandle, Unsigned> >& federateHandleCommitIdList)
  {
    for (typename FederateHandleCommitMap::const_iterator i = _federateHandleCommitMap.begin();
         i != _federateHandleCommitMap.end(); ++i) {
      if (!i->second.isInNextMessageMode())
        continue;
      federateHandleCommitIdList.push_back(std::pair<FederateHandle, Unsigned>(i->first, i->second._commitId));
    }
  }

private:
  typedef FederateHandle::value_type FederateCountType;

  class OPENRTI_LOCAL LogicalTimeFederateCountMap {
  public:
    typedef typename std::map<LogicalTime, FederateCountType>::iterator iterator;
    typedef typename std::map<LogicalTime, FederateCountType>::const_iterator const_iterator;
    typedef typename std::map<LogicalTime, FederateCountType>::value_type value_type;

    bool empty() const
    { return _countMap.empty(); }

    iterator begin()
    { return _countMap.begin(); }
    const_iterator begin() const
    { return _countMap.begin(); }

    iterator end()
    { return _countMap.end(); }
    const_iterator end() const
    { return _countMap.end(); }

    iterator find(const LogicalTime& logicalTime)
    { return _countMap.find(logicalTime); }
    const_iterator find(const LogicalTime& logicalTime) const
    { return _countMap.find(logicalTime); }

    iterator insert(const LogicalTime& logicalTime)
    {
      // Only inserts if the entry is new
      iterator i = _countMap.insert(value_type(logicalTime, 0)).first;
      // Increment the reference count for this entry
      ++(i->second);
      return i;
    }

    std::pair<iterator, bool> move(iterator i, const LogicalTime& logicalTime)
    {
      // Only inserts if the entry is new
      iterator j = _countMap.insert(value_type(logicalTime, 0)).first;
      // If we get back the same iterator, we must have gotten the same entry again
      if (j == i)
        return std::pair<iterator, bool>(i, false);

      // Change the reference count on the new entry
      ++(j->second);

      // And if at the old entry the reference count does drop to zero ...
      if (0 != --(i->second))
        return std::pair<iterator, bool>(j, false);

      // ... see if this unleaches a new time ...
      bool isFirstLogicalTime = (i == _countMap.begin());

      // .. and finally erase this entry.
      _countMap.erase(i);

      return std::pair<iterator, bool>(j, isFirstLogicalTime);
    }

    bool erase(iterator i)
    {
      if (0 != --(i->second))
        return false;
      bool isFirstLogicalTime = (i == _countMap.begin());
      _countMap.erase(i);
      return isFirstLogicalTime;
    }

  private:
    std::map<LogicalTime, FederateCountType> _countMap;
  };

  LogicalTimeFederateCountMap _timeAdvanceLogicalTimeFederateCountMap;
  LogicalTimeFederateCountMap _nextMessageLogicalTimeFederateCountMap;

  /// Hmm, want a list of federates to check that are in next event mode. So that we do not need to check everything in turn?!

  typedef typename LogicalTimeFederateCountMap::iterator LogicalTimeMapIterator;
  struct Commit {
    Commit(const LogicalTimeMapIterator& timeAdvanceCommit, const LogicalTimeMapIterator& nextMessageCommit, const Unsigned& commitId, const Unsigned& beforeOwnCommitId) :
      _timeAdvanceCommit(timeAdvanceCommit),
      _nextMessageCommit(nextMessageCommit),
      _commitId(commitId),
      _federateIsWaitingForCommitId(beforeOwnCommitId),
      _federateIsLocked(false)
    {
      OpenRTIAssert(_timeAdvanceCommit->first <= _nextMessageCommit->first);
    }
    bool isInNextMessageMode() const
    {
      OpenRTIAssert(_timeAdvanceCommit->first <= _nextMessageCommit->first);
      return _timeAdvanceCommit->first != _nextMessageCommit->first;
    }
    // Points to the logical time sorted map of time advance commits
    LogicalTimeMapIterator _timeAdvanceCommit;
    // Points to the logical time sorted map of next message commits
    LogicalTimeMapIterator _nextMessageCommit;
    // The commit id of this federate we have seen last.
    Unsigned _commitId;
    // This compares against our own commit id, if they are the same we know that the appropriate federate has received our current commit
    Unsigned _federateIsWaitingForCommitId;
    // Tells us if the federate is locked by next message request
    bool _federateIsLocked;
  };
  typedef std::map<FederateHandle, Commit> FederateHandleCommitMap;
  FederateHandleCommitMap _federateHandleCommitMap;
};

} // namespace OpenRTI

#endif
