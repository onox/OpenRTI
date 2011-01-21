/* -*-c++-*- OpenRTI - Copyright (C) 2004-2011 Mathias Froehlich 
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

#ifndef OpenRTI_Condition_h
#define OpenRTI_Condition_h

#include "Export.h"

namespace OpenRTI {

class Clock;
class Mutex;

class OPENRTI_API Condition {
public:
  Condition(void);
  ~Condition(void);

  void signal(void);
  void broadcast(void);
  void wait(Mutex& mutex);
  bool wait(Mutex& mutex, const Clock& timeout);

private:
  Condition(const Condition&);
  Condition& operator=(const Condition&);

  struct PrivateData;
  PrivateData* _privateData;
};

} // namespace OpenRTI

#endif
