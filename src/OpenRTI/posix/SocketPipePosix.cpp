/* -*-c++-*- OpenRTI - Copyright (C) 2009-2010 Mathias Froehlich 
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

#include "SocketPipe.h"

#include "ErrnoPosix.h"
#include "Exception.h"
#include "SocketPrivateDataPosix.h"

namespace OpenRTI {

SocketPipe::SocketPipe() :
  SocketStream(new PrivateData(-1))
{
}

void
SocketPipe::connect(const std::wstring& file)
{
  if (0 <= _privateData->_fd)
    throw TransportError(L"Trying to connect an already open SocketPipe!");

  int fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd == -1) {
    int errorNumber = errno;
    throw TransportError(errnoToUcs(errorNumber));
  }

  // This is nice to have, so just try and don't bail out
  int flags = fcntl(fd, F_GETFD, 0);
  if (flags != -1)
    fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
    
  std::string localeFile = ucsToLocale(file);

  // huger than needed but sufficient in any case
  size_t len = sizeof(struct sockaddr_un) + localeFile.size();
  struct sockaddr_un* addr = (struct sockaddr_un*)calloc(len, 1);
  addr->sun_family = AF_UNIX;
  memcpy(addr->sun_path, localeFile.c_str(), localeFile.size());

  int ret = ::connect(fd, (const sockaddr*)addr, SUN_LEN(addr));
  int errorNumber = errno;
  free(addr);
  if (ret == -1) {
    ::close(fd);
    throw TransportError(errnoToUcs(errorNumber));
  }

  // Past the connect use non blocking io, required.
  flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    errorNumber = errno;
    ::close(fd);
    throw TransportError(errnoToUcs(errorNumber));
  }
  ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  if (ret == -1) {
    errorNumber = errno;
    ::close(fd);
    throw TransportError(errnoToUcs(errorNumber));
  }

  _privateData->_fd = fd;
}

SocketPipe::SocketPipe(PrivateData* privateData) :
  SocketStream(privateData)
{
}

SocketPipe::~SocketPipe()
{
  if (_privateData->_fd == -1)
    return;
  ::close(_privateData->_fd);
  _privateData->_fd = -1;
}

} // namespace OpenRTI
