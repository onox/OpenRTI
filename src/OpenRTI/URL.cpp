/* -*-c++-*- OpenRTI - Copyright (C) 2009-2013 Mathias Froehlich
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

#include "URL.h"

#include <sstream>

#include "StringUtils.h"

namespace OpenRTI {

URL::URL()
{
}

URL::URL(const URL& url) :
  _protocol(url._protocol),
  _host(url._host),
  _service(url._service),
  _path(url._path)
{
}

URL::~URL()
{
}

URL
URL::fromUrl(const std::string& s)
{
  URL url;

  /// Seperate this into:
  /// <protocol>://<address>/...
  std::string::size_type pos = s.find("://");
  if (pos != std::string::npos) {
    url._protocol = s.substr(0, pos);
    // skip '://' and let pos point to the begin of the <address> part
    pos += 3;
  } else {
    // pos points to the potential begin of the <address> part
    pos = 0;
  }

  std::string::size_type addressBegin = pos;
  // Find the slach terminating the address field
  pos = s.find('/', addressBegin);
  if (pos != std::string::npos) {
    url._path = s.substr(pos);
  } else {
    pos = s.size();
  }

  std::pair<std::string, std::string> addressPortPair;
  addressPortPair = parseInetAddress(s.substr(addressBegin, pos - addressBegin));
  url._host = addressPortPair.first;
  url._service = addressPortPair.second;

  return url;
}

URL
URL::fromProtocolAddress(const std::string& protocol, const std::string& address)
{
  URL url;

  url._protocol = protocol;

  std::pair<std::string, std::string> addressPortPair;
  addressPortPair = parseInetAddress(address);
  url._host = addressPortPair.first;
  url._service = addressPortPair.second;

  return url;
}

URL
URL::fromProtocolPath(const std::string& protocol, const std::string& path)
{
  URL url;
  url._protocol = protocol;
  url._path = path;
  return url;
}

const std::string&
URL::getProtocol() const
{
  return _protocol;
}

void
URL::setProtocol(const std::string& protocol)
{
  _protocol = protocol;
}

const std::string&
URL::getHost() const
{
  return _host;
}

void
URL::setHost(const std::string& host)
{
  _host = host;
}

const std::string&
URL::getService() const
{
  return _service;
}

void
URL::setService(const std::string& service)
{
  _service = service;
}

const std::string&
URL::getPath() const
{
  return _path;
}

void
URL::setPath(const std::string& path)
{
  _path = path;
}

std::string
URL::str() const
{
  std::stringstream ss;
  ss << _protocol << "://" << _host;
  if (!_service.empty())
    ss << ":" << _service;
  ss << "/" << _path;
  return ss.str();
}

bool
URL::operator==(const URL& url) const
{
  if (_protocol != url._protocol)
    return false;
  if (_host != url._host)
    return false;
  if (_service != url._service)
    return false;
  return _path == url._path;
}

bool
URL::operator!=(const URL& url) const
{
  if (_protocol != url._protocol)
    return true;
  if (_host != url._host)
    return true;
  if (_service != url._service)
    return true;
  return _path != url._path;
}

bool
URL::operator<(const URL& url) const
{
  if (_protocol < url._protocol)
    return true;
  else if (url._protocol < _protocol)
    return false;
  if (_host < url._host)
    return true;
  else if (url._host < _host)
    return false;
  if (_service < url._service)
    return true;
  else if (url._service < _service)
    return false;
  return _path < url._path;
}

} // namespace OpenRTI
