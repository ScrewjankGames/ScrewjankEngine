################################################################################
# Define project: Model Builder
################################################################################
project(MeshBuilder)

file(GLOB_RECURSE MESH_BUILDER_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE MESH_BUILDER_INLINES CONFIGURE_DEPENDS "*.inl")
file(GLOB_RECURSE MESH_BUILDER_SOURCE CONFIGURE_DEPENDS "*.cpp")

add_library(${PROJECT_NAME} ${MESH_BUILDER_HEADERS} ${MESH_BUILDER_INLINES} ${MESH_BUILDER_SOURCE})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${MESH_BUILDER_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${MESH_BUILDER_INLINES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${MESH_BUILDER_SOURCE})

################################################################################
# Link dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} 
    PUBLIC 
        BuildersShared
        tinyobjloader
)

################################################################################
# Configure include directories
################################################################################
target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)