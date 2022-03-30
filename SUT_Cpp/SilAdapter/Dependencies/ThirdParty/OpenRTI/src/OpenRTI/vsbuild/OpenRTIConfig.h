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
#else
#define __CPlusPlusStd __cplusplus
#endif

#if 201103L > __CPlusPlusStd
#error "unsupported compiler, C++11 required"
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

#define OPENRTI_USE_DEBUG_NEW
//#undef OPENRTI_USE_DEBUG_NEW
#define _QUOTE(x)       # x
#define QUOTE(x)        _QUOTE(x)
#define __FILE__LINE__      __FILE__ "(" QUOTE(__LINE__) ") : "

#endif
