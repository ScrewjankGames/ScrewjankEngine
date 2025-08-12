
set(CMAKE_C_COMPILER /usr/bin/gcc CACHE PATH "C Compiler") # Or the path to your clang
set(CMAKE_CXX_COMPILER /usr/bin/g++ CACHE PATH "C++ Compiler") # Or the path to your clang++

include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake) # Use VCPKG on windows to discover dependencies