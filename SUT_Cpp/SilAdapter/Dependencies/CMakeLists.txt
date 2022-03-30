cmake_minimum_required(VERSION 3.10)

project(Cla DESCRIPTION "Common Language API (CLA) for the SIL Adapter use case." LANGUAGES CXX)

#####################################################
# (Third-Party) Dependencies
#####################################################

add_subdirectory(ThirdParty)

#####################################################
# Externals
#####################################################

add_subdirectory(SabSerDes/include)

option(DS_SILENT_MODE "Disables the debug messages of the DistSim adapter" ON)
add_subdirectory(DistSimAdapter)

#####################################################
# CLA
#####################################################

add_subdirectory(Cla)

#####################################################################
# Show Cla and all dependencies in VS folder SilAdapter/Dependencies
#####################################################################

set_property(TARGET Cla PROPERTY FOLDER "SilAdapter/Dependencies")
set_property(TARGET ClaDi PROPERTY FOLDER "SilAdapter/Dependencies")
set_property(TARGET ClaExperimental PROPERTY FOLDER "SilAdapter/Dependencies")
set_property(TARGET DistSimIB PROPERTY FOLDER "SilAdapter/Dependencies")
set_property(TARGET fedtime1516ev PROPERTY FOLDER "SilAdapter/Dependencies")
set_property(TARGET rti1516ev PROPERTY FOLDER "SilAdapter/Dependencies")
set_property(TARGET fom2cpp PROPERTY FOLDER "SilAdapter/Dependencies")
set_property(TARGET OpenRTI PROPERTY FOLDER "SilAdapter/Dependencies")
set_property(TARGET rtinode PROPERTY FOLDER "SilAdapter/Dependencies")
set_property(TARGET spdlog PROPERTY FOLDER "SilAdapter/Dependencies")
