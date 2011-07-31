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

#ifndef OpenRTI_Exception_h
#define OpenRTI_Exception_h

#include <exception>
#include <string>

#include "Export.h"

namespace OpenRTI {

class OPENRTI_API Exception : public std::exception {
public:
  virtual ~Exception() throw();

  const char* what() const throw();
  const std::string& getReason() const throw();

protected:
  Exception(const char* type, const char* reason);
  Exception(const char* type, const std::string& reason);
  Exception(const char* type, const std::wstring& reason);

private:
  std::string _reason;
};

class OPENRTI_API RTIinternalError : public Exception {
public:
  RTIinternalError(const char* reason = 0);
  RTIinternalError(const std::string& reason);
  RTIinternalError(const char* file, unsigned line, const char* reason = 0);
  virtual ~RTIinternalError() throw();

private:
  static std::string buildAssertMessage(const char* file, unsigned line, const char* reason = 0);
};

#ifdef NDEBUG
#define OpenRTIAssert(expr) do { } while(0)
#else
#define OpenRTIAssert(expr) if (!(expr)) throw OpenRTI::RTIinternalError(__FILE__, __LINE__, #expr)
#endif

#define RTI_EXCEPTION(name) \
class OPENRTI_API name : public Exception { \
public: \
  name(const char* reason = 0) : Exception( #name, reason) { }  \
  name(const std::string& reason) : Exception( #name, reason) { } \
  name(const std::wstring& reason) : Exception( #name, reason) { } \
  virtual ~name() throw() { } \
};

RTI_EXCEPTION(ResourceError) // unrecoverable errors when we run out of resources
RTI_EXCEPTION(TransportError) // unrecoverable errors on sockets or other transports
RTI_EXCEPTION(HTTPError) // unrecoverable errors on http connects
RTI_EXCEPTION(MessageError) // Inconsistent messages reaching a server
RTI_EXCEPTION(IgnoredError) // the rti13 interface has some exceptions that should be ignored
RTI_EXCEPTION(InconsistentFDD)
RTI_EXCEPTION(CouldNotOpenFDD)
RTI_EXCEPTION(ErrorReadingFDD)
RTI_EXCEPTION(CouldNotOpenMIM)
RTI_EXCEPTION(ErrorReadingMIM)
#undef RTI_EXCEPTION

} // namespace OpenRTI

#endif
