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

#ifndef OpenRTI_Buffer_h
#define OpenRTI_Buffer_h

#include "VariableLengthData.h"

namespace OpenRTI {

class OPENRTI_LOCAL Buffer : public VariableLengthDataList {
  template<typename list_iterator>
  class _byte_iterator {
  public:
    _byte_iterator() :
      _offset(~size_t(0))
    { }
    _byte_iterator(const list_iterator& i) :
      _listIterator(i),
      _offset(0)
    { }
    template<typename T>
    _byte_iterator(const _byte_iterator<T>& i) :
      _listIterator(i._listIterator),
      _offset(i._offset)
    { }

    size_t offset() const
    { return _offset; }
    const list_iterator& iterator() const
    { return _listIterator; }

    template<typename T>
    size_t chunk_size(const _byte_iterator<T>& end) const
    {
      if (_listIterator != end._listIterator)
        return _listIterator->size() - _offset;
      OpenRTIAssert(_offset <= end._offset);
      return end._offset - _offset;
    }

    void skip_empty_chunks(const _byte_iterator& end)
    {
      if (_offset)
        return;
      while (_listIterator != end._listIterator && _listIterator->size() == 0)
        ++_listIterator;
    }

    template<typename T>
    bool operator==(const _byte_iterator<T>& i) const
    {
      if (_offset != i._offset)
        return false;
#if !defined(__SUNPRO_CC)
      return _listIterator == i._listIterator;
#else
      return VariableLengthDataList::const_iterator(_listIterator) == VariableLengthDataList::const_iterator(i._listIterator);
#endif
    }
    template<typename T>
    bool operator!=(const _byte_iterator<T>& i) const
    { return !operator==(i); }

    bool operator==(const list_iterator& i) const
    { return _offset == 0 && _listIterator == i; }
    bool operator!=(const list_iterator& i) const
    { return _offset != 0 || _listIterator != i; }

    _byte_iterator& operator++()
    { return _increment(1); }
    _byte_iterator& operator--()
    { return _decrement(1); }

    _byte_iterator operator++(int)
    { _byte_iterator self = *this; _increment(1); return self; }
    _byte_iterator operator--(int)
    { _byte_iterator self = *this; _decrement(1); return self; }

    _byte_iterator& operator+=(ssize_t offset)
    {
      if (0 < offset) {
        return _increment(offset);
      } else if (offset < 0) {
        return _decrement(-offset);
      } else {
        return *this;
      }
    }
    _byte_iterator& operator-=(ssize_t offset)
    {
      if (0 < offset) {
        return _decrement(offset);
      } else if (offset < 0) {
        return _increment(-offset);
      } else {
        return *this;
      }
    }

    _byte_iterator operator+(ssize_t offset) const
    { return _byte_iterator(*this) += offset; }
    _byte_iterator operator-(ssize_t offset) const
    { return _byte_iterator(*this) -= offset; }

    // Sigh, a strange access problem on aCC and early gcc, just disable that control here
#if !defined(__hpux) && \
    !defined(__SUNPRO_CC) && \
    !(defined(__GNUC__) && (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ <= 1)))
  protected:
#endif
    _byte_iterator& _increment(size_t offset)
    {
      while (offset) {
        size_t size = _listIterator->size();
        OpenRTIAssert(_offset < size);
        size -= _offset;
        if (size <= offset) {
          ++_listIterator;
          _offset = 0;
          offset -= size;
        } else {
          _offset += offset;
          break;
        }
      }
      // OpenRTIAssert(_offset < _listIterator->size())
      return *this;
    }
    _byte_iterator& _decrement(size_t offset)
    {
      while (offset) {
        if (offset <= _offset) {
          _offset -= offset;
          break;
        } else {
          offset -= _offset;
          --_listIterator;
          _offset = _listIterator->size();
        }
      }
      return *this;
    }

    list_iterator _listIterator;
    size_t _offset;

    template<typename T>
    friend class _byte_iterator;
  };

public:

  class byte_iterator : public _byte_iterator<VariableLengthDataList::iterator> {
  public:
    byte_iterator()
    { }
    byte_iterator(const byte_iterator& i) :
      _byte_iterator<VariableLengthDataList::iterator>(i)
    { }
    byte_iterator(const VariableLengthDataList::iterator& i) :
      _byte_iterator<VariableLengthDataList::iterator>(i)
    { }
    void* data() const
    { return _listIterator->data(_offset); }
  };

  class const_byte_iterator : public _byte_iterator<VariableLengthDataList::const_iterator> {
  public:
    const_byte_iterator()
    { }
    const_byte_iterator(const const_byte_iterator& i) :
      _byte_iterator<VariableLengthDataList::const_iterator>(i)
    { }
    const_byte_iterator(const byte_iterator& i) :
      _byte_iterator<VariableLengthDataList::const_iterator>(i)
    { }
    const_byte_iterator(const VariableLengthDataList::const_iterator& i) :
      _byte_iterator<VariableLengthDataList::const_iterator>(i)
    { }
    const_byte_iterator(const VariableLengthDataList::iterator& i) :
      _byte_iterator<VariableLengthDataList::const_iterator>(i)
    { }
    const void* data() const
    { return _listIterator->data(_offset); }
  };

  byte_iterator byte_begin()
  { return begin(); }
  byte_iterator byte_end()
  { return end(); }

  const_byte_iterator byte_begin() const
  { return begin(); }
  const_byte_iterator byte_end() const
  { return end(); }
};

typedef std::pair<Buffer::byte_iterator, Buffer::byte_iterator> BufferRange;
typedef std::pair<Buffer::const_byte_iterator, Buffer::const_byte_iterator> ConstBufferRange;

} // namespace OpenRTI

#endif
