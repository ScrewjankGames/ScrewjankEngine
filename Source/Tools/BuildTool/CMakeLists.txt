project(BuildTool VERSION 0.1.0)

set(BuildTool_Version_Major 0)
set(BuildTool_Version_Minor 1)
set(BuildTool_Version_Patch 0)

set(BuildTool_Generated_File_Message "!!!This is a generated file. Do not Edit. Find corresponding *.template.hpp file!!!")

# Generate files
configure_file(
    BuildTool_Config.hpp.template 
    ${CMAKE_BINARY_DIR}/GeneratedSource/BuildTool/BuildTool_Config_generated.hpp
)

# Collect Generated files
set(GeneratedFiles ${CMAKE_BINARY_DIR}/GeneratedSource/BuildTool/BuildTool_Config_generated.hpp)

# Collect Source Files
file(GLOB_RECURSE SourceFiles CONFIGURE_DEPENDS "*.cpp")

# Make Executable
add_executable(${PROJECT_NAME}
    ${GeneratedFiles}
    ${SourceFiles}
)

target_include_directories(${PROJECT_NAME} 
    PRIVATE 
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_BINARY_DIR}/GeneratedSource/BuildTool
)

set_target_properties(${PROJECT_NAME} 
    PROPERTIES 
        FOLDER Tools/
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        RUNTIME_OUTPUT_DIRECTORY ${Binaries_Directory}/Tools
)