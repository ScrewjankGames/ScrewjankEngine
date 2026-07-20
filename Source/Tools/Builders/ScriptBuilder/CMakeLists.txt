################################################################################
# Define project: Model Builder
################################################################################
project(ScriptBuilder)

file(GLOB_RECURSE SCRIPT_BUILDER_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE SCRIPT_BUILDER_SOURCE CONFIGURE_DEPENDS "*.cpp")
file(GLOB_RECURSE SCRIPT_BUILDER_MODULES CONFIGURE_DEPENDS "*.cppm")

add_library(${PROJECT_NAME} ${SCRIPT_BUILDER_HEADERS} ${SCRIPT_BUILDER_SOURCE})
target_sources(${PROJECT_NAME}
    PUBLIC
      FILE_SET CXX_MODULES FILES
        ${SCRIPT_BUILDER_MODULES}
)
################################################################################
# Link dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} 
    PUBLIC 
        BuildersShared
        Luau.Compiler
)

################################################################################
# Configure include directories
################################################################################
target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)