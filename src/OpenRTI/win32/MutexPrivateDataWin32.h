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

#ifndef MutexPrivateDataWin32_h
#define MutexPrivateDataWin32_h

#include "Mutex.h"
#ifndef NOMINMAX
# define NOMINMAX
#endif
#include <windows.h>

namespace OpenRTI {

#if __cplusplus < 201103L
struct Mutex::PrivateData {
  PrivateData()
  {
    InitializeCriticalSection((LPCRITICAL_SECTION)&_criticalSection);
  }

  ~PrivateData()
  {
    DeleteCriticalSection((LPCRITICAL_SECTION)&_criticalSection);
  }

  void lock(void)
  {
    EnterCriticalSection((LPCRITICAL_SECTION)&_criticalSection);
  }

  void unlock(void)
  {
    LeaveCriticalSection((LPCRITICAL_SECTION)&_criticalSection);
  }

  CRITICAL_SECTION _criticalSection;
};
#endif

} // namespace OpenRTI

#endif
