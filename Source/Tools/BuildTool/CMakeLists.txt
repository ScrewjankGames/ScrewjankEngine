project(BuildTool VERSION 0.1.0)

set(BuildTool_Version_Major 0)
set(BuildTool_Version_Minor 1)
set(BuildTool_Version_Patch 0)


# Collect Source Files
file(GLOB_RECURSE SourceFiles CONFIGURE_DEPENDS "*.cpp")

# Make Executable
add_executable(${PROJECT_NAME}
    ${SourceFiles}
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        BuildersShared
        MeshBuilder
        TextureBuilder
        SceneBuilder
)