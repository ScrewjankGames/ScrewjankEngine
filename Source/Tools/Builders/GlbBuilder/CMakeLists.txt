################################################################################
# Define project: Model Builder
################################################################################
project(GlbBuilder)

file(GLOB_RECURSE GLB_BUILDER_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE GLB_BUILDER_SOURCE CONFIGURE_DEPENDS "*.cpp")
file(GLOB_RECURSE GLB_BUILDER_MODULES CONFIGURE_DEPENDS "*.cppm")

add_library(${PROJECT_NAME} ${GLB_BUILDER_HEADERS} ${GLB_BUILDER_SOURCE})
target_sources(${PROJECT_NAME}
    PUBLIC
      FILE_SET CXX_MODULES FILES
        ${GLB_BUILDER_MODULES}
)
################################################################################
# Link dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} 
    PUBLIC 
        BuildersShared
        TextureBuilder
        fastgltf
)

################################################################################
# Configure include directories
################################################################################
target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)