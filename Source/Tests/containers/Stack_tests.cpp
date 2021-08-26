// STD Headers
#include <vector>

// Library Headers
#include <gtest/gtest.h>

// Void Engine Headers
#include <containers/Stack.hpp>

using namespace sj;

namespace container_tests
{
    TEST(StackTests, StaticStackPushPopTest)
    {
        // Power of two sized stack
        StaticStack<int, 4> stack;
        int two = 2;
        
        ASSERT_EQ(0, stack.Count());

        stack.Push(1);
        ASSERT_EQ(1, stack.Top());
        ASSERT_EQ(1, stack.Count());

        stack.Push(two);
        ASSERT_EQ(2, stack.Top());
        ASSERT_EQ(2, stack.Count());

        stack.Push(3);
        ASSERT_EQ(3, stack.Top());
        ASSERT_EQ(3, stack.Count());

        stack.Push(4);
        ASSERT_EQ(4, stack.Top());
        ASSERT_EQ(4, stack.Count());

        stack.Pop();
        ASSERT_EQ(3, stack.Top());
        ASSERT_EQ(3, stack.Count());

        stack.Push(4);
        ASSERT_EQ(4, stack.Top());
        ASSERT_EQ(4, stack.Count());

        stack.Pop();
        ASSERT_EQ(3, stack.Top());
        ASSERT_EQ(3, stack.Count());

        stack.Pop();
        ASSERT_EQ(2, stack.Top());
        ASSERT_EQ(2, stack.Count());

        stack.Pop();
        ASSERT_EQ(1, stack.Top());
        ASSERT_EQ(1, stack.Count());

        stack.Pop();
        ASSERT_EQ(0, stack.Count());

    }
}
