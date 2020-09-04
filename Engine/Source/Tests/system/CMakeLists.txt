file(GLOB_RECURSE SYSTEM_HEADERS CONFIGURE_DEPENDS "*.hpp")
file(GLOB_RECURSE SYSTEM_SOURCE CONFIGURE_DEPENDS "*.cpp")

add_executable(SystemTestSuite ${SYSTEM_HEADERS} ${SYSTEM_SOURCE})

set_target_properties(SystemTestSuite 
	PROPERTIES 
		FOLDER Engine/Testing
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
)

################################################################################
# Link GTest and Engine Modules
################################################################################
target_link_libraries(SystemTestSuite
	PRIVATE 
		gtest_main 
		Engine
)