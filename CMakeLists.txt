PROJECT(OpenRTI CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0)

SET(PACKAGE_NAME "OpenRTI")
SET(CPACK_PACKAGE_VERSION_MAJOR 0)
SET(CPACK_PACKAGE_VERSION_MINOR 0)
SET(CPACK_PACKAGE_VERSION_PATCH 0)

# Options
#OPTION(BUILD_SHARED "Build libraries as shared library" ON)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  SET(LINUX TRUE)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
  SET(SUNOS TRUE)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "HP-UX")
  SET(HPUX TRUE)
ENDIF()

# We have unit tests
ENABLE_TESTING()

# Settings
ENABLE_LANGUAGE(C)
SET(CMAKE_VERBOSE_MAKEFILE ON)

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

# Use the DEBUG2 build type on linux to do stl debugging, note that this changes the abi,
# In theory the debug build type is the one that is the right one, but since almost nobody else uses this
# release type with these defines, we have our own here.
SET(CMAKE_CXX_FLAGS_DEBUG2 "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_GLIBCXX_CONCEPT_CHECKS")
SET(CMAKE_C_FLAGS_DEBUG2 "${CMAKE_C_FLAGS_DEBUG}")

# Change the default build type to something fast
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

ADD_SUBDIRECTORY(python)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(tests)

