################################################################################
# Define project: ScrewjankEngine
################################################################################
project(ScrewjankEngine)

file(GLOB_RECURSE RUNTIME_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE RUNTIME_INLINES CONFIGURE_DEPENDS "*.inl")
file(GLOB_RECURSE RUNTIME_SOURCE CONFIGURE_DEPENDS "*.cpp")
file(GLOB_RECURSE RUNTIME_MODULES CONFIGURE_DEPENDS "*.cppm")

add_library(${PROJECT_NAME} ${RUNTIME_HEADERS} ${RUNTIME_INLINES} ${RUNTIME_SOURCE})
target_sources(${PROJECT_NAME}
    PUBLIC
      FILE_SET CXX_MODULES FILES
        ${RUNTIME_MODULES}
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${RUNTIME_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${RUNTIME_INLINES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${RUNTIME_SOURCE})

### For Single-config generators
if(SJ_GOLD_OPT)
target_compile_definitions(${PROJECT_NAME} PUBLIC SJ_GOLD)
endif()

set_target_properties(${PROJECT_NAME} 
    PROPERTIES 
        FOLDER Engine/
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        ARCHIVE_OUTPUT_DIRECTORY "${SJ_ARCHIVE_OUTPUT_DIRECTORY}"
        LIBRARY_OUTPUT_DIRECTORY "${SJ_LIBRARY_OUTPUT_DIRECTORY}"
        RUNTIME_OUTPUT_DIRECTORY "${SJ_RUNTIME_OUTPUT_DIRECTORY}"
)

################################################################################
# Find Vulkan
################################################################################
find_package(Vulkan REQUIRED)
add_compile_definitions(SJ_VULKAN_SUPPORT)

################################################################################
# Link dependencies
################################################################################
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        glfw
        Vulkan::Vulkan
    PUBLIC
        ScrewjankStd
        ScrewjankSharedCore
        ScrewjankDataDefs
        imgui
        glaze::glaze
)

################################################################################
# Configure include directories
################################################################################
target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${ENGINE_DEPENDENCIES_DIR}/imgui/
    )

if(WIN32)
# the GLFW vcpkg doesn't correctly bring in include directories when linked against? I hate windows.
target_include_directories(
    ${PROJECT_NAME} PRIVATE $ENV{VCPKG_ROOT}/installed/x64-windows/include
)
endif()