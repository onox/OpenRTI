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

#ifndef ValueImplementation_h
#define ValueImplementation_h

#include "Export.h"
#include "Referenced.h"

#define DECLARE_VALUE_IMPLEMENTATION(ValueImpl, ValueType)              \
  class OPENRTI_LOCAL ValueImpl : public OpenRTI::Referenced {          \
  public:                                                               \
    ValueImpl(const ValueType& value) :                                 \
      _value(value)                                                     \
    {                                                                   \
      OpenRTI::Referenced::get(this);                                   \
    }                                                                   \
                                                                        \
    static bool                                                         \
    useImplementationClass()                                            \
    { return sizeof(ValueImpl*) < sizeof(ValueType); }                  \
                                                                        \
    static void                                                         \
    putAndDelete(ValueImpl* impl)                                       \
    {                                                                   \
      if (!useImplementationClass())                                    \
        return;                                                         \
      if (OpenRTI::Referenced::put(impl))                               \
        return;                                                         \
      delete impl;                                                      \
    }                                                                   \
    static void                                                         \
    get(ValueImpl* impl)                                                \
    {                                                                   \
      if (!useImplementationClass())                                    \
        return;                                                         \
      OpenRTI::Referenced::get(impl);                                   \
    }                                                                   \
                                                                        \
    /* Value accessors */                                               \
    static void                                                         \
    assign(ValueImpl*& impl, ValueImpl* const& rhsImpl)                 \
    {                                                                   \
      if (useImplementationClass()) {                                   \
        if (impl != rhsImpl) {                                          \
          get(rhsImpl);                                                 \
          putAndDelete(impl);                                           \
          impl = rhsImpl;                                               \
        }                                                               \
      } else {                                                          \
        impl = rhsImpl;                                                 \
      }                                                                 \
    }                                                                   \
    static void                                                         \
    setValue(ValueImpl*& impl, const ValueType& value)                  \
    {                                                                   \
      if (useImplementationClass()) {                                   \
        if (!impl) {                                                    \
          impl = new ValueImpl(value);                                  \
        } else {                                                        \
          if (1 < OpenRTI::Referenced::count(impl)) {                   \
            ValueImpl::putAndDelete(impl);                              \
            impl = new ValueImpl(value);                                \
          } else {                                                      \
            impl->_value = value;                                       \
          }                                                             \
        }                                                               \
      } else {                                                          \
        union {                                                         \
          ValueImpl* ptr;                                               \
          ValueType value;                                              \
        } u;                                                            \
        u.ptr = 0;                                                      \
        u.value = value;                                                \
        impl = u.ptr;                                                   \
      }                                                                 \
    }                                                                   \
    static ValueType                                                    \
    getValue(ValueImpl* impl)                                           \
    {                                                                   \
      if (ValueImpl::useImplementationClass()) {                        \
        if (impl) {                                                     \
          return impl->_value;                                          \
        } else {                                                        \
          return ValueType();                                           \
        }                                                               \
      } else {                                                          \
        union {                                                         \
          ValueImpl* ptr;                                               \
          ValueType value;                                              \
        } u;                                                            \
        u.ptr = impl;                                                   \
        return u.value;                                                 \
      }                                                                 \
    }                                                                   \
                                                                        \
    ValueType _value;                                                   \
  };

#endif
