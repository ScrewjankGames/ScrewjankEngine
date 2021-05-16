################################################################################
# Include and configure dependencies
################################################################################

set(${CMAKE_INSTALL_LIBDIR} ${CMAKE_INSTALL_PREFIX}/ScrewjankEngine-${ScrewjankEngine_VERSION}/lib)

## GLFW
option(GLFW_INSTALL "Build the GLFW example programs" OFF)

option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)

add_subdirectory(GLFW)

set_target_properties(glfw 
    PROPERTIES 
        FOLDER Engine/Dependencies/GLFW
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Dependencies/GLFW"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Dependencies/GLFW"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Dependencies/GLFW"
)
#set_target_properties(uninstall PROPERTIES FOLDER Engine/Dependencies/GLFW)

## GoogleTest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

option(BUILD_GMOCK OFF)

add_subdirectory(googletest)
set_target_properties(gtest 
    PROPERTIES 
        FOLDER Engine/Dependencies/GoogleTest
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Dependencies/GoogleTest"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Dependencies/GoogleTest"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Dependencies/GoogleTest"
)
set_target_properties(gtest_main     
    PROPERTIES 
        FOLDER Engine/Dependencies/GoogleTest
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Dependencies/GoogleTest"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Dependencies/GoogleTest"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Dependencies/GoogleTest"
)

## GoogleBenchmark
option(BENCHMARK_ENABLE_TESTING "Enable testing of the benchmark library." OFF)
option(BENCHMARK_ENABLE_INSTALL "Enable installation of benchmark. (Projects embedding benchmark may want to turn this OFF.)" OFF)
option(BENCHMARK_ENABLE_GTEST_TESTS "Enable building the unit tests which depend on gtest" OFF)
add_subdirectory(benchmark)
set_target_properties(benchmark 
    PROPERTIES 
        FOLDER Engine/Dependencies/GoogleBenchmark
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Dependencies/GoogleBenchmark"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Dependencies/GoogleBenchmark"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Dependencies/GoogleBenchmark"
)
set_target_properties(benchmark_main     
    PROPERTIES 
        FOLDER Engine/Dependencies/GoogleBenchmark
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Dependencies/GoogleBenchmark"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/Dependencies/GoogleBenchmark"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Dependencies/GoogleBenchmark"
)