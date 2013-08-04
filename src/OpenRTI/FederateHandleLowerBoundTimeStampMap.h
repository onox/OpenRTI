/* -*-c++-*- OpenRTI - Copyright (C) 2009-2013 Mathias Froehlich
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
#include <set>

#include "Handle.h"
#include "LogStream.h"

namespace OpenRTI {

template<typename T>
class OPENRTI_LOCAL FederateHandleLowerBoundTimeStampMap {
public:
  typedef T LogicalTime;
  typedef std::pair<LogicalTime, bool> LogicalTimePair;

  void insert(const FederateHandle& federateHandle, const LogicalTimePair& logicalTimePair)
  {
    // Store that in the time map, so that we cannot advance more than that until this federate
    // commits a new lbts in the rti. This will usually happen in immediate response to that sent message.
    typename LogicalTimeFederateHandleSetMap::iterator i;
    // only inserts if the entry is new
    i = _logicalTimeFederateHandleSetMap.insert(std::make_pair(logicalTimePair, FederateHandleSet())).first;
    OpenRTIAssert(i->second.find(federateHandle) == i->second.end());
    i->second.insert(federateHandle);
    OpenRTIAssert(_federateHandleLogicalTimeMap.find(federateHandle) == _federateHandleLogicalTimeMap.end());
    _federateHandleLogicalTimeMap.insert(std::make_pair(federateHandle, i));
  }

  /// Returns true if this unleaches a new logical time
  bool erase(const FederateHandle& federateHandle)
  {
    typename FederateHandleLogicalTimeMap::iterator i;
    i = _federateHandleLogicalTimeMap.find(federateHandle);
    // OpenRTIAssert(i != _federateHandleLogicalTimeMap.end());
    if (i == _federateHandleLogicalTimeMap.end())
      return false;

    typename LogicalTimeFederateHandleSetMap::iterator j = i->second;
    OpenRTIAssert(j->second.find(federateHandle) != j->second.end());
    j->second.erase(federateHandle);
    _federateHandleLogicalTimeMap.erase(i);
    if (!j->second.empty())
      return false;

    bool isFirstLogicalTime = (j == _logicalTimeFederateHandleSetMap.begin());
    _logicalTimeFederateHandleSetMap.erase(j);
    return isFirstLogicalTime;
  }

  /// Returns true if this unleaches a new logical time
  bool commit(const FederateHandle& federateHandle, const LogicalTimePair& logicalTimePair)
  {
    // Once we receive these commits, the federate must have registered as some regulating, this must be here
    OpenRTIAssert(!_logicalTimeFederateHandleSetMap.empty());
    OpenRTIAssert(!_federateHandleLogicalTimeMap.empty());

    // Register the new logical time for this federate.
    // If this one is federate handle is already registered at this given time, just return.
    // This case where nothing new is in this commit might happen when a federate started
    // being time regulating and got a correction from this current federate. Then we
    // have already noted this corrected time in the map.
    typename LogicalTimeFederateHandleSetMap::iterator i;
    // only inserts if the entry is new
    i = _logicalTimeFederateHandleSetMap.insert(std::make_pair(logicalTimePair, FederateHandleSet())).first;
    if (!i->second.insert(federateHandle).second)
      return false;

    typename FederateHandleLogicalTimeMap::iterator j;
    j = _federateHandleLogicalTimeMap.find(federateHandle);
    OpenRTIAssert(j != _federateHandleLogicalTimeMap.end());
    // The federate has a new timestamp, complete that change and remove the reference to the old timestamp now
    // store the new logical time for the federate handle
    std::swap(j->second, i);
    // and erase the old logical time
    i->second.erase(federateHandle);

    // Check if we have with this commit now enabled a time advance in some sense.
    // If this map entry has no federate handle left, then this is the case
    if (!i->second.empty())
      return false;

    bool isFirstLogicalTime = (i == _logicalTimeFederateHandleSetMap.begin());

    // This map entry could then be erased ...
    _logicalTimeFederateHandleSetMap.erase(i);

    OpenRTIAssert(!_logicalTimeFederateHandleSetMap.empty());
    OpenRTIAssert(!_federateHandleLogicalTimeMap.empty());

    return isFirstLogicalTime;
  }

  bool canAdvanceTo(const LogicalTime& logicalTime) const
  {
    if (empty())
      return true;
    return logicalTime <= _logicalTimeFederateHandleSetMap.begin()->first.first;
  }

  bool canAdvanceTo(const LogicalTimePair& logicalTimePair) const
  {
    if (empty())
      return true;
    return logicalTimePair <= _logicalTimeFederateHandleSetMap.begin()->first;
  }

  bool empty() const
  {
    OpenRTIAssert(_logicalTimeFederateHandleSetMap.empty() == _federateHandleLogicalTimeMap.empty());
    return _logicalTimeFederateHandleSetMap.empty();
  }

  const LogicalTime& getGALT() const
  {
    OpenRTIAssert(!_logicalTimeFederateHandleSetMap.empty());
    OpenRTIAssert(!_federateHandleLogicalTimeMap.empty());
    return _logicalTimeFederateHandleSetMap.begin()->first.first;
  }

private:
  typedef std::map<LogicalTimePair, FederateHandleSet> LogicalTimeFederateHandleSetMap;
  LogicalTimeFederateHandleSetMap _logicalTimeFederateHandleSetMap;

  typedef std::map<FederateHandle, typename LogicalTimeFederateHandleSetMap::iterator> FederateHandleLogicalTimeMap;
  FederateHandleLogicalTimeMap _federateHandleLogicalTimeMap;
};

} // namespace OpenRTI

#endif
