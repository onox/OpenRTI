/* -*-c++-*- OpenRTI - Copyright (C) 2004-2015 Mathias Froehlich
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

#ifndef OpenRTI_Condition_h
#define OpenRTI_Condition_h

#include "Export.h"
#include "ScopeLock.h"
#if 201103L <= __cplusplus
# include <condition_variable>
# include "Clock.h"
# include "Mutex.h"
#endif

namespace OpenRTI {

class Clock;
class ScopeLock;

class OPENRTI_API Condition {
public:
#if 201103L <= __cplusplus
  Condition(void)
  { }
#else
  Condition(void);
#endif
#if 201103L <= __cplusplus
  ~Condition(void)
  { }
#else
  ~Condition(void);
#endif

#if 201103L <= __cplusplus
  void notify_one(void)
  { _condition.notify_one(); }
#else
  void notify_one(void);
#endif
#if 201103L <= __cplusplus
  void notify_all(void)
  { _condition.notify_all(); }
#else
  void notify_all(void);
#endif

#if 201103L <= __cplusplus
  void wait(ScopeLock& scopeLock)
  { _condition.wait(scopeLock); }
#else
  void wait(ScopeLock& scopeLock);
#endif
#if 201103L <= __cplusplus
  bool wait_until(ScopeLock& scopeLock, const Clock& timeout)
  {
    std::chrono::steady_clock::time_point tp(std::chrono::nanoseconds(timeout.getNSec()));
    return std::cv_status::timeout != _condition.wait_until(scopeLock, tp);
  }
#else
  bool wait_until(ScopeLock& scopeLock, const Clock& abstime);
#endif

private:
  Condition(const Condition&);
  Condition& operator=(const Condition&);

#if 201103L <= __cplusplus
  std::condition_variable _condition;
#else
  struct PrivateData;
  PrivateData* _privateData;
#endif
};

} // namespace OpenRTI

#endif
