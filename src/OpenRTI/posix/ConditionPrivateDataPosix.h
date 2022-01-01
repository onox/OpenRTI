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

#ifndef OpenRTI_ConditionPrivateDataPosix_h
#define OpenRTI_ConditionPrivateDataPosix_h

#include <pthread.h>
#include <cerrno>

#include "Clock.h"
#include "Export.h"
#include "Mutex.h"
#include "ClockPosix.h"
#include "MutexPrivateDataPosix.h"

namespace OpenRTI {

#if __cplusplus < 201103L

struct OPENRTI_LOCAL Condition::PrivateData {
  PrivateData(void)
  {
#if defined(HAVE_PTHREAD_CONDATTR_SETCLOCK)
    pthread_condattr_t attr;
    int err = pthread_condattr_init(&attr);
    if (err != 0)
      throw ResourceError("Could not initialize Condition!");
    err = pthread_condattr_setclock(&attr, ClockPosix::getClockId());
    if (err != 0)
      throw ResourceError("Could not initialize Condition!");
    err = pthread_cond_init(&_condition, &attr);
    if (err != 0)
      throw ResourceError("Could not initialize Condition!");
    err = pthread_condattr_destroy(&attr);
    if (err != 0)
      throw ResourceError("Could not initialize Condition!");
#else
    int err = pthread_cond_init(&_condition, NULL);
    if (err != 0)
      throw ResourceError("Could not initialize Condition!");
#endif
  }
  ~PrivateData(void)
  {
    int err = pthread_cond_destroy(&_condition);
    if (err != 0)
      throw ResourceError("Could not destroy Condition!");
  }

  void notify_one(void)
  {
    int err = pthread_cond_signal(&_condition);
    if (err != 0)
      throw ResourceError("Could not signal Condition!");
  }

  void notify_all(void)
  {
    int err = pthread_cond_broadcast(&_condition);
    if (err != 0)
      throw ResourceError("Could not broadcast Condition!");
  }

  void wait(pthread_mutex_t& mutex)
  {
    int err = pthread_cond_wait(&_condition, &mutex);
    if (err != 0)
      throw ResourceError("Error waiting for Condition!");
  }

  bool wait_until(pthread_mutex_t& mutex, const Clock& absclock)
  {
    struct timespec abstime = ClockPosix::toTimespec(absclock.getNSec());
    int evalue = pthread_cond_timedwait(&_condition, &mutex, &abstime);
    if (evalue == 0)
      return true;
    if (evalue != ETIMEDOUT)
      throw ResourceError("Error waiting for Condition!");
    return false;
  }

  pthread_cond_t _condition;
};

#endif

} // namespace OpenRTI

#endif
