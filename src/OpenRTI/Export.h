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

#ifndef OpenRTI_Export_h
#define OpenRTI_Export_h

/// Revisit this??? FIXME
/// May be it is better to have public symbols explicitly public and
/// specify default=hidden on the compile line for the *shared* lib
/// When static this does not change anything???
/// Or does it harm when we are hidden when building a shared and a dynamic lib??
/// I believe that this is true on linux. But for win32???

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
# define OPENRTI_HELPER_DLL_IMPORT __declspec(dllimport)
# define OPENRTI_HELPER_DLL_EXPORT __declspec(dllexport)
# define OPENRTI_HELPER_DLL_LOCAL
#elif defined __GNUC__ && (4 <= __GNUC__)
# define OPENRTI_HELPER_DLL_IMPORT __attribute__ ((visibility("default")))
# define OPENRTI_HELPER_DLL_EXPORT __attribute__ ((visibility("default")))
# define OPENRTI_HELPER_DLL_LOCAL  __attribute__ ((visibility("hidden")))
#elif defined __SUNPRO_C && (0x550 <= __SUNPRO_C)
# define OPENRTI_HELPER_DLL_IMPORT __hidden
# define OPENRTI_HELPER_DLL_EXPORT __global
# define OPENRTI_HELPER_DLL_LOCAL  __hidden
#else
# define OPENRTI_HELPER_DLL_IMPORT
# define OPENRTI_HELPER_DLL_EXPORT
# define OPENRTI_HELPER_DLL_LOCAL
#endif

// Now we use the generic helper definitions above to define OPENRTI_API and OPENRTI_LOCAL.
// OPENRTI_API is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for static build)
// OPENRTI_LOCAL is used for non-api symbols.

#ifdef OPENRTI_DLL // defined if OPENRTI is compiled as a DLL
# ifdef OPENRTI_DLL_EXPORTS // defined if we are building the OPENRTI DLL (instead of using it)
#  define OPENRTI_API OPENRTI_HELPER_DLL_EXPORT
# else
#  define OPENRTI_API OPENRTI_HELPER_DLL_IMPORT
# endif // OPENRTI_DLL_EXPORTS
# define OPENRTI_LOCAL OPENRTI_HELPER_DLL_LOCAL
#else // OPENRTI_DLL is not defined: this means OPENRTI is a static lib.
# define OPENRTI_API
# define OPENRTI_LOCAL
#endif // OPENRTI_DLL

#if defined _WIN32 && defined _MSC_VER
// disable warnings about a "dllexport" class using a regular class
# pragma warning(disable: 4251)
// the 'this is used in base ...' warning for WeakReferenced.
# pragma warning(disable: 4355)
#endif


#endif
