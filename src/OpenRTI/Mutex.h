/* -*-c++-*- OpenRTI - Copyright (C) 2004-2010 Mathias Froehlich 
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

#ifndef OpenRTI_Mutex_h
#define OpenRTI_Mutex_h

#include "Export.h"

namespace OpenRTI {

class Condition;
class Clock;

class OPENRTI_API Mutex {
public:
  Mutex(void);
  ~Mutex(void);

  void lock(void) const;
  void unlock(void) const;

private:
  Mutex(const Mutex&);
  Mutex& operator=(const Mutex&);

  struct PrivateData;
  PrivateData* _privateData;

  friend class Condition;
};

} // namespace OpenRTI

#endif
