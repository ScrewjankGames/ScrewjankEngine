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
