/* -*-c++-*- OpenRTI - Copyright (C) 2013-2022 Mathias Froehlich
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

#ifndef OpenRTI_IntrusiveUnroderedMap_h
#define OpenRTI_IntrusiveUnroderedMap_h

#include <string>
#include <vector>

#include "Exception.h"
#include "IntrusiveList.h"

namespace OpenRTI {

template<typename T>
struct OPENRTI_LOCAL Hash {
  std::size_t operator()(const T& value) const
  { return 0; }
};

template<>
struct OPENRTI_LOCAL Hash<std::string> {
  std::size_t operator()(const std::string& string) const
  {
    std::size_t hash = 0;
    for (std::string::const_iterator i = string.begin(); i != string.end(); ++i)
      hash = 5*hash + *i;
    return hash;
  }
};

template<>
struct OPENRTI_LOCAL Hash<std::vector<std::string> > {
  std::size_t operator()(const std::vector<std::string>& stringVector) const
  {
    std::size_t hash = 0;
    for (std::vector<std::string>::const_iterator i = stringVector.begin(); i != stringVector.end(); ++i)
      for (std::string::const_iterator j = i->begin(); j != i->end(); ++j)
        hash = 5*hash + *j;
    return hash;
  }
};

template<typename Key, typename T, typename _Hash = Hash<Key>, unsigned tag = 0>
class OPENRTI_LOCAL IntrusiveUnorderedMap {
  class OPENRTI_LOCAL _TotalListTag {};
  class OPENRTI_LOCAL _BucketListTag {};
#if defined(__SUNPRO_CC)
public:
#endif
  typedef _IntrusiveList<T, _TotalListTag> _List;
  typedef _IntrusiveList<T, _BucketListTag> _BucketList;
  typedef std::vector<_BucketList> _BucketVector;
public:
  typedef T value_type;
  typedef std::size_t size_type;
  typedef T& reference;
  typedef T const& const_reference;
  typedef T* pointer;
  typedef T const* const_pointer;

  class OPENRTI_LOCAL Hook : public _List::Hook, public _BucketList::Hook {
  public:
    Hook() :
      _key()
    { }
    Hook(const Key& key) :
      _key(key)
    { }

    bool is_linked() const
    { OpenRTIAssert(_List::Hook::is_linked() == _BucketList::Hook::is_linked()); return _List::Hook::is_linked(); }

    const Key& getKey() const
    { return _key; }

  protected:
    void setKey(const Key& key)
    { OpenRTIAssert(!is_linked()); _key = key; }

  private:
    Key _key;
  };

  typedef typename _List::iterator iterator;
  typedef typename _List::const_iterator const_iterator;
  typedef typename _List::reverse_iterator reverse_iterator;
  typedef typename _List::const_reverse_iterator const_reverse_iterator;

  IntrusiveUnorderedMap(const size_type& numBuckets = 128) :
    _bucketVector(numBuckets)
  { }
  IntrusiveUnorderedMap(const IntrusiveUnorderedMap& intrusiveUnorderedMap)
  { OpenRTIAssert(intrusiveUnorderedMap.empty()); }
#if 201103L <= __cplusplus || 200610L <= __cpp_rvalue_reference
  IntrusiveUnorderedMap(IntrusiveUnorderedMap&& intrusiveUnorderedMap)
  { swap(intrusiveUnorderedMap); }
#endif
  ~IntrusiveUnorderedMap()
  { OpenRTIAssert(empty()); }

  IntrusiveUnorderedMap& operator=(const IntrusiveUnorderedMap& intrusiveUnorderedMap)
  { OpenRTIAssert(empty()); OpenRTIAssert(intrusiveUnorderedMap.empty()); return *this; }
#if 201103L <= __cplusplus || 200610L <= __cpp_rvalue_reference
  IntrusiveUnorderedMap& operator=(IntrusiveUnorderedMap&& intrusiveUnorderedMap)
  { swap(intrusiveUnorderedMap); return *this; }
#endif

  bool empty() const
  { return _list.empty(); }
  // Truely O(n)!!
  size_type size() const
  { return _list.size(); }
  /// returns true if the map contains at most one element
  bool single() const
  { return _list.single(); }
  /// returns true if the map contains exactly one element
  bool unique() const
  { return _list.unique(); }

  iterator begin()
  { return _list.begin(); }
  iterator end()
  { return _list.end(); }

  const_iterator begin() const
  { return _list.begin(); }
  const_iterator end() const
  { return _list.end(); }

  reverse_iterator rbegin()
  { return _list.rbegin(); }
  reverse_iterator rend()
  { return _list.rend(); }

  const_reverse_iterator rbegin() const
  { return _list.rbegin(); }
  const_reverse_iterator rend() const
  { return _list.rend(); }

  iterator insert(reference value)
  {
    // First insert into the bucket.
    _BucketList& bucketList = _bucketVector[_index(_select(value).getKey())];
    // Group entries with the same key.
    typename _BucketList::iterator i = bucketList.begin();
    while (i != bucketList.end()) {
      if (_select(value).getKey() == _select(*i).getKey())
        break;
      ++i;
    }
    bucketList.insert(i, value);

    // And put them into the global list, also grouped by key.
    typename _List::iterator j;
    if (i != bucketList.end())
      j = _List::it(*i);
    else
      j = _list.end();
    return _list.insert(j, value);
  }

  /// Remove from the map and delete the entry
  iterator erase(iterator i)
  { return _list.erase(i); }
  reverse_iterator erase(reverse_iterator i)
  { return _list.erase(i); }
  static void erase(reference value)
  { delete &value; }

  void clear()
  {
    iterator i = begin();
    while (i != end())
      i = erase(i);
  }

  /// Unlinks the entry from the map. Does not delete the entry.
  iterator unlink(iterator i)
  { _BucketList::unlink(*i); return _list.unlink(i); }
  reverse_iterator unlink(reverse_iterator i)
  { _BucketList::unlink(*i); return _list.unlink(i); }
  static void unlink(reference value)
  { _BucketList::unlink(value); _List::unlink(value); }

  void unlink()
  {
    iterator i = begin();
    while (i != end())
      i = unlink(i);
  }

  iterator find(const Key& key)
  {
    size_type index = _index(key);
    for (typename _BucketList::const_iterator i = _bucketVector[index].begin(), end = _bucketVector[index].end();
         i != end; ++i) {
      if (key == _select(*i).getKey())
        return _List::it(*i);
    }
    return _list.end();
  }
  const_iterator find(const Key& key) const
  {
    size_type index = _index(key);
    for (typename _BucketList::const_iterator i = _bucketVector[index].begin(), end = _bucketVector[index].end();
         i != end; ++i) {
      if (key == _select(*i).getKey())
        return _List::it(*i);
    }
    return _list.end();
  }

  const_reference front() const
  { return _list.front(); }
  reference front()
  { return _list.front(); }

  const_reference back() const
  { return _list.back(); }
  reference back()
  { return _list.back(); }

private:
  static const Hook& _select(const_reference t)
  { return static_cast<const Hook&>(t); }
  static Hook& _select(reference t)
  { return static_cast<Hook&>(t); }

  size_type _index(const Key& key) const
  { return _Hash()(key) % _bucketVector.size(); }

  _List _list;
  _BucketVector _bucketVector;
};

} // namespace OpenRTI

#endif
