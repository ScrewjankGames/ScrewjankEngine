// STD Headers
#include <forward_list>

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include "containers/ForwardList.hpp"

using namespace sj;

namespace container_tests {

    struct ListDummy
    {
        ListDummy(int val) : Value(val)
        {
        }

        int Value;
    };

    TEST(ForwardListTests, InitializerListConstructionTest)
    {
        ForwardList<std::string> list(MemorySystem::GetUnmanagedAllocator(),
                                      {"Foo", "Bar", "Biz", "Baz"});

        ASSERT_EQ("Foo", list.Front());
        list.PopFront();

        ASSERT_EQ("Bar", list.Front());
        list.PopFront();

        ASSERT_EQ("Biz", list.Front());
        list.PopFront();

        ASSERT_EQ("Baz", list.Front());
    }

    TEST(ForwardListTests, InitializerListAssignmentTest)
    {
        ForwardList<std::string> list(MemorySystem::GetUnmanagedAllocator());
        list = {"Foo", "Bar", "Biz", "Baz"};

        ASSERT_EQ("Foo", list.Front());
        list.PopFront();

        ASSERT_EQ("Bar", list.Front());
        list.PopFront();

        ASSERT_EQ("Biz", list.Front());
        list.PopFront();

        ASSERT_EQ("Baz", list.Front());
    }

    TEST(ForwardListTests, CopyConstructionTest)
    {
        ForwardList<std::string> list1(MemorySystem::GetUnmanagedAllocator(),
                                       {"Foo", "Bar", "Biz", "Baz"});

        ForwardList<std::string> list2(list1);

        ASSERT_EQ("Foo", list2.Front());
        list2.PopFront();

        ASSERT_EQ("Bar", list2.Front());
        list2.PopFront();

        ASSERT_EQ("Biz", list2.Front());
        list2.PopFront();

        ASSERT_EQ("Baz", list2.Front());

        // Make sure the copy was a deep copy
        ASSERT_EQ("Foo", list1.Front());
        list1.PopFront();

        ASSERT_EQ("Bar", list1.Front());
        list1.PopFront();

        ASSERT_EQ("Biz", list1.Front());
        list1.PopFront();

        ASSERT_EQ("Baz", list1.Front());
    }

    TEST(ForwardListTests, CopyAssignmentTest)
    {
        ForwardList<std::string> list1(MemorySystem::GetUnmanagedAllocator(),
                                       {"Foo", "Bar", "Biz", "Baz"});

        ForwardList<std::string> list2(MemorySystem::GetUnmanagedAllocator());
        list2 = list1;

        ASSERT_EQ("Foo", list2.Front());
        list2.PopFront();

        ASSERT_EQ("Bar", list2.Front());
        list2.PopFront();

        ASSERT_EQ("Biz", list2.Front());
        list2.PopFront();

        ASSERT_EQ("Baz", list2.Front());

        // Make sure the copy was a deep copy
        ASSERT_EQ("Foo", list1.Front());
        list1.PopFront();

        ASSERT_EQ("Bar", list1.Front());
        list1.PopFront();

        ASSERT_EQ("Biz", list1.Front());
        list1.PopFront();

        ASSERT_EQ("Baz", list1.Front());
    }

    TEST(ForwardListTests, MoveConstructionTest)
    {
        ForwardList<std::string> list1(MemorySystem::GetUnmanagedAllocator(),
                                       {"Foo", "Bar", "Biz", "Baz"});

        ForwardList<std::string> list2(std::move(list1));

        ASSERT_EQ("Foo", list2.Front());
        list2.PopFront();

        ASSERT_EQ("Bar", list2.Front());
        list2.PopFront();

        ASSERT_EQ("Biz", list2.Front());
        list2.PopFront();

        ASSERT_EQ("Baz", list2.Front());
    }

    TEST(ForwardListTests, MoveAssignmentTest)
    {
        ForwardList<std::string> list =
            ForwardList<std::string>(MemorySystem::GetUnmanagedAllocator(),
                                     {"Foo", "Bar", "Biz", "Baz"});

        ASSERT_EQ("Foo", list.Front());
        list.PopFront();

        ASSERT_EQ("Bar", list.Front());
        list.PopFront();

        ASSERT_EQ("Biz", list.Front());
        list.PopFront();

        ASSERT_EQ("Baz", list.Front());
    }

    TEST(ForwardListTests, PushPopFrontTest)
    {
        ForwardList<ListDummy> list(MemorySystem::GetUnmanagedAllocator());

        list.PushFront(ListDummy {1});
        ASSERT_EQ(1, list.Front().Value);

        list.PushFront(ListDummy {2});
        ASSERT_EQ(2, list.Front().Value);

        list.PushFront(ListDummy {3});
        ASSERT_EQ(3, list.Front().Value);

        list.PopFront();
        ASSERT_EQ(2, list.Front().Value);

        list.PopFront();
        ASSERT_EQ(1, list.Front().Value);

        list.PopFront();

#ifdef SJ_DEBUG
        ASSERT_DEATH(list.PopFront(), ".*");
#endif // SJ_DEBUG
    }

    TEST(ForwardListTests, EmplaceFrontTest)
    {
        ForwardList<ListDummy> list(MemorySystem::GetUnmanagedAllocator());
        list.EmplaceFront(ListDummy {1});
        ASSERT_EQ(1, list.Front().Value);

        list.EmplaceFront(2);
        ASSERT_EQ(2, list.Front().Value);
    }

    TEST(ForwardListTests, InsertAfterTest)
    {
        ForwardList<ListDummy> list(MemorySystem::GetUnmanagedAllocator());
        list.EmplaceFront(1);

        auto it = list.begin();
        auto next = list.InsertAfter(it, ListDummy(2));
        it++;
        ASSERT_EQ(next, it);
        ASSERT_EQ(2, it->Value);
    }

    TEST(ForwardListTests, EmplaceAfterTest)
    {
        ForwardList<ListDummy> list(MemorySystem::GetUnmanagedAllocator());
        list.EmplaceFront(1);

        auto it = list.begin();
        auto next = list.EmplaceAfter(it, ListDummy(2));
        it++;
        ASSERT_EQ(next, it);
        ASSERT_EQ(2, it->Value);
    }

    TEST(ForwardListTests, EraseAfterTest)
    {
        ForwardList<ListDummy> list(MemorySystem::GetUnmanagedAllocator());
        list.PushFront(ListDummy {4});
        list.PushFront(ListDummy {3});
        list.PushFront(ListDummy {2});
        list.PushFront(ListDummy {1});
        list.PushFront(ListDummy {0});

        auto it = list.begin();
        auto next_node = list.EraseAfter(it);
        it++;
        ASSERT_EQ(it, next_node);
        ASSERT_EQ(2, next_node->Value);
        ASSERT_EQ(2, it->Value);

        int i = 0;
        for (auto& element : list) {
            i++;
        }

        ASSERT_EQ(4, i);
    }

    TEST(ForwardListTests, DestructionLeakTest)
    {
        auto list = new ForwardList<ListDummy>(MemorySystem::GetUnmanagedAllocator());
        list->PushFront(ListDummy {1});
        list->PushFront(ListDummy {1});
        list->PushFront(ListDummy {1});
        list->PushFront(ListDummy {1});
        list->PushFront(ListDummy {1});

        // Destroy list
        list->~ForwardList();

        // Memory allocator will assert if there was a memory leak
    }

    TEST(ForwardListTests, IterationTest)
    {
        ForwardList<ListDummy> list(MemorySystem::GetUnmanagedAllocator());

        list.PushFront(ListDummy {4});
        list.PushFront(ListDummy {3});
        list.PushFront(ListDummy {2});
        list.PushFront(ListDummy {1});
        list.PushFront(ListDummy {0});

        int i = 0;
        for (auto it = list.begin(); it != list.end(); ++it, i++) {
            if (i > 4) {
                // Overran list
                ASSERT_FALSE(true);
            }

            ASSERT_EQ(i, it->Value);
        }

        ASSERT_EQ(5, i);
        list.Clear();
        list.PushFront(ListDummy {2});
        list.PushFront(ListDummy {2});
        list.PushFront(ListDummy {2});
        list.PushFront(ListDummy {2});

        i = 0;
        for (const auto& element : list) {
            ASSERT_EQ(2, element.Value);
            i++;
        }

        ASSERT_EQ(4, i);
    }
} // namespace container_tests
