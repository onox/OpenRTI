add_executable(logical-time-13 logical-time.cpp)
target_link_libraries(logical-time-13 FedTime OpenRTI)

# Test the different logical times by getting the values form the logical time factory
add_test(rti13/logical-time-13-1 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/logical-time-13")
