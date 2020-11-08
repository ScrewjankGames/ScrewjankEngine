add_executable(UnorderedSetBenchmarks "UnorderedSet_benchmarks.cpp")

set_target_properties(UnorderedSetBenchmarks 
	PROPERTIES 
		FOLDER Engine/Benchmarks
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
)

################################################################################
# Link GTest and Engine Modules
################################################################################
target_link_libraries(UnorderedSetBenchmarks
	PRIVATE 
		benchmark::benchmark_main
		Engine
)