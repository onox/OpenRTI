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

#include "WeakReferenced.h"

#include "Referenced.h"
#include "SharedPtr.h"

namespace OpenRTI {

WeakReferenced::WeakData::WeakData(WeakReferenced* weakReferenced) :
  mRefcount(lastbit()),
  mWeakReferenced(weakReferenced)
{
}

WeakReferenced::WeakData::~WeakData()
{
}

void
WeakReferenced::WeakData::weakReferencedGetFirst()
{
  unsigned count = mRefcount;
  for (;;) {
    unsigned newcount = (count & (~lastbit())) + 1;
    if (mRefcount.compareAndExchange(count, newcount))
      return;
    count = mRefcount;
  }
}

void
WeakReferenced::WeakData::weakReferencedRelease()
{
  unsigned count = mRefcount;
  for (;;) {
    unsigned newcount = count - 1;
    if (newcount == 0)
      newcount |= lastbit();
    if (mRefcount.compareAndExchange(count, newcount, Atomic::MemoryOrderAcqRel))
      return;
    count = mRefcount;
  }
}

WeakReferenced*
WeakReferenced::WeakData::getWeakReferenced()
{
  // Try to increment the reference count if the count is greater
  // then zero. Since it should only be incremented iff it is nonzero, we
  // need to check that value and try to do an atomic test and set. If this
  // fails, try again. The usual lockless algorithm ...
  unsigned count;
  do {
    count = mRefcount;
    if (count == 0)
      return 0;
  } while (!mRefcount.compareAndExchange(count, count + 1, Atomic::MemoryOrderAcqRel));
  // We know that as long as the refcount is not zero, the pointer still
  // points to valid data. So it is safe to work on it.
  return mWeakReferenced;
}

} // namespace OpenRTI
