/* -*-c++-*- OpenRTI - Copyright (C) 2004-2013 Mathias Froehlich
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

# include <atomic>

namespace OpenRTI {

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

  Atomic(unsigned value = 0) noexcept : std::atomic<unsigned>(value)
  { }
  Atomic(const Atomic&) = delete;
  Atomic(Atomic&&) = delete;
	Atomic& operator=(const Atomic&) = delete;
	//Atomic& operator=(const Atomic&) volatile = delete;
  //unsigned operator++()
  //{ return incFetch(); }
  //unsigned operator--()
  //{ return decFetch(); }

  unsigned incFetch(MemoryOrder memoryOrder = MemoryOrderSeqCst) noexcept
  { return fetch_add(1, _toStdMemoryOrder(memoryOrder)) + 1; }
  unsigned decFetch(MemoryOrder memoryOrder = MemoryOrderSeqCst) noexcept
  { return fetch_sub(1, _toStdMemoryOrder(memoryOrder)) - 1; }

  bool compareAndExchange(unsigned oldValue, unsigned newValue, MemoryOrder memoryOrder = MemoryOrderSeqCst) noexcept
  { return compare_exchange_weak(oldValue, newValue, _toStdMemoryOrder(memoryOrder)); }

private:

  static std::memory_order _toStdMemoryOrder(MemoryOrder memoryOrder) noexcept
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


}  // namespace OpenRTI

#endif
