
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_COMPILER clang CACHE PATH "C Compiler") # Or the path to your clang
set(CMAKE_CXX_COMPILER clang++ CACHE PATH "C++ Compiler") # Or the path to your clang++
#set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
add_compile_options(-W -Wall -Wextra -Werror)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++ -Weverything -Werror -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-c++98-c++11-compat-pedantic")

IF (WIN32)
    include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake) # Use VCPKG on windows to discover dependencies
ENDIF()
