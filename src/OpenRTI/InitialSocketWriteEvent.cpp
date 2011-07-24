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

#include "InitialSocketWriteEvent.h"

#include "EncodeDataStream.h"
#include "StringUtils.h"

namespace OpenRTI {

// Connection startup messages.
// Never change this encoding!
// It is a 4 byte aligned xdr like big endian encoding.

class OPENRTI_LOCAL InitialSocketWriteEvent::EncodeStream : public EncodeDataStream {
public:
  EncodeStream(VariableLengthData& variableLengthData) :
    EncodeDataStream(variableLengthData)
  { }

  void writeStringStringListMap(const StringStringListMap& stringStringListMap)
  {
    writeUInt32BE(stringStringListMap.size());
    for (StringStringListMap::const_iterator i = stringStringListMap.begin(); i != stringStringListMap.end(); ++i) {
      writeString(i->first);
      writeStringList(i->second);
    }
  }

  void writeStringList(const StringList& stringList)
  {
    writeUInt32BE(stringList.size());
    for (StringList::const_iterator i = stringList.begin(); i != stringList.end(); ++i)
      writeString(*i);
  }

  void writeString(const std::string& utf8)
  {
    writeUInt32BE(utf8.size());
    for (std::string::const_iterator i = utf8.begin(); i != utf8.end(); ++i)
      writeUInt8BE(*i);
    align(4);
  }

  void writeFixedString(const char* string, size_t len)
  {
    while (len--)
      writeUInt8BE(*string++);
  }
};

InitialSocketWriteEvent::InitialSocketWriteEvent(const SharedPtr<SocketStream>& socketStream) :
  StreamSocketWriteEvent(socketStream),
  _moreToSend(false)
{
}

InitialSocketWriteEvent::~InitialSocketWriteEvent()
{
}

void
InitialSocketWriteEvent::setValueMap(const StringStringListMap& valueMap)
{
  _valueMap = valueMap;
  _moreToSend = true;
}

void
InitialSocketWriteEvent::writePacket(SocketEventDispatcher& dispatcher, NetworkBuffer& networkBuffer)
{
  // Header and body
  EncodeStream headerStream(networkBuffer.addScratchBuffer());
  EncodeStream bodyStream(networkBuffer.addScratchBuffer());

  // Write the message body
  bodyStream.writeStringStringListMap(_valueMap);

  // Write the message header
  headerStream.writeFixedString("OpenRTI\0", 8);
  headerStream.writeUInt32BE(bodyStream.size() + 12);

  // Once network buffer is sent, there is not more than that.
  _moreToSend = false;
}

bool
InitialSocketWriteEvent::getMoreToSend() const
{
  return _moreToSend;
}

} // namespace OpenRTI
