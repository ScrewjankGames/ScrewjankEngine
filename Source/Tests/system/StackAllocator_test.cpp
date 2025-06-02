// Screwjank Headers
#include <ScrewjankStd/PlatformDetection.hpp>

// Library Headers
#include "gtest/gtest.h"

// STD Headers
#include <memory_resource>

// Modules
import sj.std.memory.resources.stack_allocator;

using namespace sj;

namespace system_tests
{

    struct StackAllocatorDummy
    {
        int a;
        double b;
        char c;
    };

    TEST(StackAllocatorTests, PushPopTest)
    {
        std::pmr::memory_resource* mem_resource = std::pmr::get_default_resource();
        void* memory = mem_resource->allocate(128);
        stack_allocator allocator(128, memory);

        // Assign memory and construct test data in the addresses
        auto mem1 = allocator.allocate(sizeof(StackAllocatorDummy), alignof(StackAllocatorDummy));
        auto sd1 = new(mem1) StackAllocatorDummy {.a=1, .b=1, .c=1};

        auto mem2 = allocator.allocate(sizeof(StackAllocatorDummy), alignof(StackAllocatorDummy));
        auto sd2 = new(mem2) StackAllocatorDummy {.a=2, .b=2, .c=2};

        auto mem3 = allocator.allocate(sizeof(StackAllocatorDummy), alignof(StackAllocatorDummy));
        auto sd3 = new(mem3) StackAllocatorDummy {.a=3, .b=3, .c=3};

        // Make sure no data got stomped on or corrupted
        EXPECT_EQ(1, sd1->a);
        EXPECT_EQ(1, sd1->b);
        EXPECT_EQ(1, sd1->c);

        EXPECT_EQ(2, sd2->a);
        EXPECT_EQ(2, sd2->b);
        EXPECT_EQ(2, sd2->c);

        EXPECT_EQ(3, sd3->a);
        EXPECT_EQ(3, sd3->b);
        EXPECT_EQ(3, sd3->c);

        // free mem3
        allocator.pop_alloc();

        auto mem4 = allocator.allocate(sizeof(StackAllocatorDummy), alignof(StackAllocatorDummy));
        auto sd4 = new(mem4) StackAllocatorDummy {.a=4, .b=4, .c=4};

        // mem4 should re-use mem3's space
        EXPECT_EQ(mem3, mem4);

        // Make sure 2nd allocation isn't disturbed
        EXPECT_EQ(2, sd2->a);
        EXPECT_EQ(2, sd2->b);
        EXPECT_EQ(2, sd2->c); // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)

        // Make sure 4th allocation worked
        EXPECT_EQ(4, sd4->a);
        EXPECT_EQ(4, sd4->b);
        EXPECT_EQ(4, sd4->c);

        // There are now three allocations on the stack. Pop them all
        allocator.pop_alloc();
        allocator.pop_alloc();
        allocator.pop_alloc();

        auto mem5 = allocator.allocate(sizeof(int));
        auto sd5 = new(mem5) int(5);
        // mem5 should re-use mem1's freed space exactly
        EXPECT_EQ(mem1, mem5);

        // Make sure 5th allocation worked
        EXPECT_EQ(5, *sd5);

        auto mem6 = allocator.allocate(sizeof(double));
        auto sd6 = new(mem6) double(6.0f);

        auto mem7 = allocator.allocate(sizeof(StackAllocatorDummy), alignof(StackAllocatorDummy));
        auto sd7 = new (mem7) StackAllocatorDummy {.a = 7, .b = 7, .c = 7};

        EXPECT_EQ(6.0, *sd6); // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)

        EXPECT_EQ(7, sd7->a);
        EXPECT_EQ(7, sd7->b);
        EXPECT_EQ(7, sd7->c); // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)

        allocator.pop_alloc();
        allocator.pop_alloc();
        allocator.pop_alloc();

        mem_resource->deallocate(memory, 128);
    }

} // namespace system_tests
