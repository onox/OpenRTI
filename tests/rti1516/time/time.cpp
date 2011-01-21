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

#include <cstdlib>
#include <string>
#include <memory>
#include <vector>
#include <iostream>

#include <RTI/HLAinteger64Time.h>
#include <RTI/HLAinteger64Interval.h>

#include <Options.h>
#include <StringUtils.h>
#include <Thread.h>

#include <RTI1516TestLib.h>

using namespace OpenRTI;

int
main(int argc, char* argv[])
{
  std::wstring federationExecution;
  std::wstring fddFile;
  std::vector<std::wstring> argumentList;

  unsigned numServers = 1;
  unsigned numClientsPerServers = 2;
  OpenRTI::Options options(argc, argv);
  while (options.next("C:F:O:S:T")) {
    switch (options.getOptChar()) {
    // case 'A':
    //   _numAmbassadorThreads = atoi(options.getArgument().c_str());
    //     break;
    case 'C':
      numClientsPerServers = atoi(options.getArgument().c_str());
      break;
    case 'F':
      federationExecution = localeToUcs(options.getArgument());
      break;
    case 'O':
      fddFile = localeToUcs(options.getArgument());
      break;
    case 'S':
      numServers = atoi(options.getArgument().c_str());
      break;
    // case 'T':
    //   _traceAmbassadors = true;
    //   break;
    case '\0':
      argumentList.push_back(localeToUcs(options.getArgument()));
      break;
    }
  }
  
  ServerPool _serverPool;
  _serverPool.startServerPool(numServers, numClientsPerServers);

  HLAinteger64Time logicalTime(0);
  std::wstring logicalTimeFactoryName = logicalTime.implementationName();

  // enum AmbassadorLabels {
  //   FreeAmbassador,
  //   TimeConstrainedAmbassador,
  //   TimeRegulatingAmbassador,
  // };

  RTI1516SimpleAmbassador _ambassador[5];

  for (unsigned i = 0; i < 5; ++i) {
    _ambassador[i].createAmbassador(argumentList);
    _ambassador[i].setLogicalTimeFactory(logicalTimeFactoryName);
  }

  _ambassador[0].createFederationExecution(federationExecution, fddFile);

  for (unsigned i = 0; i < 5; ++i) {
    std::wstringstream stream;
    stream << "federateType" << i;
    _ambassador[i].joinFederationExecution(stream.str(), federationExecution);
  }

  for (unsigned i = 0; i < 5; ++i) {
    HLAinteger64Time logicalTime(i + 1);
    _ambassador[i].timeAdvanceRequest(logicalTime);
    if (!_ambassador[i].evokeCallback(1))
      return EXIT_FAILURE;
    if (_ambassador[i].getTimeAdvancePending())
      return EXIT_FAILURE;
    if (!_ambassador[i].verifyGrantedLogicalTime(logicalTime))
      return EXIT_FAILURE;

    // FIXME check logical time
    // if (!_ambassador[i].verifyGrantedLogicalTime(logicalTime))
    //   return EXIT_FAILURE;
  }

  for (unsigned i = 0; i < 5; ++i) {
    _ambassador[i].enableTimeConstrained();
    if (!_ambassador[i].evokeCallback(1))
      return EXIT_FAILURE;
    // if (_ambassador[i].getTimeAdvancePending())
    //   return EXIT_FAILURE;
  }

  for (unsigned i = 0; i < 5; ++i) {
    HLAinteger64Interval logicalTimeInterval(1);
    _ambassador[i].enableTimeRegulation(logicalTimeInterval);
    // FIXME
    if (!_ambassador[i].evokeMultipleCallbacks(1, 1))
      return EXIT_FAILURE;
    // if (_ambassador[i].getTimeAdvancePending())
    //   return EXIT_FAILURE;
  }





  return 17;
}
