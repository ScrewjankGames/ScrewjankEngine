cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

################################################################################
# Project Settings
################################################################################
project(ScrewjankEngine C CXX)

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
set(Game_Data_Dir ${CMAKE_SOURCE_DIR}/Data/)

# Shaders
set(Engine_Shader_Source_Dir ${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/)
set(Game_Shader_Source_Dir ${CMAKE_SOURCE_DIR}/Source/Shaders/ )
set(Engine_Shader_Data_Dir ${CMAKE_SOURCE_DIR}/Data/Engine/Shaders/ )
set(Game_Shader_Data_Dir ${CMAKE_SOURCE_DIR}/Data/Shaders/ )

# Binaries
set(Binaries_Directory ${CMAKE_SOURCE_DIR}/bin)
set(Game_Binaries_Directory ${Binaries_Directory}/Game PARENT_SCOPE)

# Engine crap
set(ENGINE_DEPENDENCIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies)
set(SAMPLE_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SampleProject)

################################################################################
# CMake build options
################################################################################
option(ENGINE_BUILD_SAMPLE "Build the Engine's sample project" ON)

################################################################################
# Global project build settings
################################################################################
set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
    "Gold"
    CACHE STRING "" FORCE
)

set(CMAKE_CXX_FLAGS_GOLD, CMAKE_CXX_FLAGS_RELEASE)
set(CMAKE_C_FLAGS_GOLD, CMAKE_C_FLAGS_RELEASE)
add_compile_definitions($<$<CONFIG:Debug>:SJ_DEBUG>)
add_compile_definitions($<$<CONFIG:Release>:SJ_DEBUG>)
add_compile_definitions($<$<CONFIG:Gold>:SJ_GOLD>)

################################################################################
# Setup symlinks
################################################################################
add_custom_target(MakeDataSymlinks ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${Game_Binaries_Directory}/Debug
    COMMAND ${CMAKE_COMMAND} -E make_directory ${Game_Binaries_Directory}/Release
    COMMAND ${CMAKE_COMMAND} -E make_directory ${Game_Binaries_Directory}/Gold
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${Game_Data_Dir} ${Game_Binaries_Directory}/Debug/Data
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${Game_Data_Dir} ${Game_Binaries_Directory}/Release/Data
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${Game_Data_Dir} ${Game_Binaries_Directory}/Gold/Data
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

if(ENGINE_BUILD_SAMPLE)
    add_subdirectory(${SAMPLE_PROJECT_DIR})
endif()
