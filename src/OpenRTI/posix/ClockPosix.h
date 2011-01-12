/* -*-c++-*- OpenRTI - Copyright (C) 2004-2010 Mathias Froehlich
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

#ifndef OpenRTI_ClockPosix_h
#define OpenRTI_ClockPosix_h

#include <time.h>
#include <algorithm>
#include <cmath>

#include "Clock.h"
#include "Export.h"
#include "Types.h"

#if defined(_POSIX_TIMERS) && 0 < _POSIX_TIMERS
#define HAVE_POSIX_TIMERS
#endif

#if defined(_POSIX_CLOCK_SELECTION) && 0 < _POSIX_CLOCK_SELECTION
#define HAVE_PTHREAD_CONDATTR_SETCLOCK
#endif

namespace OpenRTI {

struct OPENRTI_LOCAL ClockPosix {
  static struct timespec toTimespec(const uint64_t& nsec)
  {
    struct timespec ts;
    ts.tv_nsec = nsec % 1000000000;
    ts.tv_sec = (nsec - ts.tv_nsec) / 1000000000;
    return ts;
  }
  static uint64_t toNSec(const struct timespec& ts)
  { return uint64_t(ts.tv_nsec) + uint64_t(ts.tv_sec) * 1000000000; }
  static struct timeval toTimeval(const uint64_t& nsec)
  {
    struct timeval tv;
    uint64_t usec = (nsec + 500) / 1000;
    uint64_t frac = usec % 1000000;
    tv.tv_usec = frac;
    tv.tv_sec = (usec - frac) / 1000000;
    return tv;
  }
  static uint64_t toNSec(const struct timeval& tv)
  { return uint64_t(tv.tv_usec)*1000 + uint64_t(tv.tv_sec) * 1000000000; }

  static uint64_t now();

#if defined(HAVE_PTHREAD_CONDATTR_SETCLOCK)
  /// This should return the same than the _clock variable below.
  /// The difference is that this should be independent of the ordering of static data initializers.
  /// This function just crude tests for the required features.
  static clockid_t getClockId();
#endif
};

} // namespace OpenRTI

#endif
