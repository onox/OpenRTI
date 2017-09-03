# Test if dynamic linking works as required by the standard
add_executable(link-1516 link.cpp)
target_link_libraries(link-1516 rti1516 fedtime1516)
