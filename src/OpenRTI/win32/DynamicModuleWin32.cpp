/* -*-c++-*- OpenRTI - Copyright (C) 2013-2022 Mathias Froehlich
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

#include "DynamicModule.h"

#include "StringUtils.h"
#include "Types.h"

#include <vector>
#include <windows.h>

namespace OpenRTI {

std::string
DynamicModule::getFileNameForAddress(const void* address)
{
#if 0x0501 <= _WIN32_WINNT
  DWORD flags = 0;
  flags |= GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
  flags |= GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS;
  HMODULE handle;
  if (!GetModuleHandleEx(flags, (LPCSTR)address, &handle))
    return std::string();

  std::vector<wchar_t> buf(MAX_PATH, 0);
  for (;;) {
    DWORD retval = GetModuleFileNameW(handle, &buf.front(), DWORD(buf.size()));
    DWORD errorNumber = GetLastError();
    if (retval == buf.size() && errorNumber == ERROR_INSUFFICIENT_BUFFER) {
      buf.resize(buf.size()*2, 0);
      continue;
    }

    if (retval == 0)
      return std::string();

    std::replace(buf.begin(), buf.end(), '\\', '/');
    return ucsToUtf8(std::wstring(&buf.front(), std::wstring::size_type(retval)));
  }

  return std::string();
#else
  return std::string();
#endif
}

}
