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

#include "Handle.h"
#include "LogStream.h"

namespace OpenRTI {

template<typename T>
class OPENRTI_LOCAL FederateHandleLowerBoundTimeStampMap {
public:
  typedef T LogicalTime;
  typedef std::pair<LogicalTime, int> LogicalTimePair;

  /// Insert a new federate handle with the initial logical time pair
  void insert(const FederateHandle& federateHandle, const LogicalTime& logicalTime)
  {
    // Store that in the time map, so that we cannot advance more than that until this federate
    // commits a new lbts in the rti. This will usually happen in immediate response to that sent message.
    typename LogicalTimeFederateCountMap::iterator i;
    // only inserts if the entry is new
    i = _logicalTimeFederateCountMap.insert(typename LogicalTimeFederateCountMap::value_type(logicalTime, 0)).first;
    ++(i->second);
    OpenRTIAssert(_federateHandleLogicalTimeMap.find(federateHandle) == _federateHandleLogicalTimeMap.end());
    _federateHandleLogicalTimeMap.insert(typename FederateHandleLogicalTimeMap::value_type(federateHandle, i));
  }

  /// Erase a federate handle including its logical time bounds
  /// Returns true if this unleaches a new logical time
  bool erase(const FederateHandle& federateHandle)
  {
    typename FederateHandleLogicalTimeMap::iterator i;
    i = _federateHandleLogicalTimeMap.find(federateHandle);
    // OpenRTIAssert(i != _federateHandleLogicalTimeMap.end());
    if (i == _federateHandleLogicalTimeMap.end())
      return false;

    typename LogicalTimeFederateCountMap::iterator j = i->second;
    _federateHandleLogicalTimeMap.erase(i);
    if (0 != --(j->second))
      return false;

    bool isFirstLogicalTime = (j == _logicalTimeFederateCountMap.begin());
    _logicalTimeFederateCountMap.erase(j);
    return isFirstLogicalTime;
  }

  /// Returns true if this unleaches a new logical time
  bool commit(const FederateHandle& federateHandle, const LogicalTime& logicalTime)
  {
    // Once we receive these commits, the federate must have registered as some regulating, this must be here
    OpenRTIAssert(!_logicalTimeFederateCountMap.empty());
    OpenRTIAssert(!_federateHandleLogicalTimeMap.empty());

    typename FederateHandleLogicalTimeMap::iterator i;
    i = _federateHandleLogicalTimeMap.find(federateHandle);
    OpenRTIAssert(i != _federateHandleLogicalTimeMap.end());

    // Only continue if this is a real advance
    typename LogicalTimeFederateCountMap::iterator j = i->second;
    // Only allow an advance while committing
    // OpenRTIAssert(j->first <= logicalTime);
    // And don't do something if its not a real advance!
    if (logicalTime <= j->first)
      return false;

    // Register the new logical time for this federate.
    // If this one is federate handle is already registered at this given time, just return.
    // This case where nothing new is in this commit might happen when a federate started
    // being time regulating and got a correction from this current federate. Then we
    // have already noted this corrected time in the map.
    typename LogicalTimeFederateCountMap::iterator k;
    // only inserts if the entry is new
    k = _logicalTimeFederateCountMap.insert(typename LogicalTimeFederateCountMap::value_type(logicalTime, 0)).first;
    ++(k->second);

    // The federate has a new timestamp, complete that change and remove the reference to the old timestamp now
    // store the new logical time for the federate handle
    std::swap(i->second, k);

    // Erase the old logical time and
    // check if we have with this commit now enabled a time advance in some sense.
    // If this map entry has no federate handle left, then this is the case
    if (0 != --(j->second))
      return false;

    bool isFirstLogicalTime = (j == _logicalTimeFederateCountMap.begin());

    // This map entry could then be erased ...
    _logicalTimeFederateCountMap.erase(j);

    OpenRTIAssert(!_logicalTimeFederateCountMap.empty());
    OpenRTIAssert(!_federateHandleLogicalTimeMap.empty());

    return isFirstLogicalTime;
  }

  bool canAdvanceTo(const LogicalTimePair& logicalTimePair) const
  {
    if (empty())
      return true;
    if (0 < logicalTimePair.second)
      return logicalTimePair.first < _logicalTimeFederateCountMap.begin()->first;
    else
      return logicalTimePair.first <= _logicalTimeFederateCountMap.begin()->first;
  }

  bool empty() const
  {
    OpenRTIAssert(_logicalTimeFederateCountMap.empty() == _federateHandleLogicalTimeMap.empty());
    return _logicalTimeFederateCountMap.empty();
  }

  const LogicalTime& getGALT() const
  {
    OpenRTIAssert(!_logicalTimeFederateCountMap.empty());
    OpenRTIAssert(!_federateHandleLogicalTimeMap.empty());
    return _logicalTimeFederateCountMap.begin()->first;
  }

private:
  typedef std::map<LogicalTime, FederateHandle::value_type> LogicalTimeFederateCountMap;
  LogicalTimeFederateCountMap _logicalTimeFederateCountMap;

  typedef typename LogicalTimeFederateCountMap::iterator LogicalTimeMapIterator;
  typedef std::map<FederateHandle, LogicalTimeMapIterator> FederateHandleLogicalTimeMap;
  FederateHandleLogicalTimeMap _federateHandleLogicalTimeMap;
};

} // namespace OpenRTI

#endif
