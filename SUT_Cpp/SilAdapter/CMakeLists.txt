cmake_minimum_required(VERSION 3.10)

################################################################################
# Project Definition
################################################################################

project("SilAdapter" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

################################################################################
# Include TypeLib Dependencies
################################################################################

add_subdirectory(Dependencies)

################################################################################
# Include TypeLib
################################################################################

add_subdirectory(TypeLib)
set_target_properties(TypeLib PROPERTIES FOLDER "SilAdapter")

add_library(SilAdapter ALIAS TypeLib)
