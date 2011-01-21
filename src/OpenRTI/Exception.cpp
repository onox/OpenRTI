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

#include "Exception.h"

#include <string>
#include <sstream>

#include "Export.h"
#include "StringUtils.h"
#include "LogStream.h"

namespace OpenRTI {

Exception::Exception(Type type, const char* reason) :
  _type(type),
  _reason(asciiToUcs(reason))
{
  Log(Assert, Info) << "Fired exception: type = " << type << ", reason = " << _reason << std::endl;
}

Exception::Exception(Type type, const std::wstring& reason) :
  _type(RTIinternalError),
  _reason(reason)
{
  Log(Assert, Info) << "Fired exception: type = " << type << ", reason = " << _reason << std::endl;
}

Exception::~Exception()
{
}

std::string
Exception::getReasonInLocale() const
{
  return ucsToLocale(_reason);
}


RTIinternalError::RTIinternalError(const char* reason) :
  Exception(Exception::RTIinternalError, reason)
{
}

RTIinternalError::RTIinternalError(const std::wstring& reason) :
  Exception(Exception::RTIinternalError, reason)
{
}

RTIinternalError::RTIinternalError(const char* file, unsigned line, const char* reason) :
  Exception(Exception::RTIinternalError, utf8ToUcs(buildAssertMessage(file, line, reason)))
{
}

RTIinternalError::~RTIinternalError()
{
}

std::string
RTIinternalError::buildAssertMessage(const char* file, unsigned line, const char* reason)
{
  std::stringstream stream;
  stream << "Assertion in " << file << " at line " << line;
  if (reason) {
    stream << ": " << reason;
  }
  return stream.str();
}

} // namespace OpenRTI
