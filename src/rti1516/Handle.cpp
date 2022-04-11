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

// This time, the first include is above the api include.
// the rti1516/Exception header misses that.
#include <iosfwd>

#include <RTI/Handle.h>

#include "HandleFriend.h"
#include "Referenced.h"
#include "VariableLengthDataFriend.h"

#include <string>
#include <sstream>

namespace rti1516
{

// An observation to the handles guaranteed size led to reuse the
// numeric value of the _impl pointer just as the handle number.
// Less allocations, and still standard conforming.

#define IMPLEMENT_HANDLE_CLASS(HandleKind)                              \
  class OPENRTI_LOCAL HandleKind##Implementation :                      \
    public OpenRTI::Referenced {                                        \
  public:                                                               \
    HandleKind##Implementation()                                        \
    {}                                                                  \
    HandleKind##Implementation(const OpenRTI::HandleKind& handle) :     \
      _handle(handle)                                                   \
    {}                                                                  \
                                                                        \
    static bool                                                         \
    useImplementationClass()                                            \
    {                                                                   \
      return sizeof(HandleKind##Implementation*) < sizeof(OpenRTI::HandleKind); \
    }                                                                   \
                                                                        \
    static HandleKind##Implementation*                                  \
    create(const OpenRTI::HandleKind& handle)                           \
    {                                                                   \
      if (useImplementationClass()) {                                   \
        return new HandleKind##Implementation(handle);                  \
      } else {                                                          \
        union {                                                         \
          HandleKind##Implementation* ptr;                              \
          OpenRTI::HandleKind::value_type value;                        \
        } u = { 0 };                                                    \
        u.value = handle;                                               \
        return u.ptr;                                                   \
      }                                                                 \
    }                                                                   \
                                                                        \
    static void                                                         \
    putAndDelete(HandleKind##Implementation* impl)                      \
    {                                                                   \
      if (!useImplementationClass())                                    \
        return;                                                         \
      if (OpenRTI::Referenced::put(impl))                               \
        return;                                                         \
      delete impl;                                                      \
    }                                                                   \
    static void                                                         \
    get(HandleKind##Implementation* impl)                               \
    {                                                                   \
      if (!useImplementationClass())                                    \
        return;                                                         \
      OpenRTI::Referenced::get(impl);                                   \
    }                                                                   \
                                                                        \
    static OpenRTI::HandleKind                                          \
    getHandle(HandleKind##Implementation* ptr)                          \
    {                                                                   \
      if (useImplementationClass()) {                                   \
        return ptr->_handle;                                            \
      } else {                                                          \
        union {                                                         \
          HandleKind##Implementation* ptr;                              \
          OpenRTI::HandleKind::value_type value;                        \
        } u = { ptr };                                                  \
        return u.value;                                                 \
      }                                                                 \
    }                                                                   \
    OpenRTI::HandleKind _handle;                                        \
  };                                                                    \
                                                                        \
  HandleKind::HandleKind() :                                            \
    _impl(HandleKind##Implementation::create(OpenRTI::HandleKind()))    \
  {                                                                     \
    HandleKind##Implementation::get(_impl);                             \
  }                                                                     \
                                                                        \
  HandleKind::~HandleKind()                                             \
    RTI_NOEXCEPT                                                        \
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
  VariableLengthData                                                    \
  HandleKind::encode() const                                            \
  {                                                                     \
    OpenRTI::HandleKind handle;                                         \
    handle = HandleKind##Implementation::getHandle(_impl);              \
    unsigned encodedLength = handle.getEncodedLength();                 \
    OpenRTI::VariableLengthData data(encodedLength);                    \
    handle.encode(data.data());                                         \
    return rti1516::VariableLengthDataFriend::create(data);             \
  }                                                                     \
                                                                        \
  unsigned long                                                         \
  HandleKind::encodedLength() const                                     \
  {                                                                     \
    OpenRTI::HandleKind handle;                                         \
    handle = HandleKind##Implementation::getHandle(_impl);              \
    return handle.getEncodedLength();                                   \
  }                                                                     \
                                                                        \
  unsigned long                                                         \
  HandleKind::encode(void* buffer, unsigned long bufferSize) const      \
    RTI_THROW ((CouldNotEncode))                                        \
  {                                                                     \
    OpenRTI::HandleKind handle;                                         \
    handle = HandleKind##Implementation::getHandle(_impl);              \
    unsigned long encodedLength = handle.getEncodedLength();            \
    if (bufferSize < encodedLength)                                     \
      throw CouldNotEncode(toString());                                 \
    handle.encode(buffer);                                              \
    return encodedLength;                                               \
  }                                                                     \
                                                                        \
  std::wstring HandleKind::toString() const                             \
  {                                                                     \
    OpenRTI::HandleKind handle;                                         \
    handle = HandleKind##Implementation::getHandle(_impl);              \
    std::wstringstream stream;                                          \
    if (handle.valid())                                                 \
      stream << "rti1516::" #HandleKind "(" <<handle.getHandle()<< ")"; \
    else                                                                \
      stream << "rti1516::" #HandleKind "(Invalid)";                    \
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
      throw OpenRTI::CouldNotDecode(#HandleKind);                       \
    handle.decode(encodedValue.data());                                 \
    _impl = HandleKind##Implementation::create(handle);                 \
    HandleKind##Implementation::get(_impl);                             \
  }                                                                     \
                                                                        \
  HandleKind                                                            \
  HandleKind##Friend::decode(rti1516::VariableLengthData const & encodedValue) \
  {                                                                     \
    return HandleKind(encodedValue);                                    \
  }                                                                     \
                                                                        \
  void                                                                  \
  HandleKind##Friend::copy(OpenRTI::HandleKind& dst, HandleKind const& src) \
  {                                                                     \
    dst = HandleKind##Implementation::getHandle(src._impl);             \
  }                                                                     \
                                                                        \
  void                                                                  \
  HandleKind##Friend::copy(HandleKind& dst, OpenRTI::HandleKind const& src) \
  {                                                                     \
    dst = HandleKind(HandleKind##Implementation::create(src));          \
  }                                                                     \
                                                                        \
  std::wostream&                                                        \
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
