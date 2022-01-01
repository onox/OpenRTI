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

// This time, the first include is above the api include.
// the rti1516/Exception header misses that.
#include <iosfwd>

#include <RTI/LogicalTimeFactory.h>

#include <memory>
#include <string>
#include <RTI/HLAinteger64TimeFactory.h>
#include <RTI/HLAfloat64TimeFactory.h>
#include <RTI/RTI1516fedTime.h>

namespace rti1516 {

RTI_UNIQUE_PTR<LogicalTimeFactory>
LogicalTimeFactoryFactory::makeLogicalTimeFactory(const std::wstring& implementationName)
{
  if (implementationName.empty())
    return RTI_UNIQUE_PTR<LogicalTimeFactory>(new HLAfloat64TimeFactory);
  else if (implementationName == L"HLAfloat64Time")
    return RTI_UNIQUE_PTR<LogicalTimeFactory>(new HLAfloat64TimeFactory);
  else if (implementationName == L"HLAinteger64Time")
    return RTI_UNIQUE_PTR<LogicalTimeFactory>(new HLAinteger64TimeFactory);
  else if (implementationName == L"certiFedTime1516")
    return RTI_UNIQUE_PTR<LogicalTimeFactory>(new RTI1516fedTimeFactory);
  else if (implementationName == L"CERTI")
    return RTI_UNIQUE_PTR<LogicalTimeFactory>(new RTI1516fedTimeFactory);
  else
    return RTI_UNIQUE_PTR<LogicalTimeFactory>();
}

}
