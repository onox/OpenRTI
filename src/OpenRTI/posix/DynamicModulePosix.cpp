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

#include "OpenRTIConfig.h"

#include "DynamicModule.h"

#include "StringUtils.h"
#include "Types.h"

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <dlfcn.h>

namespace OpenRTI {

std::string
DynamicModule::getFileNameForAddress(const void* address)
{
#if defined OpenRTI_HAVE_DLADDR
  Dl_info info;
  if (0 == dladdr((void*)address, &info))
    return std::string();
  if (!info.dli_fname)
    return std::string();
  return localeToUtf8(info.dli_fname);
#else
  return std::string();
#endif
}

}
