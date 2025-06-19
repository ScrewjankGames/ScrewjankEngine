################################################################################
# Define project: Texture Builder
################################################################################
project(TextureBuilder)

file(GLOB_RECURSE TEXTURE_BUILDER_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE TEXTURE_BUILDER_INLINES CONFIGURE_DEPENDS "*.inl")
file(GLOB_RECURSE TEXTURE_BUILDER_SOURCE CONFIGURE_DEPENDS "*.cpp")

add_library(${PROJECT_NAME} ${TEXTURE_BUILDER_HEADERS} ${TEXTURE_BUILDER_INLINES} ${TEXTURE_BUILDER_SOURCE})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${TEXTURE_BUILDER_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${TEXTURE_BUILDER_INLINES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${TEXTURE_BUILDER_SOURCE})

set_target_properties(${PROJECT_NAME} 
    PROPERTIES 
        FOLDER Tools/Builders
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        ARCHIVE_OUTPUT_DIRECTORY "${SJ_ARCHIVE_OUTPUT_DIRECTORY}/Builders"
        LIBRARY_OUTPUT_DIRECTORY "${SJ_LIBRARY_OUTPUT_DIRECTORY}/Builders"
        RUNTIME_OUTPUT_DIRECTORY "${SJ_RUNTIME_OUTPUT_DIRECTORY}/Builders"
)
get_target_property(TextureBuilderExe ${PROJECT_NAME} OUTPUT_NAME)

################################################################################
# Link dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} 
    PUBLIC
        ScrewjankDataDefs
        ScrewjankSharedCore
        BuildersShared
)

################################################################################
# Configure include directories
################################################################################

target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${Stb_INCLUDE_DIR}
)