################################################################################
# Include and configure dependencies
################################################################################

set(${CMAKE_INSTALL_LIBDIR} ${CMAKE_INSTALL_PREFIX}/ScrewjankEngine-${ScrewjankEngine_VERSION}/lib)

## GLFW
option(GLFW_INSTALL "Build the GLFW example programs" OFF)

option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)

if(UNIX)
option(GLFW_BUILD_X11 OFF) 
option(GLFW_BUILD_WAYLAND ON)
endif(UNIX)

find_package(glfw3 REQUIRED)

# set_target_properties(glfw3
#     PROPERTIES 
#         FOLDER Engine/Dependencies/GLFW3
#         ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Dependencies/GLFW"
#         LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Dependencies/GLFW"
#         RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Dependencies/GLFW"
# )

# set_target_properties(update_mappings 
#     PROPERTIES 
#         FOLDER Engine/Dependencies/GLFW3
# )
#set_target_properties(uninstall PROPERTIES FOLDER Engine/Dependencies/GLFW)

## GoogleTest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

option(BUILD_GMOCK OFF)

add_subdirectory(googletest)
set_target_properties(gtest 
    PROPERTIES 
        FOLDER Engine/Dependencies/GoogleTest
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Dependencies/GoogleTest"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Dependencies/GoogleTest"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Dependencies/GoogleTest"
)
set_target_properties(gtest_main     
    PROPERTIES 
        FOLDER Engine/Dependencies/GoogleTest
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Dependencies/GoogleTest"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Dependencies/GoogleTest"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Dependencies/GoogleTest"
)

## GoogleBenchmark
option(BENCHMARK_ENABLE_TESTING "Enable testing of the benchmark library." OFF)
option(BENCHMARK_ENABLE_INSTALL "Enable installation of benchmark. (Projects embedding benchmark may want to turn this OFF.)" OFF)
option(BENCHMARK_ENABLE_GTEST_TESTS "Enable building the unit tests which depend on gtest" OFF)
add_subdirectory(benchmark)
set_target_properties(benchmark 
    PROPERTIES 
        FOLDER Engine/Dependencies/GoogleBenchmark
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Dependencies/GoogleBenchmark"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Dependencies/GoogleBenchmark"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Dependencies/GoogleBenchmark"
)
set_target_properties(benchmark_main     
    PROPERTIES 
        FOLDER Engine/Dependencies/GoogleBenchmark
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Dependencies/GoogleBenchmark"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Dependencies/GoogleBenchmark"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Dependencies/GoogleBenchmark"
)

## ImGui
set(IMGUI_PATH "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp) 

set(IMGUI_BACKENDS 
    ${IMGUI_PATH}/backends/imgui_impl_vulkan.h
    ${IMGUI_PATH}/backends/imgui_impl_vulkan.cpp
    ${IMGUI_PATH}/backends/imgui_impl_glfw.h
    ${IMGUI_PATH}/backends/imgui_impl_glfw.cpp
)


add_library("imgui" STATIC ${IMGUI_SOURCES}  ${IMGUI_BACKENDS})
target_compile_definitions("imgui" PUBLIC GLFW_INCLUDE_VULKAN)
target_include_directories("imgui" 
    PUBLIC 
    ${IMGUI_PATH}
    ${IMGUI_PATH}/backends
)

find_package(Vulkan REQUIRED)

target_link_libraries(imgui
    PRIVATE 
        glfw
        ${Vulkan_LIBRARIES}
)

target_include_directories(imgui
    PRIVATE
        ${Vulkan_INCLUDE_DIRS}
    )

set_target_properties(imgui 
    PROPERTIES 
        FOLDER Engine/Dependencies)

## STB
file(GLOB STB_HEADERS CONFIGURE_DEPENDS "stb/*.h")
add_library(STB INTERFACE ${STB_HEADERS} )

set_target_properties(STB
    PROPERTIES 
        FOLDER Engine/Dependencies
)

target_include_directories(STB
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/stb
    )

## TinyObjLoader
add_subdirectory(tinyobjloader)

set_target_properties(tinyobjloader
    PROPERTIES 
        FOLDER Engine/Dependencies
)

## TinyObjLoader
add_subdirectory(json)

set_target_properties(nlohmann_json
    PROPERTIES 
        FOLDER Engine/Dependencies
)

