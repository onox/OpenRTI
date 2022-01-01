/* -*-c++-*- OpenRTI - Copyright (C) 2009-2022 Mathias Froehlich
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

#ifndef OpenRTI_AbstractMessageReceiver_h
#define OpenRTI_AbstractMessageReceiver_h

#include "Referenced.h"
#include "SharedPtr.h"

namespace OpenRTI {

class AbstractMessage;
class Clock;

class OPENRTI_API AbstractMessageReceiver : public Referenced {
public:
  virtual ~AbstractMessageReceiver() {}
  virtual SharedPtr<const AbstractMessage> receive() = 0;
  virtual SharedPtr<const AbstractMessage> receive(const Clock& timeout) = 0;
  virtual bool empty() const = 0;
  virtual bool isOpen() const = 0;
};

} // namespace OpenRTI

#endif
