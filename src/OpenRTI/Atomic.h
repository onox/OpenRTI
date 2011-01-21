/* -*-c++-*- OpenRTI - Copyright (C) 2004-2011 Mathias Froehlich
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

#ifndef OpenRTI_Atomic_h
#define OpenRTI_Atomic_h

#include "OpenRTIConfig.h"
#include "Export.h"

#if defined(OpenRTI_ATOMIC_USE_STD_ATOMIC)
# include <atomic>
#elif defined(OpenRTI_ATOMIC_USE_WIN32_INTERLOCKED)
# define OpenRTI_ATOMIC_USE_LIBRARY
#elif defined(OpenRTI_ATOMIC_USE_GCC_ASM)
# define OpenRTI_ATOMIC_USE_LIBRARY
#elif defined(OpenRTI_ATOMIC_USE_SUN)
# define OpenRTI_ATOMIC_USE_LIBRARY
#elif defined(OpenRTI_ATOMIC_USE_BSD)
# define OpenRTI_ATOMIC_USE_LIBRARY
#elif defined(OpenRTI_ATOMIC_USE_MUTEX)
# include "Mutex.h"
# include "ScopeLock.h"
#endif

namespace OpenRTI {

#ifdef OpenRTI_ATOMIC_USE_STD_ATOMIC

class OPENRTI_API Atomic : public std::atomic<unsigned> {
public:
  Atomic(unsigned value = 0) : std::atomic<unsigned>(value)
  { }

  bool compareAndExchange(unsigned oldValue, unsigned newValue)
  { return compare_exchange_weak(oldValue, newValue); }

private:
  Atomic(const Atomic&);
  Atomic& operator=(const Atomic&);
};

#else

class OPENRTI_API Atomic {
public:
  Atomic(unsigned value = 0) : _value(value)
  { }

  unsigned operator++()
  {
#if defined OpenRTI_ATOMIC_USE_LIBRARY
    return inc();
#elif defined(OpenRTI_ATOMIC_USE_GCC4_BUILTINS)
    return __sync_add_and_fetch(&_value, 1);
#elif defined(OpenRTI_ATOMIC_USE_MIPOSPRO_BUILTINS)
    return __add_and_fetch(&_value, 1);
#elif defined(OpenRTI_ATOMIC_USE_MUTEX)
    ScopeLock lock(_mutex);
    return ++_value;
#else
    return ++_value;
#endif
  }
  unsigned operator--()
  {
#if defined OpenRTI_ATOMIC_USE_LIBRARY
    return dec();
#elif defined(OpenRTI_ATOMIC_USE_GCC4_BUILTINS)
    return __sync_sub_and_fetch(&_value, 1);
#elif defined(OpenRTI_ATOMIC_USE_MIPOSPRO_BUILTINS)
    return __sub_and_fetch(&_value, 1);
#elif defined(OpenRTI_ATOMIC_USE_MUTEX)
    ScopeLock lock(_mutex);
    return --_value;
#else
    return --_value;
#endif
  }
  operator unsigned() const
  {
#if defined OpenRTI_ATOMIC_USE_LIBRARY
    return get();
#elif defined(OpenRTI_ATOMIC_USE_GCC4_BUILTINS)
    __sync_synchronize();
    return _value;
#elif defined(OpenRTI_ATOMIC_USE_MIPOSPRO_BUILTINS)
    __synchronize();
    return _value;
#elif defined(OpenRTI_ATOMIC_USE_MUTEX)
    ScopeLock lock(_mutex);
    return _value;
#else
    return _value;
#endif
 }

  bool compareAndExchange(unsigned oldValue, unsigned newValue)
  {
#if defined OpenRTI_ATOMIC_USE_LIBRARY
    return cmpxch(oldValue, newValue);
#elif defined(OpenRTI_ATOMIC_USE_GCC4_BUILTINS)
    return __sync_bool_compare_and_swap(&_value, oldValue, newValue);
#elif defined(OpenRTI_ATOMIC_USE_MIPOSPRO_BUILTINS)
    return __compare_and_swap(&_value, oldValue, newValue);
#elif defined(OpenRTI_ATOMIC_USE_MUTEX)
    ScopeLock lock(_mutex);
    if (_value != oldValue)
      return false;
    _value = newValue;
    return true;
#else
    if (_value != oldValue)
      return false;
    _value = newValue;
    return true;
#endif
  }

private:
  Atomic(const Atomic&);
  Atomic& operator=(const Atomic&);

#if defined OpenRTI_ATOMIC_USE_LIBRARY
  unsigned inc();
  unsigned dec();
  unsigned get() const;
  bool cmpxch(unsigned oldValue, unsigned newValue);
#endif

#if defined(OpenRTI_ATOMIC_USE_MUTEX)
  Mutex _mutex;
#endif
#if defined(OpenRTI_ATOMIC_USE_WIN32_INTERLOCKED)
  volatile long _value;
#elif defined(OpenRTI_ATOMIC_USE_BSD)
  volatile int32_t _value;
#elif defined(OpenRTI_ATOMIC_USE_SUN)
  volatile uint_t _value;
#else
  volatile unsigned _value;
#endif
};

#endif

}  // namespace OpenRTI

#endif
