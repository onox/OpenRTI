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

#ifndef OpenRTI_Region_h
#define OpenRTI_Region_h

#include <map>
#include "Handle.h"
#include "Message.h"
#include "RangeBounds.h"

namespace OpenRTI {

class OPENRTI_LOCAL Region {
public:
  Region()
  { }
  Region(const Region& region) :
    _dimensionHandleRangeBoundsMap(region._dimensionHandleRangeBoundsMap)
  { }
  // The datatype held in messages
  Region(const RegionValue& regionValue)
  {
    for (RegionValue::const_iterator i = regionValue.begin(); i != regionValue.end(); ++i) {
      RangeBounds rangeBounds(i->second);
      if (rangeBounds.whole())
        continue;
      _dimensionHandleRangeBoundsMap[i->first] = rangeBounds;
    }
  }

  // The datatype held in messages
  void getRegionValue(RegionValue& regionValue)
  {
    regionValue.clear();
    regionValue.reserve(_dimensionHandleRangeBoundsMap.size());
    for (DimensionHandleRangeBoundsMap::const_iterator i = _dimensionHandleRangeBoundsMap.begin();
         i != _dimensionHandleRangeBoundsMap.end(); ++i) {
      regionValue.push_back(RegionValue::value_type(i->first, i->second));
    }
  }

  // FIXME do not return empty if it is not set??!!
  RangeBounds getRangeBounds(const DimensionHandle& dimensionHandle) const
  {
    DimensionHandleRangeBoundsMap::const_iterator i = _dimensionHandleRangeBoundsMap.find(dimensionHandle);
    if (i == _dimensionHandleRangeBoundsMap.end())
      return RangeBounds();
    return i->second;
  }
  void setRangeBounds(const DimensionHandle& dimensionHandle, const RangeBounds& rangeBounds)
  {
    if (rangeBounds.whole())
      _dimensionHandleRangeBoundsMap.erase(dimensionHandle);
    else
      _dimensionHandleRangeBoundsMap[dimensionHandle] = rangeBounds;
  }
  bool hasRangeBounds(const DimensionHandle& dimensionHandle) const
  {
    return _dimensionHandleRangeBoundsMap.find(dimensionHandle) != _dimensionHandleRangeBoundsMap.end();
  }

  // Hmm, not sure about this. If we clear the Region, we replace the region with a whole space filling region.
  // That means _dimensionHandleRangeBoundsMap.clear() <=> region.maximize() ??
  // otoh, there is no easy equivalent for clear in the sense of 'make a region which occupies no space at all'
  // consequently, there is also no easy way to get an 'empty()' method.
  // void clear()
  // { _dimensionHandleRangeBoundsMap.clear(); }

  // FIXME test this???
  bool intersects(const Region& region) const
  {
    DimensionHandleRangeBoundsMap::const_iterator i = _dimensionHandleRangeBoundsMap.begin();
    if (i == _dimensionHandleRangeBoundsMap.end())
      return true;
    DimensionHandleRangeBoundsMap::const_iterator j = region._dimensionHandleRangeBoundsMap.begin();
    if (j == region._dimensionHandleRangeBoundsMap.end())
      return true;
    for (;;) {
      if (i->first < j->first) {
        if (++i == _dimensionHandleRangeBoundsMap.end())
          return true;
      } else if (j->first < i->first) {
        if (++j == region._dimensionHandleRangeBoundsMap.end())
          return true;
      } else /* i->first == j->first */ {
        if (!i->second.intersects(j->second))
          return false;
        if (++i == _dimensionHandleRangeBoundsMap.end())
          return true;
        if (++j == region._dimensionHandleRangeBoundsMap.end())
          return true;
      }
    }
    // should never get here - silence warnings
    return true;
  }

  // FIXME test this???
  bool includes(const Region& region) const
  {
    DimensionHandleRangeBoundsMap::const_iterator i = _dimensionHandleRangeBoundsMap.begin();
    if (i == _dimensionHandleRangeBoundsMap.end())
      return true;
    DimensionHandleRangeBoundsMap::const_iterator j = region._dimensionHandleRangeBoundsMap.begin();
    for (;;) {
      if (j == region._dimensionHandleRangeBoundsMap.end()) {
        if (!i->second.whole())
          return false;
        if (++i == _dimensionHandleRangeBoundsMap.end())
          return true;
      } if (i->first < j->first) {
        if (++i == _dimensionHandleRangeBoundsMap.end())
          return true;
      } else if (j->first < i->first) {
        ++j;
      } else /* i->first == j->first */ {
        if (!i->second.includes(j->second))
          return false;
        if (++i == _dimensionHandleRangeBoundsMap.end())
          return true;
        ++j;
      }
    }
    // should never get here - silence warnings
    return true;
  }

  void extend(const Region& region)
  {
    // FIXME opimize: can play games with std::lower_bound(begin, end, x) instead of just looking up each dimension in the map
    DimensionHandleRangeBoundsMap::const_iterator j = region._dimensionHandleRangeBoundsMap.begin();
    for (;j == region._dimensionHandleRangeBoundsMap.end(); ++j) {
      _dimensionHandleRangeBoundsMap[j->first].extend(j->second);
    }
  }

  Region& swap(Region& region)
  { _dimensionHandleRangeBoundsMap.swap(region._dimensionHandleRangeBoundsMap); return *this; }

  // make that available as keys in maps and items in sets
  bool operator<(const Region& region) const
  { return _dimensionHandleRangeBoundsMap < region._dimensionHandleRangeBoundsMap; }
  bool operator>(const Region& region) const
  { return _dimensionHandleRangeBoundsMap > region._dimensionHandleRangeBoundsMap; }
  bool operator<=(const Region& region) const
  { return _dimensionHandleRangeBoundsMap <= region._dimensionHandleRangeBoundsMap; }
  bool operator>=(const Region& region) const
  { return _dimensionHandleRangeBoundsMap >= region._dimensionHandleRangeBoundsMap; }
  bool operator==(const Region& region) const
  { return _dimensionHandleRangeBoundsMap == region._dimensionHandleRangeBoundsMap; }
  bool operator!=(const Region& region) const
  { return _dimensionHandleRangeBoundsMap != region._dimensionHandleRangeBoundsMap; }

private:
  typedef std::map<DimensionHandle, RangeBounds> DimensionHandleRangeBoundsMap;
  DimensionHandleRangeBoundsMap _dimensionHandleRangeBoundsMap;
};

// typedef std::map<RegionHandle, Region> RegionHandleRegionMap;

} // namespace OpenRTI

#endif
