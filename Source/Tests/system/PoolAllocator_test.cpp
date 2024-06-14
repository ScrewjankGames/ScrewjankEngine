// STD Headers

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include <ScrewjankShared/utils/PlatformDetection.hpp>
#include <ScrewjankEngine/system/memory/Memory.hpp>
#include <ScrewjankEngine/system/memory/allocators/PoolAllocator.hpp>

using namespace sj;

namespace system_tests {

    struct PoolAllocatorDummy
    {
        char Label;
        double Value;
    };

    TEST(PoolAllocatorTests, AllocationTest)
    {
        IMemSpace* mem_space = MemorySystem::GetCurrentMemSpace();
        void* memory = mem_space->Allocate(sizeof(PoolAllocatorDummy) * 4, alignof(PoolAllocatorDummy));

        PoolAllocator<sizeof(PoolAllocatorDummy)> allocator(4 * sizeof(PoolAllocatorDummy), memory);

        auto mem_loc1 = allocator.AllocateType<PoolAllocatorDummy>();
        ASSERT_NE(nullptr, mem_loc1);
        auto dummy1 = new (mem_loc1) PoolAllocatorDummy {'a', 3.14};

        auto mem_loc2 = allocator.AllocateType<PoolAllocatorDummy>();
        ASSERT_NE(nullptr, mem_loc2);
        auto dummy2 = new (mem_loc2) PoolAllocatorDummy {'b', 3.14};

        auto mem_loc3 = allocator.AllocateType<PoolAllocatorDummy>();
        ASSERT_NE(nullptr, mem_loc3);
        auto dummy3 = new (mem_loc3) PoolAllocatorDummy {'c', 3.14};

        auto mem_loc4 = allocator.AllocateType<PoolAllocatorDummy>();
        ASSERT_NE(nullptr, mem_loc4);
        auto dummy4 = new (mem_loc4) PoolAllocatorDummy {'d', 3.14};

        // Should not be able to allocate a 5th block
        ASSERT_EQ(nullptr, allocator.AllocateType<PoolAllocatorDummy>());

        // Make sure no memory was corrupted
        ASSERT_EQ('a', dummy1->Label);
        ASSERT_EQ('b', dummy2->Label);
        ASSERT_EQ('c', dummy3->Label);
        ASSERT_EQ('d', dummy4->Label); // WHAT THE FUCK IS WRONG WITH THIS
        ASSERT_EQ(3.14, dummy1->Value);
        ASSERT_EQ(dummy1->Value, dummy2->Value);
        ASSERT_EQ(dummy2->Value, dummy3->Value);
        ASSERT_EQ(dummy3->Value, dummy4->Value);

        allocator.Delete(dummy3);

        mem_loc3 = allocator.AllocateType<double>();
        ASSERT_NE(nullptr, mem_loc3);

        auto d = new (mem_loc3) double {1234.56789};

        // Make sure no memory was stomped
        ASSERT_EQ('b', dummy2->Label);
        ASSERT_EQ(3.14, dummy2->Value);
        ASSERT_EQ(3.14, dummy4->Value);
        ASSERT_EQ('d', dummy4->Label);
        allocator.Delete(d);

        mem_loc3 = allocator.AllocateType<PoolAllocatorDummy>();
        ASSERT_NE(nullptr, mem_loc3);
        dummy3 = new (mem_loc3) PoolAllocatorDummy {'c', 3.14};

        ASSERT_EQ('b', dummy2->Label);
        ASSERT_EQ('c', dummy3->Label);
        ASSERT_EQ('d', dummy4->Label);
        ASSERT_EQ(3.14, dummy2->Value);
        ASSERT_EQ(dummy2->Value, dummy3->Value);
        ASSERT_EQ(dummy3->Value, dummy4->Value);

        allocator.Free(mem_loc1);
        allocator.Free(mem_loc2);
        allocator.Free(mem_loc3);
        allocator.Free(mem_loc4);

        mem_space->Free(memory);
    }

    TEST(PoolAllocatorTests, ObjectPoolTest)
    {
        IMemSpace* mem_space = MemorySystem::GetCurrentMemSpace();
        void* memory =
            mem_space->Allocate(sizeof(PoolAllocatorDummy) * 4, alignof(PoolAllocatorDummy));

        ObjectPoolAllocator<PoolAllocatorDummy> allocator(4 * sizeof(PoolAllocatorDummy), memory);

        auto mem_loc1 = allocator.AllocateType<PoolAllocatorDummy>();
        ASSERT_NE(nullptr, mem_loc1);
        auto dummy1 = new (mem_loc1) PoolAllocatorDummy {'a', 3.14};

        auto mem_loc2 = allocator.AllocateType<PoolAllocatorDummy>();
        ASSERT_NE(nullptr, mem_loc2);
        auto dummy2 = new (mem_loc2) PoolAllocatorDummy {'b', 3.14};

        auto mem_loc3 = allocator.AllocateType<PoolAllocatorDummy>();
        ASSERT_NE(nullptr, mem_loc3);
        auto dummy3 = new (mem_loc3) PoolAllocatorDummy {'c', 3.14};

        auto mem_loc4 = allocator.AllocateType<PoolAllocatorDummy>();
        ASSERT_NE(nullptr, mem_loc4);
        auto dummy4 = new (mem_loc4) PoolAllocatorDummy {'d', 3.14};

        // Should not be able to allocate a 5th block
        ASSERT_EQ(nullptr, allocator.AllocateType<PoolAllocatorDummy>());

        // Make sure no memory was corrupted
        ASSERT_EQ('a', dummy1->Label);
        ASSERT_EQ('b', dummy2->Label);
        ASSERT_EQ('c', dummy3->Label);
        ASSERT_EQ('d', dummy4->Label);
        ASSERT_EQ(3.14, dummy1->Value);
        ASSERT_EQ(dummy1->Value, dummy2->Value);
        ASSERT_EQ(dummy2->Value, dummy3->Value);
        ASSERT_EQ(dummy3->Value, dummy4->Value);

        allocator.Delete(dummy3);

        mem_loc3 = allocator.AllocateType<double>();
        ASSERT_NE(nullptr, mem_loc3);

        auto d = new (mem_loc3) double {1234.56789};

        // Make sure no memory was stomped
        ASSERT_EQ('b', dummy2->Label);
        ASSERT_EQ(3.14, dummy2->Value);
        ASSERT_EQ(3.14, dummy4->Value);
        ASSERT_EQ('d', dummy4->Label);
        allocator.Delete(d);

        mem_loc3 = allocator.AllocateType<PoolAllocatorDummy>();
        ASSERT_NE(nullptr, mem_loc3);
        dummy3 = new (mem_loc3) PoolAllocatorDummy {'c', 3.14};

        ASSERT_EQ('b', dummy2->Label);
        ASSERT_EQ('c', dummy3->Label);
        ASSERT_EQ('d', dummy4->Label);
        ASSERT_EQ(3.14, dummy2->Value);
        ASSERT_EQ(dummy2->Value, dummy3->Value);
        ASSERT_EQ(dummy3->Value, dummy4->Value);

        allocator.Free(mem_loc1);
        allocator.Free(mem_loc2);
        allocator.Free(mem_loc3);
        allocator.Free(mem_loc4);

        mem_space->Free(memory);
    }

} // namespace system_tests
