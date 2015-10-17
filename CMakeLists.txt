PROJECT(OpenRTI CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

SET(PACKAGE_NAME "OpenRTI")
SET(CPACK_PACKAGE_VERSION_MAJOR 0)
SET(CPACK_PACKAGE_VERSION_MINOR 0)
SET(CPACK_PACKAGE_VERSION_PATCH 0)

# Options

# Not fully supported for all operating systems, but currently
# the only way to make the circular link reference work on android.
SET(OPENRTI_BUILD_SHARED TRUE CACHE BOOL "Build libraries as shared library.")
MARK_AS_ADVANCED(OPENRTI_BUILD_SHARED)

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

# Starting with cmake 3.1 we can easily change the
# c++ standard version using
# SET(CMAKE_CXX_STANDARD 14)

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
  IF(CMAKE_CL_64)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /bigobj")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
  ENDIF()
ENDIF()

IF(MINGW OR CYGWIN)
  IF(DEFINED ENV{DLLTOOL})
    SET(DLLTOOL "$ENV{DLLTOOL}")
  ELSE()
    SET(DLLTOOL dlltool)
  ENDIF()
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

SET(OPENRTI_ENABLE_RTI13 ON CACHE BOOL "Build and install the RTI13 interface.")
SET(OPENRTI_ENABLE_RTI1516 ON CACHE BOOL "Build and install the RTI1516 interface.")
SET(OPENRTI_ENABLE_RTI1516E ON CACHE BOOL "Build and install the RTI1516E interface (EXPERIMENTAL).")

SET(OPENRTI_ENABLE_PYTHON_BINDINGS ON CACHE BOOL "Build python binding extension modules.")

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

# We really want to hide all the OpenRTI private symbols.
IF(OPENRTI_BUILD_SHARED)
  ADD_DEFINITIONS(-DOPENRTI_DLL)
ENDIF()

INCLUDE(CheckCXXSourceRuns)
IF(OPENRTI_INSTALL_WITH_RPATH)
  SET(CMAKE_REQUIRED_FLAGS "-Wl,--enable-new-dtags")
  CHECK_CXX_SOURCE_RUNS("int main() { return 0; }" hasEnableNewDTags)
  SET(CMAKE_REQUIRED_FLAGS "")
  IF(hasEnableNewDTags)
    SET(OPENRTI_RPATH_LINK_FLAGS "-Wl,--enable-new-dtags" CACHE INTERNAL "Linker flag to enable RUNPATH additional to RPATH.")
  ENDIF()
ENDIF()

IF(MACOS)
  SET(CMAKE_REQUIRED_FLAGS "-Wl,-bind_at_load")
  CHECK_CXX_SOURCE_RUNS("int main() { return 0; }" hasBindAtLoad)
  SET(CMAKE_REQUIRED_FLAGS "")
  IF(hasBindAtLoad)
    SET(OPENRTI_BIND_AT_LOAD "-Wl,-bind_at_load" CACHE INTERNAL "Linker flag to resolve symbols at library load time on macos.")
  ENDIF()
ENDIF()

SET(OPENRTI_DATAROOTDIR "${CMAKE_INSTALL_FULL_DATAROOTDIR}/OpenRTI" CACHE PATH "Path where OpenRTI puts runtime required paths." FORCE)

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")

MACRO(OPENRTI_TARGET_LINK_PRIVATE_LIBRARIES _target)
  IF(CMAKE_VERSION VERSION_LESS 2.8.12)
    TARGET_LINK_LIBRARIES(${_target} ${ARGN})
    # Each library where we have used this up to now uses explicit linking
    SET_PROPERTY(TARGET ${_target} PROPERTY LINK_INTERFACE_LIBRARIES "")
  ELSEIF(CMAKE_VERSION VERSION_LESS 3.0.0)
    TARGET_LINK_LIBRARIES(${_target} LINK_PRIVATE ${ARGN})
  ELSE()
    TARGET_LINK_LIBRARIES(${_target} PRIVATE ${ARGN})
  ENDIF()
ENDMACRO()

ADD_SUBDIRECTORY(python)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(tests)

