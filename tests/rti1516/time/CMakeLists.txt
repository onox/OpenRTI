ADD_EXECUTABLE(concurrent-time-1516 concurrent-time.cpp)
SET_PROPERTY(TARGET concurrent-time-1516 PROPERTY OUTPUT_NAME concurrent-time)
TARGET_LINK_LIBRARIES(concurrent-time-1516 rti1516 fedtime1516 OpenRTI)

# This is just to see if the opaque time implementation works as well
ADD_TEST(rti1516/concurrent-opaque-time-1516-1 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -S0 -A2 -J -O ${RTI1516_FDD_FILE} force-opaque-time)

# No server - thread protocol, one ambassador
ADD_TEST(rti1516/concurrent-time-1516-1 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -M0 -S0 -A1 -J -O ${RTI1516_FDD_FILE})
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/concurrent-time-1516-2 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -M0 -S0 -A10 -J -O ${RTI1516_FDD_FILE})
# 1 server - rti protocol, one ambassador
ADD_TEST(rti1516/concurrent-time-1516-3 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -M0 -S1 -A1 -J -O ${RTI1516_FDD_FILE})
# 1 server - rti protocol, 10 ambassadors
ADD_TEST(rti1516/concurrent-time-1516-4 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -M0 -S1 -A10 -J -O ${RTI1516_FDD_FILE})
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/concurrent-time-1516-5 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -M0 -S5 -A10 -J -O ${RTI1516_FDD_FILE})

# No server - thread protocol, one ambassador
ADD_TEST(rti1516/concurrent-time-1516-6 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -M0 -S0 -A1 -J -O ${RTI1516_FDD_FILE})
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/concurrent-time-1516-7 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -M0 -S0 -A10 -J -O ${RTI1516_FDD_FILE})
# 1 server - rti protocol, one ambassador
ADD_TEST(rti1516/concurrent-time-1516-8 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -M0 -S1 -A1 -J -O ${RTI1516_FDD_FILE})
# 1 server - rti protocol, 10 ambassadors
ADD_TEST(rti1516/concurrent-time-1516-9 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -M0 -S1 -A10 -J -O ${RTI1516_FDD_FILE})
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/concurrent-time-1516-10 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -M0 -S5 -A10 -J -O ${RTI1516_FDD_FILE})

# No server - thread protocol, one ambassador, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-11 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -M1 -S0 -A1 -J -O ${RTI1516_FDD_FILE})
# No server - thread protocol, 10 ambassadors, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-12 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -M1 -S0 -A10 -J -O ${RTI1516_FDD_FILE})
# 1 server - rti protocol, one ambassador, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-13 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -M1 -S1 -A1 -J -O ${RTI1516_FDD_FILE})
# 1 server - rti protocol, 10 ambassadors, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-14 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -M1 -S1 -A10 -J -O ${RTI1516_FDD_FILE})
# 5 servers - rti protocol, 10 ambassadors, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-15 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L0 -M1 -S5 -A10 -J -O ${RTI1516_FDD_FILE})

# No server - thread protocol, one ambassador, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-16 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -M1 -S0 -A1 -J -O ${RTI1516_FDD_FILE})
# No server - thread protocol, 10 ambassadors, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-17 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -M1 -S0 -A10 -J -O ${RTI1516_FDD_FILE})
# 1 server - rti protocol, one ambassador, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-18 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -M1 -S1 -A1 -J -O ${RTI1516_FDD_FILE})
# 1 server - rti protocol, 10 ambassadors, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-19 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -M1 -S1 -A10 -J -O ${RTI1516_FDD_FILE})
# 5 servers - rti protocol, 10 ambassadors, all within a single federation created once
ADD_TEST(rti1516/concurrent-time-1516-20 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-time -L1 -M1 -S5 -A10 -J -O ${RTI1516_FDD_FILE})
