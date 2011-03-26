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

#include "ProtocolRegistry.h"

#include "Exception.h"
#include "LogStream.h"
#include "StreamProtocol.h"
#include "StringUtils.h"
#include "ThreadProtocol.h"
#include "TraceProtocol.h"

namespace OpenRTI {

const SharedPtr<ProtocolRegistry>&
ProtocolRegistry::instance()
{
  static Mutex mutex;
  static SharedPtr<ProtocolRegistry> protocolRegistry;
  ScopeLock scopeLock(mutex);
  if (!protocolRegistry.valid())
    protocolRegistry = new ProtocolRegistry;
  return protocolRegistry;
}

ProtocolRegistry::ProtocolRegistry()
{
  /// A process local rti providing this service by thread communication
  registerProtocol("thread", new ThreadProtocol);

  /// A machine local rti transfering through named pipes.
  registerProtocol("pipe", new PipeProtocol);

  /// A tcp based protocol, allowing communication by the internet
  registerProtocol("rti", new InetProtocol);

  /// A tcp based protocol, using http requests for happy firewalls
  // registerProtocol("http", new HttpProtocol);

  /// Aims to dump all the communication routed through that protocol
  /// into a file or something. The messages as such are passed to
  /// an other protocol.
  registerProtocol("trace", new TraceProtocol);
}

ProtocolRegistry::~ProtocolRegistry()
{
}

SharedPtr<const AbstractProtocol>
ProtocolRegistry::getProtocol(const std::string& name)
{
  /// May be have some hardcoded protocols??
  /// The threaded one, the tcp plain one, a http variant???
  ScopeLock scopeLock(_mutex);
  ProtocolMap::const_iterator i = _protocolMap.find(name);
  if (i == _protocolMap.end())
    return 0;
  return i->second;
}

void
ProtocolRegistry::registerProtocol(const std::string& name, const SharedPtr<const AbstractProtocol>& protocol)
{
  ScopeLock scopeLock(_mutex);
  if (_protocolMap.find(name) != _protocolMap.end())
    Log(Assert, Warning) << "Duplicate protocol registration of protocol: " << name << std::endl;
  _protocolMap[name] = protocol;
}

} // namespace OpenRTI
