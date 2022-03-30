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

#ifndef OpenRTI_SharedPtr_h
#define OpenRTI_SharedPtr_h

#include "OpenRTIConfig.h"
#include "Referenced.h"

#ifndef __CPlusPlusStd
#error "must include OpenRTIConfig.h!"
#endif

namespace OpenRTI {

template<typename T>
class WeakPtr;

// NOTE: this class is more similar to boost::intrusive_ptr than std::shared_ptr.
// T must derive from Referenced *or* WeakReferenced, which store the reference count.
// Note that Referenced and WeakReferenced implement getFirst differently, so don't
// replace them by incRef here (although these are the same for 'Referenced')
template<typename T>
class OPENRTI_LOCAL SharedPtr {
  public:
    constexpr SharedPtr() noexcept : _ptr(0) {}
    explicit SharedPtr(T* ptr) : _ptr(ptr) { T::getFirst(_ptr); }
    SharedPtr(const SharedPtr& p) : _ptr(p.get()) { T::incRef(_ptr); }
    SharedPtr(SharedPtr&& p) : _ptr(0) { swap(p); }
    template<typename U>
    SharedPtr(const SharedPtr<U>& p) : _ptr(p.get()) { T::incRef(_ptr); }
    ~SharedPtr() { reset(); }

    SharedPtr& operator=(const SharedPtr& p) { assign(p.get()); return *this; }
    SharedPtr& operator=(SharedPtr&& p) { swap(p); return *this; }
    template<typename U>
    SharedPtr& operator=(const SharedPtr<U>& p) { assign(p.get()); return *this; }

    T* operator->() const noexcept { return _ptr; }
    T& operator*() const noexcept { return *_ptr; }

    T* get() const noexcept { return _ptr; }
    T* release() noexcept {
      T* tmp = _ptr;
      _ptr = 0;
      T::release(tmp);
      return tmp;
    }

    bool valid() const noexcept { return 0 != _ptr; }

    void reset() {
      T* tmp = _ptr;
      _ptr = 0;
      if (!T::decRef(tmp))
        T::destruct(tmp);
    }
    SharedPtr& swap(SharedPtr& sharedPtr) {
      T* tmp = _ptr;
      _ptr = sharedPtr._ptr;
      sharedPtr._ptr = tmp;
      return *this;
    }

  private:
    void assign(T* p) {
      T::incRef(p);
      reset();
      _ptr = p; }
    void assignFirst(T* p) {
      T::getFirst(p);
      reset();
      _ptr = p; }
    // helper for WeakPtr
    void assignNonRef(T* p) { reset(); _ptr = p; }

    // The reference itself.
    T* _ptr;

    template<typename U>
    friend class WeakPtr;
};

#pragma warning(push)
#pragma warning(disable : 26409) // Avoid calling new and delete explicitly, use std::make_unique<T> instead
template<class T, class ...Args>
SharedPtr<T> MakeShared(Args&&... args)
{
  return SharedPtr<T>(new T(std::forward<Args>(args)...));
}
#pragma warning(pop)

template<typename T>
inline bool
operator==(const SharedPtr<T>& sharedPtr0, const SharedPtr<T>& sharedPtr1)
{
  return sharedPtr0.get() == sharedPtr1.get();
}

template<typename T>
inline bool
operator==(const SharedPtr<T>& sharedPtr0, std::nullptr_t)
{
  return sharedPtr0.get() == nullptr;
}

template<typename T>
inline bool
operator!=(const SharedPtr<T>& sharedPtr0, const SharedPtr<T>& sharedPtr1)
{
  return sharedPtr0.get() != sharedPtr1.get();
}

template<typename T>
inline bool
operator!=(const SharedPtr<T>& sharedPtr0, std::nullptr_t)
{
  return sharedPtr0.get() != nullptr;
}

template<typename T>
inline bool
operator<(const SharedPtr<T>& sharedPtr0, const SharedPtr<T>& sharedPtr1)
{
  return sharedPtr0.get() < sharedPtr1.get();
}

} // namespace OpenRTI

#endif
