/* -*-c++-*- OpenRTI - Copyright (C) 2004-2022 Mathias Froehlich
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

#ifndef OpenRTI_ClockWin32_h
#define OpenRTI_ClockWin32_h

#ifndef NOMINMAX
# define NOMINMAX
#endif
#include <winsock2.h>
#include <windows.h>

#include <limits>

namespace OpenRTI {

struct OPENRTI_LOCAL ClockWin32 {

  static struct timeval toTimeval(const uint64_t& nsec)
  {
    struct timeval tv;
    if (std::numeric_limits<uint64_t>::max() - 500 <= nsec) {
      tv.tv_usec = 999999;
      tv.tv_sec = std::numeric_limits<long>::max();
    } else {
      uint64_t usec = (nsec + 500) / 1000;
      if (uint64_t(std::numeric_limits<long>::max()) < usec) {
        tv.tv_usec = 999999;
        tv.tv_sec = std::numeric_limits<long>::max();
      } else {
        uint64_t frac = usec % 1000000;
        tv.tv_usec = (long)frac;
        tv.tv_sec = (long)((usec - frac) / 1000000);
      }
    }
    return tv;
  }
  
  static DWORD toMsec(const uint64_t& nsec)
  {
    if (std::numeric_limits<uint64_t>::max() - 500000 <= nsec)
      return INFINITE;
    uint64_t msec = (nsec + 500000)/1000000;
    if (uint64_t(std::numeric_limits<DWORD>::max()) <= msec)
      return INFINITE;
    if (msec == 0)
      return 1;
    return (DWORD)msec;
  }
};

} // namespace OpenRTI

#endif
