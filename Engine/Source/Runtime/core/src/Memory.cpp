// STD Headers
#include <iostream>
#include <cassert>

// Library Headers

// Screwjank Headers

void* operator new(size_t num_bytes) noexcept(false)
{
	#ifndef NDEBUG
		std::cout << "Heap allocating " << num_bytes << " bytes\n";
	#endif // !NDEBUG

	void* memory = malloc(num_bytes);
	return memory;
}

void operator delete(void* memory) throw()
{
	#ifndef NDEBUG
		std::cout << "Freeing memory at address 0x" << std::hex << memory << "\n";
	#endif // !NDEBUG
	
	free(memory);
}