# Name reservation
ADD_EXECUTABLE(objectname-1516 objectname.cpp)
TARGET_LINK_LIBRARIES(objectname-1516 rti1516 fedtime1516 OpenRTI)

IF(OPENRTI_ENABLE_PROJECT_FOLDERS)
  SET_PROPERTY(TARGET objectname-1516 PROPERTY FOLDER "Tests")
ENDIF(OPENRTI_ENABLE_PROJECT_FOLDERS)

# Object attribute updates
ADD_EXECUTABLE(objectinstance-1516 objectinstance.cpp)
TARGET_LINK_LIBRARIES(objectinstance-1516 rti1516 fedtime1516 OpenRTI)

IF(OPENRTI_ENABLE_PROJECT_FOLDERS)
  SET_PROPERTY(TARGET objectinstance-1516 PROPERTY FOLDER "Tests")
ENDIF(OPENRTI_ENABLE_PROJECT_FOLDERS)

# No server - thread protocol, one ambassador
ADD_TEST(rti1516/objectname-1516-1 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/objectname-1516" -S0 -A1 -J -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml")
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/objectname-1516-2 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/objectname-1516" -S0 -A10 -J -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml")
# 1 server - rti protocol, one ambassador
ADD_TEST(rti1516/objectname-1516-3 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/objectname-1516" -S1 -A1 -J -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml")
# 1 server - rti protocol, 10 ambassadors
ADD_TEST(rti1516/objectname-1516-4 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/objectname-1516" -S1 -A10 -J -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml")
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/objectname-1516-5 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/objectname-1516" -S5 -A10 -J -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml")

# No server - thread protocol, one ambassador
ADD_TEST(rti1516/objectinstance-1516-1 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/objectinstance-1516" -S0 -A1 -J -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml")
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/objectinstance-1516-2 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/objectinstance-1516" -S0 -A10 -J -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml")
# 1 server - rti protocol, one ambassador
ADD_TEST(rti1516/objectinstance-1516-3 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/objectinstance-1516" -S1 -A1 -J -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml")
# 1 server - rti protocol, 10 ambassadors
ADD_TEST(rti1516/objectinstance-1516-4 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/objectinstance-1516" -S1 -A10 -J -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml")
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/objectinstance-1516-5 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/objectinstance-1516" -S5 -A10 -J -O "${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml")
