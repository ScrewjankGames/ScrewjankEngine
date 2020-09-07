// STD Headers

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include "platform/PlatformDetection.hpp"
#include "system/Memory.hpp"
#include "system/allocators/StackAllocator.hpp"

using namespace Screwjank;

namespace system_tests {

    struct StackAllocatorDummy
    {
        int a;
        double b;
        char c;
    };

    TEST(StackAllocatorTests, PushPopTest)
    {
        StackAllocator allocator(128, MemorySystem::GetDefaultUnmanagedAllocator());
        auto mem1 = allocator.AllocateType<StackAllocatorDummy>();

        // Assign memory and construct test data in the addresses
        auto sd1 = new (mem1) StackAllocatorDummy {1, 1, 1};

        auto mem2 = allocator.Allocate(sizeof(StackAllocatorDummy), alignof(StackAllocatorDummy));
        auto sd2 = new (mem2) StackAllocatorDummy {2, 2, 2};

        auto mem3 = allocator.PushType<StackAllocatorDummy>();
        auto sd3 = new (mem3) StackAllocatorDummy {3, 3, 3};

        // Make sure no data got stomped on or corrupted
        ASSERT_EQ(1, sd1->a);
        ASSERT_EQ(1, sd1->b);
        ASSERT_EQ(1, sd1->c);

        ASSERT_EQ(2, sd2->a);
        ASSERT_EQ(2, sd2->b);
        ASSERT_EQ(2, sd2->c);

        ASSERT_EQ(3, sd3->a);
        ASSERT_EQ(3, sd3->b);
        ASSERT_EQ(3, sd3->c);

        // free mem3
        allocator.Pop();

        auto mem4 = allocator.Push(sizeof(StackAllocatorDummy), alignof(StackAllocatorDummy));
        auto sd4 = new (mem4) StackAllocatorDummy {4, 4, 4};

        // mem4 should re-use mem3's space
        ASSERT_EQ(mem3, mem4);

        // Make sure 2nd allocation isn't disturbed
        ASSERT_EQ(2, sd2->a);
        ASSERT_EQ(2, sd2->b);
        ASSERT_EQ(2, sd2->c);

        // Make sure 4th allocation worked
        ASSERT_EQ(4, sd4->a);
        ASSERT_EQ(4, sd4->b);
        ASSERT_EQ(4, sd4->c);

        // There are now three allocations on the stack. Pop them all
        allocator.Pop();
        allocator.Pop();
        allocator.Pop();

        auto mem5 = allocator.AllocateType<StackAllocatorDummy>();
        auto sd5 = new (mem5) StackAllocatorDummy {5, 5, 5};
        // mem5 should re-use mem1's freed space exactly
        ASSERT_EQ(mem1, mem5);
        // Make sure 4th allocation worked
        ASSERT_EQ(5, sd5->a);
        ASSERT_EQ(5, sd5->b);
        ASSERT_EQ(5, sd5->c);

        allocator.Pop();
    }

    TEST(StackAllocatorTests, MemoryLeakDetectionTest)
    {
        StackAllocator* allocator =
            new StackAllocator(128, MemorySystem::GetDefaultUnmanagedAllocator());

        allocator->PushType<StackAllocatorDummy>();

#ifdef SJ_DEBUG
        ASSERT_DEATH(delete allocator, ".*");
#endif // SJ_DEBUG
    }

} // namespace system_tests
