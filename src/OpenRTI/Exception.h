/* -*-c++-*- OpenRTI - Copyright (C) 2009-2010 Mathias Froehlich
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

#include <string>

#include "Export.h"

namespace OpenRTI {

class OPENRTI_API Exception {
public:
  enum Type {
    RTIinternalError,
    ResourceError, // unrecoverable errors when we run out of resources
    TransportError, // unrecoverable errors on sockets or other transports
    MessageError, // Inconsistent messages reaching a server
    IgnoredError, // the rti13 interface has some exceptions that should be ignored
    InconsistentFDD,
    CouldNotOpenFDD,
    ErrorReadingFDD,
    CouldNotOpenMIM,
    ErrorReadingMIM
  };

  virtual ~Exception();

  Type getType() const throw()
  { return _type; }
  const std::wstring& getReason() const throw()
  { return _reason; }
  std::string getReasonInLocale() const;

protected:
  Exception(Type type = RTIinternalError, const char* reason = 0);
  Exception(Type type, const std::wstring& reason);

private:
  Type _type;
  std::wstring _reason;
};

class OPENRTI_API RTIinternalError : public Exception {
public:
  RTIinternalError(const char* reason = 0);
  RTIinternalError(const std::wstring& reason);
  RTIinternalError(const char* file, unsigned line, const char* reason = 0);
  virtual ~RTIinternalError();

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
  name(const char* reason = 0) : Exception(Exception:: name, reason) { }  \
  name(const std::wstring& reason) : Exception(Exception:: name, reason) { } \
};

RTI_EXCEPTION(ResourceError)
RTI_EXCEPTION(TransportError)
RTI_EXCEPTION(MessageError)
RTI_EXCEPTION(IgnoredError)
RTI_EXCEPTION(InconsistentFDD)
RTI_EXCEPTION(CouldNotOpenFDD)
RTI_EXCEPTION(ErrorReadingFDD)
RTI_EXCEPTION(CouldNotOpenMIM)
RTI_EXCEPTION(ErrorReadingMIM)
#undef RTI_EXCEPTION

} // namespace OpenRTI

#endif
