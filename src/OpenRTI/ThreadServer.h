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

#ifndef OpenRTI_ThreadServer_h
#define OpenRTI_ThreadServer_h

#include "AbstractServer.h"
#include "Condition.h"
#include "Mutex.h"

namespace OpenRTI {

class OPENRTI_API ThreadServer : public AbstractServer {
public:
  ThreadServer(const SharedPtr<AbstractServerNode>& serverNode);
  virtual ~ThreadServer();

  virtual int exec();

protected:
  virtual void _postMessage(const _MessageConnectHandlePair& messageConnectHandlePair);
  virtual void _postOperation(const SharedPtr<_Operation>& operation);

private:
  ThreadServer(const ThreadServer&);
  ThreadServer& operator=(const ThreadServer&);

  mutable Mutex _mutex;
  Condition _condition;
  _Queue _queue;
  _MessageConnectHandlePairList _pool;
};

} // namespace OpenRTI

#endif
