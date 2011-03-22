/* -*-c++-*- OpenRTI - Copyright (C) 2009-2011 Mathias Froehlich
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

#ifndef OpenRTI_SocketStream_h
#define OpenRTI_SocketStream_h

#include "NetworkBuffer.h"
#include "SocketData.h"

namespace OpenRTI {

class SocketServer;

/// Desired error semantics:
/// send:
///  returns 0 on 'please try again'
///  returns -1 on 'message too big'
///  throws an exception on any 'serious' error
/// recv:
///  returns 0 on EOF
///  returns -1 on 'please try again'
///  throws an exception on any 'serious' error
/// Note that this is inconsistent but close to what the bare posix stuff
/// does with error numbers. So, when reusing the usual method names, also
/// reuse their semantics as much as possible.

class SocketStream : public SocketData {
public:

  ssize_t send(const NetworkBuffer& networkBuffer, bool moreToSend);
  ssize_t recv(NetworkBuffer& networkBuffer);
  virtual void cork(bool enable);

protected:
  SocketStream(PrivateData* privateData);
  virtual ~SocketStream();

  friend class SocketServer;
};

} // namespace OpenRTI

#endif
