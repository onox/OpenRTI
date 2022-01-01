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

#include "Thread.h"

#ifndef NOMINMAX
# define NOMINMAX
#endif
#include <windows.h>
#include "Exception.h"
#include "Export.h"

namespace OpenRTI {

struct OPENRTI_LOCAL Thread::PrivateData {
  PrivateData() :
    _handle(INVALID_HANDLE_VALUE)
  {
  }
  ~PrivateData()
  {
    if (_handle == INVALID_HANDLE_VALUE)
      return;
    CloseHandle(_handle);
    _handle = INVALID_HANDLE_VALUE;
  }

  static DWORD WINAPI start_routine(LPVOID data)
  {
    Thread* thread = reinterpret_cast<Thread*>(data);
    thread->run();
    if (!Thread::put(thread))
      Thread::destruct(thread);
    return 0;
  }

  bool start(Thread& thread)
  {
    if (_handle != INVALID_HANDLE_VALUE)
      return false;
    get(&thread);
    _handle = CreateThread(0, 0, start_routine, &thread, 0, 0);
    if (_handle == INVALID_HANDLE_VALUE) {
      put(&thread);
      return false;
    }
    return true;
  }

  void wait()
  {
    if (_handle == INVALID_HANDLE_VALUE)
      return;
    DWORD ret = WaitForSingleObject(_handle, INFINITE);
    if (ret != WAIT_OBJECT_0)
      return;
    CloseHandle(_handle);
    _handle = INVALID_HANDLE_VALUE;
  }

  HANDLE _handle;
};

Thread::Thread(void) :
  _privateData(new PrivateData)
{
}

Thread::~Thread(void)
{
  OpenRTIAssert(!Thread::count(this));
  delete _privateData;
  _privateData = 0;
}

void Thread::destruct(Thread* thread)
{
  delete thread;
}

bool
Thread::start()
{
  return _privateData->start(*this);
}

void
Thread::wait()
{
  _privateData->wait();
}

} // namespace OpenRTI
