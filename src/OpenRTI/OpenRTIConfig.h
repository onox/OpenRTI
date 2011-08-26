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

#ifndef OpenRTI_OpenRTIConfig_h
#define OpenRTI_OpenRTIConfig_h

#define OpenRTI_VERSION_STRING "1.0"

// Defaults for server connects
#define OpenRTI_DEFAULT_HOST_STRING "localhost"
// Just a free port from my /etc/services ...
#define OpenRTI_DEFAULT_PORT_STRING "14321"
#define OpenRTI_DEFAULT_PIPE_PATH ".OpenRTI"

#if defined(__GNUC__) && (4 <= __GNUC__) && (1 <= __GNUC_MINOR__)
#define OpenRTI_DEPRECATED __attribute__ ((deprecated))
#else
#define OpenRTI_DEPRECATED
#endif

// FIXME detect when to use this
// #define OpenRTI_ATOMIC_USE_STD_ATOMIC

#if defined _WIN32
// Neat old Win32 functions
# define OpenRTI_ATOMIC_USE_WIN32_INTERLOCKED
#elif defined(__GNUC__) && (4 <= __GNUC__) && (1 <= __GNUC_MINOR__) && defined(__x86_64__)
// No need to include something. Is a Compiler API ...
# define OpenRTI_ATOMIC_USE_GCC4_BUILTINS
#elif defined(__GNUC__) && defined(__i386)
# define OpenRTI_ATOMIC_USE_GCC_ASM
#elif defined(__sgi) && defined(_COMPILER_VERSION) && (_COMPILER_VERSION>=730)
// No need to include something. Is a Compiler API ...
# define OpenRTI_ATOMIC_USE_MIPSPRO_BUILTINS
// FIXME
// #elif defined(__sun)
// # define OpenRTI_ATOMIC_USE_SUN
#elif defined(__APPLE__)
# define OpenRTI_ATOMIC_USE_BSD
#else
// The sledge hammer ...
# define OpenRTI_ATOMIC_USE_MUTEX
#endif

#endif
