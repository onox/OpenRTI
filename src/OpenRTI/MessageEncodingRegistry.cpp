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
MessageEncodingRegistry::getEncodingPair(const StringStringListMap& valueMap) const
{
  // get the enconding ...
  StringStringListMap::const_iterator i = valueMap.find(L"encoding");
  if (i == valueMap.end() || i->second.size() != 1) {
    // Ok, nothing expected. Try to find out if we have an error message from the server?
    i = valueMap.find(L"error");
    if (i == valueMap.end() || i->second.empty())
      throw RTIinternalError(L"No response from server!");
    throw RTIinternalError(i->second.front());
  }

  std::wstring encodingName = i->second.front();
  if (encodingName == L"TightBE1")
    return MessageEncoderPair(new TightBE1MessageEncoder, new TightBE1MessageDecoder);

  throw RTIinternalError(L"No matching encoder: Dropping connection!");
}

bool
MessageEncodingRegistry::getUseCompression(const StringStringListMap& valueMap)
{
  StringStringListMap::const_iterator i = valueMap.find(L"compression");
  if (i == valueMap.end())
    return false;
  if (i->second.size() != 1)
    return false;
  return i->second.front() == L"zlib";
}

MessageEncoderPair
MessageEncodingRegistry::negotiateEncoding(const SharedPtr<SocketStream>& socketStream, const Clock& abstime, bool& compress, StringStringListMap& parentOptions) const
{
  SocketEventDispatcher dispatcher;

  // Set up the string info we want the server to know about us
  StringStringListMap valueMap;

  // The rti version we support. This is currently fixed 1
  valueMap[L"version"].push_back(L"1");
#ifdef OPENRTI_HAVE_ZLIB
  if (compress)
    valueMap[L"compression"].push_back(L"zlib");
  else
    valueMap[L"compression"].push_back(L"no");
#else
  // No zlib, no compression
  valueMap[L"compression"].push_back(L"no");
#endif
  // And all our encodings we can just do
  valueMap[L"encoding"].push_back(L"TightBE1");

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
  i = valueMap.find(L"version");
  if (i == valueMap.end()) {
    responseValueMap[L"error"].push_back(L"No version field in the connect header given.");
    return responseValueMap;
  }
  // Currently only version "1" is supported on both sides, currently just something to be
  // extensible so that we can change something in the future without crashing clients or servers
  // by a wrong protocol or behavior.
  if (!contains(i->second, L"1")) {
    responseValueMap[L"error"].push_back(L"Client does not support version 1 of the protocol.");
    return responseValueMap;
  }

  // Check the encodings
  i = valueMap.find(L"encoding");
  if (i == valueMap.end() || i->second.empty()) {
    responseValueMap[L"error"].push_back(L"Client advertises no encoding!");
    return responseValueMap;
  }
  // collect some possible encodings
  StringList encoding;
  encoding.push_back(L"TightBE1");
  // Throw out again what does not match with the client
  for (StringList::iterator j = encoding.begin(); j != encoding.end();) {
    if (contains(i->second, *j))
      ++j;
    else
      j = encoding.erase(j);
  }
  if (encoding.empty()) {
    responseValueMap[L"error"].push_back(L"Client and server have no common encoding!");
    return responseValueMap;
  }

  // Survived, respond with a valid response packet
  responseValueMap[L"version"].push_back(L"1");
#ifdef OPENRTI_HAVE_ZLIB
  if (serverOptions._preferCompression && getUseCompression(valueMap)) {
    // Compression by server policy and existing client support
    responseValueMap[L"compression"].push_back(L"zlib");
  } else {
    // No compression by server policy or missing client support
    responseValueMap[L"compression"].push_back(L"no");
  }
#else
  // No zlib, no compression
  responseValueMap[L"compression"].push_back(L"no");
#endif
  responseValueMap[L"encoding"].push_back(encoding.front());

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
