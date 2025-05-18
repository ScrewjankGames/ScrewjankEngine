// STD Headers

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include <ScrewjankShared/utils/PlatformDetection.hpp>

import sj.engine.system.memory;

using namespace sj;

namespace system_tests {
    class LinearAllocatorDummy
    {
      public:
        LinearAllocatorDummy(int num, double num2)
        {
            m_num = num;
            m_double = num2;
        }

        int m_num;
        double m_double;
    };

    struct SBS // Single Byte Struct
    {
        char byte;
    };

    TEST(LinearAllocatorTests, MemoryAlignmentTest)
    {
        IMemSpace* mem_space = MemorySystem::GetCurrentMemSpace();
        void* memory = mem_space->allocate(128);
        LinearAllocator allocator(128, memory);

        void* dummy_memory = allocator.AllocateType<LinearAllocatorDummy>();
        ASSERT_NE(nullptr, dummy_memory);

        void* aligned_dummy_memory = dummy_memory;
        size_t space = sizeof(LinearAllocatorDummy) + alignof(LinearAllocatorDummy) - 1;
        std::align(alignof(LinearAllocatorDummy),
                   sizeof(LinearAllocatorDummy),
                   aligned_dummy_memory,
                   space);

        ASSERT_EQ(dummy_memory, aligned_dummy_memory);

        allocator.Reset();

        mem_space->deallocate(memory, 128);
    }

    TEST(LinearAllocatorTests, ValidAllocationTest)
    {
        IMemSpace* mem_space = MemorySystem::GetCurrentMemSpace();
        size_t mem_size = sizeof(SBS) * 10;
        void* memory = mem_space->allocate(mem_size);

        LinearAllocator packingTest(mem_size, memory);

        auto mem = packingTest.allocate(sizeof(SBS) * 10);
        packingTest.Reset();

        // You should be able to fit ten individual single byte structures into a linear allocator
        // of buffer size 10
        for (int i = 0; i < 10; i++) {
            mem = packingTest.AllocateType<SBS>();
        }

        mem_space->deallocate(memory, mem_size);
    }

    TEST(LinearAllocatorTests, OutOfMemoryTest)
    {
        IMemSpace* mem_space = MemorySystem::GetCurrentMemSpace();
        size_t mem_size = sizeof(SBS) * 10;
        void* memory = mem_space->allocate(mem_size);

        LinearAllocator allocator(mem_size, memory);
        auto mem = allocator.allocate(sizeof(SBS) * 10);
        ASSERT_EQ(nullptr, allocator.AllocateType<SBS>());

        mem_space->deallocate(memory, mem_size);
    }

    TEST(LinearAllocatorTests, InsufficientMemoryTest)
    {
        IMemSpace* mem_space = MemorySystem::GetCurrentMemSpace();
        size_t mem_size = sizeof(SBS) * 10;
        void* memory = mem_space->allocate(mem_size);
        
        LinearAllocator allocator(mem_size, memory);

        auto mem = allocator.allocate(sizeof(SBS) * 9);
        ASSERT_EQ(nullptr, allocator.allocate(sizeof(SBS) * 2));

        mem_space->deallocate(memory, mem_size);
    }

    TEST(LinearAllocatorTests, MemoryStompTest)
    {
        // Ensure allocations don't stomp each other's memory
        // Reserve 256 bytes
        IMemSpace* mem_space = MemorySystem::GetCurrentMemSpace();
        size_t mem_size = 256;
        void* memory = mem_space->allocate(mem_size);

        LinearAllocator allocator(mem_size, memory);

        LinearAllocatorDummy* dummy1 = (LinearAllocatorDummy*)allocator.AllocateType<LinearAllocatorDummy>();
        new (dummy1) LinearAllocatorDummy( 1, 1.0 );
        LinearAllocatorDummy* dummy2 = (LinearAllocatorDummy*)allocator.AllocateType<LinearAllocatorDummy>();
        new (dummy2) LinearAllocatorDummy( 2, 2.0 );
        LinearAllocatorDummy* dummy3 = (LinearAllocatorDummy*)allocator.AllocateType<LinearAllocatorDummy>();
        new (dummy3) LinearAllocatorDummy( 3, 3.0 );

        ASSERT_EQ(1, dummy1->m_num);
        ASSERT_EQ(1.0, dummy1->m_double);
        ASSERT_EQ(2, dummy2->m_num);
        ASSERT_EQ(2.0, dummy2->m_double);
        ASSERT_EQ(3, dummy3->m_num);
        ASSERT_EQ(3.0, dummy3->m_double);

        mem_space->deallocate(memory, mem_size);
    }
} // namespace system_tests
