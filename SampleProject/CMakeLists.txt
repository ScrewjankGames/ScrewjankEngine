cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

project(SampleProject CXX)

################################################################################
# Define Game project name and build structure
################################################################################
set(GAME_PROJECT_NAME ${PROJECT_NAME})
set(GAME_PROJECT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(GAME_ASSETS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Assets/)
set(GAME_CONFIG_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Config/)
set(GAME_SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Source/)

################################################################################
# Collect Game Source Files
################################################################################
file(GLOB_RECURSE GAME_HEADERS CONFIGURE_DEPENDS ${GAME_SOURCE_DIRECTORY}/*.hpp)
source_group(TREE "${GAME_SOURCE_DIRECTORY}" PREFIX "Header Files" FILES ${GAME_HEADERS})

file(GLOB_RECURSE GAME_SOURCE CONFIGURE_DEPENDS ${GAME_SOURCE_DIRECTORY}/*.cpp)
source_group(TREE "${GAME_SOURCE_DIRECTORY}" PREFIX "Source Files" FILES ${GAME_SOURCE})

set(ALL_FILES
    ${GAME_HEADERS}
    ${GAME_SOURCE}
)

################################################################################
# Create Executable
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

set_target_properties(${PROJECT_NAME}
    PROPERTIES 
        #FOLDER Engine/
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
)

################################################################################
# Statically Link Dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} PRIVATE ScrewjankEngine)