################################################################################
# Define project: Texture Builder
################################################################################
project(BuildersShared)

file(GLOB_RECURSE BUILDER_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE BUILDER_SOURCE CONFIGURE_DEPENDS "*.cpp")
file(GLOB_RECURSE Builder_Modules CONFIGURE_DEPENDS "*.cppm")

add_library(${PROJECT_NAME} ${BUILDER_HEADERS} ${BUILDER_INLINES} ${BUILDER_SOURCE})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${BUILDER_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${BUILDER_INLINES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${BUILDER_SOURCE})
target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET CXX_MODULES FILES
      ${Builder_Modules}
)

################################################################################
# Link dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} 
    PUBLIC 
        ScrewjankStd
        ScrewjankDataDefs
        glaze::glaze
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${generated_header_dir}
)
