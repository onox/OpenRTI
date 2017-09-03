include_directories(${CMAKE_BINARY_DIR}/src/OpenRTI)
include_directories(${CMAKE_SOURCE_DIR}/src/OpenRTI)

add_executable(threads threads.cpp)
target_link_libraries(threads OpenRTI)

add_test(OpenRTI/threads-1 "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/threads")

