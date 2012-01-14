/* -*-c++-*- OpenRTI - Copyright (C) 2004-2012 Mathias Froehlich
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

#include <winsock2.h>
#include <windows.h>

namespace OpenRTI {

struct OPENRTI_LOCAL ClockWin32 {

  static struct timeval toTimeval(const uint64_t& nsec)
  {
    struct timeval tv;
    uint64_t usec = (nsec + 500) / 1000;
    uint64_t frac = usec % 1000000;
    tv.tv_usec = (long)frac;
    tv.tv_sec = (long)((usec - frac) / 1000000);
    return tv;
  }
  
  static DWORD toMsec(const uint64_t& nsec)
  {
    if (nsec == (~uint64_t(0)))
      return INFINITE;
    DWORD msec = (DWORD)((nsec + 500000)/1000000);
    if (msec == 0)
      return 1;
    return msec;
  }
};

} // namespace OpenRTI

#endif
