if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options("/MP")
endif()

# Open RTI
set(OPENRTI_BUILD_SHARED FALSE CACHE BOOL "" FORCE)
set(OPENRTI_ENABLE_PYTHON_BINDINGS OFF CACHE BOOL "Disable Python binding extension module." FORCE)
add_subdirectory(OpenRTI)

# spdlog
add_subdirectory(spdlog)
target_compile_definitions(spdlog PRIVATE "_SCL_SECURE_NO_WARNINGS")
