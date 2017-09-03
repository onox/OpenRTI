# Query something from an fdd
add_executable(connect-1516 connect.cpp)
target_link_libraries(connect-1516 rti1516 fedtime1516 OpenRTI)

# Run tests with failing connects
add_test(rti1516/connect-1516-1 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/connect-1516" -O "${RTI1516_FDD_FILE}")
