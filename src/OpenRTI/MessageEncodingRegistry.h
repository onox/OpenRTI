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

#ifndef OpenRTI_MessageEncodingRegistry_h
#define OpenRTI_MessageEncodingRegistry_h

#include "AbstractMessageEncoder.h"
#include "AbstractMessageDecoder.h"
#include "InitialSocketWriteEvent.h"
#include "SocketStream.h"

namespace OpenRTI {

class Clock;
class ServerOptions;

typedef std::pair<SharedPtr<AbstractMessageEncoder>, SharedPtr<AbstractMessageDecoder> > MessageEncoderPair;

/// MessageEncodingRegistry
///
/// Holds all the encodings we know about.
/// On connect, the client sends a message that tells something about
/// The encoding capabilities of the client. The server takes this initial
/// packet and responds with a respons of the same message encoding.
/// This response contains the selected encoding.
/// The packet format is a big endian/utf8 combination that should never change
/// and for OpenRTI messages. It contains a string map of string lists that can contain
/// any data we will need in the future to select the appropriate encoders.
///
/// At least this contains the version, endianess of the participants and the
/// supported stream encoding keys in preference order. The server compares these encodings
/// with its own available encodings and with some to be implemented policy about compression
/// and such. The server selects the used encoding and sends that back to the client.
/// The next data that is exchanged needs to happen in the selected encoding.
///
class MessageEncodingRegistry {
public:
  /// Return an encoder pair for a given encoding name
  MessageEncoderPair getEncoderPair(const std::string& encodingName) const;
  /// Return the list of supported encodings known to the registry.
  StringList getEncodings() const;
  /// Return the common subset of encodings from the encodingList and the ones known to the registry.
  StringList getCommonEncodings(const StringList& encodingList) const;

  MessageEncoderPair getEncodingPair(const StringStringListMap& valueMap) const;
  static bool getUseCompression(const StringStringListMap& valueMap);
  // Helper to set up a client
  MessageEncoderPair negotiateEncoding(const SharedPtr<SocketStream>& socketStream, const Clock& abstime, const std::string& serverName, bool& compress, StringStringListMap& parentOptions) const;
  // Helper to set up a server connect
  StringStringListMap getBestServerEncoding(const StringStringListMap& valueMap, const ServerOptions& serverOptions) const;

  /// FIXME: move this away from a singlton to something that belongs to a NetworkServer.
  /// Then just have there those encodings that we want to use in this NetworkServer instance.
  static const MessageEncodingRegistry& instance();

private:
  MessageEncodingRegistry();
  ~MessageEncodingRegistry();
};

} // namespace OpenRTI

#endif
