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

#include <Options.h>
#include <StringUtils.h>

#include <RTI13TestLib.h>

int
main(int argc, char* argv[])
{
  std::string federationExecutionName("thread://federationExecutionName");
  bool useDataUrlObjectModels = false;
  std::string fullPathNameToTheFDDfile("fdd.fed");

  std::string objectClassName;
  std::vector<std::string> attributeNameVector;

  std::string interactionClassName;
  std::vector<std::string> parameterNameVector;

  OpenRTI::Options options(argc, argv);
  std::vector<std::string> args;
  while (options.next("a:Di:F:o:O:p:")) {
    switch (options.getOptChar()) {
    case 'a':
      attributeNameVector.push_back(options.getArgument());
      break;
    case 'D':
      useDataUrlObjectModels = true;
      break;
    case 'i':
      interactionClassName = options.getArgument();
      break;
    case 'F':
      federationExecutionName = options.getArgument();
      break;
    case 'O':
      fullPathNameToTheFDDfile = options.getArgument();
      break;
    case 'o':
      objectClassName = options.getArgument();
      break;
    case 'p':
      parameterNameVector.push_back(options.getArgument());
      break;
    case '\0':
      args.push_back(options.getArgument());
      break;
    }
  }

  OpenRTI::RTI13SimpleAmbassador ambassador;
  ambassador.setUseDataUrlObjectModels(useDataUrlObjectModels);

  ambassador.connect(args);

  // create, must work
  try {
    ambassador.createFederationExecution(federationExecutionName, fullPathNameToTheFDDfile);
  } catch (const RTI::Exception& e) {
    std::cout << "RTI::Exception: \"" << e._reason << "\"" << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cout << "Unknown Exception!" << std::endl;
    return EXIT_FAILURE;
  }

  // join must work
  try {
    ambassador.joinFederationExecution("federate", federationExecutionName);
  } catch (const RTI::Exception& e) {
    std::cout << "RTI::Exception: \"" << e._reason << "\"" << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cout << "Unknown Exception!" << std::endl;
    return EXIT_FAILURE;
  }

  if (!objectClassName.empty()) {
    RTI::ObjectClassHandle objectClassHandle;
    std::string rtiName;
    try {
      objectClassHandle = ambassador.getObjectClassHandle(objectClassName);
      rtiName = ambassador.getObjectClassName(objectClassHandle);
    } catch (const RTI::Exception& e) {
      std::cout << "RTI::Exception: \"" << e._reason << "\"" << std::endl;
      return EXIT_FAILURE;
    } catch (...) {
      std::cout << "Unknown Exception!" << std::endl;
      return EXIT_FAILURE;
    }
    std::cout << objectClassName << ": \"" << rtiName << "\" " << objectClassHandle << std::endl;

    for (std::vector<std::string>::const_iterator i = attributeNameVector.begin(); i != attributeNameVector.end(); ++i) {
      RTI::AttributeHandle attributeHandle;
      try {
        attributeHandle = ambassador.getAttributeHandle(objectClassHandle, *i);
        rtiName = ambassador.getAttributeName(objectClassHandle, attributeHandle);
      } catch (const RTI::Exception& e) {
        std::cout << "RTI::Exception: \"" << e._reason << "\"" << std::endl;
        return EXIT_FAILURE;
      } catch (...) {
        std::cout << "Unknown Exception!" << std::endl;
        return EXIT_FAILURE;
      }
      std::cout << "  " << *i << ": \"" << rtiName << "\" " << attributeHandle << std::endl;
    }
  }

  if (!interactionClassName.empty()) {
    RTI::InteractionClassHandle interactionClassHandle;
    std::string rtiName;
    try {
      interactionClassHandle = ambassador.getInteractionClassHandle(interactionClassName);
      rtiName = ambassador.getInteractionClassName(interactionClassHandle);
    } catch (const RTI::Exception& e) {
      std::cout << "RTI::Exception: \"" << e._reason << "\"" << std::endl;
      return EXIT_FAILURE;
    } catch (...) {
      std::cout << "Unknown Exception!" << std::endl;
      return EXIT_FAILURE;
    }
    std::cout << interactionClassName << ": \"" << rtiName << "\" " << interactionClassHandle << std::endl;

    for (std::vector<std::string>::const_iterator i = parameterNameVector.begin(); i != parameterNameVector.end(); ++i) {
      RTI::ParameterHandle parameterHandle;
      try {
        parameterHandle = ambassador.getParameterHandle(interactionClassHandle, *i);
        rtiName = ambassador.getParameterName(interactionClassHandle, parameterHandle);
      } catch (const RTI::Exception& e) {
        std::cout << "RTI::Exception: \"" << e._reason << "\"" << std::endl;
        return EXIT_FAILURE;
      } catch (...) {
        std::cout << "Unknown Exception!" << std::endl;
        return EXIT_FAILURE;
      }
      std::cout << "  " << *i << ": \"" << rtiName << "\" " << parameterHandle << std::endl;
    }
  }

  // and now resign must work
  try {
    ambassador.resignFederationExecution(RTI::NO_ACTION);
  } catch (const RTI::Exception& e) {
    std::cout << "RTI::Exception: \"" << e._reason << "\"" << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cout << "Unknown Exception!" << std::endl;
    return EXIT_FAILURE;
  }

  // destroy, must work
  try {
    ambassador.destroyFederationExecution(federationExecutionName);
  } catch (const RTI::Exception& e) {
    std::cout << e._reason << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cout << "Unknown Exception!" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
