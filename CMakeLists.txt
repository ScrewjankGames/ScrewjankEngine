cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)

################################################################################
# Project Settings
################################################################################
project(ScrewjankEngine C CXX)

set(ScrewjankEngine_VERSION_MAJOR 0 CACHE STRING "major version" FORCE)
set(ScrewjankEngine_VERSION_MINOR 1 CACHE STRING "minor version" FORCE)
set(ScrewjankEngine_VERSION ${ScrewjankEngine_VERSION_MAJOR}.${ScrewjankEngine_VERSION_MINOR} CACHE STRING "version" FORCE)

include(CMake/modules/find_dependencies.cmake)

set(Engine_Asset_Dir ${CMAKE_CURRENT_SOURCE_DIR}/Assets/)
set(Engine_Shader_Source_Dir ${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/)

include(CMake/modules/engine_config.cmake)

################################################################################
# Include dependencies and engine modules
################################################################################
add_subdirectory(Source)