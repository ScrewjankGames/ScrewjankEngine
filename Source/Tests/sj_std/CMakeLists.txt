file(GLOB_RECURSE TEST_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE TEST_SOURCE CONFIGURE_DEPENDS "*.cpp")

add_executable(SjStdTestSuite ${TEST_HEADERS} ${TEST_SOURCE})

set_target_properties(SjStdTestSuite
	PROPERTIES 
		FOLDER Engine/Testing
		ARCHIVE_OUTPUT_DIRECTORY "${SJ_ARCHIVE_OUTPUT_DIRECTORY}/Tests/"
        LIBRARY_OUTPUT_DIRECTORY "${SJ_LIBRARY_OUTPUT_DIRECTORY}/Tests/"
        RUNTIME_OUTPUT_DIRECTORY "${SJ_RUNTIME_OUTPUT_DIRECTORY}/Tests/"
)

################################################################################
# Link GTest and Engine Modules
################################################################################
target_link_libraries(SjStdTestSuite
	PRIVATE 
		GTest::gtest 
		ScrewjankStd
)