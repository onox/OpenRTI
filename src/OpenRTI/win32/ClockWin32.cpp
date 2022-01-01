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

#include "Clock.h"

#include "Atomic.h"
#include "ClockWin32.h"

#ifndef NOMINMAX
# define NOMINMAX
#endif
#include <windows.h>

// Windows XP provides performance counters
#if 0x0500 <= WINVER
# define HAVE_PERFORMANCE_COUNTER
#endif

namespace OpenRTI {

#if __cplusplus < 201103L

#if defined(HAVE_PERFORMANCE_COUNTER)

static std::pair<uint64_t,uint64_t> getResolution()
{
  LARGE_INTEGER freq;
  freq.QuadPart = 0;
  if (!QueryPerformanceFrequency(&freq))
    return std::pair<uint64_t,uint64_t>(0, 0);

  uint64_t numerator = 1000 * 1000 * 1000;
  uint64_t denominator = freq.QuadPart;

  // Ok, poor men's gcd :), but since the numerator is (2*5)^9, this is all we can have
  while (denominator && (denominator % 5) == 0 && numerator && (numerator % 5) == 0) {
    numerator /= 5;
    denominator /= 5;
  }
  while (denominator && (denominator % 2) == 0 && numerator && (numerator % 2) == 0) {
    numerator /= 2;
    denominator /= 2;
  }

  return std::pair<uint64_t,uint64_t>(numerator, denominator);
}

Clock
Clock::now()
{
  static std::pair<uint64_t,uint64_t> resolution = getResolution();

  if (resolution.second == 0)
    return Clock(static_cast<uint64_t>(GetTickCount())*1000*1000);

  LARGE_INTEGER tick;
  tick.QuadPart = 0;
  QueryPerformanceCounter(&tick);

  return Clock(resolution.first*tick.QuadPart/resolution.second);
}

#else

Clock
Clock::now()
{
  return Clock(static_cast<uint64_t>(GetTickCount())*1000*1000);
}

#endif

void
Clock::sleep_for(const Clock& reltime)
{
  Sleep(ClockWin32::toMsec(reltime.getNSec()));
}

#endif

}
