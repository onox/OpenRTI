# The directory containing the public api for rti1516
set(RTI1516_HEADER_PATH ${CMAKE_SOURCE_DIR}/include/rti1516)
set(RTI1516_DATA_PATH ${CMAKE_SOURCE_DIR}/share/rti1516)

# All the -I arguments
include_directories(${RTI1516_HEADER_PATH})
# The OpenRTI implementation files and may be build generated sources
include_directories(${CMAKE_BINARY_DIR}/src/OpenRTI)
include_directories(${CMAKE_SOURCE_DIR}/src/OpenRTI)

set(RTI1516_PUBLIC_HEADERS
        ${RTI1516_HEADER_PATH}/RTI/Enums.h
        ${RTI1516_HEADER_PATH}/RTI/FederateAmbassador.h
        ${RTI1516_HEADER_PATH}/RTI/LogicalTimeFactory.h
        ${RTI1516_HEADER_PATH}/RTI/LogicalTimeInterval.h
        ${RTI1516_HEADER_PATH}/RTI/RangeBounds.h
        ${RTI1516_HEADER_PATH}/RTI/RTIambassadorFactory.h
        ${RTI1516_HEADER_PATH}/RTI/SpecificConfig.h
        ${RTI1516_HEADER_PATH}/RTI/VariableLengthData.h
        ${RTI1516_HEADER_PATH}/RTI/Exception.h
        ${RTI1516_HEADER_PATH}/RTI/Handle.h
        ${RTI1516_HEADER_PATH}/RTI/LogicalTime.h
        ${RTI1516_HEADER_PATH}/RTI/NullFederateAmbassador.h
        ${RTI1516_HEADER_PATH}/RTI/RTI1516.h
        ${RTI1516_HEADER_PATH}/RTI/RTIambassador.h
        ${RTI1516_HEADER_PATH}/RTI/Typedefs.h
)

set(LIB_RTI1516_SOURCES
	Exception.cpp
	FederateAmbassador.cpp
	Handle.cpp
	LogicalTime.cpp
	LogicalTimeFactory.cpp
	LogicalTimeInterval.cpp
	NullFederateAmbassador.cpp
	RangeBounds.cpp
	RTIambassador.cpp
	RTIambassadorFactory.cpp
	RTIambassadorImplementation.cpp
	RTI1516LogicalTimeFactory.cpp
	VariableLengthData.cpp
	Version.cpp
)


set(FEDTIME1516_PUBLIC_HEADERS
        ${RTI1516_HEADER_PATH}/RTI/HLAfloat64Interval.h
        ${RTI1516_HEADER_PATH}/RTI/HLAfloat64TimeFactory.h
        ${RTI1516_HEADER_PATH}/RTI/HLAfloat64Time.h
        ${RTI1516_HEADER_PATH}/RTI/HLAinteger64Interval.h
        ${RTI1516_HEADER_PATH}/RTI/HLAinteger64TimeFactory.h
        ${RTI1516_HEADER_PATH}/RTI/HLAinteger64Time.h
        ${RTI1516_HEADER_PATH}/RTI/RTI1516fedTime.h
)

set(LIB_FEDTIME1516_SOURCES
        RTI1516fedTime.cpp
        HLAfloat64Interval.cpp
        HLAfloat64Time.cpp
        HLAfloat64TimeFactory.cpp
        HLAinteger64Interval.cpp
        HLAinteger64Time.cpp
        HLAinteger64TimeFactory.cpp
        LogicalTimeFactoryFactory.cpp
)

if(OPENRTI_BUILD_SHARED)
  add_library(rti1516 SHARED ${LIB_RTI1516_SOURCES})
  set_property(TARGET rti1516 APPEND PROPERTY COMPILE_DEFINITIONS BUILDING_RTI)
  set_property(TARGET rti1516 PROPERTY VERSION 1.0.0)
  set_property(TARGET rti1516 PROPERTY SOVERSION 1)

  # rti1516, the circular reference is tricky on win32
  if(MSVC)
    add_library(fedtime1516Stub STATIC ${LIB_FEDTIME1516_SOURCES})
    set_property(TARGET fedtime1516Stub APPEND PROPERTY COMPILE_DEFINITIONS BUILDING_FEDTIME)
    # Ideally we could use
    #  set_property(TARGET fedtime1516Stub APPEND PROPERTY STATIC_LIBRARY_FLAGS_DEBUG "/DEF /NAME:$<TARGET_FILE:fedtime1516>")
    # to get the final target file name as it is produced, but STATIC_LIBRARY_FLAGS does not evaluate generator expressions
    # So use different variants of CMAKE_<CONFIG>_POSTFIX
    openrti_set_msvc_stub_dll_name(fedtime1516Stub fedtime1516)
    openrti_target_link_private_libraries(rti1516 fedtime1516Stub OpenRTI)
  elseif(MINGW OR CYGWIN)
    add_library(fedtime1516Stub STATIC ${LIB_FEDTIME1516_SOURCES})
    set_property(TARGET fedtime1516Stub APPEND PROPERTY COMPILE_DEFINITIONS BUILDING_FEDTIME)
    string(TOUPPER ${CMAKE_BUILD_TYPE} _CONFIG)
    set(_fedtimeDllName libfedtime1516${CMAKE_${_CONFIG}_POSTFIX}.dll)
    add_custom_command(TARGET fedtime1516Stub
      POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E remove -f ${_fedtimeDllName}.a
      COMMAND "${DLLTOOL}" -D${_fedtimeDllName} -l${_fedtimeDllName}.a $<TARGET_FILE:fedtime1516Stub>
      COMMAND "${CMAKE_COMMAND}" -E remove -f $<TARGET_FILE:fedtime1516Stub>
      COMMAND "${CMAKE_COMMAND}" -E rename ${_fedtimeDllName}.a $<TARGET_FILE:fedtime1516Stub>
      )
    openrti_target_link_private_libraries(rti1516 fedtime1516Stub OpenRTI)
  else()
    openrti_target_link_private_libraries(rti1516 OpenRTI)
    if(OPENRTI_INSTALL_WITH_RPATH)
      set_property(TARGET rti1516 PROPERTY INSTALL_RPATH "$ORIGIN")
      set_property(TARGET rti1516 APPEND PROPERTY LINK_FLAGS "${OPENRTI_RPATH_LINK_FLAGS}")
    endif()
  endif()
  if(MACOS)
    set_property(TARGET rti1516 APPEND PROPERTY LINK_FLAGS "-Wl,-undefined,dynamic_lookup ${OPENRTI_BIND_AT_LOAD}")
  endif()
else()
  add_library(rti1516 STATIC ${LIB_RTI1516_SOURCES})
  set_property(TARGET rti1516 APPEND PROPERTY COMPILE_DEFINITIONS STATIC_RTI STATIC_FEDTIME)
  set_property(TARGET rti1516 PROPERTY LINK_INTERFACE_MULTIPLICITY 2)
  openrti_target_position_independent(rti1516)
  openrti_target_link_private_libraries(rti1516 fedtime1516 OpenRTI)
endif()

# libfedtime1516
if(OPENRTI_BUILD_SHARED)
  add_library(fedtime1516 SHARED ${LIB_FEDTIME1516_SOURCES})
  set_property(TARGET fedtime1516 APPEND PROPERTY COMPILE_DEFINITIONS BUILDING_FEDTIME)
  set_property(TARGET fedtime1516 PROPERTY VERSION 1.0.0)
  set_property(TARGET fedtime1516 PROPERTY SOVERSION 1)
  if(OPENRTI_INSTALL_WITH_RPATH)
    set_property(TARGET fedtime1516 PROPERTY INSTALL_RPATH "$ORIGIN")
    set_property(TARGET fedtime1516 APPEND PROPERTY LINK_FLAGS "${OPENRTI_RPATH_LINK_FLAGS}")
  endif()
else()
  add_library(fedtime1516 STATIC ${LIB_FEDTIME1516_SOURCES})
  set_property(TARGET fedtime1516 APPEND PROPERTY COMPILE_DEFINITIONS STATIC_RTI STATIC_FEDTIME)
  set_property(TARGET fedtime1516 PROPERTY LINK_INTERFACE_MULTIPLICITY 2)
  openrti_target_position_independent(fedtime1516)
endif()
openrti_target_link_private_libraries(fedtime1516 rti1516 OpenRTI)

install(TARGETS fedtime1516 rti1516
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES ${RTI1516_PUBLIC_HEADERS} ${FEDTIME1516_PUBLIC_HEADERS}
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${OPENRTI_RTI1516_INCLUDE_SUBDIR}/RTI")
# install(FILES "${RTI1516_DATA_PATH}/hla.dtd"
#   DESTINATION "${OPENRTI_DATAROOTDIR}/rti1516")

if(UNIX)
  include(PkgConfig)
  create_pkg_config(rti1516 "HLA/RTI 1516 Communication Library." "${OPENRTI_RTI1516_INCLUDE_SUBDIR}" "")
  create_pkg_config(fedtime1516 "HLA/RTI 1516 Time Library." "${OPENRTI_RTI1516_INCLUDE_SUBDIR}" "")
  create_api_pkg_config(hla-rti1516 "HLA/RTI 1516 Libraries." rti1516 fedtime1516)
endif()
