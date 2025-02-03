################################################################################
# Define project: Texture Builder
################################################################################
project(BuildersShared)

file(GLOB_RECURSE BUILDER_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE BUILDER_INLINES CONFIGURE_DEPENDS "*.inl")
file(GLOB_RECURSE BUILDER_SOURCE CONFIGURE_DEPENDS "*.cpp")

# Generate files
set(BuildTool_Generated_File_Message "!!!This is a generated file. Do not Edit. Find corresponding *.template.hpp file!!!")
configure_file(
    BuildTool_Config.hpp.template 
    ${SJ_CODEGEN_OUTPUT_DIRECTORY}/BuildTool/BuildTool_Config_generated.hpp
)

# Collect Generated files
set(GeneratedFiles ${SJ_CODEGEN_OUTPUT_DIRECTORY}/BuildTool/BuildTool_Config_generated.hpp)

add_library(${PROJECT_NAME} ${BUILDER_HEADERS} ${BUILDER_INLINES} ${BUILDER_SOURCE} ${GeneratedFiles})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${BUILDER_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${BUILDER_INLINES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${BUILDER_SOURCE})

set_target_properties(${PROJECT_NAME} 
    PROPERTIES 
        FOLDER Tools/Builders
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        ARCHIVE_OUTPUT_DIRECTORY "${SJ_ARCHIVE_OUTPUT_DIRECTORY}/Builders"
        LIBRARY_OUTPUT_DIRECTORY "${SJ_LIBRARY_OUTPUT_DIRECTORY}/Builders"
        RUNTIME_OUTPUT_DIRECTORY "${SJ_RUNTIME_OUTPUT_DIRECTORY}/Builders"
)

################################################################################
# Link dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} 
    PUBLIC 
        ScrewjankSharedCore
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SJ_CODEGEN_OUTPUT_DIRECTORY}/BuildTool
)
