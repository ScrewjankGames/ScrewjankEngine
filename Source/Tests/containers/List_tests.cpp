// STD Headers

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include <ScrewjankEngine/containers/List.hpp>

using namespace sj;

namespace container_tests {

    TEST(ListTests, InitializerListConstructionTest)
    {
        List<std::string> list = {"Foo", "Bar", "Biz", "Baz"};

        ASSERT_EQ("Foo", list.Front());
        list.PopFront();

        ASSERT_EQ("Bar", list.Front());
        list.PopFront();

        ASSERT_EQ("Biz", list.Front());
        list.PopFront();

        ASSERT_EQ("Baz", list.Front());
    }

    TEST(ListTests, InitializerListAssignmentTest)
    {
        List<std::string> list;
        list = {"Foo", "Bar", "Biz", "Baz"};

        ASSERT_EQ("Foo", list.Front());
        list.PopFront();

        ASSERT_EQ("Bar", list.Front());
        list.PopFront();

        ASSERT_EQ("Biz", list.Front());
        list.PopFront();

        ASSERT_EQ("Baz", list.Front());
    }

    TEST(ListTests, CopyConstructionTest)
    {
        List<std::string> list1 = {"Foo", "Bar", "Biz", "Baz"};

        List<std::string> list2(list1);

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

    TEST(ListTests, CopyAssignmentTest)
    {
        List<std::string> list1 = {"Foo", "Bar", "Biz", "Baz"};

        List<std::string> list2;
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

    TEST(ListTests, MoveConstructionTest)
    {
        List<std::string> list1({"Foo", "Bar", "Biz", "Baz"});

        List<std::string> list2(std::move(list1));

        ASSERT_EQ("Foo", list2.Front());
        list2.PopFront();

        ASSERT_EQ("Bar", list2.Front());
        list2.PopFront();

        ASSERT_EQ("Biz", list2.Front());
        list2.PopFront();

        ASSERT_EQ("Baz", list2.Front());
    }

    TEST(ListTests, MoveAssignmentTest)
    {
        List<std::string> list = List<std::string>({"Foo", "Bar", "Biz", "Baz"});

        ASSERT_EQ("Foo", list.Front());
        list.PopFront();

        ASSERT_EQ("Bar", list.Front());
        list.PopFront();

        ASSERT_EQ("Biz", list.Front());
        list.PopFront();

        ASSERT_EQ("Baz", list.Front());
    }

    TEST(ListTests, PushPopFrontTest)
    {
        List<int> list;

        list.PushFront(1);
        ASSERT_EQ(1, list.Front());

        list.PushFront(2);
        ASSERT_EQ(2, list.Front());

        list.PushFront(3);
        ASSERT_EQ(3, list.Front());

        list.PopFront();
        ASSERT_EQ(2, list.Front());

        list.PopFront();
        ASSERT_EQ(1, list.Front());

        list.PopFront();
    }

    TEST(ListTests, EmplaceFrontTest)
    {
        List<std::string> list;
        list.EmplaceFront("Foo");
        ASSERT_EQ("Foo", list.Front());

        list.EmplaceFront(std::string("Bar"));
        ASSERT_EQ("Bar", list.Front());
    }

    TEST(ListTests, InsertAfterTest)
    {
        List<int> list;
        list.EmplaceFront(1);

        auto it = list.begin();
        auto next = list.InsertAfter(it, 2);
        it++;
        ASSERT_EQ(next, it);
        ASSERT_EQ(2, *it);
    }

    TEST(ListTests, EmplaceAfterTest)
    {
        List<std::string> list;
        list.EmplaceFront("Foo");

        auto it = list.begin();
        auto next = list.EmplaceAfter(it, "Bar");
        it++;
        ASSERT_EQ(next, it);
        ASSERT_EQ("Bar", *it);
    }

    TEST(ListTests, EraseAfterTest)
    {
        List<std::string> list;
        list.PushFront("Four");
        list.PushFront("Three");
        list.PushFront("Two");
        list.PushFront("One");
        list.PushFront("Zero");

        auto it = list.begin();
        auto next_node = list.EraseAfter(it);
        it++;
        ASSERT_EQ(it, next_node);
        ASSERT_EQ("Two", *next_node);
        ASSERT_EQ("Two", *it);

        int i = 0;
        for (auto& element : list) {
            i++;
        }

        ASSERT_EQ(4, i);
    }

    TEST(ListTests, DestructionLeakTest)
    {
        auto list = new List<std::string>;
        list->PushFront("Foo");
        list->PushFront("Bar");
        list->PushFront("Biz");
        list->PushFront("Baz");
        list->PushFront("Buzz");

        // Destroy list
        list->~List();

        delete list;

        // Memory allocator will assert if there was a memory leak
    }

    TEST(ListTests, IterationTest)
    {
        List<int> list;

        list.PushFront(4);
        list.PushFront(3);
        list.PushFront(2);
        list.PushFront(1);
        list.PushFront(0);

        int i = 0;
        for (auto it = list.begin(); it != list.end(); ++it, i++) {
            if (i > 4) {
                // Overran list
                ASSERT_FALSE(true);
            }

            ASSERT_EQ(i, *it);
        }

        ASSERT_EQ(5, i);
        list.Clear();
        list.PushFront(2);
        list.PushFront(2);
        list.PushFront(2);
        list.PushFront(2);

        i = 0;
        for (const int& element : list) {
            ASSERT_EQ(2, element);
            i++;
        }

        ASSERT_EQ(4, i);
    }

    TEST(ListTests, IteratorConversionTest)
    {
        List<int> list;
        list.PushFront(0);
        list.PushFront(1);

        List<int>::Iterator nonConstIt = list.begin();
        List<int>::ConstIterator constIt(nonConstIt);

        nonConstIt++;
        constIt = nonConstIt;
    }
} // namespace container_tests
