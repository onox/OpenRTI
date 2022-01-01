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

#ifndef OpenRTI_SocketPrivateDataPosix_h
#define OpenRTI_SocketPrivateDataPosix_h

#include "Socket.h"

#include "Export.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <string>

#ifndef SOCK_CLOEXEC
# define SOCK_CLOEXEC 0
#endif

#ifndef SUN_LEN
# define SUN_LEN(ptr) ((size_t)(((struct sockaddr_un*)0)->sun_path) + std::strlen((ptr)->sun_path))
#endif

// Define if you want to provoke latencies in message delivery for testing
// NEVER define that for production use!
// #define DEBUG_LATENCY

namespace OpenRTI {

struct OPENRTI_LOCAL Socket::PrivateData {
  PrivateData(int fd) : _fd(fd) {}
  ~PrivateData() { close(); }

  void close()
  {
    if (_fd < 0)
      return;

    ::close(_fd);
    _fd = -1;
  }

  int _fd;
};

} // namespace OpenRTI

#endif
