# The rti1516ev includes
include_directories(${OPENRTI_CMAKEROOTDIR}/include/rti1516ev)
include_directories(${OPENRTI_CMAKEROOTDIR}/src/OpenRTI)

add_library(rti1516e_py MODULE Module.cpp)
set_property(TARGET rti1516e_py PROPERTY PREFIX "")
set_property(TARGET rti1516e_py PROPERTY OUTPUT_NAME "rti1516ev")
if(WIN32)
  set_property(TARGET rti1516e_py PROPERTY SUFFIX ".pyd")
endif()
target_link_libraries(rti1516e_py rti1516ev fedtime1516ev ${PYTHON_LIBRARIES})

if(OPENRTI_BUILD_SHARED)
  if(OPENRTI_INSTALL_WITH_RPATH)
    set_property(TARGET rti1516e_py PROPERTY INSTALL_RPATH "$ORIGIN/../..")
    set_property(TARGET rti1516e_py APPEND PROPERTY LINK_FLAGS "${OPENRTI_RPATH_LINK_FLAGS}")
  endif()
endif()

install(TARGETS rti1516e_py
  RUNTIME DESTINATION "${INSTALL_PYTHON_DIRECTORY}"
  LIBRARY DESTINATION "${INSTALL_PYTHON_DIRECTORY}"
  ARCHIVE DESTINATION "${INSTALL_PYTHON_DIRECTORY}")
