#################################################################
# Sources
#################################################################

set(INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)

set(CLA_EXPERIMENTAL_PUBLIC_HEADERS
    ${INCLUDE_DIR}/cla/ClaExperimental.hpp
    ${INCLUDE_DIR}/cla/IClaServiceExperimental.hpp
)

set(CLA_EXPERIMENTAL_INTERNAL_HEADERS
    ${SOURCE_DIR}/ClaServiceExperimental.hpp
)

set(CLA_EXPERIMENTAL_SOURCES
    ${SOURCE_DIR}/ClaExperimental.cpp
    ${SOURCE_DIR}/ClaServiceExperimental.cpp
)

#################################################################
# Library Properties
#################################################################

add_library(ClaExperimental STATIC
    ${CLA_EXPERIMENTAL_PUBLIC_HEADERS}
    ${CLA_EXPERIMENTAL_INTERNAL_HEADERS}
    ${CLA_EXPERIMENTAL_SOURCES}
)

target_include_directories(ClaExperimental
    PUBLIC $<BUILD_INTERFACE:${INCLUDE_DIR}>
           $<INSTALL_INTERFACE:include>
)

target_link_libraries(ClaExperimental
    PUBLIC Cla
           DistSimIB
    PRIVATE ClaDi
)

install(
    TARGETS ClaExperimental
    EXPORT ClaTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# The experimental component has access to the CLA internals.
set_target_properties(ClaExperimental PROPERTIES CLA_INTERNALS 1)

#################################################################
# IDE Properties
#################################################################

source_group(TREE ${INCLUDE_DIR} PREFIX "public" FILES ${CLA_EXPERIMENTAL_PUBLIC_HEADERS})
source_group(TREE ${SOURCE_DIR} PREFIX "internal" FILES ${CLA_EXPERIMENTAL_INTERNAL_HEADERS})
source_group(TREE ${SOURCE_DIR} PREFIX "src" FILES ${CLA_EXPERIMENTAL_SOURCES})
