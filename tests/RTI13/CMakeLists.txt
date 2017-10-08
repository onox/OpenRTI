# The current standard fdd used for testing
#set(RTI13_FED_FILE ${CMAKE_CURRENT_SOURCE_DIR}/fed.fed)

include_directories(${CMAKE_SOURCE_DIR}/include/RTI13)
include_directories(${CMAKE_BINARY_DIR}/src/OpenRTI)
include_directories(${CMAKE_SOURCE_DIR}/src/OpenRTI)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Test directories should be listed here in sensible testing order
add_subdirectory(link)
add_subdirectory(fddget)
add_subdirectory(time)
