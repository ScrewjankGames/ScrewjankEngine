cmake_minimum_required(VERSION 3.28)

################################################################################
# Define project: Containers Library
################################################################################
project(ScrewjankStd CXX)
file(GLOB_RECURSE SJ_STD_MODULES CONFIGURE_DEPENDS "*.cppm")
file(GLOB_RECURSE SJ_STD_HEADERS CONFIGURE_DEPENDS "*.hpp")

add_library(${PROJECT_NAME} ${SJ_STD_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${CONFIGURE_DEPENDS})

target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET CXX_MODULES FILES
      ${SJ_STD_MODULES}
)

################################################################################
# Link dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} 
    PUBLIC
        spdlog::spdlog
        glaze::glaze
)

################################################################################
# Configure include directories
################################################################################
target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )