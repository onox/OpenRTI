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

#ifndef MutexPrivateDataPosix_h
#define MutexPrivateDataPosix_h

#include "Mutex.h"

#include "Exception.h"
#include "Export.h"
#include "ClockPosix.h"

#include <pthread.h>

namespace OpenRTI {

#if __cplusplus < 201103L

struct OPENRTI_LOCAL Mutex::PrivateData {
  PrivateData()
  {
    int err = pthread_mutex_init(&_mutex, 0);
    if (err != 0)
      throw ResourceError("Could not initialize mutex!");
  }

  ~PrivateData()
  {
    int err = pthread_mutex_destroy(&_mutex);
    if (err != 0)
      throw RTIinternalError("Error destroying mutex!");
  }

  void lock(void)
  {
    int err = pthread_mutex_lock(&_mutex);
    if (err != 0)
      throw RTIinternalError("Error locking mutex!");
  }

  void unlock(void)
  {
    int err = pthread_mutex_unlock(&_mutex);
    if (err != 0)
      throw RTIinternalError("Error unlocking mutex!");
  }

  pthread_mutex_t _mutex;
};

#endif

} // namespace OpenRTI

#endif
