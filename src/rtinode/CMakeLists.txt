# All the -I arguments
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
# The OpenRTI implementation files and may be build generated sources
include_directories(${CMAKE_BINARY_DIR}/src/OpenRTI)
include_directories(${CMAKE_SOURCE_DIR}/src/OpenRTI)

# The standalone main server application
add_executable(rtinode rtinode.cpp)
if(OPENRTI_INSTALL_WITH_RPATH)
  set_property(TARGET rtinode PROPERTY INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
# set_property(TARGET rtinode PROPERTY INSTALL_RPATH "\\\${ORIGIN}/../${CMAKE_INSTALL_LIBDIR}")
  set_property(TARGET rtinode APPEND PROPERTY LINK_FLAGS "${OPENRTI_RPATH_LINK_FLAGS}")
endif()
target_link_libraries(rtinode OpenRTI)

install(TARGETS rtinode
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
