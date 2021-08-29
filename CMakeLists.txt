cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

################################################################################
# Project Settings
################################################################################
project(ScrewjankEngine C CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(ScrewjankEngine_VERSION_MAJOR 0 CACHE STRING "major version" FORCE)
set(ScrewjankEngine_VERSION_MINOR 1 CACHE STRING "minor version" FORCE)
set(ScrewjankEngine_VERSION ${ScrewjankEngine_VERSION_MAJOR}.${ScrewjankEngine_VERSION_MINOR} CACHE STRING "version" FORCE)

set(ENGINE_ASSETS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Assets/)

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
add_compile_definitions($<$<CONFIG:Gold>:SJ_GOLD>)

################################################################################
# Define project-wide locations
################################################################################
set(ENGINE_DEPENDENCIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies)
set(SAMPLE_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SampleProject)

################################################################################
# Include dependencies and engine modules
################################################################################
add_subdirectory(Dependencies)
add_subdirectory(Source)

if(ENGINE_BUILD_SAMPLE)
    add_subdirectory(${SAMPLE_PROJECT_DIR})
endif()
