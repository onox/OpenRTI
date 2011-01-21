/* -*-c++-*- OpenRTI - Copyright (C) 2009-2011 Mathias Froehlich
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

#ifndef OpenRTI_AbstractMessageEncoder_h
#define OpenRTI_AbstractMessageEncoder_h

#include "Referenced.h"

namespace OpenRTI {

class AbstractMessage;
class NetworkBuffer;

class OPENRTI_LOCAL AbstractMessageEncoder : public Referenced {
public:
  virtual ~AbstractMessageEncoder();
  virtual const char* getName() const = 0;
  virtual void encodeMessage(NetworkBuffer& networkBuffer, const AbstractMessage& message) = 0;
};

} // namespace OpenRTI

#endif
