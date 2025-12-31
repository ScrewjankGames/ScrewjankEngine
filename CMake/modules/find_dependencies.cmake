include(FetchContent)

## Vulkan
find_package(Vulkan REQUIRED)

if(Vulkan_FOUND)
    message(STATUS "Vulkan SDK found: Version ${Vulkan_VERSION}")
    add_compile_definitions(SJ_VULKAN_SUPPORT)
else()
    message(FATAL_ERROR "Vulkan SDK could not be found. Make sure the SDK is installed and VULKAN_SDK environment variable is set correctly.")
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

## VK Bootstrap
FetchContent_Declare(
  vk-bootstrap
  SYSTEM
  GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap.git
  GIT_TAG v${Vulkan_VERSION}
)
FetchContent_MakeAvailable(vk-bootstrap)

## Vulkan Memory Allocator
FetchContent_Declare(
  vulkan-memory-allocator
  SYSTEM
  GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG v3.3.0
)
FetchContent_MakeAvailable(vulkan-memory-allocator)

## SDL3
FetchContent_Declare(
  SDL3
  SYSTEM
  GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
  GIT_TAG release-3.2.28
)
if(UNIX)
    set(SDL_X11 OFF)
endif()
FetchContent_MakeAvailable(SDL3)

## Spdlog
FetchContent_Declare(
  spdlog
  SYSTEM
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.16.0
)
option(SPDLOG_USE_STD_FORMAT ON)
FetchContent_MakeAvailable(spdlog)

## STB
FetchContent_Declare(
  stb
  SYSTEM
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_BRANCH master
)
FetchContent_MakeAvailable(stb)
set(stb_src
    ${stb_SOURCE_DIR}/stb_image.h
)
add_library(stb INTERFACE ${stb_src})
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})

## TinyObjLoader
FetchContent_Declare(
        tinyobjloader
        SYSTEM
        GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
        GIT_TAG v1.0.7
)
FetchContent_MakeAvailable(tinyobjloader)

## IMGUI
FetchContent_Declare(
        imgui
        SYSTEM
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.92.2-docking
)
FetchContent_MakeAvailable(imgui)
set(imgui_src
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.h
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)
add_library(imgui STATIC ${imgui_src})
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PUBLIC SDL3::SDL3 Vulkan::Vulkan)

## Glaze
FetchContent_Declare(
        glaze
        SYSTEM
        GIT_REPOSITORY https://github.com/stephenberry/glaze.git
        GIT_TAG v6.4.0
)
FetchContent_MakeAvailable(glaze)

## Gtest
FetchContent_Declare(
  googletest
  SYSTEM
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)