# Define Executable
add_executable(Application_C
    src/Logic.c
    src/Logic.h
    src/IO.h
    src/IO.c
    src/main.c
)

# Link SilAdapter with Executable
target_sources(Application_C
    PRIVATE
        TypeLibWrapper/TypeLibWrapper.h
        TypeLibWrapper/TypeLibWrapper.cpp
)
target_include_directories(Application_C
    PRIVATE
        TypeLibWrapper
)

target_link_libraries(Application_C
    PRIVATE TypeLib
)
