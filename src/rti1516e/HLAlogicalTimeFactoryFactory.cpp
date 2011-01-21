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

// This time, the first include is above the api include.
// the rti1516/Exception header misses that.
#include <iosfwd>

#include <RTI/LogicalTimeFactory.h>

#include <memory>
#include <string>
#include <RTI/time/HLAinteger64TimeFactory.h>
#include <RTI/time/HLAfloat64TimeFactory.h>

namespace rti1516e {

std::auto_ptr<LogicalTimeFactory>
HLAlogicalTimeFactoryFactory::makeLogicalTimeFactory(const std::wstring& implementationName)
{
  if (implementationName.empty())
    return std::auto_ptr<LogicalTimeFactory>(new HLAinteger64TimeFactory);
  else if (implementationName == L"HLAinteger64Time")
    return std::auto_ptr<LogicalTimeFactory>(new HLAinteger64TimeFactory);
  else if (implementationName == L"HLAfloat64Time")
    return std::auto_ptr<LogicalTimeFactory>(new HLAfloat64TimeFactory);
  else
    return std::auto_ptr<LogicalTimeFactory>(0);
}

}
