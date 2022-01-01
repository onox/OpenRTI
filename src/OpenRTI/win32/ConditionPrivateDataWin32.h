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

#ifndef OpenRTI_ConditionPrivateDataWin32_h
#define OpenRTI_ConditionPrivateDataWin32_h

#ifndef NOMINMAX
# define NOMINMAX
#endif
#include <windows.h>
#include <list>

#include "Clock.h"
#include "MutexPrivateDataWin32.h"

namespace OpenRTI {

#if __cplusplus < 201103L

// Windows Server 2008/Vista provides native conditions
#if 0x0600 <= WINVER
// ... but they seem not to work on shutdown!
// # define HAVE_CONDITION_VARIABLE
#endif

#ifdef HAVE_CONDITION_VARIABLE
struct Condition::PrivateData {
  PrivateData(void)
  {
    InitializeConditionVariable(&_condition);
  }
  ~PrivateData(void)
  {
  }

  void notify_one(void)
  {
    WakeConditionVariable(&_condition);
  }

  void notify_all(void)
  {
    WakeAllConditionVariable(&_condition);
  }

  bool wait_for(Mutex::PrivateData& mutexPrivateData, DWORD msec)
  {
    return SleepConditionVariableCS(&_condition, &mutexPrivateData._criticalSection, msec);
  }

  CONDITION_VARIABLE _condition;
};

#else

struct Condition::PrivateData {
  ~PrivateData(void)
  {
    // The waiting pool should be empty anyway
    _mutex.lock();
    while (!_pool.empty()) {
      CloseHandle(_pool.front());
      _pool.pop_front();
    }
    _mutex.unlock();
  }

  void notify_one(void)
  {
    _mutex.lock();
    if (!_waiters.empty())
      SetEvent(_waiters.back());
    _mutex.unlock();
  }

  void notify_all(void)
  {
    _mutex.lock();
    for (std::list<HANDLE>::iterator i = _waiters.begin(); i != _waiters.end(); ++i)
      SetEvent(*i);
    _mutex.unlock();
  }

  bool wait_for(Mutex::PrivateData& externalMutex, DWORD msec)
  {
    _mutex.lock();
    if (_pool.empty())
      _waiters.push_front(CreateEvent(NULL, FALSE, FALSE, NULL));
    else
      _waiters.splice(_waiters.begin(), _pool, _pool.begin());
    std::list<HANDLE>::iterator i = _waiters.begin();
    _mutex.unlock();

    externalMutex.unlock();

    DWORD result = WaitForSingleObject(*i, msec);

    externalMutex.lock();

    _mutex.lock();
    if (result != WAIT_OBJECT_0)
      result = WaitForSingleObject(*i, 0);
    _pool.splice(_pool.begin(), _waiters, i);
    _mutex.unlock();

    return result == WAIT_OBJECT_0;
  }

  // Protect the list of waiters
  Mutex::PrivateData _mutex;

  std::list<HANDLE> _waiters;
  std::list<HANDLE> _pool;
};

#endif
#endif

} // namespace OpenRTI

#endif
