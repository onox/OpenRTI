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

#ifndef OpenRTI_NameHandleVector_h
#define OpenRTI_NameHandleVector_h

#include <map>
#include <string>
#include <vector>

#include "NameHandlePair.h"
#include "SharedPtr.h"
#include "StringUtils.h"

namespace OpenRTI {

template<typename H, typename T>
class NameHandleVector {
public:
  typedef H Handle;
  typedef std::vector<SharedPtr<T> > ObjectVector;
  typedef std::map<std::wstring, Handle> StringHandleMap;
  typedef typename ObjectVector::size_type size_type;
  typedef typename ObjectVector::value_type value_type;
  typedef typename ObjectVector::iterator iterator;
  typedef typename ObjectVector::const_iterator const_iterator;

  bool empty() const
  { return _objectVector.empty(); }

  size_type size() const
  { return _objectVector.size(); }

  void clear()
  { _objectVector.clear(); _stringHandleMap.clear(); }

  iterator begin()
  { return _objectVector.begin(); }
  iterator end()
  { return _objectVector.end(); }

  const_iterator begin() const
  { return _objectVector.begin(); }
  const_iterator end() const
  { return _objectVector.end(); }

  void insert(const value_type& value)
  {
    Handle handle = value->getHandle();
    size_type index = size_type(handle);
    if (_objectVector.size() <= index)
      _objectVector.resize(index + 1);
    _objectVector[index] = value;
    _stringHandleMap.insert(typename StringHandleMap::value_type(value->getName(), handle));
  }

  iterator find(const Handle& handle)
  {
    size_type index = size_type(handle);
    if (_objectVector.size() <= index)
      return _objectVector.end();
    iterator i = _objectVector.begin();
    std::advance(i, index);
    if (!i->valid())
      return _objectVector.end();
    return i;
  }
  const_iterator find(const Handle& handle) const
  {
    size_type index = size_type(handle);
    if (_objectVector.size() <= index)
      return _objectVector.end();
    const_iterator i = _objectVector.begin();
    std::advance(i, index);
    if (!i->valid())
      return _objectVector.end();
    return i;
  }

  iterator find(const std::wstring& name)
  {
    typename StringHandleMap::const_iterator i = _stringHandleMap.find(name);
    if (i == _stringHandleMap.end())
      return _objectVector.end();
    iterator j = _objectVector.begin();
    std::advance(j, i->second);
    return j;
  }
  const_iterator find(const std::wstring& name) const
  {
    typename StringHandleMap::const_iterator i = _stringHandleMap.find(name);
    if (i == _stringHandleMap.end())
      return _objectVector.end();
    const_iterator j = _objectVector.begin();
    std::advance(j, i->second);
    return j;
  }

  T* getObject(const Handle& handle) const
  {
    size_type index = size_type(handle);
    if (_objectVector.size() <= index)
      return 0;
    const_iterator i = _objectVector.begin();
    std::advance(i, index);
    return i->get();
  }
  Handle getHandle(const std::wstring& name) const
  {
    typename StringHandleMap::const_iterator i = _stringHandleMap.find(name);
    if (i == _stringHandleMap.end())
      return Handle();
    return i->second;
  }
  bool exists(const Handle& handle) const
  {
    size_type index = size_type(handle);
    if (_objectVector.size() <= index)
      return false;
    return _objectVector[index].valid();
  }
  bool exists(const std::wstring& name) const
  { return _stringHandleMap.find(name) != _stringHandleMap.end(); }

private:
  ObjectVector _objectVector;
  StringHandleMap _stringHandleMap;
};

} // namespace OpenRTI

#endif
