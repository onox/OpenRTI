/* -*-c++-*- OpenRTI - Copyright (C) 2013-2014 Mathias Froehlich
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

#ifndef OpenRTI_IntrusiveList_h
#define OpenRTI_IntrusiveList_h

#include <cstddef>
#include <iterator>

#include "Exception.h"

namespace OpenRTI {

template<typename T, unsigned tag = 0>
class IntrusiveList;

template<typename T, unsigned tag = 0>
class _IntrusiveListIterator;

template<unsigned tag = 0>
class _IntrusiveListHook {
public:
  _IntrusiveListHook()
  { _clear(); }
  // Don't copy list membership
  _IntrusiveListHook(const _IntrusiveListHook&)
  { _clear(); }
  // Could be convenient, but collides with constness, rethink!
  // { _insert_unchecked(intrusiveListHook); }
  ~_IntrusiveListHook()
  { _unlink(); }

  // Don't copy list membership
  _IntrusiveListHook& operator=(const _IntrusiveListHook& intrusiveListHook)
  { return *this; }
  // Could be convenient, but collides with constness, rethink!
  // { _unlink(); _insert_unchecked(intrusiveListHook); return *this; }

  void unlink(void)
  {
    _unlink();
    _clear();
  }

  bool is_linked(void) const
  {
    OpenRTIAssert((_next == this) == (_prev == this));
    return _next != this;
  }

private:
  template<typename S, unsigned u>
  friend class IntrusiveList;

  template<typename S, unsigned u>
  friend class _IntrusiveListIterator;

  // Insert this before i
  _IntrusiveListHook* _insert(_IntrusiveListHook& intrusiveListHook)
  { OpenRTIAssert(!is_linked()); return _insert_unchecked(intrusiveListHook); }
  _IntrusiveListHook* _insert_unchecked(_IntrusiveListHook &intrusiveListHook)
  {
    _next = &intrusiveListHook;
    _prev = intrusiveListHook._prev;
    intrusiveListHook._prev->_next = this;
    intrusiveListHook._prev = this;
    return _next;
  }

  _IntrusiveListHook* _unlink_get_next(void)
  { _IntrusiveListHook* next = _next; unlink(); return next; }

  // Only reinitialize our own state, do not care for other list entries
  // possible pointing to this. Use with care!
  void _clear(void)
  { _next = this; _prev = this; }
  // Remove this entry from the list, dont care for our own state
  // possibly pointing to other nodes. Use with care!
  void _unlink(void)
  { _next->_prev = _prev; _prev->_next = _next; }

  _IntrusiveListHook* _prev;
  _IntrusiveListHook* _next;
};

template<typename T, unsigned tag>
class _IntrusiveListIterator :
  public std::iterator<std::bidirectional_iterator_tag, T, std::ptrdiff_t> {
  typedef std::iterator<std::bidirectional_iterator_tag, T, std::ptrdiff_t> _self;
public:
  typedef typename _self::value_type value_type;
  typedef typename _self::difference_type difference_type;
  typedef typename _self::pointer pointer;
  typedef typename _self::reference reference;
  typedef typename _self::iterator_category iterator_category;

  _IntrusiveListIterator() :
    _intrusiveListHook(NULL)
  { }
  template<typename S>
  _IntrusiveListIterator(const _IntrusiveListIterator<S, tag> &intrusiveListIterator) :
    _intrusiveListHook(intrusiveListIterator._intrusiveListHook)
  { }

  template<typename S>
  bool operator==(const _IntrusiveListIterator<S, tag> &i) const
  { return _intrusiveListHook == i._intrusiveListHook; }
  template<typename S>
  bool operator!=(const _IntrusiveListIterator<S, tag> &i) const
  { return _intrusiveListHook != i._intrusiveListHook; }

  _IntrusiveListIterator& operator++()
  { _intrusiveListHook = _intrusiveListHook->_next; return *this; }
  _IntrusiveListIterator& operator--()
  { _intrusiveListHook = _intrusiveListHook->_prev; return *this; }
  _IntrusiveListIterator operator++(int)
  { _IntrusiveListIterator i(*this); ++*this; return i; }
  _IntrusiveListIterator operator--(int)
  { _IntrusiveListIterator i(*this); --*this; return i; }

  reference operator*() const
  { return static_cast<reference>(*_intrusiveListHook); }
  pointer operator->() const
  { return static_cast<pointer>(_intrusiveListHook); }

  // Remove this entry from the list, and reinitialize this list node
  _IntrusiveListIterator unlink(void)
  { return _IntrusiveListIterator(_intrusiveListHook->_unlink_get_next()); }

private:
  template<typename S, unsigned u>
  friend class IntrusiveList;

  template<typename S, unsigned u>
  friend class _IntrusiveListIterator;

  _IntrusiveListIterator(_IntrusiveListHook<tag>* intrusiveListHook) :
    _intrusiveListHook(intrusiveListHook)
  { }

  _IntrusiveListHook<tag> *_intrusiveListHook;
};

template<typename T, unsigned tag>
class IntrusiveList {
public:
  typedef T value_type;
  typedef std::size_t size_type;
  typedef _IntrusiveListHook<tag> hook_type;
  typedef _IntrusiveListHook<tag> Hook;

  typedef _IntrusiveListIterator<T, tag> iterator;
  typedef _IntrusiveListIterator<const T, tag> const_iterator;

  bool empty() const
  { return !_intrusiveListHook.is_linked(); }
  // Truely O(n)!!
  size_type size() const
  {
    size_type size = 0;
    for (const_iterator i = begin(); i != end(); ++i)
      ++size;
    return size;
  }

  iterator begin()
  { return iterator(_intrusiveListHook._next); }
  iterator end()
  { return iterator(&_intrusiveListHook); }

  const_iterator begin() const
  { return const_iterator(_intrusiveListHook._next); }
  const_iterator end() const
  { return const_iterator(const_cast<_IntrusiveListHook<tag>*>(&_intrusiveListHook)); }

  iterator insert(const iterator& i, T& t)
  { return iterator(_select(t)._insert(_select(*i))); }
  iterator erase(const iterator& i)
  { return iterator(i->unlink()); }
  iterator erase(T& t)
  { OpenRTIAssert(_select(t).is_linked()); return iterator(_select(t)._unlink_get_next()); }

  void push_front(T& t)
  { insert(begin(), t); }
  void push_back(T& t)
  { insert(end(), t); }

  void pop_front()
  { erase(begin()); }
  void pop_back()
  { erase(--end()); }

  const T& front() const
  { return *begin(); }
  T& front()
  { return *begin(); }

  const T& back() const
  { return *--end(); }
  T& back()
  { return *--end(); }

private:
  const Hook& _select(const T& t) const
  { return static_cast<Hook&>(t); }
  Hook& _select(T& t) const
  { return static_cast<Hook&>(t); }

  Hook _intrusiveListHook;
};

} // namespace OpenRTI

#endif
