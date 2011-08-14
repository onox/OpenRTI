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

#include "SocketEventDispatcher.h"

#include "AbstractSocketEvent.h"
#include "Exception.h"
#include "LogStream.h"
#include "SocketReadEvent.h"
#include "SocketWriteEvent.h"

namespace OpenRTI {

class OPENRTI_LOCAL SocketWriteEventAdapter : public SocketWriteEvent {
public:
  SocketWriteEventAdapter(const SharedPtr<AbstractSocketEvent>& socketEvent) :
    _socketEvent(socketEvent)
  { }

  virtual void write(SocketEventDispatcher& dispatcher)
  {
    try {
      _socketEvent->write(dispatcher);
    } catch (const Exception& e) {
      Log(MessageCoding, Warning) << "Caught exception while processing socket output: " << e.what()
                                  << "\nClosing connection!" << std::endl;
      _socketEvent->error(e);
      dispatcher.eraseSocket(this);
    } catch (const std::exception& e) {
      Log(MessageCoding, Warning) << "Caught exception while processing socket output: " << e.what()
                                  << "\nClosing connection!" << std::endl;
      _socketEvent->error(RTIinternalError(e.what()));
      dispatcher.eraseSocket(this);
    }
  }
  virtual Socket* getSocket() const
  { return _socketEvent->getSocket(); }
  virtual bool getEnable() const
  { return _socketEvent->getEnableWrite(); }

private:
  SharedPtr<AbstractSocketEvent> _socketEvent;
};

class OPENRTI_LOCAL SocketReadEventAdapter : public SocketReadEvent {
public:
  SocketReadEventAdapter(const SharedPtr<AbstractSocketEvent>& socketEvent) :
    _socketEvent(socketEvent)
  { }

  virtual void read(SocketEventDispatcher& dispatcher)
  {
    /// Hmm, move that try/catch block into the socket dispatcher???
    /// alternatively enable the read/write event to communicate connection close to the dispatcher.
    try {
      _socketEvent->read(dispatcher);
    } catch (const Exception& e) {
      Log(MessageCoding, Warning) << "Caught exception while processing socket input: " << e.what()
                                  << "\nClosing connection!" << std::endl;
      _socketEvent->error(e);
      dispatcher.eraseSocket(this);
    } catch (const std::exception& e) {
      Log(MessageCoding, Warning) << "Caught exception while processing socket input: " << e.what()
                                  << "\nClosing connection!" << std::endl;
      _socketEvent->error(RTIinternalError(e.what()));
      dispatcher.eraseSocket(this);
    }
  }
  virtual Socket* getSocket() const
  { return _socketEvent->getSocket(); }
  virtual bool getEnable() const
  { return _socketEvent->getEnableRead(); }

private:
  SharedPtr<AbstractSocketEvent> _socketEvent;
};

void
SocketEventDispatcher::insert(const SharedPtr<AbstractSocketEvent>& socketEvent)
{
  SharedPtr<SocketWriteEventAdapter> writeEvent = new SocketWriteEventAdapter(socketEvent);
  SharedPtr<SocketReadEventAdapter> readEvent = new SocketReadEventAdapter(socketEvent);

  insert(readEvent.get());
  insert(writeEvent.get());
}

} // namespace OpenRTI
