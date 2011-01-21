/* -*-c++-*- OpenRTI - Copyright (C) 2004-2011 Mathias Froehlich
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

#include "OpenRTIConfig.h"
#include "Clock.h"
#include "ClockPosix.h"

#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "Atomic.h"

namespace OpenRTI {

#if defined(HAVE_PTHREAD_CONDATTR_SETCLOCK)

// Try to find out if we can use the monotonic clock.
// In the end it is available if wave it and if we can use it for the
// condition variable timeout.
// Else just use the realtime clock.
static bool
checkClockId(clockid_t clockid)
{
  struct timespec ts;
  if (0 != clock_gettime(clockid, &ts))
    return false;

  pthread_condattr_t attr;
  if (0 != pthread_condattr_init(&attr))
    return false;
  if (0 != pthread_condattr_setclock(&attr, clockid))
    return false;
  if (0 != pthread_condattr_destroy(&attr))
    return false;

  return true;
}

static clockid_t
getBestClockId()
{
#if defined(CLOCK_MONOTONIC_COARSE)
  // glibc support for that is missing
  if (checkClockId(CLOCK_MONOTONIC_COARSE))
    return CLOCK_MONOTONIC_COARSE;
#endif
#if defined(_POSIX_MONOTONIC_CLOCK)
  if (checkClockId(CLOCK_MONOTONIC))
    return CLOCK_MONOTONIC;
#endif
  // Do not need to try, posix requires this to be accessible
  return CLOCK_REALTIME;
}

static Atomic clockIsInitialized(0);
static clockid_t _clockId;

clockid_t
ClockPosix::getClockId()
{
  if (clockIsInitialized)
    return _clockId;

  _clockId = getBestClockId();
  ++clockIsInitialized;
  return _clockId;
}

#endif

uint64_t ClockPosix::now()
{
#if defined(HAVE_POSIX_TIMERS)
  struct timespec ts;
#if defined(HAVE_PTHREAD_CONDATTR_SETCLOCK)
  clock_gettime(getClockId(), &ts);
#else
  clock_gettime(CLOCK_REALTIME, &ts);
#endif
  return toNSec(ts);
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return toNSec(tv);
#endif
}

Clock
Clock::now()
{
  return Clock(ClockPosix::now());
}

} // namespace OpenRTI
