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
        ModelBuilder
        TextureBuilder
        SceneBuilder
)

set_target_properties(${PROJECT_NAME} 
    PROPERTIES 
        FOLDER Tools/
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        ARCHIVE_OUTPUT_DIRECTORY "${SJ_ARCHIVE_OUTPUT_DIRECTORY}"
        LIBRARY_OUTPUT_DIRECTORY "${SJ_LIBRARY_OUTPUT_DIRECTORY}"
        RUNTIME_OUTPUT_DIRECTORY "${SJ_RUNTIME_OUTPUT_DIRECTORY}"
)