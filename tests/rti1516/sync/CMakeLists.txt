ADD_EXECUTABLE(sync-1516 sync.cpp)
SET_PROPERTY(TARGET sync-1516 PROPERTY OUTPUT_NAME sync)
TARGET_LINK_LIBRARIES(sync-1516 rti1516 fedtime1516 OpenRTI)

# No server - thread protocol, one ambassador
ADD_TEST(rti1516/sync-1516-1 ${CMAKE_CURRENT_BINARY_DIR}/sync -S0 -A1 -O ${RTI1516_FDD_FILE})
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/sync-1516-2 ${CMAKE_CURRENT_BINARY_DIR}/sync -S0 -A10 -O ${RTI1516_FDD_FILE})
# 1 server - rti protocol, one ambassador
ADD_TEST(rti1516/sync-1516-3 ${CMAKE_CURRENT_BINARY_DIR}/sync -S1 -A1 -O ${RTI1516_FDD_FILE})
# 1 server - rti protocol, 10 ambassadors
ADD_TEST(rti1516/sync-1516-4 ${CMAKE_CURRENT_BINARY_DIR}/sync -S1 -A10 -O ${RTI1516_FDD_FILE})
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/sync-1516-5 ${CMAKE_CURRENT_BINARY_DIR}/sync -S5 -A10 -O ${RTI1516_FDD_FILE})
