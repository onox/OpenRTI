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

#include "Socket.h"
#include "SocketPrivateDataPosix.h"

#ifdef __APPLE__
# include <fstream>
#endif

namespace OpenRTI {

void
Socket::destruct(Socket* socket)
{
  delete socket;
}

bool
Socket::isOpen() const
{
  if (!_privateData)
    return false;
  return 0 <= _privateData->_fd;
}

void
Socket::close()
{
  _privateData->close();
}

Socket::Socket(PrivateData* privateData) :
  _privateData(privateData)
{
}

Socket::~Socket()
{
  delete _privateData;
  _privateData = 0;
}

#ifdef __APPLE__
// Puh, it turns out that std::ifstream does some static initialization
// that appears not to be thread safe. By that force creation of one such
// instance, opening a file at shared object load time.
// Putting this here is to make sure the object file can under no sensible
// circumstance be optimized out by the linker. The assumption is that
// a communication library just always pulls the socket implementation.
class _DummyStreamInitializationSerializer {
public:
  _DummyStreamInitializationSerializer()
  {
    std::ifstream stream("/dev/zero");
  }
};
static _DummyStreamInitializationSerializer dummyStreamInitializationSerializer;
#endif

} // namespace OpenRTI
