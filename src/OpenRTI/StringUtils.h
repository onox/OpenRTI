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

#ifndef OpenRTI_StringUtils_h
#define OpenRTI_StringUtils_h

#include <algorithm>
#include <cstring>
#include <iosfwd>
#include <istream>
#include <list>
#include <map>
#include <ostream>
#include <set>
#include <string>
#include <vector>
#include <wctype.h>
#include <cstdlib>
#include <cstring>
#include <cwchar>

#include "OpenRTIConfig.h"
#include "Types.h"

namespace OpenRTI {

typedef std::set<std::string> StringSet;
typedef std::list<std::string> StringList;
typedef std::vector<std::string> StringVector;
typedef std::map<std::string, std::string> StringMap;
typedef std::map<std::string, StringList> StringStringListMap;

inline bool contains(const StringSet& stringSet, const std::string& string)
{ return stringSet.find(string) != stringSet.end(); }
inline bool contains(const StringList& stringList, const std::string& string)
{ return std::find(stringList.begin(), stringList.end(), string) != stringList.end(); }
inline bool contains(const StringVector& stringVector, const std::string& string)
{ return std::find(stringVector.begin(), stringVector.end(), string) != stringVector.end(); }

inline bool endsWith(const std::string& s, const char* tail)
{ return s.size() - s.rfind(tail) == std::strlen(tail); }

/// Internationalization concept:
/// All internal computations are done in ucs{32,16} aka wchar_t
/// Strings are sent over the wire in utf8, that means all the indices are
/// relatively easy to compute, but transferred size is kind of small.
inline std::string ucsToUtf8(const std::wstring& ucs)
{
  std::string utf;
  // reserve a bit more than needed, string values are usually small in our cases
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

inline std::wstring utf8ToUcs(const std::string& utf)
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

inline std::wstring utf8ToUcs(const char* utf)
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

inline std::wstring localeToUcs(const char* loc)
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

inline std::wstring localeToUcs(const std::string& loc)
{
  return localeToUcs(loc.c_str());
}

inline std::string ucsToLocale(const std::wstring& ucs)
{
  // space for the worst case
  std::vector<char> data(6*ucs.size() + 1);
  std::size_t n = std::wcstombs(&data.front(), ucs.c_str(), data.size());
  if (n == std::size_t(-1))
    return std::string();

  return std::string(&data.front(), n);
}

inline std::string localeToUtf8(const std::string& locale)
{
  return ucsToUtf8(localeToUcs(locale));
}

inline std::string localeToUtf8(const char* locale)
{
  return ucsToUtf8(localeToUcs(locale));
}

inline std::string utf8ToLocale(const std::string& utf)
{
  return ucsToLocale(utf8ToUcs(utf));
}

inline std::string utf8ToLocale(const char* utf)
{
  return ucsToLocale(utf8ToUcs(utf));
}

inline std::string asciiToUtf8(const char* ascii)
{
  std::string utf8;
  if (!ascii)
    return utf8;
  size_t len = std::strlen(ascii);
  utf8.resize(len);
  std::copy(ascii, ascii+len, utf8.begin());
  return utf8;
}

/// Returns a new string as required with RTI13 interfaces
inline char* newUtf8ToLocale(const std::string& utf8)
{
  std::string s = utf8ToLocale(utf8);
  char* data = new char[s.size()+1];
  data[s.size()] = 0;
  return std::strncpy(data, s.c_str(), s.size());
}

inline std::vector<std::string>
split(const std::string& s, const char* c = ", \t\n")
{
  std::vector<std::string> v;
  std::string::size_type p0 = 0;
  std::string::size_type p = s.find_first_of(c);
  while (p != std::string::npos) {
    v.push_back(s.substr(p0, p));
    p0 = s.find_first_not_of(c, p);
    if (p0 == std::string::npos)
      return v;
    p = s.find_first_of(c, p0);
  }
  v.push_back(s.substr(p0, p));
  return v;
}

inline std::vector<std::string>
split(const char* s, const char* c = ", \t\n")
{
  if (!s)
    return std::vector<std::string>();
  return split(std::string(s), c);
}

inline std::string
trim(std::string s, const char* c = " \t\n")
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

inline std::string
trim(const char* s, const char* c = " \t\n")
{
  if (!s)
    return std::string();
  return trim(std::string(s), c);
}

inline std::pair<std::string, std::string>
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

inline std::string
getFilePart(const std::string& path)
{
  std::string::size_type pos = path.rfind('/');
  if (pos == std::string::npos || path.size() <= pos + 1)
    return path;
  return path.substr(pos + 1);
}

inline std::string
getBasePart(const std::string& path)
{
  std::string::size_type pos = path.rfind('/');
  if (pos == std::string::npos)
    return std::string();
  return path.substr(0, pos);
}

inline std::pair<std::string, std::string>
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

inline std::pair<std::string, std::string>
getProtocolRestPair(const std::string& url)
{
  /// Seperate this into:
  /// <protocol>://<address>
  std::string::size_type pos = url.find("://");
  if (pos == std::string::npos)
    return std::make_pair(std::string(), url);

  return std::make_pair(url.substr(0, pos), url.substr(pos + 3));
}

inline StringMap
getStringMapFromUrl(const StringMap& defaults, const std::string& url)
{
  /// FIXME, provide some override for interpreting execution names as url
  StringMap stringMap = defaults;
  // preinitialize, for all the fallthrough paths below
  stringMap["url"] = url;
  stringMap["federationExecutionName"] = url;

  /// FIXME extend that!!!

  /// Seperate this into:
  /// <protocol>://<address>/<path>/<name>
  std::string::size_type pos = url.find("://");
  if (pos == std::string::npos)
    return stringMap;

  std::string protocol = url.substr(0, pos);

  std::string::size_type pos0 = pos + 3;
  if (protocol == "trace" && pos0 < url.size()) {
    stringMap = getStringMapFromUrl(defaults, url.substr(pos0));
    stringMap["traceProtocol"] = stringMap["protocol"];
    stringMap["protocol"] = "trace";
    return stringMap;
  }

  // Find the slach terminating the address field
  pos = url.find('/', pos0);
  if (pos == std::string::npos)
    return stringMap;
  std::string address = url.substr(pos0, pos - pos0);

  pos0 = pos;
  pos = url.rfind('/');
  if (pos == std::string::npos)
    return stringMap;
  std::string path = url.substr(pos0, pos - pos0);
  pos0 = pos + 1;
  if (url.size() <= pos0)
    return stringMap;
  std::string name = url.substr(pos0);

  if (protocol  == "rti") {
    // if (!path.empty() || name.empty()) {
    //   Traits::throwRTIinternalError(std::string("Cannot parse rti://<address>/<federationExecutionName> url \"") + url +
    //                          std::string("\"."));
    // }
    stringMap["protocol"] = "rti";
    stringMap["address"] = address;
    stringMap["federationExecutionName"] = name;

  } else if (protocol == "http") {
    // if (!path.empty() || name.empty()) {
    //   Traits::throwRTIinternalError(std::string("Cannot parse http://<address>/<federationExecutionName> url \"") + url +
    //                          std::string("\"."));
    // }
    stringMap["protocol"] = protocol;
    stringMap["address"] = address;
    stringMap["federationExecutionName"] = name;

  } else if (protocol == "pipe") {
    // if (!address.empty() || path.empty() || name.empty()) {
    //   Traits::throwRTIinternalError(std::string("Cannot parse pipe:///<path to pipe>/<federationExecutionName> url \"") + url +
    //                          std::string("\"."));
    // }
    stringMap["protocol"] = protocol;
    stringMap["address"] = path;
    stringMap["federationExecutionName"] = name;
  } else if (protocol == "thread") {
    // if (!address.empty() || !path.empty() || name.empty()) {
    //   Traits::throwRTIinternalError(std::string("Cannot parse thread:///<federationExecutionName> url \"") + url +
    //                          std::string("\"."));
    // }
    stringMap["protocol"] = protocol;
    stringMap["federationExecutionName"] = name;
  }

  return stringMap;
}

inline std::ostream&
operator<<(std::ostream& stream, const std::wstring& s)
{ return stream << ucsToLocale(s); }

} // namespace OpenRTI

#endif
