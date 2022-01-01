/* -*-c++-*- OpenRTI - Copyright (C) 2009-2022 Mathias Froehlich
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

#include "SocketServerPipe.h"

#include "ErrnoPosix.h"
#include "Exception.h"
#include "SocketPrivateDataPosix.h"

namespace OpenRTI {

SocketServerPipe::SocketServerPipe() :
  SocketServer(new PrivateData(-1))
{
}

void
SocketServerPipe::bind(const std::string& file)
{
  int fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd == -1) {
    int errorNumber = errno;
    throw TransportError(errnoToUtf8(errorNumber));
  }

  // This is nice to have, so just try and don't bail out
  int flags = fcntl(fd, F_GETFD, 0);
  if (flags != -1)
    fcntl(fd, F_SETFD, flags | FD_CLOEXEC);

  std::string localeFile = utf8ToLocale(file);

  // huger than needed but sufficient
  size_t len = sizeof(struct sockaddr_un) + localeFile.size();
  struct sockaddr_un* addr = (struct sockaddr_un*)calloc(len, 1);
  addr->sun_family = AF_UNIX;
  memcpy(addr->sun_path, localeFile.c_str(), localeFile.size());

  // Try to bind the socket to the file,
  // Try that as long as we could not do so and see a real chance that we will succeed in next try
  for (;;) {
    int ret = ::bind(fd, (const sockaddr*)addr, SUN_LEN(addr));
    if (ret != -1)
      break;
    int errorNumber = errno;

    // We could not bind to that file, try to find out if the server behind is still alive
    try {
      SharedPtr<SocketPipe> socket = new SocketPipe;
      socket->connect(file);
      // We could connect, we assume that the server is living, don't replace that
      free(addr);
      if (ret == -1) {
        ::close(fd);
        throw TransportError(errnoToUtf8(errorNumber));
      }
    } catch (const Exception& e) {
      // If we cannot connect to that server, we assume that it is not alive anymore and at least try
      // to get rid of the file
      if (-1 == unlink(localeFile.c_str())) {
        // If that file still persists, bail out
        free(addr);
        if (ret == -1) {
          ::close(fd);
          throw TransportError(errnoToUtf8(errorNumber));
        }
      }
      // If we could remove the file, we have a real next chance to bind
      // our unix socket at that file.
    }
  }
  free(addr);

  _privateData->_fd = fd;
}

void
SocketServerPipe::listen(int backlog)
{
  int ret = ::listen(_privateData->_fd, backlog);
  if (ret == -1) {
    int errorNumber = errno;
    throw TransportError(errnoToUtf8(errorNumber));
  }
}

SocketPipe*
SocketServerPipe::accept()
{
  int fd = ::accept(_privateData->_fd, 0, 0);
  if (fd == -1) {
    int errorNumber = errno;
    throw TransportError(errnoToUtf8(errorNumber));
  }

  // Need to have non blocking IO
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    int errorNumber = errno;
    ::close(fd);
    throw TransportError(errnoToUtf8(errorNumber));
  }
  int ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  if (ret == -1) {
    int errorNumber = errno;
    ::close(fd);
    throw TransportError(errnoToUtf8(errorNumber));
  }

  return new SocketPipe(new PrivateData(fd));
}

void
SocketServerPipe::close()
{
  if (!isOpen())
    return;

  // Remove the named pipe file.

  // get the length of the address
  socklen_t addrlen = 0;
  int ret = ::getsockname(_privateData->_fd, 0, &addrlen);
  if (ret == -1)
    return;

  // get the address
  struct sockaddr_un* addr = (struct sockaddr_un*)calloc(addrlen, 1);
  ret = ::getsockname(_privateData->_fd, (struct sockaddr*)addr, &addrlen);
  if (ret == -1) {
    free(addr);
    return;
  }

  // and remove the file
  ret = ::unlink(addr->sun_path);
  free(addr);
  if (ret == -1)
    return;

  // Close the socket
  ::close(_privateData->_fd);
  _privateData->_fd = -1;
}

SocketServerPipe::~SocketServerPipe()
{
  close();
}

} // namespace OpenRTI
