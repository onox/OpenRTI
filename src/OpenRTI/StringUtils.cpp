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

#include "StringUtils.h"

#include <algorithm>
#include <cstring>
#include <wctype.h>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <limits>
#include <sstream>

#include "OpenRTIConfig.h"
#include "Types.h"

namespace OpenRTI {

template<typename D, typename S>
static void
utfToUtf(std::basic_string<D>& dst, const S* srcBegin, const S* srcEnd)
{
  const unsigned srcBits = std::numeric_limits<S>::digits + std::numeric_limits<S>::is_signed;
  // The below does
  // const uint32_t srcValueMask = (uint32_t(1) << srcBits) - 1;
  // without the risk of having a shift over the whole width of the datatype which behaves undefined
  const uint32_t srcValueMask = (uint32_t(1) << (srcBits - 1)) ^ ((uint32_t(1) << (srcBits - 1)) - 1);
  const uint32_t srcUpperBit = uint32_t(1) << (srcBits - 1);
  const uint32_t srcSecondBit = uint32_t(1) << (srcBits - 2);
  const uint32_t srcContinuationMask = srcSecondBit - 1;

  const unsigned dstBits = std::numeric_limits<D>::digits + std::numeric_limits<D>::is_signed;
  const uint32_t dstUpperBit = uint32_t(1) << (dstBits - 1);
  const unsigned dstContinuationBits = dstBits - 2;
  const uint32_t dstContinuationMask = (uint32_t(1) << dstContinuationBits) - 1;

  for (const S* i = srcBegin; i != srcEnd; ++i) {
    uint32_t cp = (srcValueMask & uint32_t(*i));
    if (cp & srcUpperBit) {
      cp &= ~srcUpperBit;
      uint32_t mask = srcSecondBit;
      while ((cp & mask) && ++i != srcEnd) {
        cp &= ~mask;
        mask <<= (srcBits - 3);
        cp <<= (srcBits - 2);
        cp |= (srcContinuationMask & *i);
      }
    }

    // now cp contains the codepoint value

    // in case of a single character encodable codepoint
    if (cp < dstUpperBit) {
      dst.push_back(D(cp));
    } else {
      // Counts the number of continuation bytes
      unsigned count = 0;
      uint32_t tmp = (cp >> dstContinuationBits);
      while (tmp) {
        tmp >>= (dstContinuationBits - 1);
        ++count;
      }

      // The leading byte containing the count encoded here
      dst.push_back(D((~((dstUpperBit >> count) - 1)) | (cp >> count*dstContinuationBits)));
      // and all the continuation bytes
      do {
        --count;
        dst.push_back(D(dstUpperBit | ((cp >> count*dstContinuationBits) & dstContinuationMask)));
      } while (count);
    }
  }
}

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
  utfToUtf(utf, ucs.data(), ucs.data() + ucs.size());
  return utf;
}

OPENRTI_API std::wstring
utf8ToUcs(const std::string& utf)
{
  std::wstring ucs;
  utfToUtf(ucs, utf.data(), utf.data() + utf.size());
  return ucs;
}

OPENRTI_API std::wstring
utf8ToUcs(const char* utf)
{
  if (!utf)
    return std::wstring();

  std::wstring ucs;
  utfToUtf(ucs, utf, utf + std::strlen(utf));
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
  if (ucs.empty())
    return std::string();

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

OPENRTI_API VariableLengthData
utf8ToUtf16BE(const std::string& utf8, bool bom)
{
  VariableLengthData utf16;
  utf16.reserve(2*utf8.size() + 2);

  // The BOM
  if (bom) {
    utf16.resize(2);
    utf16.setAlignedUInt16BE(0xfeff, 0);
  }

  size_t i = 0;
  while (i < utf8.size()) {
    uint32_t c = 0xff & uint32_t(utf8[i++]);
    if (c & 0x80) {
      c &= 0x7f;
      uint32_t mask = 0x40;
      while ((c & mask) && i < utf8.size()) {
        c &= uint32_t(~mask);
        mask <<= 5;
        c <<= 6;
        c |= (0x3f & (utf8[i++]));
      }
    }

    if (c <= 0x7fff) {
      std::size_t offset = utf16.size();
      utf16.resize(offset + 2);
      utf16.setAlignedUInt16BE(c, offset);
    } else if (c <= 0x1fffffff) {
      std::size_t offset = utf16.size();
      utf16.resize(offset + 4);
      utf16.setAlignedUInt16BE(0xc000 | ((c >> 15) & 0x1fff), offset);
      utf16.setAlignedUInt16BE(0x8000 | (c & 0x3fff), offset + 2);
    } else {
      std::size_t offset = utf16.size();
      utf16.resize(offset + 6);
      utf16.setAlignedUInt16BE(0xe000 | ((c >> 30) & 0x0fff), offset);
      utf16.setAlignedUInt16BE(0x8000 | ((c >> 15) & 0x3fff), offset + 2);
      utf16.setAlignedUInt16BE(0x8000 | (c & 0x3fff), offset + 4);
    }
  }

  return utf16;
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
  std::string hostname;
  std::string port;

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

} // namespace OpenRTI
