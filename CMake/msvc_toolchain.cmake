set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_COMPILER cl.exe CACHE PATH "C Compiler")
set(CMAKE_CXX_COMPILER cl.exe CACHE PATH "C++ Compiler")
add_compile_options(-W4 -WX)

include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
