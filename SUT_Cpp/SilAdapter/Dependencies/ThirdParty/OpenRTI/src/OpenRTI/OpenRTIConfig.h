/* -*-c++-*- OpenRTI - Copyright (C) 2004-2014 Mathias Froehlich
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

// Defaults for server connects
#define OpenRTI_DEFAULT_HOST_STRING "localhost"
// Just a free port from my /etc/services ...
#define OpenRTI_DEFAULT_PORT_STRING "14321"
#define OpenRTI_DEFAULT_PIPE_PATH ".OpenRTI"

#define OpenRTI_VERSION_CHECK(REQ_MAJ, REQ_MIN, AVAIL_MAJ, AVAIL_MIN) \
  ((((REQ_MAJ) == (AVAIL_MAJ)) && ((REQ_MIN) <= (AVAIL_MIN))) || ((REQ_MAJ) < (AVAIL_MAJ)))

#if defined(__GNUC__) && (((4 <= __GNUC__) && (1 <= __GNUC_MINOR__)) || (5 <= __GNUC__))
#define OpenRTI_DEPRECATED __attribute__ ((deprecated))
#else
#define OpenRTI_DEPRECATED
#endif

#ifdef _MSVC_LANG
#define __CPlusPlusStd _MSVC_LANG
#pragma warning(disable : 26496) // The variable 'XXX' is assigned only once, mark it as const
#pragma warning(disable : 26482) // Only index into arrays using constant expressions
#pragma warning(disable : 26485) // No array to pointer decay 
#pragma warning(disable : 26472) // Don't use a static_cast for arithmetic conversions. Use brace initialization, gsl::narrow_cast or gsl::narow
#else
#define __CPlusPlusStd __cplusplus
#endif

#if 201103L <= __CPlusPlusStd
// In any case this is the best choice as this provides a reliable api for any future
# define OpenRTI_ATOMIC_USE_STD_ATOMIC
#else
# if defined(__GNUC__) && OpenRTI_VERSION_CHECK(4, 7, __GNUC__, __GNUC_MINOR__)
// No need to include something. Is a Compiler API ...
#  define OpenRTI_ATOMIC_USE_GCC47_BUILTINS
# elif defined(__clang__) && OpenRTI_VERSION_CHECK(3, 3, __clang_major__, __clang_minor__)
// No need to include something. Is a Compiler API. Note that clang aims to be gcc compatible ...
#  define OpenRTI_ATOMIC_USE_GCC47_BUILTINS
# elif defined _WIN32
// Neat old Win32 functions
#  define OpenRTI_ATOMIC_USE_WIN32_INTERLOCKED
# elif defined(__GNUC__) && OpenRTI_VERSION_CHECK(4, 1, __GNUC__, __GNUC_MINOR__) && defined(__x86_64__)
// No need to include something. Is a Compiler API ...
#  define OpenRTI_ATOMIC_USE_GCC4_BUILTINS
# elif defined(__GNUC__) && defined(__i386)
#  define OpenRTI_ATOMIC_USE_GCC_ASM
# elif defined(__sgi) && defined(_COMPILER_VERSION) && (_COMPILER_VERSION>=730)
// No need to include something. Is a Compiler API ...
#  define OpenRTI_ATOMIC_USE_MIPSPRO_BUILTINS
// FIXME
// #elif defined(__sun)
// # define OpenRTI_ATOMIC_USE_SUN
# elif defined(__APPLE__)
#  define OpenRTI_ATOMIC_USE_BSD
# else
// The sledge hammer ...
#  define OpenRTI_ATOMIC_USE_MUTEX
# endif
#endif

#define OpenRTI_HAVE_CSTDINT
#define OpenRTI_HAVE_STDINT_H
#define OpenRTI_HAVE_INTTYPES_H

/* #undef OpenRTI_HOST_IS_BIG_ENDIAN */
#define OpenRTI_HOST_IS_LITTLE_ENDIAN

/* #undef OpenRTI_HAVE_ALLOCA */
/* #undef OpenRTI_HAVE_DLADDR */
#define OPENRTI_DATAROOTDIR "/OpenRTI"

//#define EXPERIMENTAL_NO_LOCKED_BY_NEXT_MESSAGE
#undef EXPERIMENTAL_NO_LOCKED_BY_NEXT_MESSAGE
//#define EXPERIMENTAL_NO_LBTS_RESPONSE_MESSAGE
#undef EXPERIMENTAL_NO_LBTS_RESPONSE_MESSAGE

// #define OPENRTI_USE_DEBUG_NEW
#undef OPENRTI_USE_DEBUG_NEW
#define _QUOTE(x)       # x
#define QUOTE(x)        _QUOTE(x)
#define __FILE__LINE__      __FILE__ "(" QUOTE(__LINE__) ") : "

#endif
