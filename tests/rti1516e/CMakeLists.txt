# The current standard fdd used for testing
# SET(RTI1516E_FDD_FILE ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/rti1516e)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/src/OpenRTI)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/OpenRTI)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Test directories should be listed here in sensible testing order
ADD_SUBDIRECTORY(fddget)
