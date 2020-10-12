################################################################################
# Include and configure dependencies
################################################################################

## GLFW
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)

add_subdirectory(GLFW)

set_target_properties(glfw PROPERTIES FOLDER Engine/Dependencies/GLFW)
set_target_properties(uninstall PROPERTIES FOLDER Engine/Dependencies/GLFW)

## GoogleTest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

option(BUILD_GMOCK OFF)

add_subdirectory(googletest)
set_target_properties(gtest PROPERTIES FOLDER Engine/Dependencies/googletest)
set_target_properties(gtest_main PROPERTIES FOLDER Engine/Dependencies/googletest)

## GoogleBenchmark
option(BENCHMARK_ENABLE_TESTING "Enable testing of the benchmark library." OFF)
option(BENCHMARK_ENABLE_INSTALL "Enable installation of benchmark. (Projects embedding benchmark may want to turn this OFF.)" OFF)
option(BENCHMARK_ENABLE_GTEST_TESTS "Enable building the unit tests which depend on gtest" OFF)
add_subdirectory(benchmark)
set_target_properties(benchmark PROPERTIES FOLDER Engine/Dependencies/googlebenchmark)
set_target_properties(benchmark_main PROPERTIES FOLDER Engine/Dependencies/googlebenchmark)