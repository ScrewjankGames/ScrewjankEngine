add_executable(AllocatorBenchmarks "Allocator_benchmarks.cpp")

set_target_properties(AllocatorBenchmarks 
	PROPERTIES 
		FOLDER Engine/Benchmarks
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
)

################################################################################
# Link GTest and Engine Modules
################################################################################
target_link_libraries(AllocatorBenchmarks
	PRIVATE 
		benchmark::benchmark_main
		ScrewjankEngine
)