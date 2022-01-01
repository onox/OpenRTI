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

#ifndef OpenRTI_IntrusiveList_h
#define OpenRTI_IntrusiveList_h

#include <cstddef>
#include <iterator>

#include "Exception.h"

namespace OpenRTI {

template<typename T, typename Tag>
class _IntrusiveList;

template<typename T, typename Tag>
class _IntrusiveListIterator;

template<typename Tag>
class OPENRTI_LOCAL _IntrusiveListHook {
public:
  _IntrusiveListHook()
  { _clear(); }
  // Don't copy list membership
  _IntrusiveListHook(const _IntrusiveListHook& intrusiveListHook)
  { _clear(); OpenRTIAssert(!intrusiveListHook.is_linked()); }
  ~_IntrusiveListHook()
  { _unlink(); }

  // Don't copy list membership
  _IntrusiveListHook& operator=(const _IntrusiveListHook& intrusiveListHook)
  { OpenRTIAssert(!intrusiveListHook.is_linked()); return *this; }

  bool is_linked(void) const
  {
    OpenRTIAssert((_next == this) == (_prev == this));
    return _next != this;
  }

private:
  template<typename S, typename U>
  friend class _IntrusiveList;

  template<typename S, typename U>
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
    return this;
  }

  // Only reinitialize our own state, do not care for other list entries
  // possible pointing to this. Use with care!
  void _clear(void)
  { _next = this; _prev = this; }
  // Remove this entry from the list, dont care for our own state
  // possibly pointing to other nodes. Use with care!
  void _unlink(void)
  { _next->_prev = _prev; _prev->_next = _next; }

  /// Remove this entry from the list and reinitializes this list hook.
  void _unlink_clear(void)
  {
    _unlink();
    _clear();
  }

  void _swap(_IntrusiveListHook& intrusiveListHook)
  {
    _IntrusiveListHook* this_prev = _prev;
    _IntrusiveListHook* this_next = _next;
    _IntrusiveListHook* that_prev = intrusiveListHook._prev;
    _IntrusiveListHook* that_next = intrusiveListHook._next;

    // Keep these two swaps before the next two.
    std::swap(this_next->_prev, that_next->_prev);
    std::swap(this_prev->_next, that_prev->_next);

    std::swap(_next, intrusiveListHook._next);
    std::swap(_prev, intrusiveListHook._prev);
  }

  _IntrusiveListHook* _prev;
  _IntrusiveListHook* _next;
};

template<typename T, typename Tag>
class OPENRTI_LOCAL _IntrusiveListIterator :
  public std::iterator<std::bidirectional_iterator_tag, T, std::ptrdiff_t> {
  typedef std::iterator<std::bidirectional_iterator_tag, T, std::ptrdiff_t> _Base;
public:
  typedef typename _Base::value_type value_type;
  typedef typename _Base::difference_type difference_type;
  typedef typename _Base::pointer pointer;
  typedef typename _Base::reference reference;
  typedef typename _Base::iterator_category iterator_category;

  _IntrusiveListIterator() :
    _intrusiveListHook(NULL)
  { }
  template<typename S>
  _IntrusiveListIterator(const _IntrusiveListIterator<S, Tag> &intrusiveListIterator) :
    _intrusiveListHook(intrusiveListIterator._intrusiveListHook)
  { }

  template<typename S>
  bool operator==(const _IntrusiveListIterator<S, Tag> &i) const
  { return _intrusiveListHook == i._intrusiveListHook; }
  template<typename S>
  bool operator!=(const _IntrusiveListIterator<S, Tag> &i) const
  { return _intrusiveListHook != i._intrusiveListHook; }

  reference operator*() const
  { return static_cast<reference>(*_intrusiveListHook); }
  pointer operator->() const
  { return static_cast<pointer>(_intrusiveListHook); }

  pointer get() const
  { return static_cast<pointer>(_intrusiveListHook); }

protected:
  _IntrusiveListIterator(_IntrusiveListHook<Tag>* intrusiveListHook) :
    _intrusiveListHook(intrusiveListHook)
  { }

  void _unlink(void)
  { return _intrusiveListHook->_unlink_clear(); }

  void _increment()
  { _intrusiveListHook = _intrusiveListHook->_next; }
  void _decrement()
  { _intrusiveListHook = _intrusiveListHook->_prev; }

private:
  template<typename S, typename U>
  friend class _IntrusiveList;

  template<typename S, typename U>
  friend class _IntrusiveListIterator;

  _IntrusiveListHook<Tag> *_intrusiveListHook;
};

template<typename T, typename Tag>
class OPENRTI_LOCAL _ForwardIntrusiveListIterator : public _IntrusiveListIterator<T, Tag> {
  typedef _IntrusiveListIterator<T, Tag> _Base;
public:
  typedef typename _Base::value_type value_type;
  typedef typename _Base::difference_type difference_type;
  typedef typename _Base::pointer pointer;
  typedef typename _Base::reference reference;
  typedef typename _Base::iterator_category iterator_category;

  _ForwardIntrusiveListIterator()
  { }
  template<typename S>
  _ForwardIntrusiveListIterator(const _ForwardIntrusiveListIterator<S, Tag> &intrusiveListIterator) :
    _IntrusiveListIterator<T, Tag>(intrusiveListIterator)
  { }

  _ForwardIntrusiveListIterator& operator++()
  { _IntrusiveListIterator<T, Tag>::_increment(); return *this; }
  _ForwardIntrusiveListIterator& operator--()
  { _IntrusiveListIterator<T, Tag>::_decrement(); return *this; }
  _ForwardIntrusiveListIterator operator++(int)
  { _ForwardIntrusiveListIterator i(*this); _IntrusiveListIterator<T, Tag>::_increment(); return i; }
  _ForwardIntrusiveListIterator operator--(int)
  { _ForwardIntrusiveListIterator i(*this); _IntrusiveListIterator<T, Tag>::_decrement(); return i; }

  // convenience methods not required by the bidirectional iterator
  template<typename D>
  _ForwardIntrusiveListIterator& operator +=(D n)
  { std::advance(*this, n); return *this; }
  template<typename D>
  _ForwardIntrusiveListIterator& operator -=(D n)
  { std::advance(*this, -n); return *this; }
  template<typename D>
  _ForwardIntrusiveListIterator operator+(D n) const
  { return _ForwardIntrusiveListIterator(*this) += n; }
  template<typename D>
  _ForwardIntrusiveListIterator operator-(D n) const
  { return _ForwardIntrusiveListIterator(*this) -= n; }

  // Remove this entry from the list, and reinitialize this list node
  // _ForwardIntrusiveListIterator unlink(void)
  // { return _ForwardIntrusiveListIterator(_intrusiveListHook->_unlink_get_next()); }

private:
  template<typename S, typename U>
  friend class _IntrusiveList;

  template<typename S, typename U>
  friend class _IntrusiveListIterator;

  _ForwardIntrusiveListIterator(_IntrusiveListHook<Tag>* intrusiveListHook) :
    _IntrusiveListIterator<T, Tag>(intrusiveListHook)
  { }
};

template<typename T, typename Tag>
class OPENRTI_LOCAL _ReverseIntrusiveListIterator : public _IntrusiveListIterator<T, Tag> {
  typedef _IntrusiveListIterator<T, Tag> _Base;
public:
  typedef typename _Base::value_type value_type;
  typedef typename _Base::difference_type difference_type;
  typedef typename _Base::pointer pointer;
  typedef typename _Base::reference reference;
  typedef typename _Base::iterator_category iterator_category;

  _ReverseIntrusiveListIterator()
  { }
  template<typename S>
  _ReverseIntrusiveListIterator(const _ReverseIntrusiveListIterator<S, Tag> &intrusiveListIterator) :
    _IntrusiveListIterator<T, Tag>(intrusiveListIterator)
  { }

  _ReverseIntrusiveListIterator& operator++()
  { _IntrusiveListIterator<T, Tag>::_decrement(); return *this; }
  _ReverseIntrusiveListIterator& operator--()
  { _IntrusiveListIterator<T, Tag>::_increment(); return *this; }
  _ReverseIntrusiveListIterator operator++(int)
  { _ReverseIntrusiveListIterator i(*this); _IntrusiveListIterator<T, Tag>::_decrement(); return i; }
  _ReverseIntrusiveListIterator operator--(int)
  { _ReverseIntrusiveListIterator i(*this); _IntrusiveListIterator<T, Tag>::_increment(); return i; }

  // convenience methods not required by the bidirectional iterator
  template<typename D>
  _ReverseIntrusiveListIterator& operator +=(D n)
  { std::advance(*this, n); return *this; }
  template<typename D>
  _ReverseIntrusiveListIterator& operator -=(D n)
  { std::advance(*this, -n); return *this; }
  template<typename D>
  _ReverseIntrusiveListIterator operator+(D n) const
  { return _ReverseIntrusiveListIterator(*this) += n; }
  template<typename D>
  _ReverseIntrusiveListIterator operator-(D n) const
  { return _ReverseIntrusiveListIterator(*this) -= n; }

  // Remove this entry from the list, and reinitialize this list node
  // _ReverseIntrusiveListIterator unlink(void)
  // { return _ReverseIntrusiveListIterator(_intrusiveListHook->_unlink_get_next()); }

private:
  template<typename S, typename U>
  friend class _IntrusiveList;

  template<typename S, typename U>
  friend class _IntrusiveListIterator;

  _ReverseIntrusiveListIterator(_IntrusiveListHook<Tag>* intrusiveListHook) :
    _IntrusiveListIterator<T, Tag>(intrusiveListHook)
  { }
};

template<typename T, typename Tag>
class OPENRTI_LOCAL _IntrusiveList {
public:
  typedef T value_type;
  typedef std::size_t size_type;
  typedef T& reference;
  typedef T const& const_reference;
  typedef T* pointer;
  typedef T const* const_pointer;
  typedef _IntrusiveListHook<Tag> hook_type;
  typedef _IntrusiveListHook<Tag> Hook;

  typedef _ForwardIntrusiveListIterator<T, Tag> iterator;
  typedef _ForwardIntrusiveListIterator<const T, Tag> const_iterator;

  typedef _ReverseIntrusiveListIterator<T, Tag> reverse_iterator;
  typedef _ReverseIntrusiveListIterator<const T, Tag> const_reverse_iterator;

  _IntrusiveList()
  { }
  ~_IntrusiveList()
  { OpenRTIAssert(empty()); }

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
  /// returns true if the map contains at most one element
  bool single() const
  { return ++begin() == end(); }
  /// returns true if the map contains exactly one element
  bool unique() const
  { return !empty() && single(); }

  iterator begin()
  { return iterator(_intrusiveListHook._next); }
  iterator end()
  { return iterator(&_intrusiveListHook); }

  const_iterator begin() const
  { return const_iterator(_intrusiveListHook._next); }
  const_iterator end() const
  { return const_iterator(const_cast<Hook*>(&_intrusiveListHook)); }

  reverse_iterator rbegin()
  { return reverse_iterator(_intrusiveListHook._prev); }
  reverse_iterator rend()
  { return reverse_iterator(&_intrusiveListHook); }

  const_reverse_iterator rbegin() const
  { return const_reverse_iterator(_intrusiveListHook._prev); }
  const_reverse_iterator rend() const
  { return const_reverse_iterator(const_cast<Hook*>(&_intrusiveListHook)); }

  iterator insert(const iterator& i, reference t)
  { return iterator(_select(t)._insert(_select(*i))); }

  void push_front(reference t)
  { insert(begin(), t); }
  void push_back(reference t)
  { insert(end(), t); }

  /// Remove from the list and delete the entry
  iterator erase(iterator i)
  { OpenRTIAssert(!empty()); OpenRTIAssert(_select(*i).is_linked()); delete i++.get(); return i; }
  reverse_iterator erase(reverse_iterator i)
  { OpenRTIAssert(!empty()); OpenRTIAssert(_select(*i).is_linked()); delete i++.get(); return i; }
  static void erase(reference t)
  { delete &t; }

  void clear()
  {
    iterator i = begin();
    while (i != end())
      i = erase(i);
  }

  /// Removes the front/back entry of this list from the list and deletes the entry.
  void pop_front()
  { OpenRTIAssert(!empty()); erase(begin()); }
  void pop_back()
  { OpenRTIAssert(!empty()); erase(rbegin()); }

  /// Unlinks the front/back entry of this list from the list. Does not delete the entry.
  iterator unlink(iterator i)
  { OpenRTIAssert(!empty()); OpenRTIAssert(_select(*i).is_linked()); _select(*i++)._unlink_clear(); return i; }
  reverse_iterator unlink(reverse_iterator i)
  { OpenRTIAssert(!empty()); OpenRTIAssert(_select(*i).is_linked()); _select(*i++)._unlink_clear(); return i; }
  static void unlink(reference t)
  { _select(t)._unlink_clear(); }

  void unlink()
  {
    iterator i = begin();
    while (i != end())
      i = unlink(i);
  }

  /// Unlinks the front/back entry of this list from the list. Does not delete the entry.
  void unlink_front()
  { OpenRTIAssert(!empty()); unlink(begin()); }
  void unlink_back()
  { OpenRTIAssert(!empty()); unlink(rbegin()); }

  /// Access to the first entry in the list
  const_reference front() const
  { OpenRTIAssert(!empty()); return *begin(); }
  reference front()
  { OpenRTIAssert(!empty()); return *begin(); }

  /// Access to the last entry in the list
  const_reference back() const
  { OpenRTIAssert(!empty()); return *rbegin(); }
  reference back()
  { OpenRTIAssert(!empty()); return *rbegin(); }

  void swap(_IntrusiveList& intrusiveList)
  { _intrusiveListHook._swap(intrusiveList._intrusiveListHook); }

  static iterator it(reference t)
  { OpenRTIAssert(_select(t).is_linked()); return iterator(&_select(t)); }
  static const_iterator it(const_reference t)
  { OpenRTIAssert(_select(t).is_linked()); return const_iterator(const_cast<Hook*>(&_select(t))); }

private:
  // _IntrusiveList(const _IntrusiveList&);
  // _IntrusiveList& operator=(const _IntrusiveList&);

  static const Hook& _select(const_reference t)
  { return static_cast<const Hook&>(t); }
  static Hook& _select(reference t)
  { return static_cast<Hook&>(t); }

  Hook _intrusiveListHook;
};

template<typename T, unsigned n>
class OPENRTI_LOCAL _ListTag {};

template<typename T, unsigned tag = 0>
class OPENRTI_LOCAL IntrusiveList : public _IntrusiveList<T, _ListTag<T, tag> > {
  typedef _IntrusiveList<T, _ListTag<T, tag> > _Implementation;
public:
  IntrusiveList()
  { }
  IntrusiveList(const IntrusiveList& intrusiveList)
  { OpenRTIAssert(intrusiveList.empty()); }
#if 201103L <= __cplusplus || 200610L <= __cpp_rvalue_reference
  IntrusiveList(IntrusiveList&& intrusiveList)
  { _Implementation::swap(intrusiveList); }
#endif
  ~IntrusiveList()
  { OpenRTIAssert(_Implementation::empty()); }

  IntrusiveList& operator=(const IntrusiveList& intrusiveList)
  { OpenRTIAssert(_Implementation::empty()); OpenRTIAssert(intrusiveList.empty()); return *this; }
#if 201103L <= __cplusplus || 200610L <= __cpp_rvalue_reference
  IntrusiveList& operator=(IntrusiveList&& intrusiveList)
  { _Implementation::swap(intrusiveList); return *this; }
#endif
};

} // namespace OpenRTI

#endif
