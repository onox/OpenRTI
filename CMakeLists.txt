PROJECT(OpenRTI CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

SET(PACKAGE_NAME "OpenRTI")
SET(CPACK_PACKAGE_VERSION_MAJOR 0)
SET(CPACK_PACKAGE_VERSION_MINOR 0)
SET(CPACK_PACKAGE_VERSION_PATCH 0)

# Options
#SET(BUILD_SHARED 1 CACHE BOOL "Build libraries as shared library.")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  SET(LINUX TRUE)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  SET(MACOS TRUE)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
  SET(SUNOS TRUE)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "HP-UX")
  SET(HPUX TRUE)
ENDIF()

# We have unit tests
ENABLE_TESTING()

# Settings
ENABLE_LANGUAGE(C)
SET(CMAKE_VERBOSE_MAKEFILE 1 CACHE BOOL "")

# Some whole project sensible defines
IF(UNIX)
  # Make sure we get the right libc functions
  ADD_DEFINITIONS(-D_REENTRANT -D_THREAD_SAFE)
ENDIF(UNIX)
IF(HPUX)
  IF(NOT CMAKE_COMPILER_IS_GNUCXX)
     MESSAGE("Using aCC on HP-UX requires -AA -mt somwhere in the compile line!")
  ENDIF()
ENDIF()
IF(SUNOS)
  IF(NOT CMAKE_COMPILER_IS_GNUCXX)
     MESSAGE("Using CC on Solaris requires -mt somwhere in the compile line!")
  ENDIF()
ENDIF()

IF(LINUX)
  # Use the DEBUG2 build type on linux to do stl debugging, note that this changes the abi,
  # In theory the debug build type is the one that is the right one, but since almost nobody else uses this
  # release type with these defines, we have our own here.
  SET(CMAKE_CXX_FLAGS_DEBUG2 "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_GLIBCXX_CONCEPT_CHECKS")
  SET(CMAKE_C_FLAGS_DEBUG2 "${CMAKE_C_FLAGS_DEBUG}")
ENDIF()

IF(MSVC)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4290")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4290")
ENDIF()

# Change the default build type to something fast
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

IF(CMAKE_VERSION VERSION_GREATER 2.8.4)
  INCLUDE(GNUInstallDirs)
ELSE()
  INCLUDE(GNUInstallDirsCompat)
ENDIF()

IF(UNIX)
  # the RPATH to be used when installing, but only if it's not a system directory
  LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_FULL_LIBDIR}" isSystemDir)
  IF("${isSystemDir}" STREQUAL "-1")
    SET(OPENRTI_INSTALL_WITH_RPATH_DEFAULT 1 CACHE INTERNAL "Default for rpath setting")
  ELSE()
    SET(OPENRTI_INSTALL_WITH_RPATH_DEFAULT 0 CACHE INTERNAL "Default for rpath setting")
  ENDIF()
ELSE()
  SET(OPENRTI_INSTALL_WITH_RPATH_DEFAULT 0 CACHE INTERNAL "Default for rpath setting")
ENDIF()
SET(OPENRTI_INSTALL_WITH_RPATH ${OPENRTI_INSTALL_WITH_RPATH_DEFAULT} CACHE BOOL "Build shared libraries with builtin search path.")

FIND_PACKAGE(Qt4 4.4.0 COMPONENTS QtCore QtGui QtXml)
IF(QT4_FOUND)
  SET(OPENRTI_ENABLE_QRTI OFF CACHE BOOL "Build qrti, a qt based rti browser (EXPERIMENTAL).")
ENDIF()

ADD_SUBDIRECTORY(python)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(tests)

