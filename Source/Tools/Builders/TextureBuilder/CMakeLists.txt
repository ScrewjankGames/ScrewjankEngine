################################################################################
# Define project: Texture Builder
################################################################################
project(TextureBuilder)

file(GLOB_RECURSE TEXTURE_BUILDER_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE TEXTURE_BUILDER_SOURCE CONFIGURE_DEPENDS "*.cpp")
file(GLOB_RECURSE TEXTURE_BUILDER_MODULES CONFIGURE_DEPENDS "*.cppm")

add_library(${PROJECT_NAME} ${TEXTURE_BUILDER_HEADERS} ${TEXTURE_BUILDER_SOURCE})
target_sources(${PROJECT_NAME}
    PUBLIC
      FILE_SET CXX_MODULES FILES
        ${TEXTURE_BUILDER_MODULES}
)

################################################################################
# Link dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} 
    PUBLIC
        BuildersShared
        stb
)

################################################################################
# Configure include directories
################################################################################

target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${Stb_INCLUDE_DIR}
)