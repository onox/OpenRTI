ADD_EXECUTABLE(sync sync.cpp)
TARGET_LINK_LIBRARIES(sync rti1516 fedtime1516 OpenRTI)
# No server - thread protocol, one ambassador
ADD_TEST(rti1516/sync-1 ${CMAKE_CURRENT_BINARY_DIR}/sync -S0 -A1 -O ${RTI1516_FDD_FILE})
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/sync-2 ${CMAKE_CURRENT_BINARY_DIR}/sync -S0 -A10 -O ${RTI1516_FDD_FILE})
# 1 server - rti protocol, one ambassador
ADD_TEST(rti1516/sync-3 ${CMAKE_CURRENT_BINARY_DIR}/sync -S1 -A1 -O ${RTI1516_FDD_FILE})
# 1 server - rti protocol, 10 ambassadors
ADD_TEST(rti1516/sync-4 ${CMAKE_CURRENT_BINARY_DIR}/sync -S1 -A10 -O ${RTI1516_FDD_FILE})
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/sync-5 ${CMAKE_CURRENT_BINARY_DIR}/sync -S5 -A10 -O ${RTI1516_FDD_FILE})
