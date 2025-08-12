################################################################################
# Define project: Model Builder
################################################################################
project(ModelBuilder)

file(GLOB_RECURSE MODEL_BUILDER_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE MODEL_BUILDER_INLINES CONFIGURE_DEPENDS "*.inl")
file(GLOB_RECURSE MODEL_BUILDER_SOURCE CONFIGURE_DEPENDS "*.cpp")

add_library(${PROJECT_NAME} ${MODEL_BUILDER_HEADERS} ${MODEL_BUILDER_INLINES} ${MODEL_BUILDER_SOURCE})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${MODEL_BUILDER_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${MODEL_BUILDER_INLINES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${MODEL_BUILDER_SOURCE})

set_target_properties(${PROJECT_NAME} 
    PROPERTIES 
        FOLDER Tools/Builders
        ARCHIVE_OUTPUT_DIRECTORY "${SJ_ARCHIVE_OUTPUT_DIRECTORY}/Builders"
        LIBRARY_OUTPUT_DIRECTORY "${SJ_LIBRARY_OUTPUT_DIRECTORY}/Builders"
        RUNTIME_OUTPUT_DIRECTORY "${SJ_RUNTIME_OUTPUT_DIRECTORY}/Builders"
)

################################################################################
# Link dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} 
    PUBLIC 
        tinyobjloader::tinyobjloader
        ScrewjankDataDefs
        ScrewjankStd
        BuildersShared
)

################################################################################
# Configure include directories
################################################################################
target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)