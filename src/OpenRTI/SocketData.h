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

#ifndef OpenRTI_SocketData_h
#define OpenRTI_SocketData_h

#include "Socket.h"

namespace OpenRTI {

class OPENRTI_API SocketData : public Socket {
public:

  // FIXME need to distinguish between the read side which can have an EOF
  // and the write side, which might get an EPIPE if the other endpoint is being closed
  // bool isOpen() is the wrong method here. May be isOpenForRead/isOpenForWrite ???

protected:
  SocketData(PrivateData* privateData);
  virtual ~SocketData();
};

} // namespace OpenRTI

#endif
