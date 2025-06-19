
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_COMPILER /usr/bin/gcc CACHE PATH "C Compiler") # Or the path to your clang
set(CMAKE_CXX_COMPILER /usr/bin/g++ CACHE PATH "C++ Compiler") # Or the path to your clang++

include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake) # Use VCPKG on windows to discover dependencies