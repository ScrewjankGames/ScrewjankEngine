file(GLOB_RECURSE Runtime_Test_Headers CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE Runtime_Test_Source CONFIGURE_DEPENDS "*.cpp")

add_executable(RuntimeTestSuite ${Runtime_Test_Headers} ${Runtime_Test_Source})

################################################################################
# Link GTest and Engine Modules
################################################################################
target_link_libraries(RuntimeTestSuite
	PRIVATE 
		GTest::gtest 
		ScrewjankEngine
)