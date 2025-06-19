cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)

################################################################################
# Project Settings
################################################################################
project(ScrewjankEngine C CXX)
include(CMake/find_dependencies.cmake)

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
set(Game_Built_Data_Dir ${CMAKE_SOURCE_DIR}/Data/)

# Shaders
set(Engine_Shader_Source_Dir ${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/)
set(Game_Shader_Source_Dir ${CMAKE_SOURCE_DIR}/Source/Shaders/ )

set(Engine_Shader_Built_Data_Dir ${Game_Built_Data_Dir}/Engine/Shaders/ )
set(Game_Shader_Built_Data_Dir ${Game_Built_Data_Dir}/Shaders/ )

################################################################################
# Setup symlinks
################################################################################
add_custom_target(RefreshSymlinks ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${Game_Built_Data_Dir} ${SJ_RUNTIME_OUTPUT_DIRECTORY}/Data
)
set_target_properties(RefreshSymlinks
    PROPERTIES 
        FOLDER Tools/
)

################################################################################
# Include dependencies and engine modules
################################################################################
add_subdirectory(Source)

if(OPT_Engine_Build_Sample)
    add_subdirectory(${SAMPLE_PROJECT_DIR})
endif()
