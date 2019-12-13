function(create_pkg_config _TARGET _DESCRIPTION _INCLUDESUBDIR _LIBSUBDIR)
  set(TARGET_NAME ${_TARGET})
  set(PKGCONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.pc")

  get_property(LIBRARY_NAME TARGET ${_TARGET} PROPERTY LIBRARY_OUTPUT_NAME)
  if(NOT LIBRARY_NAME)
    set(LIBRARY_NAME ${TARGET_NAME})
  endif()
  
  set(RPATH_LINK)
  if(CMAKE_SHARED_LIBRARY_RPATH_LINK_CXX_FLAG)
    set(RPATH_LINK "${CMAKE_SHARED_LIBRARY_RPATH_LINK_CXX_FLAG}\${libdir}")
  endif()

  file(WRITE ${PKGCONFIG_FILE}
    "prefix=${CMAKE_INSTALL_PREFIX}\n"
    "exec_prefix=\${prefix}\n"
    "libdir=\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}/${_LIBSUBDIR}\n"
    "includedir=\${prefix}/include/${_INCLUDESUBDIR}\n"
    "Name: ${TARGET_NAME}\n"
    "Description: ${_DESCRIPTION}\n"
    "Version: ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}\n"
    "Libs: -L\${libdir} ${RPATH_LINK} -l${LIBRARY_NAME}\n"
    "Cflags: -I\${includedir}\n"
    )
  install(FILES ${PKGCONFIG_FILE} DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endfunction()

function(create_api_pkg_config _NAME _DESCRIPTION)
  set(PKGCONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/${_NAME}.pc")
  string(REPLACE ";" " " _REQUIRES "${ARGN}")
  file(WRITE ${PKGCONFIG_FILE}
    "Name: ${_NAME}\n"
    "Description: ${_DESCRIPTION}\n"
    "Version: ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}\n"
    "Requires: ${_REQUIRES}\n"
    )
  install(FILES ${PKGCONFIG_FILE} DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endfunction()
