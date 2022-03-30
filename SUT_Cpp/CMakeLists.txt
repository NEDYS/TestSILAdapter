cmake_minimum_required(VERSION 3.10)

project(SilAdapterBasic DESCRIPTION "Basic SiL-Adapter Demo for Cpp" LANGUAGES C CXX)

# Add SilAdapter Dependency
add_subdirectory(SilAdapter)
# Add Applications
add_subdirectory(Application_C)
add_subdirectory(Application_Cpp)
