INCLUDE(CheckIncludeFiles)
# INCLUDE(CheckSymbolExists)
# INCLUDE(CheckFunctionExists)
INCLUDE(CheckCXXSourceCompiles)

CHECK_INCLUDE_FILES(cstdint OpenRTI_HAVE_CSTDINT)
CHECK_INCLUDE_FILES(stdint.h OpenRTI_HAVE_STDINT_H)
CHECK_INCLUDE_FILES(inttypes.h OpenRTI_HAVE_INTTYPES_H)
IF(NOT WIN32)
  SET(CMAKE_REQUIRED_LIBRARIES dl)
  CHECK_CXX_SOURCE_COMPILES("
#define _GNU_SOURCE
#include <dlfcn.h>
int main(int argc, char* argv[])
{
  Dl_info info;
  dladdr((void*)main, &info);
  return 0;
}
" OpenRTI_HAVE_DLADDR)
  SET(CMAKE_REQUIRED_LIBRARIES)
  IF(OpenRTI_HAVE_DLADDR)
    SET(DL_LIBRARIES dl)
  ENDIF()
ENDIF()
CHECK_CXX_SOURCE_COMPILES("
#include <alloca.h>
int main(int argc, char* argv[])
{
  void* data = alloca(17);
  return 0;
}
" OpenRTI_HAVE_ALLOCA)

# sigh, since we do network traffic, we should know that ...
INCLUDE(TestBigEndian)
TEST_BIG_ENDIAN(BIG_ENDIAN)
IF(BIG_ENDIAN)
  SET(OpenRTI_HOST_IS_BIG_ENDIAN 1)
ELSE(BIG_ENDIAN)
  SET(OpenRTI_HOST_IS_LITTLE_ENDIAN 1)
ENDIF(BIG_ENDIAN)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/OpenRTIConfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/OpenRTIConfig.h)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

IF(LINUX)
  ADD_DEFINITIONS(-D_XOPEN_SOURCE=600)
  ADD_DEFINITIONS(-D_DEFAULT_SOURCE=1)
ELSEIF(UNIX)
  ADD_DEFINITIONS(-D_XOPEN_SOURCE_EXTENDED=1)
ELSEIF(WIN32)
  # for now, silence some win32 warnings - want to review that later ...
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
  # We need XP/SP2 We are currently bound to the GetAddrInfo call introduced
  # in that version.
  ADD_DEFINITIONS(-DWINVER=0x0502 -D_WIN32_WINNT=0x0502)
ENDIF()


# A bunch of sources that are operating system dependent.
IF(WIN32)
  SET(OpenRTI_OS_SOURCES
    win32/AtomicWin32.cpp
    win32/ClockWin32.cpp
    win32/ConditionWin32.cpp
    win32/DynamicModuleWin32.cpp
    win32/ErrnoWin32.cpp
    win32/MutexWin32.cpp
    win32/SocketAddressWin32.cpp
    win32/SocketDataWin32.cpp
    win32/SocketEventDispatcherWin32.cpp
    win32/SocketPacketWin32.cpp
    win32/SocketPipeWin32.cpp
    win32/SocketServerWin32.cpp
    win32/SocketServerPipeWin32.cpp
    win32/SocketServerTCPWin32.cpp
    win32/SocketStreamWin32.cpp
    win32/SocketTCPWin32.cpp
    win32/SocketUDPWin32.cpp
    win32/SocketWin32.cpp
    win32/ThreadLocalWin32.cpp
    win32/ThreadWin32.cpp
  )
  SET(OpenRTI_OS_HEADERS
    win32/ClockWin32.h
    win32/ConditionPrivateDataWin32.h
    win32/ErrnoWin32.h
    win32/MutexPrivateDataWin32.h
    win32/SocketAddressPrivateDataWin32.h
    win32/SocketPrivateDataWin32.h
  )
ELSE(WIN32)
  SET(OpenRTI_OS_SOURCES
    posix/AtomicPosix.cpp
    posix/ClockPosix.cpp
    posix/ConditionPosix.cpp
    posix/DynamicModulePosix.cpp
    posix/MutexPosix.cpp
    posix/SocketAddressPosix.cpp
    posix/SocketDataPosix.cpp
    posix/SocketEventDispatcherPosix.cpp
    posix/SocketPacketPosix.cpp
    posix/SocketPipePosix.cpp
    posix/SocketPosix.cpp
    posix/SocketServerPosix.cpp
    posix/SocketServerPipePosix.cpp
    posix/SocketServerTCPPosix.cpp
    posix/SocketStreamPosix.cpp
    posix/SocketTCPPosix.cpp
    posix/SocketUDPPosix.cpp
    posix/ThreadLocalPosix.cpp
    posix/ThreadPosix.cpp
  )
  SET(OpenRTI_OS_HEADERS
    posix/ClockPosix.h
    posix/ConditionPrivateDataPosix.h
    posix/ErrnoPosix.h
    posix/MutexPrivateDataPosix.h
    posix/SocketAddressPrivateDataPosix.h
    posix/SocketPrivateDataPosix.h
    posix/ThreadPrivateDataPosix.h
  )
ENDIF(WIN32)

# We use expat as xml parser, if not on the system, use our own probaby outdated version
INCLUDE(FindEXPAT)
IF(EXPAT_FOUND)
  INCLUDE_DIRECTORIES(${EXPAT_INCLUDE_DIRS})
ELSE(EXPAT_FOUND)
  # Additions for expat
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/xml)
  ADD_DEFINITIONS(-DXML_STATIC)
  ADD_DEFINITIONS(-DHAVE_MEMMOVE)

  SET(OpenRTI_EXPAT_SOURCES
    xml/xmlparse.c
    xml/xmlrole.c
    xml/xmltok.c
    xml/xmltok_impl.c
    xml/xmltok_ns.c
  )
  SET(OpenRTI_EXPAT_HEADERS
    xml/amigaconfig.h
    xml/ascii.h
    xml/asciitab.h
    xml/expat.h
    xml/expat_external.h
    xml/iasciitab.h
    xml/internal.h
    xml/latin1tab.h
    xml/macconfig.h
    xml/nametab.h
    xml/utf8tab.h
    xml/winconfig.h
    xml/xmlrole.h
    xml/xmltok.h
    xml/xmltok_impl.h
  )
ENDIF(EXPAT_FOUND)

INCLUDE(FindZLIB)
IF(ZLIB_FOUND)
  INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS})
  ADD_DEFINITIONS(-DOPENRTI_HAVE_ZLIB)
  SET(OpenRTI_ZLIB_SOURCES
    ZLibProtocolLayer.cpp
  )
ELSE()
  SET(OpenRTI_ZLIB_SOURCES "")
ENDIF()

SET(OpenRTI_XML_SOURCES
  Attributes.cpp
  ContentHandler.cpp
  ErrorHandler.cpp
  ExpatXMLReader.cpp
  XMLReader.cpp
)

SET(OpenRTI_SOURCES
  AbstractMessage.cpp
  AbstractMessageEncoding.cpp
  AbstractMessageSender.cpp
  AbstractProtocolLayer.cpp
  AbstractProtocolSocket.cpp
  AbstractServerNode.cpp
  AbstractServer.cpp
  AbstractSocketEvent.cpp
  InternalAmbassador.cpp
  InternalTimeManagement.cpp
  Federate.cpp
  DefaultErrorHandler.cpp
  Exception.cpp
  FDD1516EFileReader.cpp
  FDD1516FileReader.cpp
  FEDFileReader.cpp
  InitialClientStreamProtocol.cpp
  InitialServerStreamProtocol.cpp
  InitialStreamProtocol.cpp
  LogStream.cpp
  Message.cpp
  MessageEncodingRegistry.cpp
  NestedProtocolLayer.cpp
  NetworkServer.cpp
  Options.cpp
  ParenthesesReader.cpp
  ServerModel.cpp
  ServerNode.cpp
  ProtocolSocketEvent.cpp
  RegionSet.cpp
  ServerConfigContentHandler.cpp
  LeafServerThread.cpp
  SocketEventDispatcher.cpp
  SocketServerAcceptEvent.cpp
  StreamBufferProtocol.cpp
  StringUtils.cpp
  TightBE1MessageEncoding.cpp
  ThreadServer.cpp
  URL.cpp
  WeakReferenced.cpp
  ${OpenRTI_XML_SOURCES}
  ${OpenRTI_OS_SOURCES}
  ${OpenRTI_ZLIB_SOURCES}
)

SET(OpenRTI_XML_HEADERS
  Attributes.h
  ContentHandler.h
  ErrorHandler.h
  ExpatXMLReader.h
  XMLReader.h
)

SET(OpenRTI_HEADERS
  AbstractConnect.h
  AbstractLogicalTime.h
  AbstractMessage.h
  AbstractMessageDispatcher.h
  AbstractMessageEncoding.h
  AbstractMessageQueue.h
  AbstractMessageReceiver.h
  AbstractMessageSender.h
  AbstractProtocol.h
  AbstractProtocolLayer.h
  AbstractProtocolSocket.h
  AbstractServer.h
  AbstractServerNode.h
  AbstractSocketEvent.h
  Ambassador.h
  Atomic.h
  Buffer.h
  Clock.h
  Condition.h
  DecodeDataStream.h
  DefaultErrorHandler.h
  DynamicModule.h
  EncodeDataStream.h
  Exception.h
  Export.h
  FDD1516EFileReader.h
  FDD1516FileReader.h
  Federate.h
  FederateHandleLowerBoundTimeStampMap.h
  FEDFileReader.h
  FOMStringModuleBuilder.h
  Handle.h
  HandleAllocator.h
  InitialClientStreamProtocol.h
  InitialServerStreamProtocol.h
  InitialStreamProtocol.h
  InternalAmbassador.h
  InternalTimeManagement.h
  IntrusiveList.h
  IntrusiveUnorderedMap.h
  LeafServerThread.h
  LogStream.h
  Message.h
  MessageEncodingRegistry.h
  MessageQueue.h
  Mutex.h
  NestedProtocolLayer.h
  NetworkServer.h
  Options.h
  ParenthesesReader.h
  PooledMessageList.h
  ProtocolSocketEvent.h
  Rand.h
  RangeBounds.h
  Referenced.h
  Region.h
  RegionSet.h
  ScopeLock.h
  ScopeUnlock.h
  ServerConfigContentHandler.h
  ServerModel.h
  ServerNode.h
  ServerOptions.h
  SharedPtr.h
  SingletonPtr.h
  Socket.h
  SocketAddress.h
  SocketData.h
  SocketEventDispatcher.h
  SocketPacket.h
  SocketPipe.h
  SocketServer.h
  SocketServerAcceptEvent.h
  SocketServerPipe.h
  SocketServerTCP.h
  SocketStream.h
  SocketTCP.h
  SocketUDP.h
  StreamBufferProtocol.h
  StringUtils.h
  TemplateTimeManagement.h
  TestLib.h
  Thread.h
  ThreadLocal.h
  ThreadServer.h
  TightBE1MessageEncoding.h
  TimeManagement.h
  Types.h
  URL.h
  ValueImplementation.h
  VariableLengthData.h
  WeakPtr.h
  WeakReferenced.h
  ZLibProtocolLayer.h
)
IF(OPENRTI_ENABLE_PROJECT_FOLDERS)
  SOURCE_GROUP("Header Files" FILES ${OpenRTI_HEADERS})
  SOURCE_GROUP("Source Files\\XML" FILES ${OpenRTI_XML_SOURCES})
  SOURCE_GROUP("Header Files\\XML" FILES ${OpenRTI_XML_HEADERS})
  IF(OpenRTI_EXPAT_HEADERS)
    SOURCE_GROUP("Header Files\\Expat" FILES ${OpenRTI_EXPAT_HEADERS})
  ENDIF(OpenRTI_EXPAT_HEADERS)
  IF(OpenRTI_EXPAT_SOURCES)
    SOURCE_GROUP("Source Files\\Expat" FILES ${OpenRTI_EXPAT_SOURCES})
  ENDIF(OpenRTI_EXPAT_SOURCES)
  IF(WIN32)
    SOURCE_GROUP("Source Files\\Win32" FILES ${OpenRTI_OS_SOURCES})
    SOURCE_GROUP("Header Files\\Win32" FILES ${OpenRTI_OS_HEADERS})
  ELSE(WIN32)
    SOURCE_GROUP("Source Files\\Posix" FILES ${OpenRTI_OS_SOURCES})
    SOURCE_GROUP("Header Files\\Posix" FILES ${OpenRTI_OS_HEADERS})
  ENDIF(WIN32)
ENDIF(OPENRTI_ENABLE_PROJECT_FOLDERS)

# ADD_LIBRARY(OpenRTI-STATIC STATIC ${OpenRTI_SOURCES} ${OpenRTI_HEADERS} ${OpenRTI_XML_HEADERS} ${OpenRTI_OS_HEADERS} ${OpenRTI_EXPAT_SOURCES} ${OpenRTI_EXPAT_HEADERS})
# SET_PROPERTY(TARGET OpenRTI-STATIC PROPERTY OUTPUT_NAME OpenRTI)
# SET_PROPERTY(TARGET OpenRTI-STATIC PROPERTY ARCHIVE_OUTPUT_NAME OpenRTI)

IF(OPENRTI_BUILD_SHARED)
  ADD_LIBRARY(OpenRTI SHARED ${OpenRTI_SOURCES} ${OpenRTI_HEADERS} ${OpenRTI_XML_HEADERS} ${OpenRTI_OS_HEADERS} ${OpenRTI_EXPAT_SOURCES} ${OpenRTI_EXPAT_HEADERS})
  SET_PROPERTY(TARGET OpenRTI APPEND PROPERTY COMPILE_DEFINITIONS OPENRTI_DLL)
  SET_PROPERTY(TARGET OpenRTI APPEND PROPERTY COMPILE_DEFINITIONS OPENRTI_DLL_EXPORTS)
  SET_PROPERTY(TARGET OpenRTI PROPERTY VERSION 1.0.0)
  SET_PROPERTY(TARGET OpenRTI PROPERTY SOVERSION 1)
ELSE()
  ADD_LIBRARY(OpenRTI STATIC ${OpenRTI_SOURCES} ${OpenRTI_HEADERS} ${OpenRTI_XML_HEADERS} ${OpenRTI_OS_HEADERS} ${OpenRTI_EXPAT_SOURCES} ${OpenRTI_EXPAT_HEADERS})
ENDIF()
SET_PROPERTY(TARGET OpenRTI PROPERTY OUTPUT_NAME OpenRTI)
IF(EXPAT_FOUND)
  OPENRTI_TARGET_LINK_PRIVATE_LIBRARIES(OpenRTI ${EXPAT_LIBRARIES})
ENDIF(EXPAT_FOUND)
IF(ZLIB_FOUND)
  OPENRTI_TARGET_LINK_PRIVATE_LIBRARIES(OpenRTI ${ZLIB_LIBRARIES})
ENDIF()
IF(WIN32)
  OPENRTI_TARGET_LINK_PRIVATE_LIBRARIES(OpenRTI ws2_32)
ELSE(WIN32)
  IF(SUNOS)
    OPENRTI_TARGET_LINK_PRIVATE_LIBRARIES(OpenRTI socket)
    OPENRTI_TARGET_LINK_PRIVATE_LIBRARIES(OpenRTI nsl)
  ENDIF()
  IF (NOT ANDROID)
    OPENRTI_TARGET_LINK_PRIVATE_LIBRARIES(OpenRTI pthread)
    IF (NOT MACOS)
      OPENRTI_TARGET_LINK_PRIVATE_LIBRARIES(OpenRTI rt)
    ENDIF()
  ENDIF()
  OPENRTI_TARGET_LINK_PRIVATE_LIBRARIES(OpenRTI m)
ENDIF(WIN32)
IF(DL_LIBRARIES)
  OPENRTI_TARGET_LINK_PRIVATE_LIBRARIES(OpenRTI ${DL_LIBRARIES})
ENDIF(DL_LIBRARIES)

INSTALL(TARGETS OpenRTI
  PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
