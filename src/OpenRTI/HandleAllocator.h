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

#ifndef OpenRTI_HandleAllocator_h
#define OpenRTI_HandleAllocator_h

#include <set>
#include "Exception.h"
#include "Handle.h"

namespace OpenRTI {

template<typename H>
class HandleAllocator {
public:
  typedef H Handle;

  HandleAllocator() :
    _handle(0u)
  { }

  /// Class that manages an open handle allocation transaction.
  /// If you are successful allocating the handle, call take.
  /// If you don't the handle is put back in the free handle list.
  /// Especially in presence of exceptions that class is very useful ...
  class Candidate {
  public:
    Candidate(HandleAllocator& handleAllocator, const Handle& handle = Handle()) :
      _handleAllocator(handleAllocator),
      _handle(handle)
    {
      if (_handle.valid())
        _handleAllocator.take(_handle);
      else
        _handle = _handleAllocator.get();
    }
    ~Candidate()
    {
      if (_handle.valid())
        _handleAllocator.put(_handle);
    }
    const Handle& get() const
    { return _handle; }
    Handle take()
    {
      Handle handle = _handle;
      _handle = Handle();
      return handle;
    }
  private:
    HandleAllocator& _handleAllocator;
    Handle _handle;
  };

  // take the handle out of the allocator. this is used when a slave server tracks
  // the parent servers allocations.
  void take(const Handle& handle)
  {
    if (!handle.valid())
      return;
    typename HandleSet::iterator i = _handleSet.find(handle);
    if (i != _handleSet.end()) {
      _handleSet.erase(i);
      return;
    }
    OpenRTIAssert(_handle <= handle);
    while (_handle < handle)
      _handleSet.insert(_handle++);
    ++_handle;
  }

  Handle get()
  {
    if (_handleSet.empty()) {
      if (!_handle.valid())
        throw ResourceError("Running out of handle values");
      return _handle++;
    }

    Handle handle = *_handleSet.begin();
    _handleSet.erase(_handleSet.begin());
    return handle;
  }

  Handle getOrTake(const Handle& handle)
  {
    if (!handle.valid())
      return get();
    take(handle);
    return handle;
  }

  void put(const Handle& handle)
  {
    if (!handle.valid())
      return;
    _handleSet.insert(handle);
  }

  /// Returns true when there is any handle in use
  bool used() const
  {
    OpenRTIAssert(_handleSet.size() <= _handle.getHandle());
    return _handleSet.size() < _handle.getHandle();
  }

  /// True when we have no more handles to give away.
  bool empty() const
  { return _handleSet.empty() && !_handle.valid(); }

private:

  typedef std::set<Handle> HandleSet;
  HandleSet _handleSet;
  Handle _handle;
};

typedef HandleAllocator<ObjectInstanceHandle> ObjectInstanceHandleAllocator;
typedef HandleAllocator<FederateHandle> FederateHandleAllocator;
typedef HandleAllocator<FederationHandle> FederationHandleAllocator;

typedef HandleAllocator<ModuleHandle> ModuleHandleAllocator;
typedef HandleAllocator<DimensionHandle> DimensionHandleAllocator;
typedef HandleAllocator<InteractionClassHandle> InteractionClassHandleAllocator;
typedef HandleAllocator<ObjectClassHandle> ObjectClassHandleAllocator;

} // namespace OpenRTI

#endif
