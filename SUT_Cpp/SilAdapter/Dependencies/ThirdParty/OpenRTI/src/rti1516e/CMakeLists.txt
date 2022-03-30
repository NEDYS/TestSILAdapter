# The directory containing the public api for rti1516e
set(RTI1516E_HEADER_PATH ${OPENRTI_CMAKEROOTDIR}/include/rti1516e)
set(RTI1516E_DATA_PATH ${OPENRTI_CMAKEROOTDIR}/share/rti1516e)

# All the -I arguments
include_directories(${RTI1516E_HEADER_PATH})
# The OpenRTI implementation files and may be build generated sources
include_directories(${CMAKE_BINARY_DIR}/src/OpenRTI)
include_directories(${OPENRTI_CMAKEROOTDIR}/src/OpenRTI)

include(CreateResource)
create_resource(${OPENRTI_CMAKEROOTDIR}/share/rti1516e/HLAstandardMIM.xml
  HLAstandardMIM_xml ${CMAKE_CURRENT_BINARY_DIR}/HLAstandardMIM.inc)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(RTI1516E_PUBLIC_HEADERS
        ${RTI1516E_HEADER_PATH}/RTI/Enums.h
        ${RTI1516E_HEADER_PATH}/RTI/Exception.h
        ${RTI1516E_HEADER_PATH}/RTI/FederateAmbassador.h
        ${RTI1516E_HEADER_PATH}/RTI/Handle.h
        ${RTI1516E_HEADER_PATH}/RTI/LogicalTimeFactory.h
        ${RTI1516E_HEADER_PATH}/RTI/LogicalTime.h
        ${RTI1516E_HEADER_PATH}/RTI/LogicalTimeInterval.h
        ${RTI1516E_HEADER_PATH}/RTI/RangeBounds.h
        ${RTI1516E_HEADER_PATH}/RTI/NullFederateAmbassador.h
        ${RTI1516E_HEADER_PATH}/RTI/RTI1516.h
        ${RTI1516E_HEADER_PATH}/RTI/RTIambassadorFactory.h
        ${RTI1516E_HEADER_PATH}/RTI/RTIambassador.h
        ${RTI1516E_HEADER_PATH}/RTI/SpecificConfig.h
        ${RTI1516E_HEADER_PATH}/RTI/Typedefs.h
        ${RTI1516E_HEADER_PATH}/RTI/VariableLengthData.h
)

set(RTI1516E_PUBLIC_ENCODING_HEADERS
        ${RTI1516E_HEADER_PATH}/RTI/encoding/BasicDataElements.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/DataElement.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/EncodingExceptions.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/EncodingConfig.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/HLAfixedArray.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/HLAfixedRecord.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/HLAopaqueData.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/HLAvariableArray.h
        ${RTI1516E_HEADER_PATH}/RTI/encoding/HLAvariantRecord.h
)

set(RTI1516E_PUBLIC_TIME_HEADERS
        ${RTI1516E_HEADER_PATH}/RTI/time/HLAfloat64Interval.h
        ${RTI1516E_HEADER_PATH}/RTI/time/HLAfloat64TimeFactory.h
        ${RTI1516E_HEADER_PATH}/RTI/time/HLAfloat64Time.h
        ${RTI1516E_HEADER_PATH}/RTI/time/HLAinteger64Interval.h
        ${RTI1516E_HEADER_PATH}/RTI/time/HLAinteger64TimeFactory.h
        ${RTI1516E_HEADER_PATH}/RTI/time/HLAinteger64Time.h
)

set(LIB_RTI1516E_SOURCES
		Encoding.h
		HandleFactory.cpp
		HandleFactory.h
		HandleImplementation.h
		RTI1516ELogicalTimeFactory.h
		RTI1516Efloat64TimeFactory.h
		RTI1516Einteger64TimeFactory.h
		RTIambassadorImplementation.h
		VariableLengthDataImplementation.h
        ../OpenRTI/dprintf.cpp
        ../OpenRTI/dprintf.h
        BasicDataElements.cpp
        DataElement.cpp
        EncodingExceptions.cpp
        Exception.cpp
        FederateAmbassador.cpp
        HLAfixedArray.cpp
        HLAfixedRecord.cpp
        HLAfloat64Interval.cpp
        HLAfloat64Time.cpp
        HLAfloat64TimeFactory.cpp
        HLAhandle.cpp
        HLAinteger64Interval.cpp
        HLAinteger64Time.cpp
        HLAinteger64TimeFactory.cpp
        HLAlogicalTimeFactoryFactory.cpp
        HLAopaqueData.cpp
        HLAvariableArray.cpp
        HLAvariantRecord.cpp
        Handle.cpp
        LogicalTime.cpp
        LogicalTimeFactory.cpp
        LogicalTimeInterval.cpp
        NullFederateAmbassador.cpp
        RTI1516ELogicalTimeFactory.cpp
        RTIambassador.cpp
        RTIambassadorFactory.cpp
        RTIambassadorImplementation.cpp
        RangeBounds.cpp
        Typedefs.cpp
        VariableLengthData.cpp
        Version.cpp
)
if (MSVC)
  set(LIB_RTI1516E_RCFILES
    rti1516e.rc
  )
else ()
  set(LIB_RTI1516E_RCFILES
  )
endif()

if(MSVC)
  set(LIB_FEDTIME1516E_SOURCES
    LogicalTimeFactoryFactory.cpp
	${RTI1516E_HEADER_PATH}/RTI/LogicalTimeFactory.h
  )
  set(LIB_FEDTIME1516E_RCFILES
    fedtime1516e.rc
  )
else ()
  set(LIB_FEDTIME1516E_SOURCES
    HLAfloat64Interval.cpp
    HLAfloat64Time.cpp
    HLAfloat64TimeFactory.cpp
    HLAinteger64Interval.cpp
    HLAinteger64Time.cpp
    HLAinteger64TimeFactory.cpp
    LogicalTimeFactoryFactory.cpp
  )
  set(LIB_FEDTIME1516E_RCFILES
  )
endif()

if(OPENRTI_BUILD_SHARED)
  add_library(rti1516e SHARED ${LIB_RTI1516E_SOURCES} ${RTI1516E_PUBLIC_HEADERS} ${RTI1516E_PUBLIC_ENCODING_HEADERS} ${RTI1516E_PUBLIC_TIME_HEADERS} ${LIB_RTI1516E_RCFILES})
  set_property(TARGET rti1516e APPEND PROPERTY COMPILE_DEFINITIONS BUILDING_RTI)
# ems: disable CMake so library versioning
#  set_property(TARGET rti1516e PROPERTY VERSION 1.0.0)
#  set_property(TARGET rti1516e PROPERTY SOVERSION 1)

  # rti1516e, the circular reference is tricky on win32
  if(MSVC)
    add_library(fedtime1516eStub STATIC ${LIB_FEDTIME1516E_SOURCES})
    set_property(TARGET fedtime1516eStub APPEND PROPERTY COMPILE_DEFINITIONS BUILDING_FEDTIME)
    # Ideally we could use
    #  set_property(TARGET fedtime1516eStub APPEND PROPERTY STATIC_LIBRARY_FLAGS_DEBUG "/DEF /NAME:$<TARGET_FILE:fedtime1516e>")
    # to get the final target file name as it is produced, but STATIC_LIBRARY_FLAGS does not evaluate generator expressions
    # So use different variants of CMAKE_<CONFIG>_POSTFIX
    openrti_set_msvc_stub_dll_name(fedtime1516eStub fedtime1516e)
    openrti_target_link_private_libraries(rti1516e fedtime1516eStub OpenRTI)
  elseif(MINGW OR CYGWIN)
    add_library(fedtime1516eStub STATIC ${LIB_FEDTIME1516E_SOURCES})
    set_property(TARGET fedtime1516eStub APPEND PROPERTY COMPILE_DEFINITIONS BUILDING_FEDTIME)
    string(TOUPPER ${CMAKE_BUILD_TYPE} _CONFIG)
    set(_fedtimeDllName fedtime1516e${CMAKE_${_CONFIG}_POSTFIX}.dll)
    add_custom_command(TARGET fedtime1516eStub
      POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E remove -f ${_fedtimeDllName}.a
      COMMAND "${DLLTOOL}" -D${_fedtimeDllName} -l${_fedtimeDllName}.a $<TARGET_FILE:fedtime1516eStub>
      COMMAND "${CMAKE_COMMAND}" -E remove -f $<TARGET_FILE:fedtime1516eStub>
      COMMAND "${CMAKE_COMMAND}" -E rename ${_fedtimeDllName}.a $<TARGET_FILE:fedtime1516eStub>
      )
    openrti_target_link_private_libraries(rti1516e fedtime1516eStub OpenRTI)
  else()
    openrti_target_link_private_libraries(rti1516e OpenRTI)
    if(OPENRTI_INSTALL_WITH_RPATH)
      set_property(TARGET rti1516e PROPERTY INSTALL_RPATH "$ORIGIN")
      set_property(TARGET rti1516e APPEND PROPERTY LINK_FLAGS "${OPENRTI_RPATH_LINK_FLAGS}")
    endif()
  endif()
  if(MACOS)
    set_property(TARGET rti1516e APPEND PROPERTY LINK_FLAGS "-Wl,-undefined,dynamic_lookup ${OPENRTI_BIND_AT_LOAD}")
  endif()
else()
  add_library(rti1516e STATIC ${LIB_RTI1516E_SOURCES} ${RTI1516E_PUBLIC_HEADERS} ${RTI1516E_PUBLIC_ENCODING_HEADERS} ${RTI1516E_PUBLIC_TIME_HEADERS})
  set_property(TARGET rti1516e APPEND PROPERTY COMPILE_DEFINITIONS STATIC_RTI STATIC_FEDTIME)
  set_property(TARGET rti1516e PROPERTY LINK_INTERFACE_MULTIPLICITY 2)
  openrti_target_position_independent(rti1516e)
  openrti_target_link_private_libraries(rti1516e fedtime1516e OpenRTI)
endif()
# bkd: no 'lib' for win
#if(MSVC)
  #set_property(TARGET rti1516e PROPERTY PREFIX "lib")
#endif()

# libfedtime1516e
if(OPENRTI_BUILD_SHARED)
  add_library(fedtime1516e SHARED ${LIB_FEDTIME1516E_SOURCES} ${LIB_FEDTIME1516E_RCFILES})
  set_property(TARGET fedtime1516e APPEND PROPERTY COMPILE_DEFINITIONS BUILDING_FEDTIME)
# ems: disable CMake so library versioning
#  set_property(TARGET fedtime1516e PROPERTY VERSION 1.0.0)
#  set_property(TARGET fedtime1516e PROPERTY SOVERSION 1)
  if(OPENRTI_INSTALL_WITH_RPATH)
    set_property(TARGET fedtime1516e PROPERTY INSTALL_RPATH "$ORIGIN")
    set_property(TARGET fedtime1516e APPEND PROPERTY LINK_FLAGS "${OPENRTI_RPATH_LINK_FLAGS}")
  endif()
else()
  add_library(fedtime1516e STATIC ${LIB_FEDTIME1516E_SOURCES})
  target_compile_definitions(fedtime1516e PUBLIC STATIC_RTI STATIC_FEDTIME)
  set_property(TARGET fedtime1516e PROPERTY LINK_INTERFACE_MULTIPLICITY 2)
  openrti_target_position_independent(fedtime1516e)
endif()
# bkd: no 'lib' for win
#if(MSVC)
  #set_property(TARGET fedtime1516e PROPERTY PREFIX "lib")
#endif()
openrti_target_link_private_libraries(fedtime1516e rti1516e OpenRTI)

install(TARGETS fedtime1516e rti1516e EXPORT OpenRTITargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES ${RTI1516E_PUBLIC_HEADERS}
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${OPENRTI_RTI1516E_INCLUDE_SUBDIR}/RTI")
install(FILES ${RTI1516E_PUBLIC_ENCODING_HEADERS}
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${OPENRTI_RTI1516E_INCLUDE_SUBDIR}/RTI/encoding")
install(FILES ${RTI1516E_PUBLIC_TIME_HEADERS}
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${OPENRTI_RTI1516E_INCLUDE_SUBDIR}/RTI/time")
install(FILES ${RTI1516E_DATA_PATH}/HLAstandardMIM.xml
  DESTINATION "${OPENRTI_DATAROOTDIR}/rti1516e")

if(UNIX)
  include(PkgConfig)
  create_pkg_config(rti1516e "HLA/RTI 1516E Communication Library." "${OPENRTI_RTI1516E_INCLUDE_SUBDIR}" "")
  create_pkg_config(fedtime1516e "HLA/RTI 1516E Time Library." "${OPENRTI_RTI1516E_INCLUDE_SUBDIR}" "")
  create_api_pkg_config(hla-rti1516e "HLA/RTI 1516E Libraries." rti1516e fedtime1516e)
endif()
