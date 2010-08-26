# All the -I arguments
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
# The OpenRTI implementation files and may be build generated sources
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/src/OpenRTI)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/OpenRTI)

# The standalone main server application
ADD_EXECUTABLE(rtinode rtinode.cpp)
# FIXME, make that conditional
SET_PROPERTY(TARGET rtinode PROPERTY INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
# SET_PROPERTY(TARGET rtinode PROPERTY INSTALL_RPATH "$\{ORIGIN\}/../lib")
TARGET_LINK_LIBRARIES(rtinode OpenRTI)

INSTALL(TARGETS rtinode
  RUNTIME       DESTINATION bin)
