file(GLOB_RECURSE TEST_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE TEST_SOURCE CONFIGURE_DEPENDS "*.cpp")

add_executable(SjStdTestSuite ${TEST_HEADERS} ${TEST_SOURCE})

################################################################################
# Link GTest and Engine Modules
################################################################################
target_link_libraries(SjStdTestSuite
	PRIVATE 
		GTest::gtest 
		ScrewjankStd
)