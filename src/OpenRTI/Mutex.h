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

#ifndef OpenRTI_Mutex_h
#define OpenRTI_Mutex_h

#include "Export.h"

#if 201103L <= __cplusplus
# include <mutex>
#endif

namespace OpenRTI {

class Condition;
class ScopeLock;

class OPENRTI_API Mutex {
public:
#if 201103L <= __cplusplus
  Mutex(void)
  { }
#else
  Mutex(void);
#endif
#if 201103L <= __cplusplus
  ~Mutex(void)
  { }
#else
  ~Mutex(void);
#endif

#if 201103L <= __cplusplus
  void lock(void)
  { _mutex.lock(); }
#else
  void lock(void);
#endif
#if 201103L <= __cplusplus
  void unlock(void)
  { _mutex.unlock(); }
#else
  void unlock(void);
#endif

private:
  Mutex(const Mutex&);
  Mutex& operator=(const Mutex&);

#if 201103L <= __cplusplus
  std::mutex _mutex;
#else
  struct PrivateData;
  PrivateData* _privateData;
#endif

  friend class Condition;
  friend class ScopeLock;
};

} // namespace OpenRTI

#endif
