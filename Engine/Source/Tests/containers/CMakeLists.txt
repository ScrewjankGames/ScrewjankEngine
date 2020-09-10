file(GLOB_RECURSE CONTAINER_TEST_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE CONTAINER_TEST_SOURCE CONFIGURE_DEPENDS "*.cpp")

add_executable(ContainerTestSuite ${CONTAINER_TEST_HEADERS} ${CONTAINER_TEST_SOURCE})

set_target_properties(ContainerTestSuite 
	PROPERTIES 
		FOLDER Engine/Testing
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
)

################################################################################
# Link GTest and Engine Modules
################################################################################
target_link_libraries(ContainerTestSuite
	PRIVATE 
		gtest_main 
		Engine
)