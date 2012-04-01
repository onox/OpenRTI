# The current standard fdd used for testing
#SET(RTI13_FED_FILE ${CMAKE_CURRENT_SOURCE_DIR}/fed.fed)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/RTI13)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/src/OpenRTI)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/OpenRTI)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Test directories should be listed here in sensible testing order
ADD_SUBDIRECTORY(fddget)
