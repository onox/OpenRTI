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

#include "InitialSocketReadEvent.h"

#include "DecodeDataStream.h"
#include "LogStream.h"
#include "SocketEventDispatcher.h"
#include "StringUtils.h"

namespace OpenRTI {

// Connection startup messages.
// Never change this encoding!
// It is a 4 byte aligned xdr like big endian encoding.

class OPENRTI_LOCAL InitialSocketReadEvent::DecodeStream : public DecodeDataStream {
public:
  DecodeStream(const VariableLengthData& variableLengthData) :
    DecodeDataStream(variableLengthData)
  { }

  void readStringStringListMap(StringStringListMap& stringStringListMap)
  {
    uint32_t i = readUInt32BE();
    while (i--) {
      std::wstring key;
      readString(key);
      readStringList(stringStringListMap[key]);
    }
  }

  void readStringList(StringList& stringList)
  {
    uint32_t i = readUInt32BE();
    while (i--) {
      stringList.push_back(std::wstring());
      readString(stringList.back());
    }
  }

  void readString(std::wstring& string)
  {
    uint32_t i = readUInt32BE();
    std::string utf8;
    utf8.reserve(i);
    while (i--) {
      utf8.push_back(readUInt8BE());
    }
    align(4);
    string = utf8ToUcs(utf8);
  }

  void readFixedString(char* string, size_t len)
  {
    while (len--)
      *string++ = readUInt8BE();
  }
};

InitialSocketReadEvent::InitialSocketReadEvent(const SharedPtr<SocketStream>& socketStream) :
  StreamSocketReadEvent(socketStream)
{
}

InitialSocketReadEvent::~InitialSocketReadEvent()
{
}

void
InitialSocketReadEvent::readPacket(SocketEventDispatcher& dispatcher, NetworkBuffer& networkBuffer)
{
  switch (networkBuffer.size()) {
  case 0:
    // First read a 12 byte header
    networkBuffer.addScratchBuffer().resize(12);
    break;
  case 1:
    {
      // When the header is here, ensure that the header is valid and look for the size
      DecodeStream decodeStream(networkBuffer[0]);
      
      // Check for the correct message header
      char starter[8];
      decodeStream.readFixedString(starter, 8);
      if (std::memcmp(starter, "OpenRTI\0", 8) != 0) {
        Log(MessageCoding, Warning) << "Connection does not start with the correct header: Dropping connection!" << std::endl;
        throw MessageError("Connection does not start with the correct header!");
      }
      size_t size = decodeStream.readUInt32BE();
      if (size <= 12) {
        Log(MessageCoding, Warning) << "Connection does not start with a valid size field in the header: Dropping connection!" << std::endl;
        throw MessageError("Connection does not start with a valid size field in the header!");
      }
      
      // Add buffer space for the message body
      networkBuffer.addScratchBuffer().resize(size - 12);
    }
    break;
  case 2:
    {
      // Read the key value map that occupies the whole body
      DecodeStream decodeStream(networkBuffer[1]);
      decodeStream.readStringStringListMap(_valueMap);
    }
    break;
  default:
    throw MessageError("Unexpected message buffer size!");
  }
}

} // namespace OpenRTI
