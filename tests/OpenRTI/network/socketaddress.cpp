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

#include <cstdlib>
#include <iostream>

#include "SocketAddress.h"

static bool
testSelfRelations(const OpenRTI::SocketAddress& socketAddress)
{
  if (socketAddress != socketAddress)
    return false;
  if (!(socketAddress == socketAddress))
    return false;
  if (socketAddress < socketAddress)
    return false;
  if (!(socketAddress <= socketAddress))
    return false;
  if (socketAddress > socketAddress)
    return false;
  if (!(socketAddress >= socketAddress))
    return false;

  return true;
}

static bool
testNumericNameResolvesToItSelf(const OpenRTI::SocketAddress& socketAddress, bool passive)
{
  std::string name = socketAddress.getNumericName();
  OpenRTI::SocketAddressList addressList = OpenRTI::SocketAddress::resolve(name, passive);
  // Ok, since the resolve step injects the infiniband addresses, just check if we have one that resolves back
  // FIXME rethink this
  bool matched = false;
  for (OpenRTI::SocketAddressList::const_iterator i = addressList.begin(); i != addressList.end(); ++i) {
    if (!i->valid())
      return false;
    if (i->isPipe() != socketAddress.isPipe())
      return false;
    if (i->isInet4() != socketAddress.isInet4())
      return false;
    if (i->isInet6() != socketAddress.isInet6())
      return false;
    if (*i != socketAddress)
      continue;
    matched = true;
  }

  return matched;
}

int main(int argc, char* argv[])
{
  OpenRTI::SocketAddress socketAddress;
  if (socketAddress.valid())
    return EXIT_FAILURE;
  if (socketAddress.isPipe())
    return EXIT_FAILURE;
  if (socketAddress.isInet4())
    return EXIT_FAILURE;
  if (socketAddress.isInet6())
    return EXIT_FAILURE;
  if (!testSelfRelations(socketAddress))
    return EXIT_FAILURE;

  OpenRTI::SocketAddressList addressList;

  addressList = OpenRTI::SocketAddress::resolve("0.0.0.0", true);
  for (OpenRTI::SocketAddressList::const_iterator i = addressList.begin(); i != addressList.end(); ++i) {
    if (!i->valid())
      return EXIT_FAILURE;
    if (i->isPipe())
      return EXIT_FAILURE;
    if (!i->isInet4())
      return EXIT_FAILURE;
    if (i->isInet6())
      return EXIT_FAILURE;
    if (!testSelfRelations(*i))
      return EXIT_FAILURE;
    if (!testNumericNameResolvesToItSelf(*i, true))
      return EXIT_FAILURE;
  }

  addressList = OpenRTI::SocketAddress::resolve("::", true);
  for (OpenRTI::SocketAddressList::const_iterator i = addressList.begin(); i != addressList.end(); ++i) {
    if (!i->valid())
      return EXIT_FAILURE;
    if (i->isPipe())
      return EXIT_FAILURE;
    if (i->isInet4())
      return EXIT_FAILURE;
    if (!i->isInet6())
      return EXIT_FAILURE;
    if (!testSelfRelations(*i))
      return EXIT_FAILURE;
    if (!testNumericNameResolvesToItSelf(*i, true))
      return EXIT_FAILURE;
  }

  addressList = OpenRTI::SocketAddress::resolve(std::string(), true);
  for (OpenRTI::SocketAddressList::const_iterator i = addressList.begin(); i != addressList.end(); ++i) {
    if (!i->valid())
      return EXIT_FAILURE;
    if (i->isPipe())
      return EXIT_FAILURE;
    if (!testSelfRelations(*i))
      return EXIT_FAILURE;
    if (!testNumericNameResolvesToItSelf(*i, true))
      return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
