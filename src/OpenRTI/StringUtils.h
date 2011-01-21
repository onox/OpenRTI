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

typedef std::set<std::wstring> StringSet;
typedef std::list<std::wstring> StringList;
typedef std::vector<std::wstring> StringVector;
typedef std::map<std::wstring, std::wstring> StringMap;
typedef std::map<std::wstring, StringList> StringStringListMap;

inline bool contains(const StringSet& stringSet, const std::wstring& string)
{ return stringSet.find(string) != stringSet.end(); }
inline bool contains(const StringList& stringList, const std::wstring& string)
{ return std::find(stringList.begin(), stringList.end(), string) != stringList.end(); }
inline bool contains(const StringVector& stringVector, const std::wstring& string)
{ return std::find(stringVector.begin(), stringVector.end(), string) != stringVector.end(); }

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

inline std::wstring toLower(const std::wstring& mixed)
{
  std::wstring lower;
  lower.reserve(mixed.size());
  for (std::wstring::const_iterator i = mixed.begin(); i != mixed.end(); ++i)
    lower.push_back(tolower(*i));
  return lower;
}

inline std::wstring asciiToUcs(const char* ascii)
{
  std::wstring ucs;
  if (!ascii)
    return ucs;
  size_t len = std::strlen(ascii);
  ucs.resize(len);
  std::copy(ascii, ascii+len, ucs.begin());
  return ucs;
}

/// Returns a new string as required with RTI13 interfaces
inline char* newUcsToLocale(const std::wstring& ws)
{
  std::string s = ucsToLocale(ws);
  char* data = new char[s.size()+1];
  data[s.size()] = 0;
  return std::strncpy(data, s.c_str(), s.size());
}

inline std::vector<std::wstring>
split(const std::wstring& s, const wchar_t* c = L", \t\n")
{
  std::vector<std::wstring> v;
  std::wstring::size_type p0 = 0;
  std::wstring::size_type p = s.find_first_of(c);
  while (p != std::wstring::npos) {
    v.push_back(s.substr(p0, p));
    p0 = s.find_first_not_of(c, p);
    if (p0 == std::wstring::npos)
      return v;
    p = s.find_first_of(c, p0);
  }
  v.push_back(s.substr(p0, p));
  return v;
}

inline std::wstring
trim(std::wstring s, const wchar_t* c = L" \t\n")
{
  std::wstring::size_type p = s.find_last_not_of(c);
  if (p == std::wstring::npos)
    return std::wstring();
  if (p + 1 < s.size())
    s.resize(p + 1);
  p = s.find_first_not_of(c);
  if (0 < p && p != std::wstring::npos)
    s.erase(0, p);
  return s;
}

inline std::pair<std::wstring, std::wstring>
parseInetAddress(const std::wstring& address)
{
  std::wstring hostname(localeToUcs(OpenRTI_DEFAULT_HOST_STRING));
  std::wstring port(localeToUcs(OpenRTI_DEFAULT_PORT_STRING));

  if (!address.empty()) {
    if (address[0] == '[') {
      // Ipv6 mode, '[address]:port'
      std::wstring::size_type pos = address.rfind(']');
      if (pos == std::wstring::npos) {
        hostname = address;
      } else {
        hostname = address.substr(1, pos - 1);
        pos = address.find(':', pos);
        if (pos != std::wstring::npos) {
          port = address.substr(pos + 1);
        }
      }
    } else if (2 <= std::count(address.begin(), address.end(), ':')) {
      // Ipv6 mode 'address'
      hostname = address;
    } else {
      // Ipv4 mode 'address:port'
      std::wstring::size_type pos = address.rfind(':');
      if (pos == std::wstring::npos) {
        hostname = address;
      } else {
        hostname = address.substr(0, pos);
        port = address.substr(pos + 1);
      }
    }
  }

  return std::make_pair(hostname, port);
}

inline std::wstring
getFilePart(const std::wstring& path)
{
  std::wstring::size_type pos = path.rfind('/');
  if (pos == std::wstring::npos || path.size() <= pos + 1)
    return path;
  return path.substr(pos + 1);
}

inline std::wstring
getBasePart(const std::wstring& path)
{
  std::wstring::size_type pos = path.rfind('/');
  if (pos == std::wstring::npos)
    return std::wstring();
  return path.substr(0, pos);
}

inline std::pair<std::wstring, std::wstring>
getProtocolAddressPair(const std::wstring& url)
{
  std::wstring protocol;
  std::wstring address;

  /// Seperate this into:
  /// <protocol>://<address>/...
  std::wstring::size_type pos = url.find(L"://");
  if (pos == std::wstring::npos)
    return std::make_pair(protocol, address);

  protocol = url.substr(0, pos);

  std::wstring::size_type pos0 = pos + 3;
  // Find the slach terminating the address field
  pos = url.find('/', pos0);
  if (pos == std::wstring::npos)
    return std::make_pair(protocol, address);

  address = url.substr(pos0, pos - pos0);
  return std::make_pair(protocol, address);
}

inline StringMap
getStringMapFromUrl(const StringMap& defaults, const std::wstring& url)
{
  /// FIXME, provide some override for interpreting execution names as url
  StringMap stringMap = defaults;
  // preinitialize, for all the fallthrough paths below
  stringMap[L"url"] = url;
  stringMap[L"federationExecutionName"] = url;

  /// FIXME extend that!!!

  /// Seperate this into:
  /// <protocol>://<address>/<path>/<name>
  std::wstring::size_type pos = url.find(L"://");
  if (pos == std::wstring::npos)
    return stringMap;

  std::wstring protocol = url.substr(0, pos);

  std::wstring::size_type pos0 = pos + 3;
  if (protocol == L"trace" && pos0 < url.size()) {
    stringMap = getStringMapFromUrl(defaults, url.substr(pos0));
    stringMap[L"traceProtocol"] = stringMap[L"protocol"];
    stringMap[L"protocol"] = L"trace";
    return stringMap;
  }

  // Find the slach terminating the address field
  pos = url.find('/', pos0);
  if (pos == std::wstring::npos)
    return stringMap;
  std::wstring address = url.substr(pos0, pos - pos0);

  pos0 = pos;
  pos = url.rfind('/');
  if (pos == std::wstring::npos)
    return stringMap;
  std::wstring path = url.substr(pos0, pos - pos0);
  pos0 = pos + 1;
  if (url.size() <= pos0)
    return stringMap;
  std::wstring name = url.substr(pos0);

  if (protocol  == L"rti") {
    // if (!path.empty() || name.empty()) {
    //   Traits::throwRTIinternalError(std::wstring(L"Cannot parse rti://<address>/<federationExecutionName> url \"") + url +
    //                          std::wstring(L"\"."));
    // }
    stringMap[L"protocol"] = L"rti";
    stringMap[L"address"] = address;
    stringMap[L"federationExecutionName"] = name;

  } else if (protocol == L"http") {
    // if (!path.empty() || name.empty()) {
    //   Traits::throwRTIinternalError(std::wstring(L"Cannot parse http://<address>/<federationExecutionName> url \"") + url +
    //                          std::wstring(L"\"."));
    // }
    stringMap[L"protocol"] = protocol;
    stringMap[L"address"] = address;
    stringMap[L"federationExecutionName"] = name;

  } else if (protocol == L"pipe") {
    // if (!address.empty() || path.empty() || name.empty()) {
    //   Traits::throwRTIinternalError(std::wstring(L"Cannot parse pipe:///<path to pipe>/<federationExecutionName> url \"") + url +
    //                          std::wstring(L"\"."));
    // }
    stringMap[L"protocol"] = protocol;
    stringMap[L"address"] = path;
    stringMap[L"federationExecutionName"] = name;
  } else if (protocol == L"thread") {
    // if (!address.empty() || !path.empty() || name.empty()) {
    //   Traits::throwRTIinternalError(std::wstring(L"Cannot parse thread:///<federationExecutionName> url \"") + url +
    //                          std::wstring(L"\"."));
    // }
    stringMap[L"protocol"] = protocol;
    stringMap[L"federationExecutionName"] = name;
  }

  return stringMap;
}

inline std::ostream&
operator<<(std::ostream& stream, const std::wstring& s)
{ return stream << ucsToLocale(s); }

inline std::wostream&
operator<<(std::wostream& stream, const std::string& s)
{ return stream << localeToUcs(s); }

} // namespace OpenRTI

#endif
