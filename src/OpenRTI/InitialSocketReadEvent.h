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

#ifndef OpenRTI_InitialSocketReadEvent_h
#define OpenRTI_InitialSocketReadEvent_h

#include <list>
#include <map>
#include <string>

#include "StreamSocketReadEvent.h"
#include "StringUtils.h"

namespace OpenRTI {

/// InitialSocketReadEvent
///
/// Handles the socket connection setup packets.
/// These packets have a fixed 12 bytes message header consisting of the zero terminated
/// string OpenRTI followed by the total length of this packet.
/// The packets payload is a list of key/value pairs.
/// Each key is an utf8 encoded string, each value is a list of utf8 encoded strings.
/// The length fields are 32-bit big endian values.
/// The this packet layout should never change!
/// See EncodingRegistry for an explanation.
///
class OPENRTI_LOCAL InitialSocketReadEvent : public StreamSocketReadEvent {
public:
  InitialSocketReadEvent(const SharedPtr<SocketStream>& socketStream);
  virtual ~InitialSocketReadEvent();

  virtual void readPacket(SocketEventDispatcher& dispatcher, NetworkBuffer& networkBuffer);

  const StringStringListMap& getValueMap() const
  { return _valueMap; }
  StringStringListMap& getValueMap()
  { return _valueMap; }

protected:
  StringStringListMap _valueMap;

private:
  class DecodeStream;
};

} // namespace OpenRTI

#endif
