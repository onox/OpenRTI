/* -*-c++-*- OpenRTI - Copyright (C) 2009-2015 Mathias Froehlich
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

#include <RTI/Handle.h>

#include "HandleImplementation.h"
#include "VariableLengthDataImplementation.h"

#include <string>
#include <sstream>

namespace rti1516e
{

#define IMPLEMENT_HANDLE_CLASS(HandleKind)                              \
  HandleKind::HandleKind() :                                            \
    _impl(HandleKind##Implementation::create(OpenRTI::HandleKind()))    \
  {                                                                     \
    HandleKind##Implementation::get(_impl);                             \
  }                                                                     \
                                                                        \
  HandleKind::~HandleKind()                                             \
    throw()                                                             \
  {                                                                     \
    HandleKind##Implementation::putAndDelete(_impl);                    \
  }                                                                     \
                                                                        \
  HandleKind::HandleKind(HandleKind const & rhs) :                      \
    _impl(rhs._impl)                                                    \
  {                                                                     \
    HandleKind##Implementation::get(_impl);                             \
  }                                                                     \
                                                                        \
  HandleKind &                                                          \
  HandleKind::operator=(HandleKind const & rhs)                         \
  {                                                                     \
    if (_impl != rhs._impl) {                                           \
      HandleKind##Implementation::get(rhs._impl);                       \
      HandleKind##Implementation::putAndDelete(_impl);                  \
      _impl = rhs._impl;                                                \
    }                                                                   \
    return *this;                                                       \
  }                                                                     \
                                                                        \
  bool                                                                  \
  HandleKind::isValid() const                                           \
  {                                                                     \
    return HandleKind##Implementation::getHandle(_impl).valid();        \
  }                                                                     \
                                                                        \
  bool HandleKind::operator==(HandleKind const & rhs) const             \
  {                                                                     \
    return HandleKind##Implementation::getHandle(_impl) ==              \
      HandleKind##Implementation::getHandle(rhs._impl);                 \
  }                                                                     \
  bool HandleKind::operator!=(HandleKind const & rhs) const             \
  {                                                                     \
    return HandleKind##Implementation::getHandle(_impl) !=              \
      HandleKind##Implementation::getHandle(rhs._impl);                 \
  }                                                                     \
  bool HandleKind::operator<(HandleKind const & rhs) const              \
  {                                                                     \
    return HandleKind##Implementation::getHandle(_impl) <               \
      HandleKind##Implementation::getHandle(rhs._impl);                 \
  }                                                                     \
                                                                        \
  long HandleKind::hash() const                                         \
  {                                                                     \
    return HandleKind##Implementation::getHandle(_impl);                \
  }                                                                     \
                                                                        \
  VariableLengthData                                                    \
  HandleKind::encode() const                                            \
  {                                                                     \
    OpenRTI::HandleKind handle;                                         \
    handle = HandleKind##Implementation::getHandle(_impl);              \
    size_t encodedLength = handle.getEncodedLength();                   \
    OpenRTI::VariableLengthData data(encodedLength);                    \
    handle.encode(data.data());                                         \
    return VariableLengthDataFriend::create(data);                      \
  }                                                                     \
                                                                        \
  void                                                                  \
  HandleKind::encode(VariableLengthData& buffer) const                  \
  {                                                                     \
    OpenRTI::HandleKind handle;                                         \
    handle = HandleKind##Implementation::getHandle(_impl);              \
    size_t encodedLength = handle.getEncodedLength();                   \
    OpenRTI::VariableLengthData data(encodedLength);                    \
    handle.encode(data.data());                                         \
    buffer = VariableLengthDataFriend::create(data);                    \
  }                                                                     \
                                                                        \
  size_t                                                                \
  HandleKind::encode(void* buffer, size_t bufferSize) const             \
    throw (CouldNotEncode)                                              \
  {                                                                     \
    OpenRTI::HandleKind handle;                                         \
    handle = HandleKind##Implementation::getHandle(_impl);              \
    size_t encodedLength = handle.getEncodedLength();                   \
    if (bufferSize < encodedLength)                                     \
      throw CouldNotEncode(toString());                                 \
    handle.encode(buffer);                                              \
    return encodedLength;                                               \
  }                                                                     \
                                                                        \
  size_t                                                                \
  HandleKind::encodedLength() const                                     \
  {                                                                     \
    OpenRTI::HandleKind handle;                                         \
    handle = HandleKind##Implementation::getHandle(_impl);              \
    return handle.getEncodedLength();                                   \
  }                                                                     \
                                                                        \
  std::wstring HandleKind::toString() const                             \
  {                                                                     \
    OpenRTI::HandleKind handle;                                         \
    handle = HandleKind##Implementation::getHandle(_impl);              \
    std::wstringstream stream;                                          \
    if (handle.valid())                                                 \
      stream << "rti1516e::" #HandleKind "(" <<handle.getHandle()<< ")";\
    else                                                                \
      stream << "rti1516e::" #HandleKind "(Invalid)";                   \
    return stream.str();                                                \
  }                                                                     \
                                                                        \
                                                                        \
  const HandleKind##Implementation*                                     \
  HandleKind::getImplementation() const                                 \
  {                                                                     \
    return _impl;                                                       \
  }                                                                     \
                                                                        \
  HandleKind##Implementation*                                           \
  HandleKind::getImplementation()                                       \
  {                                                                     \
    if (!HandleKind##Implementation::useImplementationClass())          \
      return _impl;                                                     \
    if (OpenRTI::Referenced::count(_impl) <= 1)                         \
      return _impl;                                                     \
    HandleKind##Implementation* impl;                                   \
    impl = new HandleKind##Implementation(*_impl);                      \
    HandleKind##Implementation::putAndDelete(_impl);                    \
    _impl = impl;                                                       \
    HandleKind##Implementation::get(_impl);                             \
    return _impl;                                                       \
  }                                                                     \
                                                                        \
  HandleKind::HandleKind(HandleKind##Implementation* impl) :            \
    _impl(impl)                                                         \
  {                                                                     \
    HandleKind##Implementation::get(_impl);                             \
  }                                                                     \
                                                                        \
  HandleKind::HandleKind(VariableLengthData const & encodedValue) :     \
    _impl(0)                                                            \
  {                                                                     \
    OpenRTI::HandleKind handle;                                         \
    if (encodedValue.size() < handle.getEncodedLength())                \
      throw CouldNotDecode(L"Handle");                                  \
    handle.decode(encodedValue.data());                                 \
    _impl = HandleKind##Implementation::create(handle);                 \
    HandleKind##Implementation::get(_impl);                             \
  }                                                                     \
                                                                        \
  std::wostream RTI_EXPORT&                                             \
  operator<<(std::wostream& stream, HandleKind const& handle)           \
  {                                                                     \
    return stream << handle.toString();                                 \
  }

IMPLEMENT_HANDLE_CLASS(FederateHandle)
IMPLEMENT_HANDLE_CLASS(ObjectClassHandle)
IMPLEMENT_HANDLE_CLASS(InteractionClassHandle)
IMPLEMENT_HANDLE_CLASS(ObjectInstanceHandle)
IMPLEMENT_HANDLE_CLASS(AttributeHandle)
IMPLEMENT_HANDLE_CLASS(ParameterHandle)
IMPLEMENT_HANDLE_CLASS(DimensionHandle)
IMPLEMENT_HANDLE_CLASS(RegionHandle)
IMPLEMENT_HANDLE_CLASS(MessageRetractionHandle)

}
