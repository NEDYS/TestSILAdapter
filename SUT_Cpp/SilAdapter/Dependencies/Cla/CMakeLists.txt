###########################################################
# Properties / Options / Definitions for all CLA Components
###########################################################

option(CLA_ENABLE_LOGGING "Enables the logging of the CLA." OFF)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options("/MP")
    add_compile_definitions("_SCL_SECURE_NO_WARNINGS")
    add_compile_definitions("_CRT_SECURE_NO_WARNINGS")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

    if(${MSVC_TOOLSET_VERSION} GREATER_EQUAL 141)
        # VS2017 or higher
        add_compile_options("/permissive-")
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options("-pedantic")
    add_compile_options("-Wall" "-Wextra")
endif()

###########################################################
# CLA Components
###########################################################

add_subdirectory(Cla)
add_subdirectory(ClaDi)
add_subdirectory(ClaExperimental)
