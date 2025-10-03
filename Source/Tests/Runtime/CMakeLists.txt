file(GLOB_RECURSE Runtime_Test_Headers CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE Runtime_Test_Source CONFIGURE_DEPENDS "*.cpp")

add_executable(RuntimeTestSuite ${Runtime_Test_Headers} ${Runtime_Test_Source})

set_target_properties(RuntimeTestSuite
	PROPERTIES 
		FOLDER Engine/Testing
		ARCHIVE_OUTPUT_DIRECTORY "${SJ_ARCHIVE_OUTPUT_DIRECTORY}/Tests/"
        LIBRARY_OUTPUT_DIRECTORY "${SJ_LIBRARY_OUTPUT_DIRECTORY}/Tests/"
        RUNTIME_OUTPUT_DIRECTORY "${SJ_RUNTIME_OUTPUT_DIRECTORY}/Tests/"
)

################################################################################
# Link GTest and Engine Modules
################################################################################
target_link_libraries(RuntimeTestSuite
	PRIVATE 
		GTest::gtest 
		ScrewjankEngine
)