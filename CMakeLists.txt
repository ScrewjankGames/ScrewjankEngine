cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

################################################################################
# Project Settings
################################################################################
project(ScrewjankEngine C CXX)
set(CMAKE_CONFIGURATION_TYPES
    "Debug"
    "Release"
    CACHE STRING "" FORCE
)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(ENGINE_DEPENDENCIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies)
set(SAMPLE_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SampleProject)

################################################################################
# Include dependencies and engine modules
################################################################################
add_subdirectory(Dependencies)
add_subdirectory(Engine)