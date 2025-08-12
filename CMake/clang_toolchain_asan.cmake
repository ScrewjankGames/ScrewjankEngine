set(CMAKE_C_COMPILER clang CACHE PATH "C Compiler") # Or the path to your clang
set(CMAKE_CXX_COMPILER clang++ CACHE PATH "C++ Compiler") # Or the path to your clang++

add_compile_options(-W -Wall -Wextra -Werror)
add_compile_options(-fsanitize=address,undefined)
add_link_options(-fsanitize=address,undefined)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libstdc++")
