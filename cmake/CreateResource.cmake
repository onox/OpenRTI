# Creates a c file containing the content of the input
# file in a static variable with the gven name.
FUNCTION(CREATE_RESOURCE _inputFile _variableName _outputFile)
  # Read and in HEX form
  FILE(READ ${_inputFile} _filedata HEX)
  # Convert hex data to C values
  STRING(REGEX REPLACE "([0-9a-fA-F][0-9a-fA-F])" "0x\\1,\n" _filedata ${_filedata})
  # Write to the output file
  FILE(WRITE ${_outputFile} "static const char ${_variableName}[] = {\n${_filedata}};\n")
ENDFUNCTION()

