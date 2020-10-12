add_executable(AllocatorBenchmarks "allocator_benchmarks.cpp")

set_target_properties(AllocatorBenchmarks 
	PROPERTIES 
		FOLDER Engine/Benchmarks
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
)

################################################################################
# Link GTest and Engine Modules
################################################################################
target_link_libraries(AllocatorBenchmarks
	PRIVATE 
		benchmark::benchmark_main
		Engine
)