# All the -I arguments
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
# The OpenRTI implementation files and may be build generated sources
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../OpenRTI)
include_directories(${OPENRTI_CMAKEROOTDIR}/src/OpenRTI)

if (MSVC)
  set(RTINODE_RCFILES
    rtinode.rc
  )
else ()
  set(RTINODE_RCFILES
  )
endif()

# The standalone main server application
add_executable(rtinode rtinode.cpp ${RTINODE_RCFILES})
if(OPENRTI_INSTALL_WITH_RPATH)
  set_property(TARGET rtinode PROPERTY INSTALL_RPATH "\$ORIGIN:${CMAKE_INSTALL_FULL_LIBDIR}")
  set_property(TARGET rtinode APPEND PROPERTY LINK_FLAGS "${OPENRTI_RPATH_LINK_FLAGS}")
endif()
target_link_libraries(rtinode OpenRTI)

install(TARGETS rtinode
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
