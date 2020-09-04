file(GLOB_RECURSE MATH_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE MATH_SOURCE CONFIGURE_DEPENDS "*.cpp")

add_executable(MathTestSuite ${MATH_HEADERS} ${MATH_SOURCE})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${TEST_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${TEST_SOURCE})

set_target_properties(MathTestSuite 
	PROPERTIES 
		FOLDER Engine/Testing
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
)

################################################################################
# Link GTest and Engine Modules
################################################################################
target_link_libraries(MathTestSuite
	PRIVATE 
		gtest_main 
		Engine
)