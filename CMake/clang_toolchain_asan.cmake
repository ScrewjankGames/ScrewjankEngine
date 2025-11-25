set(CMAKE_C_COMPILER clang CACHE PATH "C Compiler") # Or the path to your clang
set(CMAKE_CXX_COMPILER clang++ CACHE PATH "C++ Compiler") # Or the path to your clang++

add_compile_options(-fsanitize=address,undefined)
add_link_options(-fsanitize=address,undefined)

include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake) # Use VCPKG on windows to discover dependencies

