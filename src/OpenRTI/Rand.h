/* -*-c++-*- OpenRTI - Copyright (C) 2011-2022 Mathias Froehlich
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

#ifndef OpenRTI_Rand_h
#define OpenRTI_Rand_h

#include "Export.h"
#include "Types.h"

namespace OpenRTI {

class OPENRTI_LOCAL Rand {
public:
  Rand(uint32_t w = 1, uint32_t z = 2) :
    _w(w), _z(z)
  { }

  uint32_t get()
  {
    _z = 36969 * (_z & 65535) + (_z >> 16);
    _w = 18000 * (_w & 65535) + (_w >> 16);
    return (_z << 16) + _w;
  }

private:
  uint32_t _w;
  uint32_t _z;
};

} // namespace OpenRTI

#endif
