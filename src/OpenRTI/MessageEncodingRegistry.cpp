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

#include "MessageEncodingRegistry.h"

#include "LogStream.h"
#include "ServerOptions.h"
#include "SocketEventDispatcher.h"
#include "StringUtils.h"

#include "TightBE1MessageEncoding.h"

namespace OpenRTI {

SharedPtr<AbstractMessageEncoding>
MessageEncodingRegistry::getEncoding(const std::string& encodingName) const
{
  if (encodingName == "TightBE1")
    return new TightBE1MessageEncoding;
  return 0;
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
