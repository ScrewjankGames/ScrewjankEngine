################################################################################
# Define project: Math Library
################################################################################
project(ScrewjankMath)

file(GLOB_RECURSE MATH_SOURCE CONFIGURE_DEPENDS "*.cpp")
file(GLOB_RECURSE MATH_MODULES CONFIGURE_DEPENDS "*.cppm")

add_library(${PROJECT_NAME} ${MATH_SOURCE})
target_sources(${PROJECT_NAME}
    PUBLIC
      FILE_SET CXX_MODULES FILES
        ${MATH_MODULES}
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${MATH_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${MATH_INLINES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${MATH_SOURCE})

set_target_properties(${PROJECT_NAME} 
    PROPERTIES 
        FOLDER Shared/
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        ARCHIVE_OUTPUT_DIRECTORY "${SJ_ARCHIVE_OUTPUT_DIRECTORY}/Shared/"
        LIBRARY_OUTPUT_DIRECTORY "${SJ_LIBRARY_OUTPUT_DIRECTORY}/Shared/"
        RUNTIME_OUTPUT_DIRECTORY "${SJ_RUNTIME_OUTPUT_DIRECTORY}/Shared/"
)

################################################################################
# Link dependencies
################################################################################

################################################################################
# Configure include directories
################################################################################
target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )