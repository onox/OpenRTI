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

#include "Atomic.h"

#if defined(OpenRTI_ATOMIC_USE_SUN)
# include <atomic.h>
#elif defined(OpenRTI_ATOMIC_USE_BSD)
# include <libkern/OSAtomic.h>
#endif

namespace OpenRTI {

#if defined OpenRTI_ATOMIC_USE_LIBRARY

unsigned
Atomic::inc()
{
#if defined(OpenRTI_ATOMIC_USE_GCC_ASM)
  unsigned result;
  __asm__ __volatile__("lock; xadd{l} {%0,%1|%1,%0}"
                       : "=r" (result), "=m" (_value)
                       : "0" (1), "m" (_value)
                       : "memory");
  return result + 1;
#elif defined(OpenRTI_ATOMIC_USE_SUN)
  return atomic_inc_uint_nv(&_value);
#elif defined(OpenRTI_ATOMIC_USE_BSD)
  return OSAtomicIncrement32(&_value);
#else
# error Atomics defined as library but not implemented there
#endif
}

unsigned
Atomic::dec()
{
#if defined(OpenRTI_ATOMIC_USE_GCC_ASM)
  unsigned result;
  __asm__ __volatile__("lock; xadd{l} {%0,%1|%1,%0}"
                       : "=r" (result), "=m" (_value)
                       : "0" (-1), "m" (_value)
                       : "memory");
  return result - 1;
#elif defined(OpenRTI_ATOMIC_USE_SUN)
  return atomic_dec_uint_nv(&_value);
#elif defined(OpenRTI_ATOMIC_USE_BSD)
  return OSAtomicDecrement32(&_value);
#else
# error Atomics defined as library but not implemented there
#endif
}

unsigned
Atomic::get() const
{
#if defined(OpenRTI_ATOMIC_USE_GCC_ASM)
  __asm__ __volatile__("": : : "memory");
  return _value;
#elif defined(OpenRTI_ATOMIC_USE_SUN)
  membar_consumer();
  return _value;
#elif defined(OpenRTI_ATOMIC_USE_BSD)
  OSMemoryBarrier();
  return _value;
#else
# error Atomics defined as library but not implemented there
#endif
}

bool
Atomic::cmpxch(unsigned oldValue, unsigned newValue)
{
#if defined(OpenRTI_ATOMIC_USE_GCC_ASM)
  unsigned before;
  __asm__ __volatile__("lock; cmpxchg{l} {%1,%2|%1,%2}"
                       : "=a"(before)
                       : "q"(newValue), "m"(_value), "0"(oldValue)
                       : "memory");
  return before == oldValue;
#elif defined(OpenRTI_ATOMIC_USE_SUN)
  return atomic_cas_uint(&_value, oldValue, newValue);
#elif defined(OpenRTI_ATOMIC_USE_BSD)
  return OSAtomicCompareAndSwap32(oldValue, newValue, &_value);
#else
# error Atomics defined as library but not implemented there
#endif
}

#endif

}
