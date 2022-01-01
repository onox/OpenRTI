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

#ifndef OpenRTI_ScopeLock_h
#define OpenRTI_ScopeLock_h

#include "Mutex.h"

namespace OpenRTI {

#if 201103L <= __cplusplus

class ScopeLock : public std::unique_lock<std::mutex> {
public:
  ScopeLock(Mutex& mutex) : std::unique_lock<std::mutex>(mutex._mutex)
  { }
  ~ScopeLock(void)
  { }

private:
  ScopeLock(void);
  ScopeLock(const ScopeLock&);
  ScopeLock& operator=(const ScopeLock&);
};

#else

class ScopeLock {
public:
  ScopeLock(Mutex& mutex) : mMutex(mutex)
  { mMutex.lock(); }
  ~ScopeLock(void)
  { mMutex.unlock(); }

  Mutex* mutex() const
  { return &mMutex; }

private:
  ScopeLock(void);
  ScopeLock(const ScopeLock&);
  ScopeLock& operator=(const ScopeLock&);

  Mutex& mMutex;
};

#endif

} // namespace OpenRTI

#endif
