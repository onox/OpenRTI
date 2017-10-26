add_executable(encode-1516e encode.cpp)
target_link_libraries(encode-1516e rti1516e fedtime1516e)

add_test(rti1516e/encode-1516e "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/encode-1516e")
