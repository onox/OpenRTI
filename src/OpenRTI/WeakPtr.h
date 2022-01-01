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

#ifndef OpenRTI_WeakPtr_H
#define OpenRTI_WeakPtr_H

#include "OpenRTIConfig.h"
#include "WeakReferenced.h"

namespace OpenRTI {

template<typename T>
inline bool operator==(const WeakPtr<T>& p1, const WeakPtr<T>& p2);
template<typename T>
inline bool operator<(const WeakPtr<T>& p1, const WeakPtr<T>& p2);

template<typename T>
class OPENRTI_LOCAL WeakPtr {
public:
  WeakPtr(void)
  { }
  WeakPtr(const WeakPtr& p) : mWeakData(p.mWeakData)
  { }
  template<typename U>
  WeakPtr(const SharedPtr<U>& p)
  { SharedPtr<T> sharedPtr = p; assign(sharedPtr.get()); }
  template<typename U>
  WeakPtr(const WeakPtr<U>& p)
  { SharedPtr<T> sharedPtr = p.lock(); assign(sharedPtr.get()); }
  WeakPtr(T* ptr) // OpenRTI_DEPRECATED // Hmm, shall we??
  { assign(ptr); }
  ~WeakPtr(void)
  { }

  template<typename U>
  WeakPtr& operator=(const SharedPtr<U>& p)
  { SharedPtr<T> sharedPtr = p; assign(sharedPtr.get()); return *this; }
  template<typename U>
  WeakPtr& operator=(const WeakPtr<U>& p)
  { SharedPtr<T> sharedPtr = p.lock(); assign(sharedPtr.get()); return *this; }
  WeakPtr& operator=(const WeakPtr& p)
  { mWeakData = p.mWeakData; return *this; }

  SharedPtr<T> lock(void) const
  {
    if (!mWeakData.valid())
      return SharedPtr<T>();
    SharedPtr<T> sharedPtr;
    sharedPtr.assignNonRef(static_cast<T*>(mWeakData->getWeakReferenced()));
    return sharedPtr;
  }

  void clear()
  { mWeakData = 0; }
  WeakPtr& swap(WeakPtr& weakPtr)
  { mWeakData.swap(weakPtr.mWeakData); return *this; }

private:
  void assign(T* p)
  {
    if (p)
      mWeakData = p->mWeakData;
    else
      mWeakData = 0;
  }

  // The indirect reference itself.
  SharedPtr<WeakReferenced::WeakData> mWeakData;

  template<typename S>
  friend bool operator==(const WeakPtr<S>& p1, const WeakPtr<S>& p2);
  template<typename S>
  friend bool operator<(const WeakPtr<S>& p1, const WeakPtr<S>& p2);
};

template<typename T>
inline bool
operator==(const WeakPtr<T>& p1, const WeakPtr<T>& p2)
{ return p1.mWeakData == p2.mWeakData; }

template<typename T>
inline bool
operator!=(const WeakPtr<T>& p1, const WeakPtr<T>& p2)
{ return !(p1 == p2); }

template<typename T>
inline bool
operator<(const WeakPtr<T>& p1, const WeakPtr<T>& p2)
{ return p1.mWeakData < p2.mWeakData; }

template<typename T>
inline bool
operator>(const WeakPtr<T>& p1, const WeakPtr<T>& p2)
{ return p2 < p1; }

template<typename T>
inline bool
operator<=(const WeakPtr<T>& p1, const WeakPtr<T>& p2)
{ return !(p1 > p2); }

template<typename T>
inline bool
operator>=(const WeakPtr<T>& p1, const WeakPtr<T>& p2)
{ return !(p1 < p2); }

} // namespace OpenRTI

#endif
