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

#include "MessageEncodingRegistry.h"

#include "InitialClientSocketReadEvent.h"
#include "InitialClientSocketWriteEvent.h"
#include "LogStream.h"
#include "ServerOptions.h"
#include "SocketEventDispatcher.h"
#include "StringUtils.h"

#include "TightBE1MessageEncoder.h"
#include "TightBE1MessageDecoder.h"

namespace OpenRTI {

MessageEncoderPair
MessageEncodingRegistry::getEncoderPair(const std::string& encodingName) const
{
  /// Currently only one encoding available.
  if (encodingName == "TightBE1")
    return MessageEncoderPair(new TightBE1MessageEncoder, new TightBE1MessageDecoder);
  return MessageEncoderPair();
}

StringList
MessageEncodingRegistry::getEncodings() const
{
  StringList encodingList;
  encodingList.push_back("TightBE1");
  return encodingList;
}

StringList
MessageEncodingRegistry::getCommonEncodings(const StringList& encodingList) const
{
  /// Currently only one encoding available.
  StringList commonEncodingList;
  for (StringList::const_iterator i = encodingList.begin(); i != encodingList.end(); ++i) {
    if (*i == "TightBE1")
      commonEncodingList.push_back(*i);
  }
  return commonEncodingList;
}

MessageEncoderPair
MessageEncodingRegistry::getEncodingPair(const StringStringListMap& valueMap) const
{
  // get the enconding ...
  StringStringListMap::const_iterator i = valueMap.find("encoding");
  if (i == valueMap.end() || i->second.size() != 1) {
    // Ok, nothing expected. Try to find out if we have an error message from the server?
    i = valueMap.find("error");
    if (i == valueMap.end() || i->second.empty())
      throw RTIinternalError("No response from server!");
    throw RTIinternalError(i->second.front());
  }

  MessageEncoderPair encoderPair = getEncoderPair(i->second.front());
  if (encoderPair.first.valid() && encoderPair.second.valid())
    return encoderPair;

  throw RTIinternalError("No matching encoder: Dropping connection!");
}

bool
MessageEncodingRegistry::getUseCompression(const StringStringListMap& valueMap)
{
  StringStringListMap::const_iterator i = valueMap.find("compression");
  if (i == valueMap.end())
    return false;
  if (i->second.size() != 1)
    return false;
  return i->second.front() == "zlib";
}

MessageEncoderPair
MessageEncodingRegistry::negotiateEncoding(const SharedPtr<SocketStream>& socketStream, const Clock& abstime, const std::string& serverName, bool& compress, StringStringListMap& parentOptions) const
{
  SocketEventDispatcher dispatcher;

  // Set up the string info we want the server to know about us
  StringStringListMap valueMap;

  // The rti version we support. This is currently fixed 1
  valueMap["version"].push_back("2");
  // The current servers name
  valueMap["serverName"].push_back(serverName);
#ifdef OPENRTI_HAVE_ZLIB
  if (compress)
    valueMap["compression"].push_back("zlib");
  else
    valueMap["compression"].push_back("no");
#else
  // No zlib, no compression
  valueMap["compression"].push_back("no");
#endif
  // And all our encodings we can just do
  valueMap["encoding"] = getEncodings();

  SharedPtr<InitialClientSocketWriteEvent> writeEvent = new InitialClientSocketWriteEvent(socketStream);
  writeEvent->setValueMap(valueMap);
  dispatcher.insert(writeEvent.get());

  SharedPtr<InitialClientSocketReadEvent> readEvent = new InitialClientSocketReadEvent(socketStream);
  dispatcher.insert(readEvent.get());

  // Process messages until no socket events are left or the timeout expires
  dispatcher.exec(abstime);

  compress = getUseCompression(readEvent->getValueMap());

  // return the parents options map to the caller
  parentOptions = readEvent->getValueMap();

  return getEncodingPair(readEvent->getValueMap());
}

StringStringListMap
MessageEncodingRegistry::getBestServerEncoding(const StringStringListMap& valueMap, const ServerOptions& serverOptions) const
{
  StringStringListMap responseValueMap;

  // Given the clients values in value map, choose something sensible.
  // Here we might want to have something configurable to prefer something over the other??
  StringStringListMap::const_iterator i;
  i = valueMap.find("version");
  if (i == valueMap.end()) {
    responseValueMap["error"].push_back("No version field in the connect header given.");
    return responseValueMap;
  }
  // Currently only version "2" is supported on both sides, currently just something to be
  // extensible so that we can change something in the future without crashing clients or servers
  // by a wrong protocol or behavior.
  if (!contains(i->second, "2")) {
    responseValueMap["error"].push_back("Client does not support version 1 of the protocol.");
    return responseValueMap;
  }

  // Check the encodings
  i = valueMap.find("encoding");
  if (i == valueMap.end() || i->second.empty()) {
    responseValueMap["error"].push_back("Client advertises no encoding!");
    return responseValueMap;
  }
  // collect some possible encodings
  StringList encoding = getCommonEncodings(i->second);
  if (encoding.empty()) {
    responseValueMap["error"].push_back("Client and server have no common encoding!");
    return responseValueMap;
  }

  // Survived, respond with a valid response packet
  responseValueMap["version"].push_back("2");
#ifdef OPENRTI_HAVE_ZLIB
  if (serverOptions._preferCompression && getUseCompression(valueMap)) {
    // Compression by server policy and existing client support
    responseValueMap["compression"].push_back("zlib");
  } else {
    // No compression by server policy or missing client support
    responseValueMap["compression"].push_back("no");
  }
#else
  // No zlib, no compression
  responseValueMap["compression"].push_back("no");
#endif
  responseValueMap["encoding"].push_back(encoding.front());

  return responseValueMap;
}

const MessageEncodingRegistry&
MessageEncodingRegistry::instance()
{
  static MessageEncodingRegistry messageEncodingRegistry;
  return messageEncodingRegistry;
}

MessageEncodingRegistry::MessageEncodingRegistry()
{
}

MessageEncodingRegistry::~MessageEncodingRegistry()
{
}

} // namespace OpenRTI
