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

#ifndef HandleImplementation_h
#define HandleImplementation_h

#include <RTI/Handle.h>
#include "Handle.h"
#include "Referenced.h"

namespace rti1516
{
// An observation to the handles guaranteed size led to reuse the
// numeric value of the _impl pointer just as the handle number.
// Less allocations, and still standard conforming.

#define DECLARE_HANDLE_CLASS(HandleKind)                                \
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
      union {                                                           \
        HandleKind##Implementation* ptr;                                \
        OpenRTI::HandleKind::value_type value;                          \
      } u;                                                              \
      if (useImplementationClass()) {                                   \
        u.ptr = new HandleKind##Implementation(handle);                 \
      } else {                                                          \
        u.ptr = 0;                                                      \
        u.value = handle;                                               \
      }                                                                 \
      return u.ptr;                                                     \
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
        } u;                                                            \
        u.ptr = ptr;                                                    \
        return u.value;                                                 \
      }                                                                 \
    }                                                                   \
    OpenRTI::HandleKind _handle;                                        \
  };                                                                    \
                                                                        \
  class OPENRTI_LOCAL HandleKind##Friend {                              \
  public:                                                               \
    static HandleKind                                                   \
    decode(rti1516::VariableLengthData const & encodedValue)            \
    { return HandleKind(encodedValue); }                                \
    static void                                                         \
    copy(OpenRTI::HandleKind& dst, HandleKind const& src)               \
    { dst = HandleKind##Implementation::getHandle(src._impl); }         \
    static void                                                         \
    copy(HandleKind& dst, OpenRTI::HandleKind const& src)               \
    { dst = HandleKind(HandleKind##Implementation::create(src)); }      \
  };

DECLARE_HANDLE_CLASS(FederateHandle)
DECLARE_HANDLE_CLASS(ObjectClassHandle)
DECLARE_HANDLE_CLASS(InteractionClassHandle)
DECLARE_HANDLE_CLASS(ObjectInstanceHandle)
DECLARE_HANDLE_CLASS(AttributeHandle)
DECLARE_HANDLE_CLASS(ParameterHandle)
DECLARE_HANDLE_CLASS(DimensionHandle)
DECLARE_HANDLE_CLASS(RegionHandle)
DECLARE_HANDLE_CLASS(MessageRetractionHandle)

#undef DECLARE_HANDLE_CLASS

}

#endif
