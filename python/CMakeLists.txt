# Try to find out if we have python
if(OPENRTI_ENABLE_PYTHON_BINDINGS)
  find_package(PythonInterp)
  if(CMAKE_VERSION VERSION_GREATER 2.8.7)
    find_package(PythonLibs)
  else()
    find_package(PythonLibsCompat)
  endif()
else()
  set(PYTHONLIBS_FOUND FALSE)
endif()

# Ok, had issues with too old python versions, may be 2.6 is too crude ...
include(CheckCXXSourceCompiles)
if(PYTHONLIBS_FOUND)
  # All the -I arguments
  set(CMAKE_REQUIRED_INCLUDES ${PYTHON_INCLUDE_PATH})
  check_cxx_source_compiles("
#include <Python.h>
#if PY_MAJOR_VERSION < 2
# error
#elif PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION < 6
# error
#endif
int main(void)
{
  return 0;
}
" PYTHON_VERSION_OK)
endif()

# If so, do what we need to do
if(PYTHONLIBS_FOUND AND PYTHON_VERSION_OK)
  if(WIN32)
    string(REGEX REPLACE "^([0-9]+).([0-9]+).*$" "\\1\\2" PYTHONLIBS_VERSION_STRING "${PYTHONLIBS_VERSION_STRING}")
    set(INSTALL_PYTHON_DIRECTORY "Python${PYTHONLIBS_VERSION_STRING}/site-packages")
  else()
    string(REGEX REPLACE "^([0-9]+.[0-9]+).*$" "\\1" PYTHONLIBS_VERSION_STRING "${PYTHONLIBS_VERSION_STRING}")
    set(INSTALL_PYTHON_DIRECTORY "${CMAKE_INSTALL_LIBDIR}/python${PYTHONLIBS_VERSION_STRING}/site-packages")
  endif()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${INSTALL_PYTHON_DIRECTORY}")

  include_directories(${PYTHON_INCLUDE_PATH})
  if(OPENRTI_ENABLE_RTI1516)
    add_subdirectory(rti1516)
  endif()
  if(OPENRTI_ENABLE_RTI1516E)
    add_subdirectory(rti1516e)
  endif()
endif()
