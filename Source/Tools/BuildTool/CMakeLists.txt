project(BuildTool VERSION 0.1.0 LANGUAGES CSharp)

include(CSharpUtilities)

set(BuildTool_Version_Major 0)
set(BuildTool_Version_Minor 1)
set(BuildTool_Version_Patch 0)

set(BuildTool_Generated_File_Message "!!!This is a generated file. Do not Edit. Find corresponding *.template.cs file!!!")

# Generate files
configure_file(
    BuildTool_Config.cs.template 
    ${CMAKE_BINARY_DIR}/GeneratedSource/BuildTool/BuildTool_Config_generated.cs
)

# Collect Generated files
set(GeneratedFiles ${CMAKE_BINARY_DIR}/GeneratedSource/BuildTool/BuildTool_Config_generated.cs)

# Collect Source Files
file(GLOB_RECURSE SourceFiles CONFIGURE_DEPENDS "*.cs")

# Make Executable
add_executable(${PROJECT_NAME}
    ${GeneratedFiles}
    ${SourceFiles}
)

target_include_directories(${PROJECT_NAME} 
    PRIVATE 
        ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(${PROJECT_NAME} 
    PROPERTIES 
        FOLDER Tools/
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Tools
        DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2"
        VS_PACKAGE_REFERENCES "Serilog_2.10.0;Serilog.Sinks.Console_4.0.0"           
)