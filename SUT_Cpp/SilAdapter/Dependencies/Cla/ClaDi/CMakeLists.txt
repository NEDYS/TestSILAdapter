#################################################################
# Sources
#################################################################

set(INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/source)

set(CLA_DI_HEADERS
    ${INCLUDE_DIR}/ClaDi/ServiceFactory.hpp
)

set(CLA_DI_SOURCES
    ${SOURCE_DIR}/ServiceFactory.cpp
)

#################################################################
# Library Properties
#################################################################

add_library(ClaDi STATIC
    ${CLA_DI_HEADERS}
    ${CLA_DI_SOURCES}
)

target_include_directories(ClaDi
    PUBLIC $<BUILD_INTERFACE:${INCLUDE_DIR}>
           $<INSTALL_INTERFACE:include>
)

target_link_libraries(ClaDi
    PRIVATE DistSimIB
)

#################################################################
# Install
#################################################################

install(
    TARGETS ClaDi
    EXPORT ClaTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

#################################################################
# IDE Properties
#################################################################

source_group(TREE ${INCLUDE_DIR} PREFIX "public" FILES ${CLA_DI_HEADERS})
source_group(TREE ${SOURCE_DIR} PREFIX "src" FILES ${CLA_DI_SOURCES})
