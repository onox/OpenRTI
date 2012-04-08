/* -*-c++-*- OpenRTI - Copyright (C) 2009-2012 Mathias Froehlich
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

#include "StringUtils.h"

#include <algorithm>
#include <cstring>
#include <wctype.h>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <sstream>

#include "OpenRTIConfig.h"
#include "Types.h"

namespace OpenRTI {

OPENRTI_API bool
caseCompare(const std::string& l, const char* r)
{
  size_t size = std::strlen(r);
  if (size != l.size())
    return false;
  for (size_t i = 0; i < size; ++i) {
    std::string::value_type cl = l[i];
    char cr = r[i];
    if (cl == cr)
      continue;
    if ('A' <= cr && cr <= 'Z') {
      if (cl == (cr + 'a' - 'A'))
        continue;
    } else if ('a' <= cr && cr <= 'z') {
      if (cl == (cr + 'A' - 'a'))
        continue;
    }
    return false;
  }
  return true;
}

OPENRTI_API bool
endsWith(const std::string& s, const char* tail)
{
  return s.size() - s.rfind(tail) == std::strlen(tail);
}

OPENRTI_API bool
matchExtension(const std::string& name, const char* extension)
{
  size_t extensionSize = std::strlen(extension);
  if (name.size() < extensionSize)
    return false;
  for (size_t i = name.size() - extensionSize, j = 0; j < extensionSize; ++i, ++j) {
    std::string::value_type s = name[i];
    char e = extension[j];
    if (s == e)
      continue;
    if ('A' <= e && e <= 'Z') {
      if (s == (e + 'a' - 'A'))
        continue;
    } else if ('a' <= e && e <= 'z') {
      if (s == (e + 'A' - 'a'))
        continue;
    }
    return false;
  }
  return true;
}

OPENRTI_API std::string
ucsToUtf8(const std::wstring& ucs)
{
  std::string utf;
  // reserve a bit more than needed, string values are usually small
  utf.reserve(6*ucs.size());
  for (std::wstring::const_iterator i = ucs.begin(); i != ucs.end(); ++i) {
    uint32_t wc = *i;
    if (wc <= 0x7f) {
      utf.push_back(wc);
    } else if (wc <= 0x1fff + 0x80) {
      wc -= 0x80;
      utf.push_back(0x80 | (wc >> 7));
      utf.push_back(0x80 | (wc & 0x7f));
    } else if (wc <= 0x7ffff + 0x0002080) {
      wc -= 0x0002080;
      utf.push_back(0xc0 | (wc >> 14));
      utf.push_back(0x80 | ((wc >> 7) & 0x7f));
      utf.push_back(0x80 | (wc & 0x7f));
    } else if (wc <= 0x1ffffff + 0x0082080) {
      wc -= 0x0082080;
      utf.push_back(0xe0 | (wc >> 21));
      utf.push_back(0x80 | ((wc >> 14) & 0x7f));
      utf.push_back(0x80 | ((wc >> 7) & 0x7f));
      utf.push_back(0x80 | (wc & 0x7f));
    } else {
      wc -= 0x2082080;
      utf.push_back(0xf0 | (wc >> 28));
      utf.push_back(0x80 | ((wc >> 21) & 0x7f));
      utf.push_back(0x80 | ((wc >> 14) & 0x7f));
      utf.push_back(0x80 | ((wc >> 7) & 0x7f));
      utf.push_back(0x80 | (wc & 0x7f));
    }
  }
  return utf;
}

OPENRTI_API std::wstring
utf8ToUcs(const std::string& utf)
{
  std::wstring ucs;
  // reserve a bit more than needed, string values are usually small in our cases
  ucs.reserve(utf.size());
  std::string::const_iterator i = utf.begin();
  while (i != utf.end()) {
    std::wstring::value_type c = 0xff & std::wstring::value_type(*i++);
    if (c & 0x80) {
      c &= 0x7f;
      uint32_t mask = 0x40;
      while ((c & mask) && i != utf.end()) {
        c &= std::wstring::value_type(~mask);
        mask <<= 5;
        c <<= 6;
        c |= (0x3f & (*i++));
      }
    }
    ucs.push_back(c);
  }
  return ucs;
}

OPENRTI_API std::wstring
utf8ToUcs(const char* utf)
{
  if (!utf)
    return std::wstring();

  std::wstring ucs;
  size_t size = std::strlen(utf);
  // reserve a bit more than needed, string values are usually small in our cases
  ucs.reserve(size);
  size_t i = 0;
  while (i < size) {
    std::wstring::value_type c = 0xff & std::wstring::value_type(utf[i++]);
    if (c & 0x80) {
      c &= 0x7f;
      uint32_t mask = 0x40;
      while ((c & mask) && i < size) {
        c &= std::wstring::value_type(~mask);
        mask <<= 5;
        c <<= 6;
        c |= (0x3f & (utf[i++]));
      }
    }
    ucs.push_back(c);
  }
  return ucs;
}

OPENRTI_API std::wstring
localeToUcs(const char* loc)
{
  if (!loc)
    return std::wstring();

  std::size_t n = std::strlen(loc);
  if (n == 0)
    return std::wstring();

  std::vector<wchar_t> data(n+1);
  n = std::mbstowcs(&data.front(), loc, n+1);
  if (n == std::size_t(-1))
    return std::wstring();

  return std::wstring(&data.front(), n);
}

OPENRTI_API std::string
ucsToLocale(const std::wstring& ucs)
{
  std::size_t n = std::wcstombs(NULL, ucs.c_str(), 0);
  if (n == std::size_t(-1))
    return std::string();

  std::vector<char> data(n + 1);
  n = std::wcstombs(&data.front(), ucs.c_str(), data.size());
  if (n == std::size_t(-1))
    return std::string();

  return std::string(&data.front(), n);
}

OPENRTI_API std::string
asciiToUtf8(const char* ascii)
{
  std::string utf8;
  if (!ascii)
    return utf8;
  size_t len = std::strlen(ascii);
  utf8.resize(len);
  std::copy(ascii, ascii+len, utf8.begin());
  return utf8;
}

OPENRTI_API std::vector<std::string>
split(const std::string& s, const char* c)
{
  std::vector<std::string> v;
  std::string::size_type p0 = 0;
  std::string::size_type p = s.find_first_of(c);
  while (p != std::string::npos) {
    v.push_back(s.substr(p0, p - p0));
    p0 = s.find_first_not_of(c, p);
    if (p0 == std::string::npos)
      return v;
    p = s.find_first_of(c, p0);
  }
  v.push_back(s.substr(p0, p - p0));
  return v;
}

OPENRTI_API std::string
trim(std::string s, const char* c)
{
  std::string::size_type p = s.find_last_not_of(c);
  if (p == std::string::npos)
    return std::string();
  if (p + 1 < s.size())
    s.resize(p + 1);
  p = s.find_first_not_of(c);
  if (0 < p && p != std::string::npos)
    s.erase(0, p);
  return s;
}

OPENRTI_API std::string
fqClassName(const StringVector& name)
{
  std::stringstream ss;
  for (StringVector::const_iterator i = name.begin(); i != name.end(); ++i) {
    if (i != name.begin())
      ss << ".";
    ss << *i;
  }
  return ss.str();
}

OPENRTI_API std::pair<std::string, std::string>
parseInetAddress(const std::string& address)
{
  std::string hostname(OpenRTI_DEFAULT_HOST_STRING);
  std::string port(OpenRTI_DEFAULT_PORT_STRING);

  if (!address.empty()) {
    if (address[0] == '[') {
      // Ipv6 mode, '[address]:port'
      std::string::size_type pos = address.rfind(']');
      if (pos == std::string::npos) {
        hostname = address.substr(1);
      } else {
        hostname = address.substr(1, pos - 1);
        pos = address.find(':', pos);
        if (pos != std::string::npos) {
          port = address.substr(pos + 1);
        }
      }
    } else {
      std::string::size_type pos = address.rfind(':');
      // this if means 'we have 2 times ":"'
      if (pos != std::string::npos && pos != address.find(':')) {
        // Ipv6 mode 'address'
        hostname = address;
      } else {
        // Ipv4 mode 'address:port'
        std::string::size_type pos = address.rfind(':');
        if (pos == std::string::npos) {
          hostname = address;
        } else {
          hostname = address.substr(0, pos);
          port = address.substr(pos + 1);
        }
      }
    }
  }

  return std::make_pair(hostname, port);
}

OPENRTI_API std::string
getFilePart(const std::string& path)
{
  std::string::size_type pos = path.rfind('/');
  if (pos == std::string::npos || path.size() <= pos + 1)
    return path;
  return path.substr(pos + 1);
}

OPENRTI_API std::string
getBasePart(const std::string& path)
{
  std::string::size_type pos = path.rfind('/');
  if (pos == std::string::npos)
    return std::string();
  return path.substr(0, pos);
}

OPENRTI_API std::pair<std::string, std::string>
getProtocolAddressPair(const std::string& url)
{
  std::string protocol;
  std::string address;

  /// Seperate this into:
  /// <protocol>://<address>/...
  std::string::size_type pos = url.find("://");
  if (pos == std::string::npos)
    return std::make_pair(protocol, address);

  protocol = url.substr(0, pos);

  std::string::size_type pos0 = pos + 3;
  // Find the slach terminating the address field
  pos = url.find('/', pos0);
  if (pos == std::string::npos)
    return std::make_pair(protocol, address);

  address = url.substr(pos0, pos - pos0);
  return std::make_pair(protocol, address);
}

OPENRTI_API std::pair<std::string, std::string>
getProtocolRestPair(const std::string& url)
{
  /// Seperate this into:
  /// <protocol>://<address>
  std::string::size_type pos = url.find("://");
  if (pos == std::string::npos)
    return std::make_pair(std::string(), url);

  return std::make_pair(url.substr(0, pos), url.substr(pos + 3));
}

} // namespace OpenRTI
