################################################################################
# Define project: IO Library
################################################################################
project(ScrewjankSharedCore)

file(GLOB_RECURSE CORE_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE CORE_INLINES CONFIGURE_DEPENDS "*.inl")
file(GLOB_RECURSE CORE_SOURCE CONFIGURE_DEPENDS "*.cpp")

add_library(${PROJECT_NAME} ${CORE_HEADERS} ${CORE_INLINES} ${CORE_SOURCE})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${CORE_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${CORE_INLINES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${CORE_SOURCE})

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
        ${ENGINE_DEPENDENCIES_DIR}/spdlog/include
    )