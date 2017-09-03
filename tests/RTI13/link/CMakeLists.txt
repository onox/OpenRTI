# Test if dynamic linking works as required by the standard
add_executable(link-13 link.cpp)
target_link_libraries(link-13 RTI-NG FedTime)
