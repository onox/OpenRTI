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

#include "InitialStreamProtocol.h"

#include "DecodeDataStream.h"
#include "EncodeDataStream.h"
#include "LogStream.h"

namespace OpenRTI {

class OPENRTI_LOCAL InitialStreamProtocol::DecodeStream : public DecodeDataStream {
public:
  DecodeStream(const VariableLengthData& variableLengthData) :
    DecodeDataStream(variableLengthData)
  { }

  void readStringStringListMap(StringStringListMap& stringStringListMap)
  {
    uint32_t i = readUInt32BE();
    while (i--) {
      std::string key;
      readString(key);
      readStringList(stringStringListMap[key]);
    }
  }

  void readStringList(StringList& stringList)
  {
    uint32_t i = readUInt32BE();
    while (i--) {
      stringList.push_back(std::string());
      readString(stringList.back());
    }
  }

  void readString(std::string& utf8)
  {
    uint32_t i = readUInt32BE();
    utf8.clear();
    utf8.reserve(i);
    while (i--) {
      utf8.push_back(readUInt8BE());
    }
    align(4);
  }

  void readFixedString(char* string, size_t len)
  {
    while (len--)
      *string++ = readUInt8BE();
  }
};

class OPENRTI_LOCAL InitialStreamProtocol::EncodeStream : public EncodeDataStream {
public:
  EncodeStream(VariableLengthData& variableLengthData) :
    EncodeDataStream(variableLengthData)
  { }

  void writeStringStringListMap(const StringStringListMap& stringStringListMap)
  {
    writeUInt32BE(uint32_t(stringStringListMap.size()));
    for (StringStringListMap::const_iterator i = stringStringListMap.begin(); i != stringStringListMap.end(); ++i) {
      writeString(i->first);
      writeStringList(i->second);
    }
  }

  void writeStringList(const StringList& stringList)
  {
    writeUInt32BE(uint32_t(stringList.size()));
    for (StringList::const_iterator i = stringList.begin(); i != stringList.end(); ++i)
      writeString(*i);
  }

  void writeString(const std::string& utf8)
  {
    writeUInt32BE(uint32_t(utf8.size()));
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

InitialStreamProtocol::InitialStreamProtocol()
{
}

InitialStreamProtocol::~InitialStreamProtocol()
{
}

void
InitialStreamProtocol::writeOptionMap(const StringStringListMap& stringStringListMap)
{
  // Header and body
  EncodeStream headerStream(addScratchWriteBuffer());
  EncodeStream bodyStream(addScratchWriteBuffer());

  // Write the message body
  bodyStream.writeStringStringListMap(stringStringListMap);

  // Write the message header
  headerStream.writeFixedString("OpenRTI\0", 8);
  headerStream.writeUInt32BE(uint32_t(bodyStream.size() + 12));
}

void
InitialStreamProtocol::readPacket(const Buffer& buffer)
{
  switch (buffer.size()) {
  case 0:
    // First read a 12 byte header
    addScratchReadBuffer(12);
    break;
  case 1:
    {
      // When the header is here, ensure that the header is valid and look for the size
      DecodeStream decodeStream(buffer.front());

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
      addScratchReadBuffer(size - 12);
    }
    break;
  case 2:
    {
      // Read the key value map that occupies the whole body
      DecodeStream decodeStream(buffer.back());
      StringStringListMap stringStringListMap;
      decodeStream.readStringStringListMap(stringStringListMap);

      // Call back into the implementing code.
      readOptionMap(stringStringListMap);
    }
    break;
  default:
    throw MessageError("Unexpected message buffer size!");
  }
}

void
InitialStreamProtocol::writePacket()
{
}

bool
InitialStreamProtocol::getMoreToSend() const
{
  return false;
}

void
InitialStreamProtocol::read(AbstractProtocolSocket& protocolSocket)
{
  StreamBufferProtocol::read(protocolSocket);
  activateFollowupProtocol(protocolSocket);
}

bool
InitialStreamProtocol::getEnableRead() const
{
  return StreamBufferProtocol::getEnableRead();
}

void
InitialStreamProtocol::write(AbstractProtocolSocket& protocolSocket)
{
  StreamBufferProtocol::write(protocolSocket);
  activateFollowupProtocol(protocolSocket);
}

bool
InitialStreamProtocol::getEnableWrite() const
{
  return StreamBufferProtocol::getEnableWrite();
}

void
InitialStreamProtocol::error(const Exception& e)
{
  Log(MessageCoding, Warning) << "Unhandled exception during initials OpenRTI connection negotiation!" << std::endl;
}

void
InitialStreamProtocol::setFollowupProtocol(const SharedPtr<AbstractProtocolLayer>& followupProtocol)
{
  _followupProtocol = followupProtocol;
}

void
InitialStreamProtocol::activateFollowupProtocol(AbstractProtocolSocket& protocolSocket)
{
  if (!getBuffersComplete())
    return;
  if (_followupProtocol.valid()) {
    protocolSocket.replaceProtocol(_followupProtocol);
    _followupProtocol = 0;
  } else {
    // Ok, if we have no followup, there must be an error, so close.
    // FIXME throw????
    protocolSocket.close();
  }
}

} // namespace OpenRTI
