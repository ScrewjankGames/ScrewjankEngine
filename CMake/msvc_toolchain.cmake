set(CMAKE_C_COMPILER cl.exe CACHE PATH "C Compiler")
set(CMAKE_CXX_COMPILER cl.exe CACHE PATH "C++ Compiler")
add_compile_options(-W4 -WX)

include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
