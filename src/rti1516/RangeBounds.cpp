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

#include <RTI/RangeBounds.h>

#include <limits>

namespace rti1516
{

RangeBounds::RangeBounds() :
  _lowerBound(std::numeric_limits<unsigned long>::max()),
  _upperBound(std::numeric_limits<unsigned long>::min())
{
}

RangeBounds::RangeBounds(unsigned long lowerBound,
                         unsigned long upperBound) :
  _lowerBound(lowerBound),
  _upperBound(upperBound)
{
}

RangeBounds::~RangeBounds()
  RTI_NOEXCEPT
{
}

RangeBounds::RangeBounds(RangeBounds const & rhs) :
  _lowerBound(rhs._lowerBound),
  _upperBound(rhs._upperBound)
{
}

RangeBounds &
RangeBounds::operator=(RangeBounds const & rhs)
{
  _lowerBound = rhs._lowerBound;
  _upperBound = rhs._upperBound;
  return *this;
}

unsigned long
RangeBounds::getLowerBound() const
{
  return _lowerBound;
}

unsigned long
RangeBounds::getUpperBound() const
{
  return _upperBound;
}

void 
RangeBounds::setLowerBound(unsigned long lowerBound)
{
  _lowerBound = lowerBound;
}

void
RangeBounds::setUpperBound(unsigned long upperBound)
{
  _upperBound = upperBound;
}

}
