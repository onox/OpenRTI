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

SET(OpenRTI_SOURCES
  AbstractMessage.cpp
  AbstractMessageEncoding.cpp
  AbstractMessageSender.cpp
  AbstractProtocolLayer.cpp
  AbstractProtocolSocket.cpp
  AbstractServerNode.cpp
  AbstractServer.cpp
  AbstractSocketEvent.cpp
  Attributes.cpp
  InternalAmbassador.cpp
  InternalTimeManagement.cpp
  Federate.cpp
  ContentHandler.cpp
  DefaultErrorHandler.cpp
  ErrorHandler.cpp
  ExpatXMLReader.cpp
  Exception.cpp
  FDD1516EFileReader.cpp
  FDD1516FileReader.cpp
  FEDFileReader.cpp
  FOMModuleSet.cpp
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
  XMLReader.cpp
  ${OpenRTI_OS_SOURCES}
  ${OpenRTI_EXPAT_SOURCES}
  ${OpenRTI_ZLIB_SOURCES}
)

# ADD_LIBRARY(OpenRTI-STATIC STATIC ${OpenRTI_SOURCES})
# SET_PROPERTY(TARGET OpenRTI-STATIC PROPERTY OUTPUT_NAME OpenRTI)
# SET_PROPERTY(TARGET OpenRTI-STATIC PROPERTY ARCHIVE_OUTPUT_NAME OpenRTI)

IF(OPENRTI_BUILD_SHARED)
  ADD_LIBRARY(OpenRTI SHARED ${OpenRTI_SOURCES})
  SET_PROPERTY(TARGET OpenRTI APPEND PROPERTY COMPILE_DEFINITIONS OPENRTI_DLL)
  SET_PROPERTY(TARGET OpenRTI APPEND PROPERTY COMPILE_DEFINITIONS OPENRTI_DLL_EXPORTS)
  SET_PROPERTY(TARGET OpenRTI PROPERTY VERSION 1.0.0)
  SET_PROPERTY(TARGET OpenRTI PROPERTY SOVERSION 1)
ELSE()
  ADD_LIBRARY(OpenRTI STATIC ${OpenRTI_SOURCES})
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
