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

set_target_properties(${PROJECT_NAME} 
    PROPERTIES 
        FOLDER Engine/
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        ARCHIVE_OUTPUT_DIRECTORY "${Binaries_Directory}/lib/${PROJECT_NAME}"
        LIBRARY_OUTPUT_DIRECTORY "${Binaries_Directory}/lib/${PROJECT_NAME}"
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
        ScrewjankMath
        ${Vulkan_LIBRARIES}
    PUBLIC
        imgui
)

################################################################################
# Configure include directories
################################################################################
target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${ENGINE_DEPENDENCIES_DIR}/spdlog/include
    PRIVATE
        ${ENGINE_DEPENDENCIES_DIR}/glfw/include
        ${ENGINE_DEPENDENCIES_DIR}/glm/
        ${ENGINE_DEPENDENCIES_DIR}/imgui/
        ${Vulkan_INCLUDE_DIRS}
    )

