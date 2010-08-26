ADD_EXECUTABLE(join join.cpp)
TARGET_LINK_LIBRARIES(join rti1516 fedtime1516 OpenRTI)
# No server - thread protocol, one ambassador
ADD_TEST(rti1516/join-1 ${CMAKE_CURRENT_BINARY_DIR}/join -S0 -A1 -O ${RTI1516_FDD_FILE})
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/join-2 ${CMAKE_CURRENT_BINARY_DIR}/join -S0 -A10 -O ${RTI1516_FDD_FILE})
# 1 server - rti protocol, one ambassador
ADD_TEST(rti1516/join-3 ${CMAKE_CURRENT_BINARY_DIR}/join -S1 -A1 -O ${RTI1516_FDD_FILE})
# 1 server - rti protocol, 10 ambassadors
ADD_TEST(rti1516/join-4 ${CMAKE_CURRENT_BINARY_DIR}/join -S1 -A10 -O ${RTI1516_FDD_FILE})
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/join-5 ${CMAKE_CURRENT_BINARY_DIR}/join -S5 -A10 -O ${RTI1516_FDD_FILE})

ADD_EXECUTABLE(concurrent-join concurrent-join.cpp)
TARGET_LINK_LIBRARIES(concurrent-join rti1516 fedtime1516 OpenRTI)
# No server - thread protocol, one ambassador
ADD_TEST(rti1516/concurrent-join-1 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-join -S0 -A10 -O ${RTI1516_FDD_FILE})
# 1 server - rti protocol, 10 ambassadors
ADD_TEST(rti1516/concurrent-join-2 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-join -S1 -A10 -O ${RTI1516_FDD_FILE})
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/concurrent-join-3 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-join -S5 -A10 -O ${RTI1516_FDD_FILE})
