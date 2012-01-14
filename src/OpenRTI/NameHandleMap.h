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

#ifndef OpenRTI_NameHandleMap_h
#define OpenRTI_NameHandleMap_h

#include <map>
#include <string>

#include "NameHandlePair.h"
#include "SharedPtr.h"
#include "StringUtils.h"

namespace OpenRTI {

template<typename H, typename T>
class NameHandleMap {
public:
  typedef H Handle;
  typedef std::map<Handle, SharedPtr<T> > HandleObjectMap;
  typedef std::map<std::string, typename HandleObjectMap::iterator> StringIteratorMap;
  typedef typename HandleObjectMap::size_type size_type;
  typedef typename HandleObjectMap::value_type value_type;
  typedef typename HandleObjectMap::iterator iterator;
  typedef typename HandleObjectMap::const_iterator const_iterator;
  typedef typename HandleObjectMap::reverse_iterator reverse_iterator;
  typedef typename HandleObjectMap::const_reverse_iterator const_reverse_iterator;

  bool empty() const
  { return _handleObjectMap.empty(); }

  size_type size() const
  { return _handleObjectMap.size(); }

  void clear()
  { _handleObjectMap.clear(); _stringIteratorMap.clear(); }

  iterator begin()
  { return _handleObjectMap.begin(); }
  iterator end()
  { return _handleObjectMap.end(); }

  const_iterator begin() const
  { return _handleObjectMap.begin(); }
  const_iterator end() const
  { return _handleObjectMap.end(); }

  reverse_iterator rbegin()
  { return _handleObjectMap.rbegin(); }
  reverse_iterator rend()
  { return _handleObjectMap.rend(); }

  const_reverse_iterator rbegin() const
  { return _handleObjectMap.rbegin(); }
  const_reverse_iterator rend() const
  { return _handleObjectMap.rend(); }

  std::pair<iterator, bool> insert(const value_type& value)
  {
    std::pair<iterator, bool> p = _handleObjectMap.insert(value);
    _stringIteratorMap.insert(typename StringIteratorMap::value_type(value.second->getName(), p.first));
    return p;
  }

  void erase(iterator i)
  {
    typename StringIteratorMap::iterator j = _stringIteratorMap.find(i->second->getName());
    if (j != _stringIteratorMap.end())
      _stringIteratorMap.erase(j);
    _handleObjectMap.erase(i);
  }

  void eraseName(iterator i)
  {
    typename StringIteratorMap::iterator j = _stringIteratorMap.find(i->second->getName());
    if (j != _stringIteratorMap.end())
      _stringIteratorMap.erase(j);
  }

  iterator find(const Handle& handle)
  { return _handleObjectMap.find(handle); }
  const_iterator find(const Handle& handle) const
  { return _handleObjectMap.find(handle); }

  iterator find(const std::string& name)
  {
    typename StringIteratorMap::const_iterator i = _stringIteratorMap.find(name);
    if (i == _stringIteratorMap.end())
      return _handleObjectMap.end();
    return i->second;
  }
  const_iterator find(const std::string& name) const
  {
    typename StringIteratorMap::const_iterator i = _stringIteratorMap.find(name);
    if (i == _stringIteratorMap.end())
      return _handleObjectMap.end();
    return i->second;
  }

  T* getObject(const Handle& handle) const
  {
    typename HandleObjectMap::const_iterator i = _handleObjectMap.find(handle);
    if (i == _handleObjectMap.end())
      return 0;
    return i->second.get();
  }
  Handle getHandle(const std::string& name) const
  {
    typename StringIteratorMap::const_iterator i = _stringIteratorMap.find(name);
    if (i == _stringIteratorMap.end())
      return Handle();
    return i->second->first;
  }
  bool exists(const Handle& handle) const
  { return _handleObjectMap.find(handle) != _handleObjectMap.end(); }
  bool exists(const std::string& name) const
  {
    return _stringIteratorMap.find(name) != _stringIteratorMap.end();
  }

private:
  HandleObjectMap _handleObjectMap;
  StringIteratorMap _stringIteratorMap;
};

} // namespace OpenRTI

#endif
