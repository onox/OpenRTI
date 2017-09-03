add_executable(join-1516 join.cpp)
target_link_libraries(join-1516 rti1516 fedtime1516 OpenRTI)

# No server - thread protocol, one ambassador
add_test(rti1516/join-1516-1 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/join-1516" -S0 -A1 -O "${RTI1516_FDD_FILE}")
# No server - thread protocol, 10 ambassadors
add_test(rti1516/join-1516-2 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/join-1516" -S0 -A10 -O "${RTI1516_FDD_FILE}")
# 1 server - rti protocol, one ambassador
add_test(rti1516/join-1516-3 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/join-1516" -S1 -A1 -O "${RTI1516_FDD_FILE}")
# 1 server - rti protocol, 10 ambassadors
add_test(rti1516/join-1516-4 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/join-1516" -S1 -A10 -O "${RTI1516_FDD_FILE}")
# 5 servers - rti protocol, 10 ambassadors
add_test(rti1516/join-1516-5 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/join-1516" -S5 -A10 -O "${RTI1516_FDD_FILE}")

add_executable(concurrent-join-1516 concurrent-join.cpp)
target_link_libraries(concurrent-join-1516 rti1516 fedtime1516 OpenRTI)

# No server - thread protocol, one ambassador
add_test(rti1516/concurrent-join-1516-1 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/concurrent-join-1516" -S0 -A10 -O "${RTI1516_FDD_FILE}")
# 1 server - rti protocol, 10 ambassadors
add_test(rti1516/concurrent-join-1516-2 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/concurrent-join-1516" -S1 -A10 -O "${RTI1516_FDD_FILE}")
# 5 servers - rti protocol, 10 ambassadors
add_test(rti1516/concurrent-join-1516-3 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/concurrent-join-1516" -S5 -A10 -O "${RTI1516_FDD_FILE}")
