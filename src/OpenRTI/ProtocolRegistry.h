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

#ifndef OpenRTI_ProtocolRegistry_h
#define OpenRTI_ProtocolRegistry_h

#include <string>
#include "AbstractProtocol.h"
#include "Export.h"
#include "Mutex.h"
#include "Referenced.h"
#include "ScopeLock.h"
#include "SharedPtr.h"

namespace OpenRTI {

/// Return a protocol for a protocol name.
/// Here a user might insert a new factory for new protocols.
/// In the far term this is a place where we can support protocol plugins
/// by foreign shared objects/dlls.
class OPENRTI_API ProtocolRegistry : public Referenced {
public:
  static const SharedPtr<ProtocolRegistry>& instance();

  ProtocolRegistry();
  ~ProtocolRegistry();

  SharedPtr<const AbstractProtocol> getProtocol(const std::string& name);
  void registerProtocol(const std::string& name, const SharedPtr<const AbstractProtocol>& protocol);

private:
  Mutex _mutex;
  typedef std::map<std::string, SharedPtr<const AbstractProtocol> > ProtocolMap;
  ProtocolMap _protocolMap;
};

} // namespace OpenRTI

#endif
