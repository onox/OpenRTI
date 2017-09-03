include_directories(${CMAKE_BINARY_DIR}/src/OpenRTI)
include_directories(${CMAKE_SOURCE_DIR}/src/OpenRTI)

add_executable(socketaddress socketaddress.cpp)
target_link_libraries(socketaddress OpenRTI)

add_test(OpenRTI/socketaddress "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/socketaddress")

add_executable(url url.cpp)
target_link_libraries(url OpenRTI)

add_test(OpenRTI/url "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/url")
