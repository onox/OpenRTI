add_executable(sync-1516 sync.cpp)
target_link_libraries(sync-1516 rti1516 fedtime1516 OpenRTI)

# No server - thread protocol, one ambassador
add_test(rti1516/sync-1516-1 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/sync-1516" -S0 -A1 -O "${RTI1516_FDD_FILE}")
# No server - thread protocol, 10 ambassadors
add_test(rti1516/sync-1516-2 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/sync-1516" -S0 -A10 -O "${RTI1516_FDD_FILE}")
# 1 server - rti protocol, one ambassador
add_test(rti1516/sync-1516-3 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/sync-1516" -S1 -A1 -O "${RTI1516_FDD_FILE}")
# 1 server - rti protocol, 10 ambassadors
add_test(rti1516/sync-1516-4 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/sync-1516" -S1 -A10 -O "${RTI1516_FDD_FILE}")
# 5 servers - rti protocol, 10 ambassadors
add_test(rti1516/sync-1516-5 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/sync-1516" -S5 -A10 -O "${RTI1516_FDD_FILE}")
