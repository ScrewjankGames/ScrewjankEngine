cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

################################################################################
# CMake build options
################################################################################
option(OPT_Engine_Build_Sample "Build the Engine's sample project" ON)

################################################################################
# Project Settings
################################################################################
project(ScrewjankEngine C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(ScrewjankEngine_VERSION_MAJOR 0 CACHE STRING "major version" FORCE)
set(ScrewjankEngine_VERSION_MINOR 1 CACHE STRING "minor version" FORCE)
set(ScrewjankEngine_VERSION ${ScrewjankEngine_VERSION_MAJOR}.${ScrewjankEngine_VERSION_MINOR} CACHE STRING "version" FORCE)

################################################################################
# Define project-wide locations
################################################################################
# Assets
set(Engine_Asset_Dir ${CMAKE_CURRENT_SOURCE_DIR}/Assets/)
set(Game_Asset_Dir ${CMAKE_SOURCE_DIR}/Assets/)
set(Game_Built_Data_Dir ${CMAKE_SOURCE_DIR}/BuiltData/)

# Shaders
set(Engine_Shader_Source_Dir ${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/)
set(Game_Shader_Source_Dir ${CMAKE_SOURCE_DIR}/Source/Shaders/ )

set(Engine_Shader_Built_Data_Dir ${Game_Built_Data_Dir}/Engine/Shaders/ )
set(Game_Shader_Built_Data_Dir ${Game_Built_Data_Dir}/Shaders/ )

# Binaries
set(Binaries_Directory ${CMAKE_SOURCE_DIR}/bin)
set(Game_Binaries_Directory ${Binaries_Directory}/Game PARENT_SCOPE)
set(LOC_Game_Binaries_Directory ${Binaries_Directory}/Game ) # for some dumbass reason PARENT_SCOPE empties out the variable in this scope cause fuck me I guess.

# Engine crap
set(ENGINE_DEPENDENCIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies)
set(SAMPLE_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SampleProject)

################################################################################
# Global project build settings
################################################################################
set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
    "DebugGold"
    "ReleaseGold"
    CACHE STRING "" FORCE
)
set(CMAKE_CXX_FLAGS_ReleaseGold, CMAKE_CXX_FLAGS_RELEASE)
set(CMAKE_C_FLAGS_ReleaseGold, CMAKE_C_FLAGS_RELEASE)

################################################################################
# Setup symlinks
################################################################################
add_custom_target(MakeDataSymlinks ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${LOC_Game_Binaries_Directory}/Debug
    COMMAND ${CMAKE_COMMAND} -E make_directory ${LOC_Game_Binaries_Directory}/Release
    COMMAND ${CMAKE_COMMAND} -E make_directory ${LOC_Game_Binaries_Directory}/Gold
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${Game_Built_Data_Dir} ${LOC_Game_Binaries_Directory}/Debug/Data
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${Game_Built_Data_Dir} ${LOC_Game_Binaries_Directory}/Release/Data
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${Game_Built_Data_Dir} ${LOC_Game_Binaries_Directory}/Gold/Data
)
set_target_properties(MakeDataSymlinks
    PROPERTIES 
        FOLDER Tools/
)

################################################################################
# Include dependencies and engine modules
################################################################################
add_subdirectory(Dependencies)
add_subdirectory(Source)

if(OPT_Engine_Build_Sample)
    add_subdirectory(${SAMPLE_PROJECT_DIR})
endif()
