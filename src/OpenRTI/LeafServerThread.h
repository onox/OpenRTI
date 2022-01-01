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

#ifndef OpenRTI_LeafServerThread_h
#define OpenRTI_LeafServerThread_h

#include "AbstractConnect.h"
#include "AbstractServer.h"
#include "Export.h"
#include "StringUtils.h"
#include "Thread.h"
#include "URL.h"

namespace OpenRTI {

class OPENRTI_API LeafServerThread : public Thread {
public:
  LeafServerThread(const SharedPtr<AbstractServer>& server);
  virtual ~LeafServerThread(void);

  /// Shutdow this thread and wait for it to finish.
  void postShutdown();

  /// Connect to thi particular thread.
  SharedPtr<AbstractConnect> connect(const StringStringListMap& clientOptions);

  /// Call this to get a connect to a possibly new leaf server.
  static SharedPtr<AbstractConnect> connect(const URL& url, const StringStringListMap& clientOptions);

protected:
  virtual void run();

private:
  LeafServerThread(const LeafServerThread&);
  LeafServerThread& operator=(const LeafServerThread&);

  /// Store the server threads available per url
  typedef std::map<URL, SharedPtr<LeafServerThread> > UrlServerMap;

  class _Registry;

  /// The server loop that runs into this thread
  SharedPtr<AbstractServer> _server;

  /// The iterator into the registries map of server threads
  UrlServerMap::iterator _iterator;
};

} // namespace OpenRTI

#endif
