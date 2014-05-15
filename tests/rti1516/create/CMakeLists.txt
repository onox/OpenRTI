ADD_EXECUTABLE(create-1516 create.cpp)
TARGET_LINK_LIBRARIES(create-1516 rti1516 fedtime1516 OpenRTI)

# No server - thread protocol, one ambassador
ADD_TEST(rti1516/create-1516-1 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/create-1516" -S0 -A1 -O "${RTI1516_FDD_FILE}")
# No server - thread protocol, 10 ambassadors
ADD_TEST(rti1516/create-1516-2 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/create-1516" -S0 -A10 -O "${RTI1516_FDD_FILE}")
# 1 server - rti protocol, one ambassador
ADD_TEST(rti1516/create-1516-3 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/create-1516" -S1 -A1 -O "${RTI1516_FDD_FILE}")
# 1 server - rti protocol, 10 ambassadors
ADD_TEST(rti1516/create-1516-4 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/create-1516" -S1 -A10 -O "${RTI1516_FDD_FILE}")
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/create-1516-5 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/create-1516" -S5 -A10 -O "${RTI1516_FDD_FILE}")

ADD_EXECUTABLE(concurrent-create-1516 concurrent-create.cpp)
TARGET_LINK_LIBRARIES(concurrent-create-1516 rti1516 fedtime1516 OpenRTI)

# No server - thread protocol, one ambassador
ADD_TEST(rti1516/concurrent-create-1516-1 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/concurrent-create-1516" -S0 -A10 -O "${RTI1516_FDD_FILE}")
# 1 server - rti protocol, 10 ambassadors
ADD_TEST(rti1516/concurrent-create-1516-2 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/concurrent-create-1516" -S1 -A10 -O "${RTI1516_FDD_FILE}")
# 5 servers - rti protocol, 10 ambassadors
ADD_TEST(rti1516/concurrent-create-1516-3 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/concurrent-create-1516" -S5 -A10 -O "${RTI1516_FDD_FILE}")
