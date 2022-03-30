# Define Executable
add_executable(Application_Cpp
   src/Logic.cpp
   src/Logic.hpp
   src/IO.hpp
   src/IO.cpp
   src/main.cpp
)

# Link SilAdapter with Executable
target_link_libraries(Application_Cpp PRIVATE TypeLib)
