project(BuildTool)

file(GLOB_RECURSE source CONFIGURE_DEPENDS "*.cpp")

add_compile_definitions(ENGINE_ASSET_FOLDER="${ENGINE_ASSETS_DIRECTORY}")
add_compile_definitions(GAME_ASSET_FOLDER="${GAME_ASSETS_DIRECTORY}")

add_executable(${PROJECT_NAME} ${source})

set_target_properties(${PROJECT_NAME}
    PROPERTIES 
        FOLDER Tools/
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Tools/BuildTool
)