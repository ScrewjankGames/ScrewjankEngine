################################################################################
# Define project: Screwjank Shader Compiler
################################################################################
project(SJShaderCompiler C CXX)

file(GLOB_RECURSE SJShaderCompiler_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE SJShaderCompiler_SOURCE CONFIGURE_DEPENDS "*.cpp")

add_executable(${PROJECT_NAME}  ${SJShaderCompiler_HEADERS} ${SJShaderCompiler_SOURCE})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SJShaderCompiler_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SJShaderCompiler_SOURCE})

set_target_properties(${PROJECT_NAME}  
    PROPERTIES 
        FOLDER Tools/
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Tools"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Tools"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Tools"
)

################################################################################
# Find Vulkan
################################################################################
find_package(Vulkan REQUIRED)


################################################################################
# Link dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        ${Vulkan_LIBRARIES}
)

################################################################################
# Configure include directories
################################################################################
target_include_directories(${PROJECT_NAME}  
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${Vulkan_INCLUDE_DIRS}
    )

################################################################################
# Install Settings
################################################################################
install(TARGETS SJShaderCompiler DESTINATION "${SJ_TOOL_DEST}")