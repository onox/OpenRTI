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

#ifndef HandleFriend_h
#define HandleFriend_h

#include <RTI/Handle.h>
#include "Handle.h"

namespace rti1516e
{

#define DECLARE_HANDLE_CLASS(HandleKind)                                \
  class RTI_EXPORT HandleKind##Friend {                                 \
  public:                                                               \
    static HandleKind                                                   \
    decode(rti1516e::VariableLengthData const & encodedValue);          \
    static void                                                         \
    copy(OpenRTI::HandleKind& dst, HandleKind const& src);              \
    static void                                                         \
    copy(HandleKind& dst, OpenRTI::HandleKind const& src);              \
  };

DECLARE_HANDLE_CLASS(FederateHandle)
DECLARE_HANDLE_CLASS(ObjectClassHandle)
DECLARE_HANDLE_CLASS(InteractionClassHandle)
DECLARE_HANDLE_CLASS(ObjectInstanceHandle)
DECLARE_HANDLE_CLASS(AttributeHandle)
DECLARE_HANDLE_CLASS(ParameterHandle)
DECLARE_HANDLE_CLASS(DimensionHandle)
DECLARE_HANDLE_CLASS(RegionHandle)
DECLARE_HANDLE_CLASS(MessageRetractionHandle)

#undef DECLARE_HANDLE_CLASS

}

#endif
