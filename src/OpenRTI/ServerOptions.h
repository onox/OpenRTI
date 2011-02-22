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

#ifndef OpenRTI_ServerOptions_h
#define OpenRTI_ServerOptions_h

#include "Export.h"
#include "Message.h"
#include "Referenced.h"

namespace OpenRTI {

// Just a set of options that might be configurable at some time from outside.
// May be add some callback mechanism here: depending on the subnet??
class OPENRTI_API ServerOptions : public Referenced {
public:
  ServerOptions() :
    _preferCompression(true) // Default to compression for now FIXME
  { }

  const std::wstring& getServerName() const
  { return _serverName; }
  void setServerName(const std::wstring& serverName)
  {
    _serverName = serverName;
    StringStringListMap::iterator i;
    i = _optionMap.insert(StringStringListMap::value_type(L"serverPath", StringList())).first;
    i->second = _parentServerPath;
    i->second.push_back(serverName);
    _setServerPath(i->second);
  }

  void setParentOptionMap(const StringStringListMap& optionMap)
  {
    _optionMap = optionMap;
    StringStringListMap::iterator i;
    i = _optionMap.insert(StringStringListMap::value_type(L"serverPath", StringList())).first;
    _parentServerPath = i->second;
    i->second.push_back(_serverName);
    _setServerPath(i->second);
  }

  const std::wstring& getServerPath() const
  { return _serverPath; }
private:
  void _setServerPath(const StringList& serverPath)
  {
    _serverPath.clear();
    for (StringList::const_iterator i = serverPath.begin(); i != serverPath.end(); ++i) {
      _serverPath.append(L"/");
      _serverPath.append(*i);
    }
  }
public:

  /// The servers name
  std::wstring _serverName;

  /// For pretty printing
  std::wstring _serverPath;

  /// The server path as given by the parent
  StringList _parentServerPath;

  /// What is transferred to a child server at connection setup
  StringStringListMap _optionMap;

  /// Connection encoding settings
  bool _preferCompression;
  // bool _preferTightEncoding;
  // bool _preferServerSideByteSwap;

  /// Connection options
  // bool _enableUDP;
  // bool _enableMulticast;
  // bool _enableRDP;

  // OpenRTI child servers can just continue working on its sub branch of the tree if the root
  // server dies. This controls if we better continue working as best as can or if we also close
  // the client connections then.
  // bool _closeOnLostRootServer;

  /// Federate crash behavior
  // ResignAction _resignBehaviorOnLostFederate;
};

} // namespace OpenRTI

#endif
