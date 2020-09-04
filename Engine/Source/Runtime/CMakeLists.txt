################################################################################
# Define project: Engine
################################################################################
project(Engine C CXX)
option(ENGINE_BUILD_SAMPLE "Build the Engine's sample project" ON)

file(GLOB_RECURSE RUNTIME_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE RUNTIME_SOURCE CONFIGURE_DEPENDS "*.cpp")

add_library(Engine ${RUNTIME_HEADERS} ${RUNTIME_SOURCE})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${RUNTIME_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${RUNTIME_SOURCE})

set_target_properties(Engine 
    PROPERTIES 
        FOLDER Engine/
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
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
        glfw
        ${Vulkan_LIBRARIES}
)

################################################################################
# Configure include directories
################################################################################
target_include_directories(Engine 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/core/include
        ${CMAKE_CURRENT_SOURCE_DIR}/math/include
        ${ENGINE_DEPENDENCIES_DIR}/spdlog/include
    PRIVATE
        ${ENGINE_DEPENDENCIES_DIR}/glfw/include
        ${ENGINE_DEPENDENCIES_DIR}/glm/
        ${Vulkan_INCLUDE_DIRS}
    )

if(ENGINE_BUILD_SAMPLE)
    add_subdirectory(${SAMPLE_PROJECT_DIR} ${CMAKE_BINARY_DIR}/SampleProject)
endif()
