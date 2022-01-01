/* -*-c++-*- OpenRTI - Copyright (C) 2004-2022 Mathias Froehlich
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
  enum MemoryOrder {
    MemoryOrderRelaxed,
    MemoryOrderConsume,
    MemoryOrderAcquire,
    MemoryOrderRelease,
    MemoryOrderAcqRel,
    MemoryOrderSeqCst
  };

  Atomic(unsigned value = 0) : std::atomic<unsigned>(value)
  { }

  unsigned operator++()
  { return incFetch(); }
  unsigned operator--()
  { return decFetch(); }

  unsigned incFetch(MemoryOrder memoryOrder = MemoryOrderSeqCst)
  { return fetch_add(1, _toStdMemoryOrder(memoryOrder)) + 1; }
  unsigned decFetch(MemoryOrder memoryOrder = MemoryOrderSeqCst)
  { return fetch_sub(1, _toStdMemoryOrder(memoryOrder)) - 1; }

  bool compareAndExchange(unsigned oldValue, unsigned newValue, MemoryOrder memoryOrder = MemoryOrderSeqCst)
  { return compare_exchange_weak(oldValue, newValue, _toStdMemoryOrder(memoryOrder)); }

private:
  Atomic(const Atomic&);
  Atomic& operator=(const Atomic&);

  static std::memory_order _toStdMemoryOrder(MemoryOrder memoryOrder)
  {
    switch (memoryOrder) {
    case MemoryOrderRelaxed:
      return std::memory_order_relaxed;
    case MemoryOrderConsume:
      return std::memory_order_consume;
    case MemoryOrderAcquire:
      return std::memory_order_acquire;
    case MemoryOrderRelease:
      return std::memory_order_release;
    case MemoryOrderAcqRel:
      return std::memory_order_acq_rel;
    case MemoryOrderSeqCst:
    default:
      return std::memory_order_seq_cst;
    }
  }
};

#else

class OPENRTI_API Atomic {
public:
  enum MemoryOrder {
    MemoryOrderRelaxed,
    MemoryOrderConsume,
    MemoryOrderAcquire,
    MemoryOrderRelease,
    MemoryOrderAcqRel,
    MemoryOrderSeqCst
  };

  Atomic(unsigned value = 0) : _value(value)
  { }

  unsigned operator++()
  { return incFetch(); }
  unsigned operator--()
  { return decFetch(); }

  unsigned incFetch(MemoryOrder memoryOrder = MemoryOrderSeqCst)
  {
#if defined OpenRTI_ATOMIC_USE_LIBRARY
    return inc();
#elif defined(OpenRTI_ATOMIC_USE_GCC4_BUILTINS)
    return __sync_add_and_fetch(&_value, 1);
#elif defined(OpenRTI_ATOMIC_USE_GCC47_BUILTINS)
    return __atomic_add_fetch(&_value, 1, _toMemoryOrder(memoryOrder));
#elif defined(OpenRTI_ATOMIC_USE_MIPOSPRO_BUILTINS)
    return __add_and_fetch(&_value, 1);
#elif defined(OpenRTI_ATOMIC_USE_MUTEX)
    ScopeLock lock(_mutex);
    return ++_value;
#else
    return ++_value;
#endif
  }
  unsigned decFetch(MemoryOrder memoryOrder = MemoryOrderSeqCst)
  {
#if defined OpenRTI_ATOMIC_USE_LIBRARY
    return dec();
#elif defined(OpenRTI_ATOMIC_USE_GCC4_BUILTINS)
    return __sync_sub_and_fetch(&_value, 1);
#elif defined(OpenRTI_ATOMIC_USE_GCC47_BUILTINS)
    return __atomic_sub_fetch(&_value, 1, _toMemoryOrder(memoryOrder));
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
#elif defined(OpenRTI_ATOMIC_USE_GCC47_BUILTINS)
    unsigned value;
    __atomic_load(&_value, &value, __ATOMIC_CONSUME);
    return value;
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

  bool compareAndExchange(unsigned oldValue, unsigned newValue, MemoryOrder memoryOrder = MemoryOrderSeqCst)
  {
#if defined OpenRTI_ATOMIC_USE_LIBRARY
    return cmpxch(oldValue, newValue);
#elif defined(OpenRTI_ATOMIC_USE_GCC4_BUILTINS)
    return __sync_bool_compare_and_swap(&_value, oldValue, newValue);
#elif defined(OpenRTI_ATOMIC_USE_GCC47_BUILTINS)
    return __atomic_compare_exchange(&_value, &oldValue, &newValue, 1, _toMemoryOrder(memoryOrder), _toReleaseMemoryOrder(memoryOrder));
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

#if defined(OpenRTI_ATOMIC_USE_GCC47_BUILTINS)
  static int _toMemoryOrder(MemoryOrder memoryOrder)
  {
    switch (memoryOrder) {
    case MemoryOrderRelaxed:
      return __ATOMIC_RELAXED;
    case MemoryOrderConsume:
      return __ATOMIC_CONSUME;
    case MemoryOrderAcquire:
      return __ATOMIC_ACQUIRE;
    case MemoryOrderRelease:
      return __ATOMIC_RELEASE;
    case MemoryOrderAcqRel:
      return __ATOMIC_ACQ_REL;
    case MemoryOrderSeqCst:
    default:
      return __ATOMIC_SEQ_CST;
    }
  }
  static int _toReleaseMemoryOrder(MemoryOrder memoryOrder)
  {
    switch (memoryOrder) {
    case MemoryOrderRelaxed:
    case MemoryOrderRelease:
      return __ATOMIC_RELAXED;
    case MemoryOrderConsume:
      return __ATOMIC_CONSUME;
    case MemoryOrderAcquire:
    case MemoryOrderAcqRel:
      return __ATOMIC_ACQUIRE;
    case MemoryOrderSeqCst:
    default:
      return __ATOMIC_SEQ_CST;
    }
  }
#endif

#if defined(OpenRTI_ATOMIC_USE_MUTEX)
  mutable Mutex _mutex;
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
