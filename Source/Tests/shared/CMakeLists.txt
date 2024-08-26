file(GLOB_RECURSE TEST_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE TEST_SOURCE CONFIGURE_DEPENDS "*.cpp")

add_executable(SharedTestSuite ${TEST_HEADERS} ${TEST_SOURCE})

set_target_properties(SharedTestSuite
	PROPERTIES 
		FOLDER Engine/Testing
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
)

################################################################################
# Link GTest and Engine Modules
################################################################################
target_link_libraries(SharedTestSuite
	PRIVATE 
		gtest 
		ScrewjankSharedCore
)