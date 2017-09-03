# The directory containing the public api for RTI13
set(RTI13_HEADER_PATH ${CMAKE_SOURCE_DIR}/include/RTI13)

# All the -I arguments
include_directories(${RTI13_HEADER_PATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
# The OpenRTI implementation files and may be build generated sources
include_directories(${CMAKE_BINARY_DIR}/src/OpenRTI)
include_directories(${CMAKE_SOURCE_DIR}/src/OpenRTI)

# The Headers for libRTI-NG and libFedTime
set(RTI13_PUBLIC_HEADERS
        ${RTI13_HEADER_PATH}/baseTypes.hh
        ${RTI13_HEADER_PATH}/federateAmbServices.hh
        ${RTI13_HEADER_PATH}/fedtime.hh
        ${RTI13_HEADER_PATH}/NullFederateAmbassador.hh
        ${RTI13_HEADER_PATH}/RTIambServices.hh
        ${RTI13_HEADER_PATH}/RTI.hh
        ${RTI13_HEADER_PATH}/RTItypes.hh
)

set(LIB_RTI_NG_SOURCES
	RTIambPrivateRefs.cpp
        AttributeHandleSetCallback.cpp
        AttributeHandleSetImplementation.cpp
        AttributeHandleValuePairSetCallback.cpp
        AttributeHandleValuePairSetImplementation.cpp
        Exception.cpp
        FederateHandleSetImplementation.cpp
        ParameterHandleValuePairSetCallback.cpp
        ParameterHandleValuePairSetImplementation.cpp
        RTI13LogicalTimeFactory.cpp
)

set(LIB_FEDTIME_SOURCES RTIfedTime.cpp)

if(OPENRTI_BUILD_SHARED)
  add_library(RTI-NG SHARED ${LIB_RTI_NG_SOURCES})
  set_property(TARGET RTI-NG APPEND PROPERTY COMPILE_DEFINITIONS RTI_EXPORTS)
  set_property(TARGET RTI-NG PROPERTY VERSION 1.3.0)
  set_property(TARGET RTI-NG PROPERTY SOVERSION 1)

  # RTI-NG, the circular reference is tricky on win32
  if(MSVC)
    add_library(FedTimeStub STATIC ${LIB_FEDTIME_SOURCES})
    set_property(TARGET FedTimeStub APPEND PROPERTY COMPILE_DEFINITIONS FedTime_EXPORTS)
    # Ideally we could use
    #  set_property(TARGET FedTimeStub APPEND PROPERTY STATIC_LIBRARY_FLAGS_DEBUG "/DEF /NAME:$<TARGET_FILE:FedTime>")
    # to get the final target file name as it is produced, but STATIC_LIBRARY_FLAGS does not evaluate generator expressions
    # So use different variants of CMAKE_<CONFIG>_POSTFIX
    openrti_set_msvc_stub_dll_name(FedTimeStub FedTime)
    openrti_target_link_private_libraries(RTI-NG FedTimeStub OpenRTI)
  elseif(MINGW OR CYGWIN)
    add_library(FedTimeStub STATIC ${LIB_FEDTIME_SOURCES})
    set_property(TARGET FedTimeStub APPEND PROPERTY COMPILE_DEFINITIONS FedTime_EXPORTS)
    string(TOUPPER ${CMAKE_BUILD_TYPE} _CONFIG)
    set(_fedtimeDllName libFedTime${CMAKE_${_CONFIG}_POSTFIX}.dll)
    add_custom_command(TARGET FedTimeStub
      POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E remove -f ${_fedtimeDllName}.a
      COMMAND "${DLLTOOL}" -D${_fedtimeDllName} -l${_fedtimeDllName}.a $<TARGET_FILE:FedTimeStub>
      COMMAND "${CMAKE_COMMAND}" -E remove -f $<TARGET_FILE:FedTimeStub>
      COMMAND "${CMAKE_COMMAND}" -E rename ${_fedtimeDllName}.a $<TARGET_FILE:FedTimeStub>
      )
    openrti_target_link_private_libraries(RTI-NG FedTimeStub OpenRTI)
  else()
    openrti_target_link_private_libraries(RTI-NG OpenRTI)
    if(OPENRTI_INSTALL_WITH_RPATH)
      set_property(TARGET RTI-NG PROPERTY INSTALL_RPATH "$ORIGIN")
      set_property(TARGET RTI-NG APPEND PROPERTY LINK_FLAGS "${OPENRTI_RPATH_LINK_FLAGS}")
    endif()
  endif()
  if(MACOS)
    set_property(TARGET RTI-NG APPEND PROPERTY LINK_FLAGS "-Wl,-undefined,dynamic_lookup ${OPENRTI_BIND_AT_LOAD}")
  endif()
else()
  add_library(RTI-NG STATIC ${LIB_RTI_NG_SOURCES})
  openrti_target_link_private_libraries(RTI-NG FedTime OpenRTI)
endif()

# libFedTime
if(OPENRTI_BUILD_SHARED)
  add_library(FedTime SHARED ${LIB_FEDTIME_SOURCES})
  set_property(TARGET FedTime APPEND PROPERTY COMPILE_DEFINITIONS FedTime_EXPORTS)
  set_property(TARGET FedTime PROPERTY VERSION 1.3.0)
  set_property(TARGET FedTime PROPERTY SOVERSION 1)
  if(OPENRTI_INSTALL_WITH_RPATH)
    set_property(TARGET FedTime PROPERTY INSTALL_RPATH "$ORIGIN")
    set_property(TARGET FedTime APPEND PROPERTY LINK_FLAGS "${OPENRTI_RPATH_LINK_FLAGS}")
  endif()
else()
  add_library(FedTime STATIC ${LIB_FEDTIME_SOURCES})
endif()
openrti_target_link_private_libraries(FedTime RTI-NG)


install(TARGETS FedTime RTI-NG
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES ${RTI13_PUBLIC_HEADERS}
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${OPENRTI_RTI13_INCLUDE_SUBDIR}")

if(UNIX)
  include(PkgConfig)
  create_pkg_config(RTI-NG "HLA/RTI 1.3 Communication Library." "${OPENRTI_RTI13_INCLUDE_SUBDIR}" "")
  create_pkg_config(FedTime "HLA/RTI 1.3 Time Library." "${OPENRTI_RTI13_INCLUDE_SUBDIR}" "")
  create_api_pkg_config(hla-rti13 "HLA/RTI 1.3 Libraries." RTI-NG FedTime)
endif()
