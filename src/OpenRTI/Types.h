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

#ifndef OpenRTI_Types_h
#define OpenRTI_Types_h

#include "OpenRTIConfig.h"

#include <cstdlib>
#if defined(OpenRTI_HAVE_CSTDINT)
# include <cstdint>
#elif defined(OpenRTI_HAVE_STDINT_H)
# include <stdint.h>
#elif defined(OpenRTI_HAVE_INTTYPES_H)
# include <inttypes.h>
#elif defined(_WIN32) && defined(_MSC_VER)
typedef unsigned __int64 uint64_t;
typedef __int64 int64_t;
typedef unsigned __int32 uint32_t;
typedef __int32 int32_t;
typedef unsigned __int16 uint16_t;
typedef __int16 int16_t;
typedef unsigned __int8 uint8_t;
typedef __int8 int8_t;
#else
# error No integer types available
#endif
#if !(defined(_WIN32) && defined(_MSC_VER))
# include <sys/types.h>
#endif

namespace OpenRTI {

#if defined(_WIN32) && defined(_MSC_VER)
#if defined(_WIN64)
typedef ::uint64_t size_t;
typedef ::int64_t ssize_t;
#else
typedef ::uint32_t size_t;
typedef ::int32_t ssize_t;
#endif
#else
typedef ::size_t size_t;
typedef ::ssize_t ssize_t;
#endif

typedef ::uint8_t uint8_t;
typedef ::int8_t int8_t;
typedef ::uint16_t uint16_t;
typedef ::int16_t int16_t;
typedef ::uint32_t uint32_t;
typedef ::int32_t int32_t;
typedef ::uint64_t uint64_t;
typedef ::int64_t int64_t;

// Make use of the present defines
#if defined OpenRTI_HOST_IS_BIG_ENDIAN
#elif defined OpenRTI_HOST_IS_LITTLE_ENDIAN
#else
# error No endianess
#endif

// inline uint16_t
// bswap16(uint16_t n)
// {
// // #if defined(__GNUC__)
// //    return __builtin_bswap16(n);
// // #else
//    return (n >> 8) | (n << 8);
// // #endif
// }

// inline uint32_t
// bswap32(uint32_t n)
// {
// #if defined(__GNUC__)
//    return __builtin_bswap32(n);
// #else
//    return (n >> 24) |
//      ((n >> 8) & 0x0000ff00) |
//      ((n << 8) & 0x00ff0000) |
//      (n << 24);
// #endif
// }

// inline uint64_t
// bswap64(uint64_t n)
// {
// #if defined(__GNUC__)
//    return __builtin_bswap64(n);
// #else
//    return (n >> 56) |
//      ((n >> 40) & 0x0000ff00) |
//      ((n >> 24) & 0x00ff0000) |
//      ((n >> 8) & 0xff000000) |
//      ((n & 0xff000000) << 8) |
//      ((n & 0x00ff0000) << 24) |
//      ((n & 0x0000ff00) << 40) |
//      (n << 56);
// #endif
// }

} // namespace OpenRTI

#endif
