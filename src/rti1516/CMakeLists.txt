# The directory containing the public api for RTI13
SET(RTI1516_HEADER_PATH ${CMAKE_SOURCE_DIR}/include/rti1516)

# All the -I arguments
INCLUDE_DIRECTORIES(${RTI1516_HEADER_PATH})
# The OpenRTI implementation files and may be build generated sources
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/src/OpenRTI)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/OpenRTI)

SET(RTI1516_PUBLIC_HEADERS
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

SET(LIB_RTI1516_SOURCES
	Exception.cpp
	FederateAmbassador.cpp
	Handle.cpp
	HandleImplementation.h
	LogicalTime.cpp
	LogicalTimeFactory.cpp
	LogicalTimeInterval.cpp
	RangeBounds.cpp
	RTIambassador.cpp
	RTIambassadorFactory.cpp
	RTIambassadorImplementation.cpp
	RTI1516LogicalTimeFactory.cpp
	VariableLengthData.cpp
	VariableLengthDataImplementation.h
)

SET(LIB_FEDTIME1516_SOURCES
        RTI1516fedTime.cpp
        HLAfloat64Interval.cpp
        HLAfloat64Time.cpp
        HLAfloat64TimeFactory.cpp
        HLAinteger64Interval.cpp
        HLAinteger64Time.cpp
        HLAinteger64TimeFactory.cpp
        LogicalTimeFactoryFactory.cpp
)

# rti1516, the circular reference is tricky on win32
ADD_LIBRARY(rti1516 SHARED ${LIB_RTI1516_SOURCES})
SET_PROPERTY(TARGET rti1516 APPEND PROPERTY COMPILE_DEFINITIONS BUILDING_RTI)
SET_PROPERTY(TARGET rti1516 PROPERTY VERSION 1.0.0)
SET_PROPERTY(TARGET rti1516 PROPERTY SOVERSION 1)
SET_PROPERTY(TARGET rti1516 PROPERTY LINK_INTERFACE_LIBRARIES fedtime1516)
IF(WIN32)
  ADD_LIBRARY(fedtime1516Stub STATIC ${LIB_FEDTIME1516_SOURCES})
  SET_PROPERTY(TARGET fedtime1516Stub APPEND PROPERTY COMPILE_DEFINITIONS BUILDING_FEDTIME)
  SET_PROPERTY(TARGET fedtime1516Stub APPEND PROPERTY STATIC_LIBRARY_FLAGS "/DEF /NAME:fedtime1516.dll")
  TARGET_LINK_LIBRARIES(rti1516 fedtime1516Stub OpenRTI)
ELSEIF(WIN32)
  TARGET_LINK_LIBRARIES(rti1516 OpenRTI)
ENDIF(WIN32)

# libfedtime1516
ADD_LIBRARY(fedtime1516 SHARED ${LIB_FEDTIME1516_SOURCES})
SET_PROPERTY(TARGET fedtime1516 APPEND PROPERTY COMPILE_DEFINITIONS BUILDING_FEDTIME)
SET_PROPERTY(TARGET fedtime1516 PROPERTY VERSION 1.0.0)
SET_PROPERTY(TARGET fedtime1516 PROPERTY SOVERSION 1)
SET_PROPERTY(TARGET fedtime1516 PROPERTY LINK_INTERFACE_LIBRARIES "")
TARGET_LINK_LIBRARIES(fedtime1516 rti1516 OpenRTI)


INSTALL(TARGETS fedtime1516 rti1516
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
INSTALL(FILES ${RTI1516_PUBLIC_HEADERS}
  DESTINATION include/rti1516/RTI)
