# Query something from an fdd
ADD_EXECUTABLE(connect-1516 connect.cpp)
TARGET_LINK_LIBRARIES(connect-1516 rti1516 fedtime1516 OpenRTI)

# Run tests with failing connects
ADD_TEST(rti1516/connect-1516-1 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/connect-1516" -O "${RTI1516_FDD_FILE}")
