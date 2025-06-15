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

set_target_properties(${PROJECT_NAME} 
    PROPERTIES 
        FOLDER SJSTD/
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        ARCHIVE_OUTPUT_DIRECTORY "${SJ_ARCHIVE_OUTPUT_DIRECTORY}/SJSTD/"
        LIBRARY_OUTPUT_DIRECTORY "${SJ_LIBRARY_OUTPUT_DIRECTORY}/SJSTD/"
        RUNTIME_OUTPUT_DIRECTORY "${SJ_RUNTIME_OUTPUT_DIRECTORY}/SJSTD/"
)
################################################################################
# Link dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} 
    PUBLIC
        spdlog::spdlog
)

################################################################################
# Configure include directories
################################################################################
target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

## Windows only- include SG14 for flat_* containers
if(WIN32)
    find_path(WG21_SG14_INCLUDE_DIRS "SG14/algorithm_ext.h")
    target_include_directories(${PROJECT_NAME} PRIVATE ${WG21_SG14_INCLUDE_DIRS})
endif()