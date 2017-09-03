# Test if dynamic linking works as required by the standard
add_executable(link-1516e link.cpp)
target_link_libraries(link-1516e rti1516e fedtime1516e)
