# Creates a c file containing the content of the input
# file in a static variable with the gven name.
FUNCTION(CREATE_RESOURCE _inputFile _variableName _outputFile)
  # Work around a bug that hits us with older cmake versions.
  # Note the version is just one that is completely known to work.
  IF(CMAKE_VERSION VERSION_GREATER 2.8.10)
    # Read and in HEX form
    FILE(READ ${_inputFile} _filedata HEX)
    # Convert hex data to C values
    STRING(REGEX REPLACE "([0-9a-fA-F][0-9a-fA-F])" "char(0x\\1),\n" _filedata ${_filedata})
    # Write to the output file
    FILE(WRITE ${_outputFile} "static const char ${_variableName}[] = {\n${_filedata}};\n")
  ELSE()
    # Read in string format
    FILE(READ ${_inputFile} _filedata)
    # Convert characters to single escaped chars ... puh!
    STRING(REGEX REPLACE "(.)" "'\\1',\n" _filedata ${_filedata})
    STRING(REPLACE "'\n'" "0x0a" _filedata ${_filedata})
    STRING(REPLACE "'\r'" "0x0d" _filedata ${_filedata})
    STRING(REPLACE "'\t'" "' '" _filedata ${_filedata})
    STRING(REPLACE "'\"'" "0x22" _filedata ${_filedata})
    STRING(REPLACE "'\\'" "0x5c" _filedata ${_filedata})
    STRING(REPLACE "'''" "0x27" _filedata ${_filedata})
    # Write to the output file
    FILE(WRITE ${_outputFile} "static const char ${_variableName}[] = {\n${_filedata}};\n")
  ENDIF()
ENDFUNCTION()

