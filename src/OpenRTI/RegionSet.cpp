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

#include "RegionSet.h"

#include "Region.h"

namespace OpenRTI {

// FIXME implement the bounding volume tree we have a sketch for in this file

struct OPENRTI_LOCAL RegionSet::Visitor {
  virtual void apply(Leaf& leaf) = 0;
  virtual void apply(Branch& branch) = 0;
};

struct OPENRTI_LOCAL RegionSet::Node : public Referenced /* FIXME Memory management could be done explicit easily here */ {
  virtual ~Node() {}
  virtual void accept(Visitor& visitor) = 0;
  Region _region;
};

struct OPENRTI_LOCAL RegionSet::Leaf : public RegionSet::Node {
  virtual void accept(Visitor& visitor)
  { visitor.apply(*this); }
};

struct OPENRTI_LOCAL RegionSet::Branch : public RegionSet::Node {
  virtual void accept(Visitor& visitor)
  { visitor.apply(*this); }
  SharedPtr<Node> _left;
  SharedPtr<Node> _right;
};

struct OPENRTI_LOCAL RegionSet::LeafIntersectVisitor : public RegionSet::Visitor {
  LeafIntersectVisitor(Leaf& leaf) :
    _foundIntersection(false),
    _leaf(leaf)
  {
  }
  virtual void apply(Leaf& leaf)
  {
    if (_foundIntersection)
      return;
    _foundIntersection = _leaf._region.intersects(leaf._region);
  }
  virtual void apply(Branch& branch)
  {
    if (_foundIntersection)
      return;
    if (!branch._region.intersects(_leaf._region))
      return;
    branch._left->accept(*this);
    if (_foundIntersection)
      return;
    branch._right->accept(*this);
  }

  bool _foundIntersection;
  Leaf& _leaf;
};

struct OPENRTI_LOCAL RegionSet::NodeIntersectVisitor : public RegionSet::Visitor {
  NodeIntersectVisitor(Node& node) :
    _foundIntersection(false),
    _node(node)
  {
  }
  virtual void apply(Leaf& leaf)
  {
    if (_foundIntersection)
      return;
    LeafIntersectVisitor visitor(leaf);
    _node.accept(visitor);
    _foundIntersection = visitor._foundIntersection;
  }
  virtual void apply(Branch& branch)
  {
    if (_foundIntersection)
      return;
    if (!branch._region.intersects(_node._region))
      return;
    // FIXME retink, is this the right thing to do?
    if (branch._region.includes(_node._region)) {
      branch._left->accept(*this);
      if (_foundIntersection)
        return;
      branch._right->accept(*this);
    } else {
      {
        NodeIntersectVisitor visitor(*branch._left);
        _node.accept(visitor);
        _foundIntersection = visitor._foundIntersection;
      }
      if (_foundIntersection)
        return;
      {
        NodeIntersectVisitor visitor(*branch._right);
        _node.accept(visitor);
        _foundIntersection = visitor._foundIntersection;
      }
    }
  }

  bool _foundIntersection;
  Node& _node;
};

RegionSet::RegionSet() :
  _regionHandleLeafMap(new RegionHandleLeafMap)
{
}

RegionSet::RegionSet(const RegionSet& regionSet) :
  _regionHandleLeafMap(new RegionHandleLeafMap(*regionSet._regionHandleLeafMap)),
  _node(regionSet._node)
{
}

RegionSet::~RegionSet()
{
  delete _regionHandleLeafMap;
}

RegionSet&
RegionSet::operator=(const RegionSet& regionSet)
{
  *_regionHandleLeafMap = *regionSet._regionHandleLeafMap;
  _node = regionSet._node;
  return *this;
}

bool
RegionSet::empty() const
{
  return _regionHandleLeafMap->empty();
}

void
RegionSet::insert(const RegionHandle& regionHandle, const Region& region)
{
  SharedPtr<Leaf> leaf = new Leaf;
  leaf->_region = region;
  (*_regionHandleLeafMap)[regionHandle] = leaf;
}

void
RegionSet::erase(const RegionHandle& regionHandle)
{
  _regionHandleLeafMap->erase(regionHandle);
}

void
RegionSet::erase(const FederateHandle& federateHandle)
{
  RegionHandleLeafMap::iterator i = _regionHandleLeafMap->lower_bound(RegionHandle(federateHandle, LocalRegionHandle(0)));
  RegionHandleLeafMap::iterator e = _regionHandleLeafMap->upper_bound(RegionHandle(federateHandle, LocalRegionHandle()));
  while (i != e) {
    // Not just erase all, we need to remove these from the bv tree when it is there
    _regionHandleLeafMap->erase(i++);
  }
}

bool
RegionSet::intersects(const RegionSet& regionSet) const
{
  // Puh, O(n^2) ... that bouding volume tree ... FIXME
  for (RegionHandleLeafMap::const_iterator i = _regionHandleLeafMap->begin(); i != _regionHandleLeafMap->end(); ++i) {
    for (RegionHandleLeafMap::const_iterator j = regionSet._regionHandleLeafMap->begin(); j != regionSet._regionHandleLeafMap->end(); ++j) {
      if (i->second->_region.intersects(j->second->_region))
        return true;
    }
  }
  return false;
}

} // namespace OpenRTI
