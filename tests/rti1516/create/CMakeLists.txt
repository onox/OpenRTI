ADD_EXECUTABLE(create create.cpp)
TARGET_LINK_LIBRARIES(create rti1516 fedtime1516 OpenRTI)
# No server - thread protocol, one ambassador
ADD_TEST(rti1516/create-1 ${CMAKE_CURRENT_BINARY_DIR}/create -S0 -A1 -O ${RTI1516_FDD_FILE})
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/create-2 ${CMAKE_CURRENT_BINARY_DIR}/create -S0 -A10 -O ${RTI1516_FDD_FILE})
# 1 server - rti protocol, one ambassador
ADD_TEST(rti1516/create-3 ${CMAKE_CURRENT_BINARY_DIR}/create -S1 -A1 -O ${RTI1516_FDD_FILE})
# 1 server - rti protocol, 10 ambassadors
ADD_TEST(rti1516/create-4 ${CMAKE_CURRENT_BINARY_DIR}/create -S1 -A10 -O ${RTI1516_FDD_FILE})
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/create-5 ${CMAKE_CURRENT_BINARY_DIR}/create -S5 -A10 -O ${RTI1516_FDD_FILE})

ADD_EXECUTABLE(concurrent-create concurrent-create.cpp)
TARGET_LINK_LIBRARIES(concurrent-create rti1516 fedtime1516 OpenRTI)
# No server - thread protocol, one ambassador
ADD_TEST(rti1516/concurrent-create-1 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-create -S0 -A10 -O ${RTI1516_FDD_FILE})
# 1 server - rti protocol, 10 ambassadors
ADD_TEST(rti1516/concurrent-create-2 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-create -S1 -A10 -O ${RTI1516_FDD_FILE})
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/concurrent-create-3 ${CMAKE_CURRENT_BINARY_DIR}/concurrent-create -S5 -A10 -O ${RTI1516_FDD_FILE})
