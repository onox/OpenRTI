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

#include "Condition.h"

#include "Clock.h"
#include "ClockWin32.h"
#include "ScopeLock.h"
#include "MutexPrivateDataWin32.h"
#include "ConditionPrivateDataWin32.h"

namespace OpenRTI {

#if __cplusplus < 201103L

Condition::Condition(void) :
  _privateData(new PrivateData)
{
}

Condition::~Condition(void)
{
  delete _privateData;
  _privateData = 0;
}

void
Condition::notify_one(void)
{
  _privateData->notify_one();
}

void
Condition::notify_all(void)
{
  _privateData->notify_all();
}

void
Condition::wait(ScopeLock& scopeLock)
{
  _privateData->wait_for(*scopeLock.mutex()->_privateData, INFINITE);
}

bool
Condition::wait_until(ScopeLock& scopeLock, const Clock& absclock)
{
  Clock now = Clock::now();
  if (absclock < now)
    return false;
  DWORD msec = ClockWin32::toMsec((absclock - now).getNSec());
  return _privateData->wait_for(*scopeLock.mutex()->_privateData, msec);
}

#endif

} // namespace OpenRTI
