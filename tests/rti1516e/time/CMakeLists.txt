add_executable(logical-time-1516e logical-time.cpp)
target_link_libraries(logical-time-1516e fedtime1516e OpenRTI)

# Test the different logical times by getting the values form the logical time factory
add_test(rti1516e/logical-time-1516e-1 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/logical-time-1516e")
add_test(rti1516e/logical-time-1516e-2 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/logical-time-1516e" -F HLAfloat64Time)
add_test(rti1516e/logical-time-1516e-3 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/logical-time-1516e" -F HLAinteger64Time)
