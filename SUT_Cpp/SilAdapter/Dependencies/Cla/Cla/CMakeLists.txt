#################################################################
# Sources
#################################################################

set(INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)

set(CLA_PUBLIC_HEADERS
    ${INCLUDE_DIR}/cla/DataTypes.hpp
    ${INCLUDE_DIR}/cla/IDo.hpp
    ${INCLUDE_DIR}/cla/IDoMember.hpp
    ${INCLUDE_DIR}/cla/IDoData.hpp
    ${INCLUDE_DIR}/cla/IDoEvent.hpp
    ${INCLUDE_DIR}/cla/IDoField.hpp
    ${INCLUDE_DIR}/cla/IDoMethod.hpp
    ${INCLUDE_DIR}/cla/IDoConsumedData.hpp
    ${INCLUDE_DIR}/cla/IDoConsumedEvent.hpp
    ${INCLUDE_DIR}/cla/IDoConsumedField.hpp
    ${INCLUDE_DIR}/cla/IDoConsumedMethod.hpp
    ${INCLUDE_DIR}/cla/IDoProvidedData.hpp
    ${INCLUDE_DIR}/cla/IDoProvidedEvent.hpp
    ${INCLUDE_DIR}/cla/IDoProvidedField.hpp
    ${INCLUDE_DIR}/cla/IDoProvidedMethod.hpp
    ${INCLUDE_DIR}/cla/IValueEntity.hpp
    ${INCLUDE_DIR}/cla/ISerializer.hpp
    ${INCLUDE_DIR}/cla/IDeserializer.hpp
    ${INCLUDE_DIR}/cla/ICallContext.hpp
    ${INCLUDE_DIR}/cla/IConsumerCallContext.hpp
    ${INCLUDE_DIR}/cla/IProviderCallContext.hpp
    ${INCLUDE_DIR}/cla/ClaMacros.hpp
    ${INCLUDE_DIR}/cla/Version.hpp

    # SIL Use Case only

    ${INCLUDE_DIR}/cla/ClaSil.hpp
    ${INCLUDE_DIR}/cla/IClaService.hpp
    ${INCLUDE_DIR}/cla/IClaServiceSil.hpp
)

set(CLA_INTERNAL_HEADERS
    ${SOURCE_DIR}/ClaService.hpp
    ${SOURCE_DIR}/DoRegistry.hpp
    ${SOURCE_DIR}/Do.hpp
    ${SOURCE_DIR}/DoMember.hpp
    ${SOURCE_DIR}/DoData.hpp
    ${SOURCE_DIR}/DoEvent.hpp
    ${SOURCE_DIR}/DoMethod.hpp
    ${SOURCE_DIR}/DoField.hpp
    ${SOURCE_DIR}/DoConsumedData.hpp
    ${SOURCE_DIR}/DoConsumedEvent.hpp
    ${SOURCE_DIR}/DoConsumedMethod.hpp
    ${SOURCE_DIR}/DoConsumedField.hpp
    ${SOURCE_DIR}/DoProvidedData.hpp
    ${SOURCE_DIR}/DoProvidedEvent.hpp
    ${SOURCE_DIR}/DoProvidedMethod.hpp
    ${SOURCE_DIR}/DoProvidedField.hpp
    ${SOURCE_DIR}/CallContext.hpp
    ${SOURCE_DIR}/ConsumerCallContext.hpp
    ${SOURCE_DIR}/ProviderCallContext.hpp
    ${SOURCE_DIR}/ValueEntity.hpp
    ${SOURCE_DIR}/Serializer.hpp
    ${SOURCE_DIR}/Deserializer.hpp
    ${SOURCE_DIR}/DistSimIBHelper.hpp
)

set(CLA_SOURCES
    ${SOURCE_DIR}/ClaSil.cpp
    ${SOURCE_DIR}/ClaService.cpp
    ${SOURCE_DIR}/DoRegistry.cpp
    ${SOURCE_DIR}/Do.cpp
    ${SOURCE_DIR}/DoMember.cpp
    ${SOURCE_DIR}/DoData.cpp
    ${SOURCE_DIR}/DoEvent.cpp
    ${SOURCE_DIR}/DoMethod.cpp
    ${SOURCE_DIR}/DoField.cpp
    ${SOURCE_DIR}/DoConsumedData.cpp
    ${SOURCE_DIR}/DoConsumedEvent.cpp
    ${SOURCE_DIR}/DoConsumedMethod.cpp
    ${SOURCE_DIR}/DoConsumedField.cpp
    ${SOURCE_DIR}/DoProvidedData.cpp
    ${SOURCE_DIR}/DoProvidedEvent.cpp
    ${SOURCE_DIR}/DoProvidedMethod.cpp
    ${SOURCE_DIR}/DoProvidedField.cpp
    ${SOURCE_DIR}/ValueEntity.cpp
    ${SOURCE_DIR}/CallContext.cpp
    ${SOURCE_DIR}/ConsumerCallContext.cpp
    ${SOURCE_DIR}/ProviderCallContext.cpp
    ${SOURCE_DIR}/Serializer.cpp
    ${SOURCE_DIR}/Deserializer.cpp
    ${SOURCE_DIR}/Version.cpp
    ${SOURCE_DIR}/DistSimIBHelper.cpp
)

#################################################################
# Library Properties
#################################################################

add_library(Cla STATIC
    ${CLA_PUBLIC_HEADERS}
    ${CLA_INTERNAL_HEADERS}
    ${CLA_SOURCES}
)

if (CLA_ENABLE_LOGGING)
    target_compile_definitions(Cla PRIVATE "CLA_ENABLE_LOGGING")
endif()


target_include_directories(Cla
    PUBLIC $<BUILD_INTERFACE:$<$<BOOL:$<TARGET_PROPERTY:CLA_INTERNALS>>:${SOURCE_DIR}>>
           $<BUILD_INTERFACE:${INCLUDE_DIR}>
           $<INSTALL_INTERFACE:include>
)

target_link_libraries(Cla
    PUBLIC $<$<BOOL:$<TARGET_PROPERTY:CLA_INTERNALS>>:spdlog>
    PRIVATE ClaDi
            DistSimIB
            SabSerDes
            spdlog
)

#################################################################
# Install
#################################################################

install(
    TARGETS Cla
    EXPORT ClaTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY ${INCLUDE_DIR} DESTINATION .)

#################################################################
# IDE Properties
#################################################################

source_group(TREE ${INCLUDE_DIR} PREFIX "public" FILES ${CLA_PUBLIC_HEADERS})
source_group(TREE ${SOURCE_DIR} PREFIX "internal" FILES ${CLA_INTERNAL_HEADERS})
source_group(TREE ${SOURCE_DIR} PREFIX "src" FILES ${CLA_SOURCES})
