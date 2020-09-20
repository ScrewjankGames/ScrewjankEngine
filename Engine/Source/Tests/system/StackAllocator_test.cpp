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

        // Assign memory and construct test data in the addresses
        auto mem1 = allocator.AllocateType<StackAllocatorDummy>();
        auto sd1 = new (mem1) StackAllocatorDummy {1, 1, 1};

        auto mem2 = allocator.Allocate(sizeof(StackAllocatorDummy), alignof(StackAllocatorDummy));
        auto sd2 = new (mem2) StackAllocatorDummy {2, 2, 2};

        auto mem3 = allocator.PushType<StackAllocatorDummy>();
        auto sd3 = new (mem3) StackAllocatorDummy {3, 3, 3};

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
        allocator.Pop();

        auto mem4 = allocator.Push(sizeof(StackAllocatorDummy), alignof(StackAllocatorDummy));
        auto sd4 = new (mem4) StackAllocatorDummy {4, 4, 4};

        // mem4 should re-use mem3's space
        EXPECT_EQ(mem3, mem4);

        // Make sure 2nd allocation isn't disturbed
        EXPECT_EQ(2, sd2->a);
        EXPECT_EQ(2, sd2->b);
        EXPECT_EQ(2, sd2->c);

        // Make sure 4th allocation worked
        EXPECT_EQ(4, sd4->a);
        EXPECT_EQ(4, sd4->b);
        EXPECT_EQ(4, sd4->c);

        // There are now three allocations on the stack. Pop them all
        allocator.Pop();
        allocator.Pop();
        allocator.Pop();

        auto mem5 = allocator.PushType<int>();
        auto sd5 = new (mem5) int(5);
        // mem5 should re-use mem1's freed space exactly
        EXPECT_EQ(mem1, mem5);

        // Make sure 5th allocation worked
        EXPECT_EQ(5, *sd5);

        auto mem6 = allocator.PushType<double>();
        auto sd6 = new (mem6) double(6.0f);

        auto mem7 = allocator.PushType<StackAllocatorDummy>();
        auto sd7 = new StackAllocatorDummy {7, 7, 7};

        EXPECT_EQ(6.0, *sd6);

        EXPECT_EQ(7, sd7->a);
        EXPECT_EQ(7, sd7->b);
        EXPECT_EQ(7, sd7->c);

        allocator.Pop();
        allocator.Pop();
        allocator.Pop();
    }

    TEST(StackAllocatorTests, InvalidPopTest)
    {
        StackAllocator allocator(128, MemorySystem::GetDefaultUnmanagedAllocator());

#ifdef SJ_DEBUG
        ASSERT_DEATH(allocator.Pop(), ".*");
#endif

        StackAllocator allocator2(128, MemorySystem::GetDefaultUnmanagedAllocator());
        auto mem1 = allocator2.PushType<char>();
        auto mem2 = allocator2.PushType<double>();

#ifdef SJ_DEBUG
        ASSERT_DEATH(allocator2.Free(mem1), ".*");
#endif
        allocator2.Pop();
        allocator2.Pop();
    }

    TEST(StackAllocatorTests, MemoryLeakDetectionTest)
    {
        StackAllocator* allocator =
            new StackAllocator(128, MemorySystem::GetDefaultUnmanagedAllocator());

        auto mem = allocator->PushType<StackAllocatorDummy>();

#ifdef SJ_DEBUG
        ASSERT_DEATH(delete allocator, ".*");
#endif // SJ_DEBUG
    }

} // namespace system_tests
