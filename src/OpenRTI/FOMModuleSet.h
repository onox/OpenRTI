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

#ifndef OpenRTI_FOMModuleSet_h
#define OpenRTI_FOMModuleSet_h

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

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

class OPENRTI_API FOMModuleSet {
public:
  FOMModuleSet();
  ~FOMModuleSet();

  FOMModuleHandleSet insertModuleList(const FOMStringModuleList& moduleList);
  bool testModuleList(const FOMStringModuleList& moduleList);
  void insertModuleList(const FOMModuleList& moduleList);

  // // Should return the full set of fom module handles that are needed to satisfy a new federate with the given
  // // modules handle list. The returned list should contain the base modules from the original
  // // create request and the mim module.
  // FOMModuleHandleSet getFullHandleList(const FOMModuleHandleSet& moduleHandleList) const
  // {
  //   FOMModuleHandleSet handleSet;
  //   std::set_union(moduleHandleList.begin(), moduleHandleList.end(),
  //                  _createModuleHandles.begin(), _createModuleHandles.end(),
  //                  std::inserter(handleSet, handleSet.begin()));
  //   return handleSet;
  // }

  // FOMModuleHandleSet _createModuleHandles;

  FOMModule getFOMModule(const FOMModuleHandle& moduleHandle) const;
  FOMModuleList getModuleList() const;

private:
  FOMModuleSet(const FOMModuleSet&);
  FOMModuleSet& operator=(const FOMModuleSet&);

  struct AllocatorMap;
  SharedPtr<AllocatorMap> _allocatorMap;
};

} // namespace OpenRTI

#endif
