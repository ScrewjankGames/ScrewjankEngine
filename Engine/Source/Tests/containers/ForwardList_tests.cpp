// STD Headers
#include <forward_list>

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include "containers/ForwardList.hpp"

using namespace Screwjank;

namespace container_tests {

    struct ListDummy
    {
        ListDummy(int val) : Value(val)
        {
        }

        int Value;
    };

    TEST(ForwardListsTests, PushPopFrontTest)
    {
        ForwardList<ListDummy> list(MemorySystem::GetDefaultUnmanagedAllocator());

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

    TEST(ForwardListsTests, EmplaceFrontTest)
    {
        ForwardList<ListDummy> list(MemorySystem::GetDefaultUnmanagedAllocator());
        list.EmplaceFront(ListDummy {1});
        ASSERT_EQ(1, list.Front().Value);

        list.EmplaceFront(2);
        ASSERT_EQ(2, list.Front().Value);
    }

    TEST(ForwardListsTests, InsertAfterTest)
    {
        // std::forward_list<ListDummy> list;
        // list.push_front(ListDummy(1));

        // auto it = list.begin();

        // auto dummy = *it;
        // it->Value

        // list.insert_after(it, ListDummy(2));

        ForwardList<ListDummy> list(MemorySystem::GetDefaultUnmanagedAllocator());
        list.EmplaceFront(1);

        auto it = list.begin();
        auto next = list.InsertAfter(it, ListDummy(2));
        it++;
        ASSERT_EQ(next, it);
        ASSERT_EQ(2, it->Value);
    }

    TEST(ForwardListsTests, EmplaceAfterTest)
    {
        ForwardList<ListDummy> list(MemorySystem::GetDefaultUnmanagedAllocator());
        list.EmplaceFront(1);

        auto it = list.begin();
        // auto next = list.EmplaceAfter(it, ListDummy(2));
        it++;
        // ASSERT_EQ(next, it);
        ASSERT_EQ(2, it->Value);
    }

    TEST(ForwardListsTests, EraseAfterTest)
    {
        ForwardList<ListDummy> list(MemorySystem::GetDefaultUnmanagedAllocator());
        list.PushFront(ListDummy {4});
        list.PushFront(ListDummy {3});
        list.PushFront(ListDummy {2});
        list.PushFront(ListDummy {1});
        list.PushFront(ListDummy {0});

        auto it = list.begin();
        // list.EraseAfter(it);
    }

    TEST(ForwardListsTests, DestructionLeakTest)
    {
        auto list = new ForwardList<ListDummy>(MemorySystem::GetDefaultUnmanagedAllocator());
        list->PushFront(ListDummy {1});
        list->PushFront(ListDummy {1});
        list->PushFront(ListDummy {1});
        list->PushFront(ListDummy {1});
        list->PushFront(ListDummy {1});

        // Destroy list
        list->~ForwardList();

        // Memory allocator will assert if there was a memory leak
    }

    TEST(ForwardListsTests, IterationTest)
    {
        ForwardList<ListDummy> list(MemorySystem::GetDefaultUnmanagedAllocator());

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
