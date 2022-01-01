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
#include <string>
#include <memory>
#include <vector>
#include <iostream>

#include <RTI/RTIambassadorFactory.h>
#include <RTI/RTIambassador.h>

#include <Options.h>
#include <StringUtils.h>

#include <RTI1516TestLib.h>
#include "NetworkServer.h"

int
main(int argc, char* argv[])
{
  std::wstring fullPathNameToTheFDDfile(L"fdd.xml");

  OpenRTI::Options options(argc, argv);
  std::vector<std::wstring> args;
  while (options.next("O:")) {
    switch (options.getOptChar()) {
    case 'O':
      fullPathNameToTheFDDfile = OpenRTI::localeToUcs(options.getArgument());
      break;
    case '\0':
      args.push_back(OpenRTI::localeToUcs(options.getArgument()));
      break;
    }
  }

  std::list<OpenRTI::SocketAddress> addressList = OpenRTI::SocketAddress::resolve("localhost", "0", true);
  if (addressList.empty())
    return EXIT_FAILURE;

  // Set up a stream socket for the server connect
  bool success = false;
  OpenRTI::SocketAddress listenAddress;
  while (!addressList.empty()) {
    OpenRTI::SocketAddress address = addressList.front();
    addressList.pop_front();

    // At first set up a listening server and determine an unused socket address
    OpenRTI::SharedPtr<OpenRTI::NetworkServer> listeningNetworkServer = new OpenRTI::NetworkServer;
    try {
      listenAddress = listeningNetworkServer->listenInet(address, 1);
      success = true;
    } catch (const OpenRTI::Exception&) {
      continue;
    }

    // Now retract the listen, which must result in an immediate connection refused
    // Note that this address should not even be reused within this time so this test is (hopefully?) safe.
    listeningNetworkServer.clear();

    std::wstring federationExecutionName(L"rti://");
    federationExecutionName += OpenRTI::localeToUcs(listenAddress.getNumericName());
    federationExecutionName += L"/nothingthrere";

    OpenRTI::RTI1516SimpleAmbassador ambassador;

    ambassador.connect(args);

    // all of them must not run into any timeout!! FIXME check this!!
    OpenRTI::Clock maxtime = OpenRTI::Clock::now() + OpenRTI::Clock::fromSeconds(20);

    // All those must not work
    // Note that rti1516 does not have a specific way to report connection failures.
    // Se we check here aginst RTIinternalError
    try {
      ambassador.createFederationExecution(federationExecutionName, fullPathNameToTheFDDfile);
      std::wcout << L"Must not be able to connect to the given address" << std::endl;
      return EXIT_FAILURE;
    } catch (const rti1516::RTIinternalError&) {
      // should report this
      // Note that rti1516 does not have a specific way to report connection failures.
      // So we check here aginst RTIinternalError.
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return EXIT_FAILURE;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return EXIT_FAILURE;
    }
    if (maxtime < OpenRTI::Clock::now()) {
      std::wcout << L"Timeout expecrienced!" << std::endl;
      return EXIT_FAILURE;
    }

    maxtime = OpenRTI::Clock::now() + OpenRTI::Clock::fromSeconds(20);

    try {
      ambassador.joinFederationExecution(L"federate", federationExecutionName);
      std::wcout << L"Must not be able to connect to the given address" << std::endl;
      return EXIT_FAILURE;
    } catch (const rti1516::FederationExecutionDoesNotExist&) {
      // should report this
    } catch (const rti1516::RTIinternalError&) {
      // or this
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return EXIT_FAILURE;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return EXIT_FAILURE;
    }
    if (maxtime < OpenRTI::Clock::now()) {
      std::wcout << L"Timeout expecrienced!" << std::endl;
      return EXIT_FAILURE;
    }

    maxtime = OpenRTI::Clock::now() + OpenRTI::Clock::fromSeconds(20);

    try {
      ambassador.resignFederationExecution(rti1516::NO_ACTION);
      std::wcout << L"Must not be able to connect to the given address" << std::endl;
      return EXIT_FAILURE;
    } catch (const rti1516::FederateNotExecutionMember&) {
      // should report this
    } catch (const rti1516::RTIinternalError&) {
      // or this
    } catch (const rti1516::Exception& e) {
      std::wcout << L"rti1516::Exception: \"" << e.what() << L"\"" << std::endl;
      return EXIT_FAILURE;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return EXIT_FAILURE;
    }
    if (maxtime < OpenRTI::Clock::now()) {
      std::wcout << L"Timeout expecrienced!" << std::endl;
      return EXIT_FAILURE;
    }

    maxtime = OpenRTI::Clock::now() + OpenRTI::Clock::fromSeconds(20);

    try {
      ambassador.destroyFederationExecution(federationExecutionName);
      std::wcout << L"Must not be able to connect to the given address" << std::endl;
      return EXIT_FAILURE;
    } catch (const rti1516::FederationExecutionDoesNotExist&) {
      // should report this
    } catch (const rti1516::RTIinternalError&) {
      // or this
    } catch (const rti1516::Exception& e) {
      std::wcout << e.what() << std::endl;
      return EXIT_FAILURE;
    } catch (...) {
      std::wcout << L"Unknown Exception!" << std::endl;
      return EXIT_FAILURE;
    }
    if (maxtime < OpenRTI::Clock::now()) {
      std::wcout << L"Timeout expecrienced!" << std::endl;
      return EXIT_FAILURE;
    }
  }
  if (!success) {
    std::wcout << L"Could not get any listening server to check against refused connects!" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
