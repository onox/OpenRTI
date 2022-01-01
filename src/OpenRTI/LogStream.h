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

#ifndef OpenRTI_LogStream_H
#define OpenRTI_LogStream_H

#include <ostream>
#include "Export.h"
#include "Referenced.h"

namespace OpenRTI {

class OPENRTI_API LogStream {
public:
  enum Category {
    Assert               = 1,
    Network              = Assert << 1,
    MessageCoding        = Network << 1,
    FederateAmbassador   = MessageCoding << 1,
    ServerMessage        = FederateAmbassador << 1,
    ServerConnect        = ServerMessage << 1,
    ServerFederation     = ServerConnect << 1,
    ServerFederate       = ServerFederation << 1,
    ServerSyncronization = ServerFederate << 1,
    ServerTime           = ServerSyncronization << 1,
    ServerObjectInstance = ServerTime << 1,
    All                  = ~0u
  };

  enum Priority {
    /// Non recoverable error, either due to an implementation problem or
    /// due to a user problem probably ignoring previous error return values.
    /// Simulation results may not be valid.
    Error            = 0,
    /// Error Conditions that should be avoided, but could be
    /// worked around in some way.
    /// Simulation results may not be valid.
    Warning          = Error + 1,
    /// Error Conditions that are marked with an error return.
    /// The exact reason is probably explained in this kind of messages.
    /// These kind of errors do not lead to immediate problems in the
    /// simulation code. Anyway, when not handled correctly they might lead to
    /// Error or Warning conditions.
    Info             = Warning + 1,
    /// Blubber for debugging ...
    Debug            = Info + 1,
    Debug1           = Debug + 1,
    Debug2           = Debug1 + 1,
    Debug3           = Debug2 + 1
  };

  static void setCategoryEnable(Category category, bool enable = true);
  static void setCategoryDisable(Category category);
  static void setPriority(Priority priority);

  static std::ostream* getStaticStream(Category category, Priority priority)
  {
#if defined(NDEBUG) || defined(_NDEBUG)
    // In the NDEBUG case, give the compilers optimizer a chance to
    // completely remove some code.
    if (Debug <= priority)
      return 0;
#endif
    return Instance().getStream(category, priority);
  }

  LogStream();
protected:
  static LogStream& Instance(void);
  std::ostream* getStream(Category category, Priority priority);
  bool getEnabled(Category category, Priority priority)
  {
    if (priority == Error)
      return true;
    if (!(category & mCategory))
      return false;
    return priority <= mPriority;
  }

private:
  struct StreamPair;
  unsigned mCategory;
  int mPriority;
};

#define Log(c, p) \
if (std::ostream* s = OpenRTI::LogStream::getStaticStream(OpenRTI::LogStream::c, OpenRTI::LogStream::p)) *s

#define Bulk Log(All, Debug3)

} // namespace OpenRTI

#endif
