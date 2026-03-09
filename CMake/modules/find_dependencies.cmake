include(FetchContent)

## SDL3
FetchContent_Declare(
  SDL3
  SYSTEM
  GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
  GIT_TAG release-3.4.0
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
        GIT_TAG v1.92.6-docking
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
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlgpu3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlgpu3.h
)
add_library(imgui STATIC ${imgui_src})
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PUBLIC SDL3::SDL3)

## IMPLOT
FetchContent_Declare(
        implot
        SYSTEM
        GIT_REPOSITORY https://github.com/epezent/implot.git
        GIT_TAG v0.17
)
FetchContent_MakeAvailable(implot)
set(implot_src
    ${implot_SOURCE_DIR}/implot.h
    ${implot_SOURCE_DIR}/implot_internal.h
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_items.cpp
)
add_library(implot STATIC ${implot_src})
target_include_directories(implot PUBLIC ${implot_SOURCE_DIR})
target_link_libraries(implot PUBLIC imgui)

## Glaze
FetchContent_Declare(
        glaze
        SYSTEM
        GIT_REPOSITORY https://github.com/stephenberry/glaze.git
        GIT_TAG v7.1.1
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