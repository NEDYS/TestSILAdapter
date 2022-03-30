# Creates a c file containing the content of the input
# file in a static variable with the gven name.
function(create_resource _inputFile _variableName _outputFile)
  # Read and in HEX form
  file(READ ${_inputFile} _filedata HEX)
  # Convert hex data to C values
  string(REGEX REPLACE "([0-9a-fA-F][0-9a-fA-F])" "char(0x\\1),\n" _filedata ${_filedata})
  # Write to the output file
  file(WRITE ${_outputFile} "static const char ${_variableName}[] = {\n${_filedata}};\n")
endfunction()

