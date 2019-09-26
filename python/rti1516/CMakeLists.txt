# The rti1516 includes
include_directories(${CMAKE_SOURCE_DIR}/include/rti1516)

add_library(rti1516_py MODULE Module.cpp)
set_property(TARGET rti1516_py PROPERTY PREFIX "")
set_property(TARGET rti1516_py PROPERTY OUTPUT_NAME "rti1516")
if(WIN32)
  set_property(TARGET rti1516_py PROPERTY SUFFIX ".pyd")
endif()
target_link_libraries(rti1516_py rti1516 fedtime1516 Python::Python)

if(OPENRTI_BUILD_SHARED)
  if(OPENRTI_INSTALL_WITH_RPATH)
    set_property(TARGET rti1516_py PROPERTY INSTALL_RPATH "$ORIGIN/../..")
    set_property(TARGET rti1516_py APPEND PROPERTY LINK_FLAGS "${OPENRTI_RPATH_LINK_FLAGS}")
  endif()
endif()
  
install(TARGETS rti1516_py
  RUNTIME DESTINATION "${INSTALL_PYTHON_DIRECTORY}"
  LIBRARY DESTINATION "${INSTALL_PYTHON_DIRECTORY}"
  ARCHIVE DESTINATION "${INSTALL_PYTHON_DIRECTORY}")
