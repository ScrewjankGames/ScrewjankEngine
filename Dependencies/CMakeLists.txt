################################################################################
# Include and configure dependencies
################################################################################

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY SYSTEM 1)

set(${CMAKE_INSTALL_LIBDIR} ${CMAKE_INSTALL_PREFIX}/ScrewjankEngine-${ScrewjankEngine_VERSION}/lib)

## Vulkan
find_package(Vulkan REQUIRED)

## GLFW
option(GLFW_INSTALL "Build the GLFW example programs" OFF)
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
if(UNIX)
    option(GLFW_BUILD_X11 OFF) 
    option(GLFW_BUILD_WAYLAND ON)
endif(UNIX)

find_package(glfw3 CONFIG REQUIRED)

## Spdlog
option(SPDLOG_USE_STD_FORMAT ON)
add_subdirectory(spdlog)
set_target_properties(spdlog PROPERTIES CXX_CLANG_TIDY "")

## GoogleTest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

option(BUILD_GMOCK OFF)

add_subdirectory(googletest)
set_target_properties(gtest 
    PROPERTIES 
        FOLDER Engine/Dependencies/GoogleTest
        ARCHIVE_OUTPUT_DIRECTORY "${SJ_ARCHIVE_OUTPUT_DIRECTORY}/Dependencies/"
        LIBRARY_OUTPUT_DIRECTORY "${SJ_LIBRARY_OUTPUT_DIRECTORY}/Dependencies/"
        RUNTIME_OUTPUT_DIRECTORY "${SJ_RUNTIME_OUTPUT_DIRECTORY}/Dependencies"
)
set_target_properties(gtest_main     
    PROPERTIES 
        FOLDER Engine/Dependencies/GoogleTest
)

## ImGui
set(IMGUI_PATH "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp) 

set(IMGUI_BACKENDS 
    ${IMGUI_PATH}/backends/imgui_impl_vulkan.cpp
    ${IMGUI_PATH}/backends/imgui_impl_glfw.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUI_BACKENDS})
set_target_properties(imgui PROPERTIES CXX_CLANG_TIDY "")
target_compile_definitions(imgui PUBLIC GLFW_INCLUDE_VULKAN)
target_include_directories(imgui 
    PUBLIC 
    ${IMGUI_PATH}
    ${IMGUI_PATH}/backends
)

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
        FOLDER Engine/Dependencies
        ARCHIVE_OUTPUT_DIRECTORY "${SJ_ARCHIVE_OUTPUT_DIRECTORY}/Dependencies/"
        LIBRARY_OUTPUT_DIRECTORY "${SJ_LIBRARY_OUTPUT_DIRECTORY}/Dependencies/"
        RUNTIME_OUTPUT_DIRECTORY "${SJ_RUNTIME_OUTPUT_DIRECTORY}/Dependencies/"
)

## STB
file(GLOB STB_HEADERS CONFIGURE_DEPENDS "stb/*.h")
add_library(STB INTERFACE ${STB_HEADERS} )

set_target_properties(STB
    PROPERTIES 
        FOLDER Engine/Dependencies
        ARCHIVE_OUTPUT_DIRECTORY "${SJ_ARCHIVE_OUTPUT_DIRECTORY}/Dependencies/"
        LIBRARY_OUTPUT_DIRECTORY "${SJ_LIBRARY_OUTPUT_DIRECTORY}/Dependencies/"
        RUNTIME_OUTPUT_DIRECTORY "${SJ_RUNTIME_OUTPUT_DIRECTORY}/Dependencies/"
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
        ARCHIVE_OUTPUT_DIRECTORY "${SJ_ARCHIVE_OUTPUT_DIRECTORY}/Dependencies/"
        LIBRARY_OUTPUT_DIRECTORY "${SJ_LIBRARY_OUTPUT_DIRECTORY}/Dependencies/"
        RUNTIME_OUTPUT_DIRECTORY "${SJ_RUNTIME_OUTPUT_DIRECTORY}/Dependencies/"
)

## Glaze
add_subdirectory(glaze)