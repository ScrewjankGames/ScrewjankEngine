################################################################################
# Define project: ScrewjankEngine
################################################################################
project(ScrewjankEngine)

file(GLOB_RECURSE RUNTIME_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE RUNTIME_INLINES CONFIGURE_DEPENDS "*.inl")
file(GLOB_RECURSE RUNTIME_SOURCE CONFIGURE_DEPENDS "*.cpp")

add_library(${PROJECT_NAME} ${RUNTIME_HEADERS} ${RUNTIME_INLINES} ${RUNTIME_SOURCE})
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
        ${Vulkan_LIBRARIES}
    PUBLIC
        ScrewjankMath
        ScrewjankSharedCore
        ScrewjankDataDefs
        imgui
)

################################################################################
# Configure include directories
################################################################################
target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${ENGINE_DEPENDENCIES_DIR}/spdlog/include
        ${ENGINE_DEPENDENCIES_DIR}/SG14/
    PRIVATE
        ${ENGINE_DEPENDENCIES_DIR}/glfw/include
        ${ENGINE_DEPENDENCIES_DIR}/glm/
        ${ENGINE_DEPENDENCIES_DIR}/imgui/
        ${Vulkan_INCLUDE_DIRS}
    )

