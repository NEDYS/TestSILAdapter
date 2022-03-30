# This file was generated automatically. Changes to this file may
# cause incorrect behavior and will be lost if the code is regenerated.

set(TYPELIB_STATIC_PUBLIC_HEADERS
    include/TypeLib/DataTypes.hpp
    include/TypeLib/DoConsumedData.hpp
    include/TypeLib/DoConsumedMethod.hpp
    include/TypeLib/DoData.hpp
    include/TypeLib/DistributedObject.hpp
    include/TypeLib/DistributedObjectBase.hpp
    include/TypeLib/DoMethod.hpp
    include/TypeLib/DoProvidedData.hpp
    include/TypeLib/DoProvidedMethod.hpp
    include/TypeLib/Serialization.hpp
    include/TypeLib/Serializer.hpp
    include/TypeLib/Deserializer.hpp
    include/TypeLib/StrongType.hpp
    include/TypeLib/EmbeddedMember.hpp
    include/TypeLib/Accessor.hpp
    include/TypeLib/DoEvent.hpp
    include/TypeLib/DoConsumedEvent.hpp
    include/TypeLib/DoProvidedEvent.hpp
    include/TypeLib/ThreadContext.hpp
)

set(TYPELIB_STATIC_INTERNAL_HEADERS
    src/Path.hpp
)

set(TYPELIB_STATIC_SOURCES
    src/DistributedObjectBase.cpp
    src/DoMethod.cpp
    src/DoProvidedMethod.cpp
    src/DoConsumedMethod.cpp
    src/Serializer.cpp
    src/Deserializer.cpp
    src/ThreadContext.cpp
    src/Path.cpp
)

set(TYPELIB_PUBLIC_HEADERS
    include/TypeLib/TypeLib.hpp
    include/TypeLib/RoomTemperatureControl.hpp
    include/TypeLib/RoomTemperatureControl/RoomTemperatureControl_Types.hpp
    include/TypeLib/RoomTemperatureControl/ISensor.hpp
    include/TypeLib/RoomTemperatureControl/IHeating.hpp
    include/TypeLib/RoomTemperatureControl/State.hpp
)

set(TYPELIB_INTERNAL_HEADERS
    src/RoomTemperatureControl/RoomTemperatureControl_Initialization.hpp
)

set(TYPELIB_SOURCES
    src/TypeLib.cpp
    src/RoomTemperatureControl.cpp
    src/RoomTemperatureControl/RoomTemperatureControl_Initialization.cpp
    src/RoomTemperatureControl/ISensor.cpp
    src/RoomTemperatureControl/IHeating.cpp
)

add_library(TypeLib STATIC
    ${TYPELIB_STATIC_SOURCES}
    ${TYPELIB_STATIC_INTERNAL_HEADERS}
    ${TYPELIB_STATIC_PUBLIC_HEADERS}
    ${TYPELIB_SOURCES}
    ${TYPELIB_PUBLIC_HEADERS}
    ${TYPELIB_INTERNAL_HEADERS}
)

target_include_directories(TypeLib
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(TypeLib
    PUBLIC Cla
)

source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/include PREFIX generated/public FILES ${TYPELIB_PUBLIC_HEADERS})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/src PREFIX generated/internal FILES ${TYPELIB_INTERNAL_HEADERS})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/src PREFIX generated/src FILES ${TYPELIB_SOURCES})

source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/include PREFIX static/public FILES ${TYPELIB_STATIC_PUBLIC_HEADERS})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/src PREFIX static/internal FILES ${TYPELIB_STATIC_INTERNAL_HEADERS})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/src PREFIX static/src FILES ${TYPELIB_STATIC_SOURCES})
