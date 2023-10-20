################################################################################
# Define project: Math Library
################################################################################
project(ScrewjankMath)

file(GLOB_RECURSE MATH_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE MATH_INLINES CONFIGURE_DEPENDS "*.inl")
file(GLOB_RECURSE MATH_SOURCE CONFIGURE_DEPENDS "*.cpp")

add_library(${PROJECT_NAME} ${MATH_HEADERS} ${MATH_INLINES} ${MATH_SOURCE})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${MATH_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${MATH_INLINES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${MATH_SOURCE})

set_target_properties(${PROJECT_NAME} 
    PROPERTIES 
        FOLDER Shared/
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        ARCHIVE_OUTPUT_DIRECTORY "${Binaries_Directory}/lib/${PROJECT_NAME}"
        LIBRARY_OUTPUT_DIRECTORY "${Binaries_Directory}/lib/${PROJECT_NAME}"
)

################################################################################
# Link dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        ScrewjankCore
)

################################################################################
# Configure include directories
################################################################################
target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )