INCLUDE(${QT_USE_FILE})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/rti1516)
INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR})

SET(HEADERS
  QHLAArrayDataElement.h
  QHLAArrayDataType.h
  QHLABasicDataType.h
  QHLADataElement.h
  QHLADataType.h
  QHLADataTypeVisitor.h
  QHLAEnumeratedDataElement.h
  QHLAEnumeratedDataType.h
  QHLAFixedRecordDataElement.h
  QHLAFixedRecordDataType.h
  QHLAScalarDataElement.h
  QHLASimpleDataType.h
  QHLAStringDataElement.h
  QHLAVariantRecordDataElement.h
  QHLAVariantRecordDataType.h
  QRTI1516Federate.h
  QRTI1516OMTHandler.h
  QRTIFederate.h
  QRTIObjectClass.h
  QRTIObjectClassAttribute.h
  QRTIObjectInstance.h
  QRTIObjectInstanceAttribute.h
)
SET(SOURCES
  qrti.cpp
  QHLAArrayDataElement.cpp
  QHLAArrayDataType.cpp
  QHLABasicDataType.cpp
  QHLADataElement.cpp
  QHLADataType.cpp
  QHLAEnumeratedDataElement.cpp
  QHLAEnumeratedDataType.cpp
  QHLAFixedRecordDataElement.cpp
  QHLAFixedRecordDataType.cpp
  QHLAScalarDataElement.cpp
  QHLASimpleDataType.cpp
  QHLAStringDataElement.cpp
  QHLAVariantRecordDataElement.cpp
  QHLAVariantRecordDataType.cpp
  QRTI1516OMTHandler.cpp
  QRTIFederate.cpp
  QRTIObjectClass.cpp
  QRTIObjectClassAttribute.cpp
  QRTIObjectInstance.cpp
  QRTIObjectInstanceAttribute.cpp
)

QT4_WRAP_CPP(MOC ${HEADERS})
ADD_EXECUTABLE(qrti ${HEADERS} ${SOURCES} ${MOC})
IF(OPENRTI_INSTALL_WITH_RPATH)
  SET_PROPERTY(TARGET qrti PROPERTY INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
# SET_PROPERTY(TARGET qrti PROPERTY INSTALL_RPATH "$\{ORIGIN\}/../${CMAKE_INSTALL_LIBDIR}")
ENDIF()
TARGET_LINK_LIBRARIES(qrti ${QT_LIBRARIES})
TARGET_LINK_LIBRARIES(qrti rti1516 fedtime1516)

INSTALL(TARGETS qrti
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
