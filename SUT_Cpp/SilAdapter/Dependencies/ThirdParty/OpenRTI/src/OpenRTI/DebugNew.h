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

#pragma once

#include "OpenRTIConfig.h"
#include <type_traits>
#include <memory>

#ifndef _OPENRTI_PLACEMENT_NEW_DEFINED
#pragma push_macro("new")
#undef new
template <class X, typename ... Types>
inline X* placement_new(void* addr, Types&& ... args)
{
  return new (addr) X(std::forward<Types>(args)...);
}
#pragma pop_macro("new")
#define _OPENRTI_PLACEMENT_NEW_DEFINED
#endif

#if defined(_WIN32)
#pragma warning(push)
#pragma warning(disable : 26409)
#if defined(_DEBUG) && defined (OPENRTI_USE_DEBUG_NEW)
#if !defined ( _OPENRTI_DEBUG_NEW )
  #include <crtdbg.h>
  #pragma push_macro("new")
  #undef new
  inline void* alloc_new_impl(size_t _Size, int _BlockUse, char const* _FileName, int _LineNumber)
  {
    return ::operator new(_Size, _BlockUse, _FileName, _LineNumber);
  }
  #pragma pop_macro("new")
  #define _OPENRTI_DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__)
  #define new _OPENRTI_DEBUG_NEW
  #define alloc_new(size) alloc_new_impl(size, _NORMAL_BLOCK, __FILE__, __LINE__)
#endif // !defined ( _OPENRTI_DEBUG_NEW )
#else // !defined(_DEBUG)
#ifndef _OPENRTI_ALLOC_NEW_DEFINED
  inline void* alloc_new(size_t _Size)
  {
    return ::operator new(_Size);
  }
  #define _OPENRTI_ALLOC_NEW_DEFINED
#endif

#endif // defined(_DEBUG)
#pragma warning(pop)

#else // !defined(_WIN32)
#ifndef _OPENRTI_ALLOC_NEW_DEFINED
  inline void* alloc_new(size_t _Size)
  {
    return ::operator new(_Size);
  }
  #define _OPENRTI_ALLOC_NEW_DEFINED
#endif
#endif
