#pragma once
// STD Headers
#include <cstddef>

// Library Headers

// Screwjank Headers
#include "system/Allocator.hpp"

namespace Screwjank {

    /** Returns a reference to the statically allocated Global Allocator */
    Allocator* GlobalAllocator();

    /**
     * Function that returns the first aligned memory address given certain constraints
     * @param align_of The alignment of the memory being allocated
     * @param size The size of the memory being allocated
     * @param buffer_start The start of the memory region in which we are aligning the pointer
     * @param buffer_size How large the buffer we're aligning in is
     */
    void* AlignMemory(size_t align_of, size_t size, void* buffer_start, size_t buffer_size);

    class MemorySystem
    {
      public:
        MemorySystem() = default;
        ~MemorySystem() = default;

        void Initialize();

        void LogStatus();

      private:
        friend void* ::operator new(size_t num_bytes);
        friend void ::operator delete(void* memory);

        static char s_GlobalAllocatorBuffer[sizeof(BasicAllocator)];
        static Allocator* s_GlobalAllocator;
    };

} // namespace Screwjank
