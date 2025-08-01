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
find_package(spdlog REQUIRED)

## STB
find_package(Stb REQUIRED)

## TinyObjLoader
find_package(tinyobjloader CONFIG REQUIRED)

## IMGUI
find_package(imgui CONFIG REQUIRED)

## Glaze
find_package(glaze REQUIRED)

## Gtest
find_package(GTest CONFIG REQUIRED)

## VMA
find_package(VulkanMemoryAllocator CONFIG REQUIRED)