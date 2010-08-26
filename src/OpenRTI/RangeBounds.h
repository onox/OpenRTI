/* -*-c++-*- OpenRTI - Copyright (C) 2009-2010 Mathias Froehlich
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

#ifndef OpenRTI_RangeBounds_h
#define OpenRTI_RangeBounds_h

#include <limits>
#include "Types.h"

namespace OpenRTI {

class OPENRTI_LOCAL RangeBounds {
public:
  /// Note that the upper bound does not belong to the range.
  /// It is [lower, upper)
  typedef uint32_t value_type;
  RangeBounds() :
    _lower(std::numeric_limits<value_type>::max()),
    _upper(std::numeric_limits<value_type>::min())
  { }
  RangeBounds(const value_type& value) :
    _lower(value),
    _upper(value)
  { }
  RangeBounds(const value_type& lower, const value_type& upper) :
    _lower(lower),
    _upper(upper)
  { }

  bool empty() const
  { return _upper <= _lower; }

  bool whole() const
  { return _lower == std::numeric_limits<value_type>::min() && _upper == std::numeric_limits<value_type>::max(); }

  const value_type& getLower() const
  { return _lower; }
  const value_type& getUpper() const
  { return _upper; }

  value_type getCenter() const
  { return (_upper >> 1) + (_lower >> 1); }

  bool intersects(const RangeBounds& rangeBounds) const
  { return _lower < rangeBounds._upper && rangeBounds._lower < _upper; }

  bool includes(const RangeBounds& rangeBounds) const
  {
    if (empty())
      return false;
    return _lower <= rangeBounds._lower && rangeBounds._upper <= _upper;
  }

  void extend(const RangeBounds& rangeBounds)
  {
    _lower = std::min(_lower, rangeBounds._lower);
    _upper = std::max(_upper, rangeBounds._upper);
  }

  RangeBounds& swap(RangeBounds& rangeBounds)
  { std::swap(_lower, rangeBounds._lower); std::swap(_upper, rangeBounds._upper); return *this; }

  // make that available as keys in maps and items in sets
  bool operator==(const RangeBounds& rangeBounds) const
  {
    if (_lower != rangeBounds._lower) return false;
    if (_upper != rangeBounds._upper) return false;
    return true;
  }
  bool operator<(const RangeBounds& rangeBounds) const
  {
    if (_lower < rangeBounds._lower) return true;
    if (rangeBounds._lower < _lower) return false;
    if (_upper < rangeBounds._upper) return true;
    if (rangeBounds._upper < _upper) return false;
    return false;
  }
  bool operator!=(const RangeBounds& rangeBounds) const
  { return !operator==(rangeBounds); }
  bool operator>(const RangeBounds& rangeBounds) const
  { return rangeBounds.operator<(*this); }
  bool operator>=(const RangeBounds& rangeBounds) const
  { return !operator<(rangeBounds); }
  bool operator<=(const RangeBounds& rangeBounds) const
  { return !operator>(rangeBounds); }

private:
  value_type _lower;
  value_type _upper;
};

} // namespace OpenRTI

#endif
