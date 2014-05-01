# The directory containing the public api for RTI13
SET(RTI1516E_HEADER_PATH ${CMAKE_SOURCE_DIR}/include/rti1516e)
SET(RTI1516E_DATA_PATH ${CMAKE_SOURCE_DIR}/share/rti1516e)

# All the -I arguments
INCLUDE_DIRECTORIES(${RTI1516E_HEADER_PATH})
# The OpenRTI implementation files and may be build generated sources
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/src/OpenRTI)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/OpenRTI)

SET(RTI1516E_PUBLIC_HEADERS
        ${RTI1516E_HEADER_PATH}/RTI/Enums.h
        ${RTI1516E_HEADER_PATH}/RTI/Exception.h
        ${RTI1516E_HEADER_PATH}/RTI/FederateAmbassador.h
        ${RTI1516E_HEADER_PATH}/RTI/Handle.h
        ${RTI1516E_HEADER_PATH}/RTI/LogicalTimeFactory.h
        ${RTI1516E_HEADER_PATH}/RTI/LogicalTime.h
        ${RTI1516E_HEADER_PATH}/RTI/LogicalTimeInterval.h
        ${RTI1516E_HEADER_PATH}/RTI/RangeBounds.h
        ${RTI1516E_HEADER_PATH}/RTI/NullFederateAmbassador.h
        ${RTI1516E_HEADER_PATH}/RTI/RTI1516.h
        ${RTI1516E_HEADER_PATH}/RTI/RTIambassadorFactory.h
        ${RTI1516E_HEADER_PATH}/RTI/RTIambassador.h
        ${RTI1516E_HEADER_PATH}/RTI/SpecificConfig.h
        ${RTI1516E_HEADER_PATH}/RTI/Typedefs.h
        ${RTI1516E_HEADER_PATH}/RTI/VariableLengthData.h
)

SET(RTI1516E_PUBLIC_ENCODING_HEADERS
        ${RTI1516E_HEADER_PATH}/RTI/encoding/BasicDataElements.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/DataElement.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/EncodingExceptions.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/EncodingConfig.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/HLAfixedArray.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/HLAfixedRecord.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/HLAopaqueData.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/HLAvariableArray.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/HLAvariantRecord.h
)

SET(RTI1516E_PUBLIC_TIME_HEADERS
        ${RTI1516E_HEADER_PATH}/RTI/time/HLAfloat64Interval.h
        ${RTI1516E_HEADER_PATH}/RTI/time/HLAfloat64TimeFactory.h
        ${RTI1516E_HEADER_PATH}/RTI/time/HLAfloat64Time.h
        ${RTI1516E_HEADER_PATH}/RTI/time/HLAinteger64Interval.h
        ${RTI1516E_HEADER_PATH}/RTI/time/HLAinteger64TimeFactory.h
        ${RTI1516E_HEADER_PATH}/RTI/time/HLAinteger64Time.h
)

SET(LIB_RTI1516E_SOURCES
        BasicDataElements.cpp
        DataElement.cpp
        EncodingExceptions.cpp
        Exception.cpp
        FederateAmbassador.cpp
        Handle.cpp
        HandleImplementation.h
        HLAfixedArray.cpp
        HLAfixedRecord.cpp
        HLAopaqueData.cpp
        HLAvariableArray.cpp
        HLAvariantRecord.cpp
        HLAfloat64Interval.cpp
        HLAfloat64Time.cpp
        HLAfloat64TimeFactory.cpp
        HLAinteger64Interval.cpp
        HLAinteger64Time.cpp
        HLAinteger64TimeFactory.cpp
        HLAlogicalTimeFactoryFactory.cpp
        LogicalTime.cpp
        LogicalTimeFactory.cpp
        LogicalTimeInterval.cpp
        NullFederateAmbassador.cpp
        RangeBounds.cpp
        VariableLengthData.cpp
        VariableLengthDataImplementation.h
        Version.cpp
        RTI1516ELogicalTimeFactory.cpp
        RTIambassador.cpp
        RTIambassadorFactory.cpp
        RTIambassadorImplementation.cpp
        Typedefs.cpp
)

SET(LIB_FEDTIME1516E_SOURCES
        LogicalTimeFactoryFactory.cpp
)

# rti1516, the circular reference is tricky on win32
IF(OPENRTI_BUILD_SHARED)
  ADD_LIBRARY(rti1516e SHARED ${LIB_RTI1516E_SOURCES})
  SET_PROPERTY(TARGET rti1516e APPEND PROPERTY COMPILE_DEFINITIONS BUILDING_RTI)
  SET_PROPERTY(TARGET rti1516e PROPERTY VERSION 1.0.0)
  SET_PROPERTY(TARGET rti1516e PROPERTY SOVERSION 1)
ELSE()
  ADD_LIBRARY(rti1516e STATIC ${LIB_RTI1516E_SOURCES})
ENDIF()
IF(MSVC)
  SET_PROPERTY(TARGET rti1516e PROPERTY PREFIX "lib")
  ADD_LIBRARY(fedtime1516eStub STATIC ${LIB_FEDTIME1516E_SOURCES})
  SET_PROPERTY(TARGET fedtime1516eStub APPEND PROPERTY COMPILE_DEFINITIONS BUILDING_FEDTIME)
  SET_PROPERTY(TARGET fedtime1516eStub APPEND PROPERTY STATIC_LIBRARY_FLAGS "/DEF /NAME:libfedtime1516e.dll")
  OPENRTI_TARGET_LINK_PRIVATE_LIBRARIES(rti1516e fedtime1516eStub OpenRTI)
ELSEIF(MINGW OR CYGWIN)
  ADD_LIBRARY(fedtime1516eStub STATIC ${LIB_FEDTIME1516E_SOURCES})
  SET_PROPERTY(TARGET fedtime1516eStub APPEND PROPERTY COMPILE_DEFINITIONS BUILDING_FEDTIME)
  ADD_CUSTOM_COMMAND(TARGET fedtime1516eStub
          POST_BUILD
          COMMAND "${CMAKE_COMMAND}" -E remove -f libfedtime1516e.dll.a
          COMMAND "${DLLTOOL}" -Dlibfedtime1516e.dll -llibfedtime1516e.dll.a libfedtime1516eStub.a
          COMMAND "${CMAKE_COMMAND}" -E remove -f libfedtime1516eStub.a
          COMMAND "${CMAKE_COMMAND}" -E rename libfedtime1516e.dll.a libfedtime1516eStub.a
    )
  OPENRTI_TARGET_LINK_PRIVATE_LIBRARIES(rti1516e fedtime1516eStub OpenRTI)
ELSE()
  OPENRTI_TARGET_LINK_PRIVATE_LIBRARIES(rti1516e OpenRTI)
  IF(OPENRTI_INSTALL_WITH_RPATH)
    SET_PROPERTY(TARGET rti1516e PROPERTY INSTALL_RPATH "\\\${ORIGIN}")
    SET_PROPERTY(TARGET rti1516e APPEND PROPERTY LINK_FLAGS "${OPENRTI_RPATH_LINK_FLAGS}")
  ENDIF()
ENDIF()
IF(MACOS)
  SET_PROPERTY(TARGET rti1516e APPEND PROPERTY LINK_FLAGS "-Wl,-undefined,dynamic_lookup ${OPENRTI_BIND_AT_LOAD}")
ENDIF()

# libfedtime1516
IF(OPENRTI_BUILD_SHARED)
  ADD_LIBRARY(fedtime1516e SHARED ${LIB_FEDTIME1516E_SOURCES})
  SET_PROPERTY(TARGET fedtime1516e APPEND PROPERTY COMPILE_DEFINITIONS BUILDING_FEDTIME)
  SET_PROPERTY(TARGET fedtime1516e PROPERTY VERSION 1.0.0)
  SET_PROPERTY(TARGET fedtime1516e PROPERTY SOVERSION 1)
ELSE()
  ADD_LIBRARY(fedtime1516e STATIC ${LIB_FEDTIME1516E_SOURCES})
ENDIF()
IF(MSVC)
  SET_PROPERTY(TARGET fedtime1516e PROPERTY PREFIX "lib")
ENDIF()
IF(OPENRTI_INSTALL_WITH_RPATH)
  SET_PROPERTY(TARGET fedtime1516e PROPERTY INSTALL_RPATH "\\\${ORIGIN}")
  SET_PROPERTY(TARGET fedtime1516e APPEND PROPERTY LINK_FLAGS "${OPENRTI_RPATH_LINK_FLAGS}")
ENDIF()
OPENRTI_TARGET_LINK_PRIVATE_LIBRARIES(fedtime1516e rti1516e OpenRTI)

INSTALL(TARGETS fedtime1516e rti1516e
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
INSTALL(FILES ${RTI1516E_PUBLIC_HEADERS}
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/rti1516e/RTI")
INSTALL(FILES ${RTI1516E_PUBLIC_ENCODING_HEADERS}
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/rti1516e/RTI/encoding")
INSTALL(FILES ${RTI1516E_PUBLIC_TIME_HEADERS}
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/rti1516e/RTI/time")
INSTALL(FILES ${RTI1516E_DATA_PATH}/HLAstandardMIM.xml
  DESTINATION "${OPENRTI_DATAROOTDIR}/rti1516e")
