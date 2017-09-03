# The current standard fdd used for testing
set(RTI1516_FDD_FILE ${CMAKE_CURRENT_SOURCE_DIR}/fdd.xml)

include_directories(${CMAKE_SOURCE_DIR}/include/rti1516)
include_directories(${CMAKE_BINARY_DIR}/src/OpenRTI)
include_directories(${CMAKE_SOURCE_DIR}/src/OpenRTI)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

if(NOT OPENRTI_BUILD_SHARED)
  add_definitions(-DSTATIC_RTI)
  add_definitions(-DSTATIC_FEDTIME)
endif()

# Test directories should be listed here in sensible testing order
add_subdirectory(link)
add_subdirectory(connect)
add_subdirectory(create)
add_subdirectory(join)
add_subdirectory(fddget)
add_subdirectory(sync)
add_subdirectory(time)
add_subdirectory(interaction)
add_subdirectory(objectinstance)
