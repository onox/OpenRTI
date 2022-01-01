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

#ifndef OpenRTI_SharedPtr_h
#define OpenRTI_SharedPtr_h

#include "OpenRTIConfig.h"
#include "Referenced.h"

namespace OpenRTI {

template<typename T>
class WeakPtr;

template<typename T>
class OPENRTI_LOCAL SharedPtr {
public:
  SharedPtr(void) : _ptr(0)
  {}
  SharedPtr(T* ptr) : _ptr(ptr) // explicit???
  { T::getFirst(_ptr); }
  SharedPtr(const SharedPtr& p) : _ptr(p.get())
  { T::get(_ptr); }
#if 201103L <= __cplusplus || 200610L <= __cpp_rvalue_reference
  SharedPtr(SharedPtr&& p) : _ptr(0)
  { swap(p); }
#endif
  template<typename U>
  SharedPtr(const SharedPtr<U>& p) : _ptr(p.get())
  { T::get(_ptr); }
  ~SharedPtr(void)
  { put(); }

  SharedPtr& operator=(const SharedPtr& p)
  { assign(p.get()); return *this; }
#if 201103L <= __cplusplus || 200610L <= __cpp_rvalue_reference
  SharedPtr& operator=(SharedPtr&& p)
  { swap(p); return *this; }
#endif
  template<typename U>
  SharedPtr& operator=(const SharedPtr<U>& p)
  { assign(p.get()); return *this; }
  template<typename U>
  SharedPtr& operator=(U* p)
  { assignFirst(p); return *this; }

  T* operator->(void) const
  { return _ptr; }

  T& operator*(void) const
  { return *_ptr; }

  T* get() const
  { return _ptr; }
  T* release()
  { T* tmp = _ptr; _ptr = 0; T::release(tmp); return tmp; }
  SharedPtr take()
  { SharedPtr sharedPtr; sharedPtr.swap(*this); return sharedPtr; }

  bool isShared(void) const
  { return T::shared(_ptr); }
  unsigned getNumRefs(void) const
  { return T::count(_ptr); }

  bool valid(void) const
  { return 0 != _ptr; }

  void clear()
  { put(); }
  SharedPtr& swap(SharedPtr& sharedPtr)
  { T* tmp = _ptr; _ptr = sharedPtr._ptr; sharedPtr._ptr = tmp; return *this; }

private:
  void assign(T* p)
  { T::get(p); put(); _ptr = p; }
  void assignFirst(T* p)
  { T::getFirst(p); put(); _ptr = p; }
  void assignNonRef(T* p)
  { put(); _ptr = p; }

  void put(void)
  { T* tmp = _ptr; _ptr = 0; if (!T::put(tmp)) T::destruct(tmp); }

  // The reference itself.
  T* _ptr;

  template<typename U>
  friend class WeakPtr;
};

// Hmmm, what if we get an automatic cast to a shared pointer of a fresh
// allocated object pointer?
// This will be gone after destruction of the temporary object ...
// The same applies to the < operator below ...

template<typename T>
inline bool
operator==(const SharedPtr<T>& sharedPtr0, const SharedPtr<T>& sharedPtr1)
{ return sharedPtr0.get() == sharedPtr1.get(); }

template<typename T>
inline bool
operator!=(const SharedPtr<T>& sharedPtr0, const SharedPtr<T>& sharedPtr1)
{ return sharedPtr0.get() != sharedPtr1.get(); }

template<typename T>
inline bool
operator<(const SharedPtr<T>& sharedPtr0, const SharedPtr<T>& sharedPtr1)
{ return sharedPtr0.get() < sharedPtr1.get(); }

} // namespace OpenRTI

#endif
