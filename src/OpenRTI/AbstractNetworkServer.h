/* -*-c++-*- OpenRTI - Copyright (C) 2009-2012 Mathias Froehlich
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

#ifndef OpenRTI_AbstractNetworkServer_h
#define OpenRTI_AbstractNetworkServer_h

#include "AbstractServerNode.h"
#include "Referenced.h"

namespace OpenRTI {

class OPENRTI_API AbstractNetworkServer : public Referenced {
public:
  virtual ~AbstractNetworkServer() {}

  /// Gives access to the rti server node running in this NetworkServer.
  /// The method is guaranteed to return a valid ServerNode.
  virtual AbstractServerNode& getServerNode() = 0;

  /// The non thread safe variant
  virtual void setDone(bool done) = 0;
};

} // namespace OpenRTI

#endif
