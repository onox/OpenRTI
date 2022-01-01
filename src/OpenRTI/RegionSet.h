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

#ifndef OpenRTI_RegionSet_h
#define OpenRTI_RegionSet_h

#include "Region.h"
#include "SharedPtr.h"

namespace OpenRTI {

class OPENRTI_LOCAL RegionSet {
public:
  RegionSet();
  RegionSet(const RegionSet&);
  ~RegionSet();

  RegionSet& operator=(const RegionSet&);

  // Empty means no regions in there, not even the default one.
  // So an empty RegionSet does not intersect with any other RegionSet.
  bool empty() const;

  // Insert a new region with a given region handle
  void insert(const RegionHandle& regionHandle, const Region& region);
  // Erase a specific region
  void erase(const RegionHandle& regionHandle);
  // Erase all regions belonging to a given federate handle
  void erase(const FederateHandle& federateHandle);

  // Test for intersection of regions sets.
  bool intersects(const RegionSet& regionSet) const;

private:
  struct Visitor;
  struct LeafIntersectVisitor;
  struct NodeIntersectVisitor;
  struct Node;
  struct Leaf;
  struct Branch;

  typedef std::map<RegionHandle, SharedPtr<Leaf> > RegionHandleLeafMap;
  RegionHandleLeafMap* _regionHandleLeafMap;

  SharedPtr<Node> _node;
};

} // namespace OpenRTI

#endif
