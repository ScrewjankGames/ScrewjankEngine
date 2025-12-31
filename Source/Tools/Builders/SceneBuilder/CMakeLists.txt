################################################################################
# Define project: Scene Builder
################################################################################
project(SceneBuilder)

file(GLOB_RECURSE SCENE_BUILDER_MODULES CONFIGURE_DEPENDS "*.cppm")
file(GLOB_RECURSE SCENE_BUILDER_SOURCE CONFIGURE_DEPENDS "*.cpp")

add_library(${PROJECT_NAME} ${SCENE_BUILDER_SOURCE})
target_sources(${PROJECT_NAME}
    PUBLIC
      FILE_SET CXX_MODULES FILES
        ${SCENE_BUILDER_MODULES}
)

################################################################################
# Link dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} 
    PUBLIC
        BuildersShared
)