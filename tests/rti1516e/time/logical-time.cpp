/* -*-c++-*- OpenRTI - Copyright (C) 2012-2022 Mathias Froehlich
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
#include <memory>
#include <string>

#include <RTI/LogicalTime.h>
#include <RTI/LogicalTimeFactory.h>
#include <RTI/LogicalTimeInterval.h>

#include <Options.h>
#include <StringUtils.h>

// FIXME adapt the tests to check for these explicitly.
//
// The standard paper wants:
// let Iz = 0, Ie = epsilon, Iz < I
// any T denotes a value of type time,
// any I denotes a value of type interval:
//
// T' = T + I then T' > T          (1)
// T' = T - I then T' < T          (1)
// T' = T + Iz then T' == T        (2)
// T' = T - Iz then T' == T        (2)
// T1 != T2 then |T1 - T2| >= Ie   (3)
// T1 == T2 then T1 - T2 == Iz     (3)
//
// I' = I + Iz then I' == I        (4)
// I' = I - Iz then I' == I        (4)
// I1 != I2 then |I1 - I2| >= Ie   (5)
// I1 == I2 then I1 - I2 == Iz     (5)
// I != Iz then I >= Ie            (6)
//
// T > T' then T' < T              (7)

int
main(int argc, char* argv[])
{
  std::string implementationName;

  OpenRTI::Options options(argc, argv);
  while (options.next("F:")) {
    switch (options.getOptChar()) {
    case 'F':
      implementationName = options.getArgument();
      break;
    }
  }

  RTI_UNIQUE_PTR<rti1516e::LogicalTimeFactory> factory;
  factory = rti1516e::LogicalTimeFactoryFactory::makeLogicalTimeFactory(OpenRTI::localeToUcs(implementationName));
  if (!factory.get()) {
    std::cerr << "Can not create logical time factory: \"" << implementationName << "\"!" << std::endl;
    return EXIT_FAILURE;
  }

  RTI_UNIQUE_PTR<rti1516e::LogicalTimeInterval> logicalTimeInterval = factory->makeZero();
  if (!logicalTimeInterval.get()) {
    std::cerr << "Can not create logical time interval: \"" << implementationName << "\"!" << std::endl;
    return EXIT_FAILURE;
  }
  if (!logicalTimeInterval->isZero()) {
    std::cerr << "Logical time interval is not zero after setting it to zero!" << std::endl;
    return EXIT_FAILURE;
  }
  if (logicalTimeInterval->isEpsilon()) {
    std::cerr << "Logical time interval is epsilon after setting it to zero!" << std::endl;
    return EXIT_FAILURE;
  }

  logicalTimeInterval->setEpsilon();
  if (logicalTimeInterval->isZero()) {
    std::cerr << "Logical time interval is zero after setting it to epsilon!" << std::endl;
    return EXIT_FAILURE;
  }
  if (!logicalTimeInterval->isEpsilon()) {
    std::cerr << "Logical time interval is not epsilon after setting it to epsilon!" << std::endl;
    return EXIT_FAILURE;
  }

  RTI_UNIQUE_PTR<rti1516e::LogicalTime> logicalTime = factory->makeInitial();
  if (!logicalTime.get()) {
    std::cerr << "Can not create logical time: \"" << implementationName << "\"" << std::endl;
    return EXIT_FAILURE;
  }
  if (!logicalTime->isInitial()) {
    std::cerr << "Logical time is not initial after setting it to initial!" << std::endl;
    return EXIT_FAILURE;
  }
  if (logicalTime->isFinal()) {
    std::cerr << "Logical time is final after setting it to initial!" << std::endl;
    return EXIT_FAILURE;
  }

  logicalTime->setFinal();
  if (logicalTime->isInitial()) {
    std::cerr << "Logical time is initial after setting it to final!" << std::endl;
    return EXIT_FAILURE;
  }
  if (!logicalTime->isFinal()) {
    std::cerr << "Logical time is not final after setting it to final!" << std::endl;
    return EXIT_FAILURE;
  }

  RTI_UNIQUE_PTR<rti1516e::LogicalTime> initialTime = factory->makeInitial();
  if (!initialTime.get()) {
    std::cerr << "Can not create logical time: \"" << implementationName << "\"!" << std::endl;
    return EXIT_FAILURE;
  }

  RTI_UNIQUE_PTR<rti1516e::LogicalTime> finalTime = factory->makeFinal();
  if (!finalTime.get()) {
    std::cerr << "Can not create logical time: \"" << implementationName << "\"!" << std::endl;
    return EXIT_FAILURE;
  }


  logicalTime->setInitial();
  if (!(*initialTime == *logicalTime)) {
    std::cerr << "Initial times are not equal!" << std::endl;
    return EXIT_FAILURE;
  }
  if (*finalTime == *logicalTime) {
    std::cerr << "Final time and initial time are equal!" << std::endl;
    return EXIT_FAILURE;
  }

  logicalTime->setFinal();
  if (*initialTime == *logicalTime) {
    std::cerr << "Final time and initial time are equal!" << std::endl;
    return EXIT_FAILURE;
  }
  if (!(*finalTime == *logicalTime)) {
    std::cerr << "Initial times are not equal!" << std::endl;
    return EXIT_FAILURE;
  }

  RTI_UNIQUE_PTR<rti1516e::LogicalTime> logicalTime2 = factory->makeInitial();
  if (!logicalTime2.get()) {
    std::cerr << "Can not create logical time: \"" << implementationName << "\"!" << std::endl;
    return EXIT_FAILURE;
  }
  RTI_UNIQUE_PTR<rti1516e::LogicalTimeInterval> logicalTimeInterval2 = factory->makeZero();
  if (!logicalTimeInterval2.get()) {
    std::cerr << "Can not create logical time interval: \"" << implementationName << "\"!" << std::endl;
    return EXIT_FAILURE;
  }

  // For any order of magnitude in start time ...
  logicalTimeInterval->setEpsilon();
  for (;;) {
    logicalTime->setInitial();
    *logicalTime += *logicalTimeInterval;
    *logicalTimeInterval += *logicalTimeInterval;
    if (*finalTime <= *logicalTime)
      break;

    // ... check if we can add arbitrary small increments to huge ones
    logicalTimeInterval2->setEpsilon();
    for (;;) {
      *logicalTime2 = *logicalTime;
      *logicalTime += *logicalTimeInterval2;
      *logicalTimeInterval2 += *logicalTimeInterval2;
      if (*finalTime <= *logicalTime)
        break;
      if (*logicalTime2 == *logicalTime) {
        std::cerr << "Previous time " << OpenRTI::ucsToLocale(logicalTime2->toString())
                  << " and initial time plus epsilon " << OpenRTI::ucsToLocale(logicalTime->toString())
                  << " are equal!" << std::endl;
        return EXIT_FAILURE;
      }
      if (!(*logicalTime2 < *logicalTime)) {
        std::cerr << "Initial time plus epsilon " << OpenRTI::ucsToLocale(logicalTime->toString())
                  << " is not greater than previous time " << OpenRTI::ucsToLocale(logicalTime2->toString())
                  << "!" << std::endl;
        return EXIT_FAILURE;
      }
      if (*initialTime == *logicalTime) {
        std::cerr << "Initial time " << OpenRTI::ucsToLocale(initialTime->toString())
                  << " and initial time plus epsilon " << OpenRTI::ucsToLocale(logicalTime->toString())
                  << " are equal!" << std::endl;
        return EXIT_FAILURE;
      }
      if (!(*initialTime < *logicalTime)) {
        std::cerr << "Initial time plus epsilon " << OpenRTI::ucsToLocale(logicalTime->toString())
                  << " is not greater than initial time " << OpenRTI::ucsToLocale(initialTime->toString())
                  << "!" << std::endl;
        return EXIT_FAILURE;
      }
    }
  }

  return EXIT_SUCCESS;
}
