cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

################################################################################
# Project Settings
################################################################################
project(ScrewjankEngine C CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(ScrewjankEngine_VERSION_MAJOR 1 CACHE STRING "major version" FORCE)
set(ScrewjankEngine_VERSION_MINOR 0 CACHE STRING "minor version" FORCE)
set(ScrewjankEngine_VERSION ${ScrewjankEngine_VERSION_MAJOR}.${ScrewjankEngine_VERSION_MINOR} CACHE STRING "version" FORCE)

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

add_compile_definitions($<$<CONFIG:Gold>:-DSJ_GOLD>)
if(MSVC)
    add_compile_options("$<$<CONFIG:Gold>:${CMAKE_CXX_FLAGS_RELEASE} /LTCG /GL>")
elseif(Clang)
    message(FATAL_ERROR "Need to configure c++ gold flags for clang.")
endif()

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
