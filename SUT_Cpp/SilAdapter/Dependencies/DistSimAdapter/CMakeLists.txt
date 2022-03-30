if (DS_SILENT_MODE)
  add_definitions(-DSILENT)
endif()

if (DS_SUPPRESS_ERRORS)
  add_definitions(-DSUPPRESS_ERRORS)
endif()

add_library(DistSimIB STATIC
    include/distSim/IDistSimIB.h
    include/distSim/IServices.h
    include/distSim/RTFederateDataTypes.h
    src/DistSimIBConfig.h
    src/RTFederateEncodings.h
    src/RTFederateEncodings.cpp
    src/DistSimIB.h
    src/DistSimIB.cpp
    src/AbstractService.h
    src/RemoteAmbassador.h
    src/RemoteAmbassador.cpp
    src/DOService.h
    src/DOService.cpp
    src/BusService.h
    src/BusService.cpp
    src/TextLogService.h
    src/TextLogService.cpp
    src/TimingService.h
    src/TimingService.cpp
    src/UserTimer.h
    src/UserTimer.cpp
    src/EventNotification.h
    src/EventNotification.cpp
    src/FOMDefinitions.h
    src/UUIDT4.h
    src/UUIDT4.cpp
    src/Utils.h
    src/Utils.cpp
	src/RTFederateFOM.inc
)

#target_compile_options(DistSimIB PRIVATE "-Werror -Wall -Wno-unused-value -Wno-inconsistent-missing-override -Wno-implicit-exception-spec-mismatch -Wno-format-security")
target_compile_definitions(DistSimIB PRIVATE "NOMINMAX")

if (DS_TREAT_WARNINGS_AS_ERRORS)
  message("Treat warnigns as errors")
  set(WERROR_MSVC "/WX")
  set(WERROR_OTHER "-Werror")
else()
  set(WERROR_MSVC "")
  set(WERROR_OTHER "")
endif()

target_compile_options(DistSimIB PRIVATE
  $<$<CXX_COMPILER_ID:Clang>:
    ${WERROR_OTHER} -Woverloaded-virtual -Wno-unused-lambda-capture -Wno-format-security -Wno-unused-value -Wno-inconsistent-missing-override -Wno-implicit-exception-spec-mismatch>

  $<$<CXX_COMPILER_ID:GNU>:
    ${WERROR_OTHER} -Woverloaded-virtual -Wno-format-security>

  # /wd4503: Disable C4503, that appears for toolset v140: decorated name length exceeded, name was truncated 
  $<$<CXX_COMPILER_ID:MSVC>:
    ${WERROR_MSVC} /W3 /wd4503>
 )

#message(WARNING "DistSimAdapter: CHANGE THIS: move OpenRTI include directory from PUBLIC to PRIVATE section") 
target_include_directories(DistSimIB
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/distSim>
           $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
           $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../ThirdParty/OpenRTI/include/rti1516ev>
           $<INSTALL_INTERFACE:include/rti1516ev>
           $<INSTALL_INTERFACE:include>
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(DistSimIB
    PUBLIC rti1516ev fedtime1516ev
)

###############################################################################
# Install
###############################################################################

install(
    TARGETS DistSimIB
    EXPORT DistSimIBTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include DESTINATION .)
