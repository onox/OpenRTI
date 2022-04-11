project(OpenRTI CXX)
cmake_minimum_required(VERSION 2.8.12)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(PACKAGE_NAME "OpenRTI")
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 10)
set(CPACK_PACKAGE_VERSION_PATCH 0)

# Options

# Not fully supported for all operating systems, but currently
# the only way to make the circular link reference work on android.
set(OPENRTI_BUILD_SHARED TRUE CACHE BOOL "Build libraries as shared library.")
mark_as_advanced(OPENRTI_BUILD_SHARED)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(LINUX TRUE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(MACOS TRUE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
  set(SUNOS TRUE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "HP-UX")
  set(HPUX TRUE)
endif()

# We have unit tests
enable_testing()

# Settings
enable_language(C)

# Starting with cmake 3.1 we can easily change the
# c++ standard version using
# set(CMAKE_CXX_STANDARD 14)

# Some whole project sensible defines
if(UNIX)
  # Make sure we get the right libc functions
  add_definitions(-D_REENTRANT -D_THREAD_SAFE)
endif()
if(HPUX)
  if(NOT CMAKE_COMPILER_IS_GNUCXX)
     message("Using aCC on HP-UX requires -AA -mt somewhere in the compile line!")
  endif()
endif()
if(SUNOS)
  if(NOT CMAKE_COMPILER_IS_GNUCXX)
     message("Using CC on Solaris requires -mt somewhere in the compile line!")
  endif()
endif()

if(LINUX)
  # Use the DEBUG2 build type on linux to do stl debugging, note that this changes the abi,
  # In theory the debug build type is the one that is the right one, but since almost nobody else uses this
  # release type with these defines, we have our own here.
  set(CMAKE_CXX_FLAGS_DEBUG2 "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_GLIBCXX_CONCEPT_CHECKS")
  set(CMAKE_C_FLAGS_DEBUG2 "${CMAKE_C_FLAGS_DEBUG}")
endif()

if(MSVC)
  # Enable multi processor builds
  add_compile_options(/MP)
  # Treat warnings
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4290")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4290")
  if(CMAKE_CL_64)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /bigobj")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
  endif()
endif()
add_definitions(-DRTI_DISABLE_WARNINGS)

if(MINGW OR CYGWIN)
  if(DEFINED ENV{DLLTOOL})
    set(DLLTOOL "$ENV{DLLTOOL}")
  else()
    set(DLLTOOL dlltool)
  endif()
endif()

# Change the default build type to something fast
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif()

if(CMAKE_VERSION VERSION_GREATER 2.8.4)
  include(GNUInstallDirs)
else()
  include(GNUInstallDirsCompat)
endif()

set(OPENRTI_ENABLE_RTI13 ON CACHE BOOL "Build and install the RTI13 interface.")
set(OPENRTI_ENABLE_RTI1516 ON CACHE BOOL "Build and install the RTI1516 interface.")
set(OPENRTI_ENABLE_RTI1516E ON CACHE BOOL "Build and install the RTI1516E interface (EXPERIMENTAL).")

set(OPENRTI_ENABLE_PYTHON_BINDINGS ON CACHE BOOL "Build python binding extension modules.")
if(NOT CMAKE_VERSION VERSION_LESS 3.12)
  set(OPENRTI_FORCE_PYTHON2_BINDINGS OFF CACHE BOOL "When building python bindings, force python version 2.")
endif()

set(OPENRTI_RTI13_INCLUDE_SUBDIR "RTI13" CACHE PATH "Install the RTI13 include files into \${CMAKE_INSTALL_INCLUDEDIR}/\${OPENRTI_RTI13_INCLUDE_SUBDIR}.")
set(OPENRTI_RTI1516_INCLUDE_SUBDIR "rti1516" CACHE PATH "Install the RTI1516 include files into \${CMAKE_INSTALL_INCLUDEDIR}/\${OPENRTI_RTI1516_INCLUDE_SUBDIR}/RTI.")
set(OPENRTI_RTI1516E_INCLUDE_SUBDIR "rti1516e" CACHE PATH "Install the RTI1516E include files into \${CMAKE_INSTALL_INCLUDEDIR}/\${OPENRTI_RTI1516E_INCLUDE_SUBDIR}/RTI.")
mark_as_advanced(OPENRTI_RTI13_INCLUDE_SUBDIR OPENRTI_RTI1516_INCLUDE_SUBDIR OPENRTI_RTI1516E_INCLUDE_SUBDIR)

if(UNIX)
  # the RPATH to be used when installing, but only if it's not a system directory
  list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_FULL_LIBDIR}" isSystemDir)
  if("${isSystemDir}" STREQUAL "-1")
    set(OPENRTI_INSTALL_WITH_RPATH_DEFAULT 1 CACHE INTERNAL "Default for rpath setting")
  else()
    set(OPENRTI_INSTALL_WITH_RPATH_DEFAULT 0 CACHE INTERNAL "Default for rpath setting")
  endif()
else()
  set(OPENRTI_INSTALL_WITH_RPATH_DEFAULT 0 CACHE INTERNAL "Default for rpath setting")
endif()
set(OPENRTI_INSTALL_WITH_RPATH ${OPENRTI_INSTALL_WITH_RPATH_DEFAULT} CACHE BOOL "Build shared libraries with builtin search path.")

# The only known compiler with this binary incompatibility of debug libraries is cl
if(MSVC)
  set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Filename postfix for debug libraries.")
endif()

# We really want to hide all the OpenRTI private symbols.
if(OPENRTI_BUILD_SHARED)
  add_definitions(-DOPENRTI_DLL)
endif()

include(CheckCXXSourceRuns)
if(OPENRTI_INSTALL_WITH_RPATH)
  set(CMAKE_REQUIRED_FLAGS "-Wl,--enable-new-dtags")
  check_cxx_source_runs("int main() { return 0; }" hasEnableNewDTags)
  set(CMAKE_REQUIRED_FLAGS "")
  if(hasEnableNewDTags)
    set(OPENRTI_RPATH_LINK_FLAGS "-Wl,--enable-new-dtags" CACHE INTERNAL "Linker flag to enable RUNPATH additional to RPATH.")
  endif()
endif()

if(MACOS)
  set(CMAKE_REQUIRED_FLAGS "-Wl,-bind_at_load")
  check_cxx_source_runs("int main() { return 0; }" hasBindAtLoad)
  set(CMAKE_REQUIRED_FLAGS "")
  if(hasBindAtLoad)
    set(OPENRTI_BIND_AT_LOAD "-Wl,-bind_at_load" CACHE INTERNAL "Linker flag to resolve symbols at library load time on macos.")
  endif()
endif()

# Work around some linux linkers that seem to default to --as-needed
# Actually this also works around cmake reordering the link dependencies
# ... well and not to forget that the problem originates from that
# /creative/ way enable user defined logical times for an RTI.
# Not sure if this is the right approach, but seems to help for now.
if(CMAKE_CROSSCOMPILING)
  set(hasNoAsNeeded FALSE)
else()
  set(CMAKE_REQUIRED_FLAGS "-Wl,--no-as-needed")
  check_cxx_source_runs("int main() { return 0; }" hasNoAsNeeded)
  set(CMAKE_REQUIRED_FLAGS "")
endif()
if(hasNoAsNeeded)
  set(CMAKE_EXE_LINKER_FLAGS "-Wl,--no-as-needed")
endif()

set(OPENRTI_DATAROOTDIR "${CMAKE_INSTALL_FULL_DATAROOTDIR}/OpenRTI" CACHE PATH "Path where OpenRTI puts runtime required paths." FORCE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")

macro(openrti_target_link_private_libraries _target)
  if(CMAKE_VERSION VERSION_LESS 2.8.12)
    target_link_libraries(${_target} ${ARGN})
    # Each library where we have used this up to now uses explicit linking
    set_property(TARGET ${_target} PROPERTY LINK_INTERFACE_LIBRARIES "")
  elseif(CMAKE_VERSION VERSION_LESS 3.0.0)
    target_link_libraries(${_target} LINK_PRIVATE ${ARGN})
  else()
    target_link_libraries(${_target} PRIVATE ${ARGN})
  endif()
endmacro()

macro(openrti_target_position_independent _target)
  if(CMAKE_VERSION VERSION_GREATER 2.8.8)
    set_property(TARGET ${_target} PROPERTY POSITION_INDEPENDENT_CODE TRUE)
  else()
    set_property(TARGET ${_target} APPEND PROPERTY COMPILE_FLAGS ${CMAKE_SHARED_LIBRARY_C_FLAGS})
  endif()
endmacro()

function(openrti_set_msvc_stub_dll_name _target _name)
  # Ideally we could use
  #  set_property(TARGET fedtime1516Stub APPEND PROPERTY STATIC_LIBRARY_FLAGS_DEBUG "/DEF /NAME:$<TARGET_FILE:fedtime1516>")
  # to get the final target file name as it is produced, but STATIC_LIBRARY_FLAGS does not evaluate generator expressions
  set_property(TARGET ${_target} APPEND PROPERTY STATIC_LIBRARY_FLAGS "/DEF /NAME:${_name}.dll")
  foreach(_config ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${_config} _CONFIG)
    set(_dllName "${_name}${CMAKE_${_CONFIG}_POSTFIX}.dll")
    set_property(TARGET ${_target} APPEND PROPERTY STATIC_LIBRARY_FLAGS_${_CONFIG} "/DEF /NAME:${_dllName}")
  endforeach()
endfunction()

add_subdirectory(src)
add_subdirectory(tests)
if(OPENRTI_ENABLE_PYTHON_BINDINGS)
  add_subdirectory(python)
endif()
