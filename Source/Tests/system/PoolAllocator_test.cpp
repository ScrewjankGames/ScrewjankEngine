// STD Headers

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include <ScrewjankStd/PlatformDetection.hpp>

import sj.engine.system.memory;

using namespace sj;

namespace system_tests {

    struct PoolAllocatorDummy
    {
        char Label;
        double Value;
    };

    TEST(PoolAllocatorTests, AllocationTest)
    {
        std::pmr::memory_resource* mem_resource = MemorySystem::GetUnmanagedMemoryResource();
        void* memory = mem_resource->allocate(sizeof(PoolAllocatorDummy) * 4, alignof(PoolAllocatorDummy));

        PoolAllocator<sizeof(PoolAllocatorDummy)> allocator(4 * sizeof(PoolAllocatorDummy), memory);

        auto mem_loc1 = allocator.allocate(sizeof(PoolAllocatorDummy), alignof(PoolAllocatorDummy));
        ASSERT_NE(nullptr, mem_loc1); // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
        auto dummy1 = new (mem_loc1) PoolAllocatorDummy {.Label='a', .Value=3.14};

        auto mem_loc2 = allocator.allocate(sizeof(PoolAllocatorDummy), alignof(PoolAllocatorDummy));
        ASSERT_NE(nullptr, mem_loc2); // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
        auto dummy2 = new (mem_loc2) PoolAllocatorDummy {.Label='b', .Value=3.14};

        auto mem_loc3 = allocator.allocate(sizeof(PoolAllocatorDummy), alignof(PoolAllocatorDummy));
        ASSERT_NE(nullptr, mem_loc3); // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
        auto dummy3 = new (mem_loc3) PoolAllocatorDummy {.Label='c', .Value=3.14};

        auto mem_loc4 = allocator.allocate(sizeof(PoolAllocatorDummy), alignof(PoolAllocatorDummy));
        ASSERT_NE(nullptr, mem_loc4); // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
        auto dummy4 = new (mem_loc4) PoolAllocatorDummy {.Label='d', .Value=3.14};

        // Should not be able to allocate a 5th block
        ASSERT_EQ(nullptr, allocator.allocate(sizeof(PoolAllocatorDummy), alignof(PoolAllocatorDummy)));

        // Make sure no memory was corrupted
        ASSERT_EQ('a', dummy1->Label);
        ASSERT_EQ('b', dummy2->Label);
        ASSERT_EQ('c', dummy3->Label);
        ASSERT_EQ('d', dummy4->Label); // WHAT THE FUCK IS WRONG WITH THIS
        ASSERT_EQ(3.14, dummy1->Value);
        ASSERT_EQ(dummy1->Value, dummy2->Value);
        ASSERT_EQ(dummy2->Value, dummy3->Value);
        ASSERT_EQ(dummy3->Value, dummy4->Value);

        allocator.deallocate(dummy3, sizeof(PoolAllocatorDummy));

        mem_loc3 = allocator.allocate(sizeof(double));
        ASSERT_NE(nullptr, mem_loc3);

        auto d = new (mem_loc3) double {1234.56789};

        // Make sure no memory was stomped
        ASSERT_EQ('b', dummy2->Label);
        ASSERT_EQ(3.14, dummy2->Value);
        ASSERT_EQ(3.14, dummy4->Value);
        ASSERT_EQ('d', dummy4->Label);
        allocator.deallocate(d, sizeof(double));

        mem_loc3 = allocator.allocate(sizeof(PoolAllocatorDummy), alignof(PoolAllocatorDummy));
        ASSERT_NE(nullptr, mem_loc3);
        dummy3 = new (mem_loc3) PoolAllocatorDummy {.Label='c', .Value=3.14};

        ASSERT_EQ('b', dummy2->Label);
        ASSERT_EQ('c', dummy3->Label);
        ASSERT_EQ('d', dummy4->Label);
        ASSERT_EQ(3.14, dummy2->Value);
        ASSERT_EQ(dummy2->Value, dummy3->Value);
        ASSERT_EQ(dummy3->Value, dummy4->Value);

        allocator.deallocate(mem_loc1, sizeof(PoolAllocatorDummy));
        allocator.deallocate(mem_loc2, sizeof(PoolAllocatorDummy));
        allocator.deallocate(mem_loc3, sizeof(PoolAllocatorDummy));
        allocator.deallocate(mem_loc4, sizeof(PoolAllocatorDummy));

        mem_resource->deallocate(memory, sizeof(PoolAllocatorDummy) * 4);
    }
} // namespace system_tests
