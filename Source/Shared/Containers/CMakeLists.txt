cmake_minimum_required(VERSION 3.28)

################################################################################
# Define project: Containers Library
################################################################################
project(ScrewjankSharedContainers CXX)
file(GLOB_RECURSE CONTAINER_MODULES CONFIGURE_DEPENDS "*.cppm")

add_library(${PROJECT_NAME})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${CONTAINER_MODULES})

target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET CXX_MODULES FILES
      ${CONTAINER_MODULES}
)

set_target_properties(${PROJECT_NAME} 
    PROPERTIES 
        FOLDER Shared/Container
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        ARCHIVE_OUTPUT_DIRECTORY "${SJ_ARCHIVE_OUTPUT_DIRECTORY}/Shared/"
        LIBRARY_OUTPUT_DIRECTORY "${SJ_LIBRARY_OUTPUT_DIRECTORY}/Shared/"
        RUNTIME_OUTPUT_DIRECTORY "${SJ_RUNTIME_OUTPUT_DIRECTORY}/Shared/"
)

################################################################################
# Link dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} 
    PUBLIC
        ScrewjankSharedCore
)
################################################################################
# Configure include directories
################################################################################
target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${ENGINE_DEPENDENCIES_DIR}/spdlog/include
    )