file(GLOB_RECURSE TEST_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE TEST_SOURCE CONFIGURE_DEPENDS "*.cpp")

add_executable(DataDefinitionsTestSuite ${TEST_HEADERS} ${TEST_SOURCE})

################################################################################
# Link GTest and Engine Modules
################################################################################
target_link_libraries(DataDefinitionsTestSuite
	PRIVATE 
		GTest::gtest 
		ScrewjankDataDefs
)