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

#include "InitialServerStreamProtocol.h"

#include "LogStream.h"
#include "AbstractServer.h"
#include "MessageEncodingRegistry.h"
#include "ServerOptions.h"
#include "ZLibProtocolLayer.h"

namespace OpenRTI {

InitialServerStreamProtocol::InitialServerStreamProtocol(AbstractServer& abstractServer) :
  _abstractServer(abstractServer)
{
  // Add space for the initial header
  addScratchReadBuffer(12);
}

InitialServerStreamProtocol::~InitialServerStreamProtocol()
{
}

void
InitialServerStreamProtocol::readOptionMap(const StringStringListMap& clientOptionMap)
{
#if defined OPENRTI_ENCODING_DEVELOPMENT_WARNING
  Log(MessageCoding, Warning) << OPENRTI_ENCODING_DEVELOPMENT_WARNING << std::endl;
#endif

  // Given the clients values in value map, choose something sensible.
  // Here we might want to have something configurable to prefer something over the other??
  StringStringListMap::const_iterator i;
  i = clientOptionMap.find("version");
  if (i == clientOptionMap.end()) {
    errorResponse("No version field in the connect header given.");
    return;
  }
  // Currently only version OPENRTI_ENCODING_VERSION is supported on both sides, currently just something to be
  // extensible so that we can change something in the future without crashing clients or servers
  // by a wrong protocol or behavior.
  if (!contains(i->second, OPENRTI_ENCODING_VERSION)) {
    errorResponse("Client does not support version " OPENRTI_ENCODING_VERSION " of the protocol.");
    return;
  }

  // Check the encodings
  i = clientOptionMap.find("encoding");
  if (i == clientOptionMap.end() || i->second.empty()) {
    errorResponse("Client advertises no encoding!");
    return;
  }
  // collect some possible encodings
  StringList encodingList = MessageEncodingRegistry::instance().getCommonEncodings(i->second);
  if (encodingList.empty()) {
    errorResponse("Client and server have no common encoding!");
    return;
  }
  // FIXME Here, we could negotiate with a server configuration which one to take...

  // Preload this with the server nodes configuration
  StringStringListMap responseValueMap;
  responseValueMap = _abstractServer.getServerNode().getServerOptions()._optionMap;

  // Since we already asked for the common encodings, this must be successful now.
  SharedPtr<AbstractMessageEncoding> messageProtocol;
  messageProtocol = MessageEncodingRegistry::instance().getEncoding(encodingList.front());

  // Get a new client connect from the server implementation.
  SharedPtr<AbstractConnect> connect;
  connect = _abstractServer.sendConnect(clientOptionMap, false /*parent*/);
  if (!connect.valid()) {
    errorResponse("Could not get an internal connect structure from the server!");
    return;
  }
  messageProtocol->setConnect(connect);

  // This is the part of the protocol stack that replaces this initial stuff.
  SharedPtr<AbstractProtocolLayer> protocolStack = messageProtocol;

  // Survived, respond with a valid response packet
  responseValueMap["version"].clear();
  responseValueMap["version"].push_back(OPENRTI_ENCODING_VERSION);
  responseValueMap["encoding"].clear();
  responseValueMap["encoding"].push_back(encodingList.front());
  responseValueMap["compression"].clear();

  if (_abstractServer.getServerNode().getServerOptions()._preferCompression) {
    i = clientOptionMap.find("compression");
    if (i != clientOptionMap.end()) {
      for (StringList::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
// #if defined(OPENRTI_HAVE_XZ)
//           if (*j == "lzma") {
//             SharedPtr<LZMACompressionProtocolLayer> layer = new LZMACompressionProtocolLayer;
//             layer->setProtocolLayer(protocolStack);
//             protocolStack = layer;
//             responseValueMap["compression"].push_back("lzma");
//             break;
//           }
// #endif
#if defined(OPENRTI_HAVE_ZLIB)
        if (*j == "zlib") {
          SharedPtr<ZLibProtocolLayer> layer = new ZLibProtocolLayer;
          layer->setProtocolLayer(protocolStack);
          protocolStack = layer;
          responseValueMap["compression"].push_back("zlib");
          break;
        }
#endif
      }
    }
  }
  if (responseValueMap["compression"].empty())
    responseValueMap["compression"].push_back("no");

  writeOptionMap(responseValueMap);
  setFollowupProtocol(protocolStack);
}

void
InitialServerStreamProtocol::errorResponse(const std::string& errorMessage)
{
  StringStringListMap responseValueMap;
  responseValueMap["error"].push_back(errorMessage);
  writeOptionMap(responseValueMap);
}

} // namespace OpenRTI
