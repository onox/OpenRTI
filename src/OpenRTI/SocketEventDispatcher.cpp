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

namespace OpenRTI {

void
SocketEventDispatcher::read(const SharedPtr<AbstractSocketEvent>& socketEvent)
{
  try {
    socketEvent->read(*this);
  } catch (const Exception& e) {
    Log(MessageCoding, Warning) << "Caught exception while processing socket input: " << e.what()
                                << "\nClosing connection!" << std::endl;
    socketEvent->error(e);
    erase(socketEvent);
  } catch (const std::exception& e) {
    Log(MessageCoding, Warning) << "Caught exception while processing socket input: " << e.what()
                                << "\nClosing connection!" << std::endl;
    socketEvent->error(RTIinternalError(e.what()));
    erase(socketEvent);
  }
}

void
SocketEventDispatcher::write(const SharedPtr<AbstractSocketEvent>& socketEvent)
{
  try {
    socketEvent->write(*this);
  } catch (const Exception& e) {
    Log(MessageCoding, Warning) << "Caught exception while processing socket output: " << e.what()
                                << "\nClosing connection!" << std::endl;
    socketEvent->error(e);
    erase(socketEvent);
  } catch (const std::exception& e) {
    Log(MessageCoding, Warning) << "Caught exception while processing socket output: " << e.what()
                                << "\nClosing connection!" << std::endl;
    socketEvent->error(RTIinternalError(e.what()));
    erase(socketEvent);
  }
}

void
SocketEventDispatcher::timeout(const SharedPtr<AbstractSocketEvent>& socketEvent)
{
  try {
    socketEvent->timeout(*this);
  } catch (const Exception& e) {
    Log(MessageCoding, Warning) << "Caught exception while processing socket output: " << e.what()
                                << "\nClosing connection!" << std::endl;
    socketEvent->error(e);
    erase(socketEvent);
  } catch (const std::exception& e) {
    Log(MessageCoding, Warning) << "Caught exception while processing socket output: " << e.what()
                                << "\nClosing connection!" << std::endl;
    socketEvent->error(RTIinternalError(e.what()));
    erase(socketEvent);
  }
}

} // namespace OpenRTI
