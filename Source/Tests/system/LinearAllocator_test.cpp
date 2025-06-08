// STD Headers

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include <ScrewjankStd/PlatformDetection.hpp>
#include <memory_resource>

import sj.std.memory.resources.linear_allocator;

using namespace sj;

namespace system_tests
{
    class LinearAllocatorDummy
    {
    public:
        LinearAllocatorDummy(int num, double num2) : m_num(num), m_double(num2)
        {
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
        std::pmr::memory_resource* mem_resource = std::pmr::get_default_resource();
        void* memory = mem_resource->allocate(128);
        linear_allocator test_resource(128, reinterpret_cast<std::byte*>(memory));

        void* dummy_memory =
            test_resource.allocate(sizeof(LinearAllocatorDummy), alignof(LinearAllocatorDummy));
        ASSERT_NE(nullptr, dummy_memory); // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)

        void* aligned_dummy_memory = dummy_memory;
        size_t space = sizeof(LinearAllocatorDummy) + alignof(LinearAllocatorDummy) - 1;
        std::align(alignof(LinearAllocatorDummy),
                   sizeof(LinearAllocatorDummy),
                   aligned_dummy_memory,
                   space);

        ASSERT_EQ(dummy_memory, aligned_dummy_memory);

        test_resource.reset();

        mem_resource->deallocate(memory, 128);
    }

    TEST(LinearAllocatorTests, ValidAllocationTest)
    {
        std::pmr::memory_resource* mem_resource = std::pmr::get_default_resource();
        size_t mem_size = sizeof(SBS) * 10;
        void* memory = mem_resource->allocate(mem_size);

        linear_allocator packingTest(mem_size, reinterpret_cast<std::byte*>(memory));

        [[maybe_unused]] auto mem = packingTest.allocate(sizeof(SBS) * 10);
        packingTest.reset();

        // You should be able to fit ten individual single byte structures into a linear allocator
        // of buffer size 10
        for(int i = 0; i < 10; i++) // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
        {
            mem = packingTest.allocate(sizeof(SBS), alignof(SBS)); // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
        }

        mem_resource->deallocate(memory, mem_size);
    }

    TEST(LinearAllocatorTests, MemoryStompTest)
    {
        // Ensure allocations don't stomp each other's memory
        // Reserve 256 bytes
        std::pmr::memory_resource* mem_resource = std::pmr::get_default_resource();
        size_t mem_size = 256;
        void* memory = mem_resource->allocate(mem_size);

        linear_allocator test_resource(mem_size, reinterpret_cast<std::byte*>(memory));
        std::pmr::polymorphic_allocator<LinearAllocatorDummy> allocator(&test_resource);

        LinearAllocatorDummy* dummy1 = allocator.new_object<LinearAllocatorDummy>(1, 1.0);
        LinearAllocatorDummy* dummy2 = allocator.new_object<LinearAllocatorDummy>(2, 2.0);
        LinearAllocatorDummy* dummy3 = allocator.new_object<LinearAllocatorDummy>(3, 3.0);

        ASSERT_EQ(1, dummy1->m_num); // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
        ASSERT_EQ(1.0, dummy1->m_double);
        ASSERT_EQ(2, dummy2->m_num); // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
        ASSERT_EQ(2.0, dummy2->m_double);
        ASSERT_EQ(3, dummy3->m_num); // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
        ASSERT_EQ(3.0, dummy3->m_double);

        mem_resource->deallocate(memory, mem_size);
    }
} // namespace system_tests
