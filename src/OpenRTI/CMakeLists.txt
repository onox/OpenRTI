include(CheckIncludeFiles)
# include(CheckSymbolExists)
# include(CheckFunctionExists)
include(CheckCXXSourceCompiles)

check_include_files(cstdint OpenRTI_HAVE_CSTDINT)
check_include_files(stdint.h OpenRTI_HAVE_STDINT_H)
check_include_files(inttypes.h OpenRTI_HAVE_INTTYPES_H)
if(NOT WIN32)
  set(CMAKE_REQUIRED_LIBRARIES dl)
  check_cxx_source_compiles("
#define _GNU_SOURCE
#include <dlfcn.h>
int main(int argc, char* argv[])
{
  Dl_info info;
  dladdr((void*)main, &info);
  return 0;
}
" OpenRTI_HAVE_DLADDR)
  set(CMAKE_REQUIRED_LIBRARIES)
  if(OpenRTI_HAVE_DLADDR)
    set(DL_LIBRARIES dl)
  endif()
endif()
check_cxx_source_compiles("
#include <alloca.h>
int main(int argc, char* argv[])
{
  void* data = alloca(17);
  return 0;
}
" OpenRTI_HAVE_ALLOCA)

# sigh, since we do network traffic, we should know that ...
include(TestBigEndian)
test_big_endian(BIG_ENDIAN)
if(BIG_ENDIAN)
  set(OpenRTI_HOST_IS_BIG_ENDIAN 1)
else(BIG_ENDIAN)
  set(OpenRTI_HOST_IS_LITTLE_ENDIAN 1)
endif(BIG_ENDIAN)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/OpenRTIConfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/OpenRTIConfig.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(LINUX)
  add_definitions(-D_XOPEN_SOURCE=600)
  add_definitions(-D_DEFAULT_SOURCE=1)
elseif(UNIX)
  add_definitions(-D_XOPEN_SOURCE_EXTENDED=1)
elseif(WIN32)
  # for now, silence some win32 warnings - want to review that later ...
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  # We need XP/SP2 We are currently bound to the GetAddrInfo call introduced
  # in that version.
  add_definitions(-DWINVER=0x0502 -D_WIN32_WINNT=0x0502)
endif()


# A bunch of sources that are operating system dependent.
if(WIN32)
  set(OpenRTI_OS_SOURCES
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
else(WIN32)
  set(OpenRTI_OS_SOURCES
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
endif(WIN32)

# We use expat as xml parser, if not on the system, use our own probably outdated version
include(FindEXPAT)
if(EXPAT_FOUND)
  include_directories(${EXPAT_INCLUDE_DIRS})
else(EXPAT_FOUND)
  # Additions for expat
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/xml)
  add_definitions(-DXML_STATIC)
  add_definitions(-DHAVE_MEMMOVE)

  set(OpenRTI_EXPAT_SOURCES
    xml/xmlparse.c
    xml/xmlrole.c
    xml/xmltok.c
    xml/xmltok_impl.c
    xml/xmltok_ns.c
  )
endif(EXPAT_FOUND)

include(FindZLIB)
if(ZLIB_FOUND)
  include_directories(${ZLIB_INCLUDE_DIRS})
  add_definitions(-DOPENRTI_HAVE_ZLIB)
  set(OpenRTI_ZLIB_SOURCES
    ZLibProtocolLayer.cpp
  )
else()
  set(OpenRTI_ZLIB_SOURCES "")
endif()

set(OpenRTI_SOURCES
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

if(OPENRTI_BUILD_SHARED)
  add_library(OpenRTI SHARED ${OpenRTI_SOURCES})
  set_property(TARGET OpenRTI APPEND PROPERTY COMPILE_DEFINITIONS OPENRTI_DLL)
  set_property(TARGET OpenRTI APPEND PROPERTY COMPILE_DEFINITIONS OPENRTI_DLL_EXPORTS)
  set_property(TARGET OpenRTI PROPERTY VERSION 1.0.0)
  set_property(TARGET OpenRTI PROPERTY SOVERSION 1)
else()
  add_library(OpenRTI STATIC ${OpenRTI_SOURCES})
  openrti_target_position_independent(OpenRTI)
endif()
set_property(TARGET OpenRTI PROPERTY OUTPUT_NAME OpenRTI)
if(EXPAT_FOUND)
  openrti_target_link_private_libraries(OpenRTI ${EXPAT_LIBRARIES})
endif(EXPAT_FOUND)
if(ZLIB_FOUND)
  openrti_target_link_private_libraries(OpenRTI ${ZLIB_LIBRARIES})
endif()
if(WIN32)
  openrti_target_link_private_libraries(OpenRTI ws2_32)
else(WIN32)
  if(SUNOS)
    openrti_target_link_private_libraries(OpenRTI socket)
    openrti_target_link_private_libraries(OpenRTI nsl)
  endif()
  if (NOT ANDROID)
    openrti_target_link_private_libraries(OpenRTI pthread)
    if (NOT MACOS)
      openrti_target_link_private_libraries(OpenRTI rt)
    endif()
  endif()
  openrti_target_link_private_libraries(OpenRTI m)
endif(WIN32)
if(DL_LIBRARIES)
  openrti_target_link_private_libraries(OpenRTI ${DL_LIBRARIES})
endif(DL_LIBRARIES)

install(TARGETS OpenRTI
  PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
