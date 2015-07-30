# Try to find out if we have python
FIND_PACKAGE(PythonLibs)

# Ok, had issues with too old python versions, may be 2.6 is too crude ...
INCLUDE(CheckCXXSourceCompiles)
IF(PYTHONLIBS_FOUND)
  # All the -I arguments
  SET(CMAKE_REQUIRED_INCLUDES ${PYTHON_INCLUDE_PATH})
  CHECK_CXX_SOURCE_COMPILES("
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
ENDIF()

# If so, do what we need to do
IF(PYTHONLIBS_FOUND AND PYTHON_VERSION_OK)
  INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
  IF(OPENRTI_ENABLE_RTI1516)
    ADD_SUBDIRECTORY(rti1516)
  ENDIF()
  IF(OPENRTI_ENABLE_RTI1516E)
    ADD_SUBDIRECTORY(rti1516e)
  ENDIF()
ENDIF()
