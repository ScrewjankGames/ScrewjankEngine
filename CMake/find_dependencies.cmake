## Vulkan
find_package(Vulkan REQUIRED)

# Require Vulkan version ≥ 1.3.256 (earliest version when the Vulkan module was available)
if( ${Vulkan_VERSION} VERSION_LESS "1.3.256" )
  message( FATAL_ERROR "Minimum required Vulkan version for C++ modules is 1.3.256. "
           "Found ${Vulkan_VERSION}."
  )
endif()

# set up Vulkan C++ module as a library
add_library( VulkanHppModule )
target_sources( VulkanHppModule PUBLIC
  FILE_SET CXX_MODULES
  BASE_DIRS ${Vulkan_INCLUDE_DIR}
  FILES ${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm
)
target_compile_definitions( VulkanHppModule PUBLIC
  VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)
target_link_libraries( VulkanHppModule PUBLIC Vulkan::Vulkan )

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

## VK Bootstrap
find_package(vk-bootstrap CONFIG REQUIRED)