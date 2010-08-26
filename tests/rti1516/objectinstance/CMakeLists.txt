# Name reservation
ADD_EXECUTABLE(objectname objectname.cpp)
TARGET_LINK_LIBRARIES(objectname rti1516 fedtime1516 OpenRTI)

# Object attribute updates
ADD_EXECUTABLE(objectinstance objectinstance.cpp)
TARGET_LINK_LIBRARIES(objectinstance rti1516 fedtime1516 OpenRTI)

# No server - thread protocol, one ambassador
ADD_TEST(rti1516/objectname-1 ${CMAKE_CURRENT_BINARY_DIR}/objectname -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/objectname-2 ${CMAKE_CURRENT_BINARY_DIR}/objectname -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
ADD_TEST(rti1516/objectname-3 ${CMAKE_CURRENT_BINARY_DIR}/objectname -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
ADD_TEST(rti1516/objectname-4 ${CMAKE_CURRENT_BINARY_DIR}/objectname -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/objectname-5 ${CMAKE_CURRENT_BINARY_DIR}/objectname -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

# No server - thread protocol, one ambassador
ADD_TEST(rti1516/objectinstance-1 ${CMAKE_CURRENT_BINARY_DIR}/objectinstance -S0 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/objectinstance-2 ${CMAKE_CURRENT_BINARY_DIR}/objectinstance -S0 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, one ambassador
ADD_TEST(rti1516/objectinstance-3 ${CMAKE_CURRENT_BINARY_DIR}/objectinstance -S1 -A1 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 1 server - rti protocol, 10 ambassadors
ADD_TEST(rti1516/objectinstance-4 ${CMAKE_CURRENT_BINARY_DIR}/objectinstance -S1 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/objectinstance-5 ${CMAKE_CURRENT_BINARY_DIR}/objectinstance -S5 -A10 -J -O ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)
