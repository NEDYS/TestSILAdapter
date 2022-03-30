# Just for propper recursion
add_subdirectory(OpenRTI)

option(OPENRTI_BUILD_RTINODE "Build the rtinode" ON)
if (OPENRTI_BUILD_RTINODE)
	add_subdirectory(rtinode)
endif()

option(OPENRTI_BUILD_TOOLS "Build additional tools (fom2cpp, FOMCodeGen)" ON)
if (OPENRTI_BUILD_TOOLS)
	add_subdirectory(Tools)
endif()

add_subdirectory(rti1516ev)
