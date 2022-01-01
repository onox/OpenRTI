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

#ifndef OpenRTI_URL_h
#define OpenRTI_URL_h

#include <string>

#include "Export.h"
#include "StringUtils.h"

namespace OpenRTI {

/// Implement a traditional uniform resource locator
/// protocol://host:service/path?query#ref
class OPENRTI_API URL {
public:
  URL();
  URL(const URL& url);
  ~URL();

  static URL fromUrl(const std::string& url);
  static URL fromProtocolAddress(const std::string& protocol, const std::string& address);
  static URL fromProtocolPath(const std::string& protocol, const std::string& path);

  const std::string& getProtocol() const;
  void setProtocol(const std::string& protocol);

  const std::string& getHost() const;
  void setHost(const std::string& host);

  const std::string& getService() const;
  void setService(const std::string& service);

  const std::string& getPath() const;
  void setPath(const std::string& path);

  const StringPairVector& getQuery() const;
  void setQuery(const StringPairVector& query);
  std::size_t getNumQueries() const;
  std::size_t getQueryIndex(const std::string& key) const;
  StringPair getQuery(std::size_t index) const;
  std::string getQuery(const std::string& key) const;

  const std::string& getRef() const;
  void setRef(const std::string& ref);

  std::string str() const;

  bool operator==(const URL& url) const;
  bool operator!=(const URL& url) const;
  bool operator<(const URL& url) const;

private:
  std::string _protocol;
  std::string _host;
  std::string _service;
  std::string _path;
  StringPairVector _query;
  std::string _ref;
};

} // namespace OpenRTI

#endif
