################################################################################
# Define project: Texture Builder
################################################################################
project(SceneBuilder)

file(GLOB_RECURSE BUILDER_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE BUILDER_INLINES CONFIGURE_DEPENDS "*.inl")
file(GLOB_RECURSE BUILDER_SOURCE CONFIGURE_DEPENDS "*.cpp")

add_library(${PROJECT_NAME} ${BUILDER_HEADERS} ${BUILDER_INLINES} ${BUILDER_SOURCE})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${BUILDER_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${BUILDER_INLINES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${BUILDER_SOURCE})

################################################################################
# Link dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} 
    PUBLIC 
        BuildersShared
)

################################################################################
# Configure include directories
################################################################################
target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)