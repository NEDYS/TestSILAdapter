# All the -I arguments
# The standalone main server application
add_executable(fom2cpp fom2cpp.cpp list2cpp.cpp)

install(TARGETS fom2cpp
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
