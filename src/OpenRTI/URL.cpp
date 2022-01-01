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

#include "URL.h"

#include <sstream>

#include "StringUtils.h"

namespace OpenRTI {

static char
hexChar(unsigned char c)
{
  if (c < 10)
    return '0' + c;
  else
    return 'A' - 10 + c;
}

static void
quote(std::string& quoted, unsigned char c)
{
  quoted.push_back('%');
  quoted.push_back(hexChar((c >> 4) & 0xf));
  quoted.push_back(hexChar(c & 0xf));
}

static std::string
quote(const std::string& unquoted, bool pathMode = false)
{
  std::string quoted;
  quoted.reserve(2*unquoted.size());
  for (std::string::const_iterator i = unquoted.begin(); i != unquoted.end(); ++i) {
    switch (*i) {
    case ' ':
    case '!':
    case '#':
    case '$':
    case '\'':
    case '(':
    case ')':
    case '*':
    case '+':
    case ',':
    case ':':
    case ';':
    case '=':
    case '?':
    case '@':
    case '[':
    case ']':
      quote(quoted, *i);
      break;
    case '/':
      if (pathMode)
        quoted.push_back(*i);
      else
        quote(quoted, *i);
      break;
    default:
      if (((unsigned char)*i) <= 127) {
        quoted.push_back(*i);
      } else {
        quote(quoted, *i);
      }
      break;
    }
  }
  return quoted;
}

static std::string
unquote(const std::string& quoted)
{
  std::string unquoted;
  unquoted.reserve(quoted.size());
  unsigned quotePos = 0;
  for (std::string::const_iterator i = quoted.begin(); i != quoted.end(); ++i) {
    if (quotePos) {
      if ('0' <= *i && *i <= '9') {
        (*unquoted.rbegin()) += (*i - '0');
      } else if ('A' <= *i && *i <= 'F') {
        (*unquoted.rbegin()) += (*i - 'A' + 10);
      } else if ('a' <= *i && *i <= 'f') {
        (*unquoted.rbegin()) += (*i - 'a' + 10);
      } else {
        // be tolerant and assume that the user really meant %
        (*unquoted.rbegin()) = '%';
        i -= quotePos;
        quotePos = 0;
      }
      if (quotePos == 1) {
        (*unquoted.rbegin()) <<= 4;
        ++quotePos;
      } else if (quotePos == 2) {
        quotePos = 0;
      }
    } else if (*i == '%') {
      unquoted.push_back(0);
      ++quotePos;
    } else {
      unquoted.push_back(*i);
    }
  }
  return unquoted;
}

URL::URL()
{
}

URL::URL(const URL& url) :
  _protocol(url._protocol),
  _host(url._host),
  _service(url._service),
  _path(url._path),
  _query(url._query),
  _ref(url._ref)
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
  std::string::size_type addressBegin = s.find("://");
  if (addressBegin != std::string::npos) {
    url._protocol = unquote(s.substr(0, addressBegin));
    // skip '://' and let pos point to the begin of the <address> part
    addressBegin += 3;
  } else {
    // pos points to the potential begin of the <address> part
    addressBegin = 0;
  }

  // Find the slach terminating the address field
  std::string::size_type pathBegin = s.find('/', addressBegin);

  // Seperate out host and service
  StringPair addressPortPair;
  addressPortPair = parseInetAddress(s.substr(addressBegin, pathBegin - addressBegin));
  url._host = unquote(addressPortPair.first);
  url._service = unquote(addressPortPair.second);

  if (pathBegin != std::string::npos) {
    // Find a query part past a ?
    std::string::size_type queryBegin = s.find('?', pathBegin);
    std::string::size_type refBegin = s.find('#', pathBegin);
    if (queryBegin != std::string::npos) {
      StringVector queryList = split(s.substr(queryBegin + 1, refBegin - queryBegin - 1), "&;");
      for (StringVector::const_iterator i = queryList.begin(); i != queryList.end(); ++i) {
        std::string::size_type pos = i->find('=');
        if (pos != std::string::npos)
          url._query.push_back(StringPair(unquote(i->substr(0, pos)), unquote(i->substr(pos + 1))));
        else
          url._query.push_back(StringPair(unquote(*i), std::string()));
      }
    }
    if (refBegin != std::string::npos) {
      url._ref = unquote(s.substr(refBegin + 1));
    }
    url._path = unquote(s.substr(pathBegin, std::min(queryBegin, refBegin) - pathBegin));
  }

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

const StringPairVector&
URL::getQuery() const
{
  return _query;
}

void
URL::setQuery(const StringPairVector& query)
{
  _query = query;
}

std::size_t
URL::getNumQueries() const
{
  return _query.size();
}

std::size_t
URL::getQueryIndex(const std::string& key) const
{
  for (std::size_t i = 0; i < _query.size(); ++i) {
    if (key == _query[i].first)
      return i;
  }
  return ~std::size_t(0);
}

StringPair
URL::getQuery(std::size_t index) const
{
  if (_query.size() <= index)
    return StringPair();
  return _query[index];
}

std::string
URL::getQuery(const std::string& key) const
{
  for (StringPairVector::const_iterator i = _query.begin(); i != _query.end(); ++i) {
    if (key == i->first)
      return i->second;
  }
  return std::string();
}

const std::string&
URL::getRef() const
{
  return _ref;
}

void
URL::setRef(const std::string& ref)
{
  _ref = ref;
}

std::string
URL::str() const
{
  std::stringstream ss;
  if (!_protocol.empty())
    ss << _protocol << ":";

  ss << "//" << _host;
  if (!_service.empty())
    ss << ":" << _service;

  ss << quote(_path, true);

  for (StringPairVector::const_iterator i = _query.begin(); i != _query.end(); ++i) {
    if (i == _query.begin()) {
      ss << "?";
    } else {
      ss << "&";
    }
    ss << quote(i->first);
    if (!i->second.empty())
      ss << "=" << quote(i->second);
  }

  if (!_ref.empty())
    ss << "#" << quote(_ref);

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
  if (_path != url._path)
    return false;
  if (_query != url._query)
    return false;
  return _ref == url._ref;
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
  if (_path != url._path)
    return true;
  if (_query != url._query)
    return true;
  return _ref != url._ref;
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
  if (_path < url._path)
    return true;
  else if (url._path < _path)
    return false;
  if (_query < url._query)
    return true;
  else if (url._query < _query)
    return false;
  return _ref < url._ref;
}

} // namespace OpenRTI
