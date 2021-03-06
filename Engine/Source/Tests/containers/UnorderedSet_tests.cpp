// STD Headers
#include <unordered_set>
#include <string>

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include "containers/UnorderedSet.hpp"

using namespace sj;

namespace container_tests {

    TEST(UnorderedSetTests, IterationTest)
    {
        UnorderedSet<std::string> set(MemorySystem::GetUnmanagedAllocator());
        set = {"Foo", "Bar", "Biz", "Baz"};

        // Assert that iterator to begin actually points to an element
        ASSERT_TRUE(set.Contains(*(set.begin())));

        size_t i = 0;
        for (auto it = set.begin(); it != set.end(); it++, i++) {
            auto element = *it;
            ASSERT_TRUE(set.Contains(element));
        }
        ASSERT_EQ(i, set.Count());

        i = 0;
        for (auto& element : set) {
            ASSERT_TRUE(set.Contains(element));
            i++;
        }
        ASSERT_EQ(i, set.Count());
    }

    TEST(UnorderedSetTests, InitializerListTest)
    {
        UnorderedSet<std::string> set(MemorySystem::GetUnmanagedAllocator());
        set = {"Foo", "Bar", "Biz", "Baz"};

        ASSERT_EQ(4, set.Count());

        ASSERT_TRUE(set.Contains("Foo"));
        ASSERT_TRUE(set.Contains("Bar"));
        ASSERT_TRUE(set.Contains("Biz"));
        ASSERT_TRUE(set.Contains("Baz"));
        ASSERT_FALSE(set.Contains(""));
    }

    TEST(UnorderedSetTests, CopyConstructorTest)
    {
        UnorderedSet<std::string> set1(MemorySystem::GetUnmanagedAllocator());
        set1 = {"Foo", "Bar", "Biz", "Baz"};

        UnorderedSet<std::string> set2(set1);

        ASSERT_EQ(set1, set2);

        set1.Erase("Bar");
        ASSERT_FALSE(set1.Contains("Bar"));
        ASSERT_TRUE(set2.Contains("Bar"));

        ASSERT_TRUE(set2.Erase("Bar"));

        // Ensure elements are deeply copied
        auto foo1 = set1.Find("Foo");
        auto foo2 = set2.Find("Foo");
        ASSERT_NE(foo1, foo2);
    }

    TEST(UnorderedSetTests, MoveConstructorTest)
    {
        UnorderedSet<std::string> set1(MemorySystem::GetUnmanagedAllocator(),
                                       {"Foo", "Bar", "Biz", "Baz"});

        UnorderedSet<std::string> set2(std::move(set1));
        ASSERT_FALSE(set1.Contains("Foo"));
        ASSERT_EQ(set1.Count(), 0);

        ASSERT_EQ(4, set2.Count());
        ASSERT_TRUE(set2.Contains("Foo"));
        ASSERT_TRUE(set2.Contains("Bar"));
        ASSERT_TRUE(set2.Contains("Biz"));
        ASSERT_TRUE(set2.Contains("Baz"));
    }

    TEST(UnorderedSetTests, CopyAssignmentTest)
    {
        UnorderedSet<std::string> set1(MemorySystem::GetUnmanagedAllocator(),
                                       {"Foo", "Bar", "Biz", "Baz"});

        auto set2 = set1;
        ASSERT_EQ(set1.Count(), set2.Count());
        ASSERT_EQ(set1.Capacity(), set2.Capacity());

        auto foo1 = set1.Find("Foo");
        auto foo2 = set2.Find("Foo");
        ASSERT_NE(foo1, foo2);
    }

    TEST(UnorderedSetTests, MoveAssignmentTest)
    {
        UnorderedSet<std::string> set1(MemorySystem::GetUnmanagedAllocator(),
                                       {"Foo", "Bar", "Biz", "Baz"});

        auto set2 = std::move(set1);
        ASSERT_EQ(4, set2.Count());

        auto foo1 = set1.Find("Foo");
        ASSERT_EQ(set1.end(), foo1);

        ASSERT_TRUE(set2.Contains("Foo"));

        set1.Insert("Foo");
        ASSERT_EQ(1, set1.Count());
    }

    TEST(UnorderedSetTests, InsertTest)
    {
        UnorderedSet<std::string> set(MemorySystem::GetUnmanagedAllocator());

        auto res = set.Insert("Foo");
        ASSERT_EQ(true, res.second);
        ASSERT_EQ("Foo", *(res.first));
        ASSERT_EQ(1, set.Count());
        ASSERT_EQ(2, set.Capacity());

        // Try duplicate insertion
        res = set.Insert("Foo");
        ASSERT_EQ(false, res.second);
        ASSERT_EQ(1, set.Count());
        ASSERT_EQ(2, set.Capacity());

        res = set.Insert("Bar");
        ASSERT_EQ(true, res.second);
        ASSERT_EQ("Bar", *(res.first));
        ASSERT_EQ(2, set.Count());
        ASSERT_EQ(4, set.Capacity());

        res = set.Insert("Biz");
        ASSERT_EQ(true, res.second);
        ASSERT_EQ("Biz", *(res.first));
        ASSERT_EQ(3, set.Count());
        ASSERT_EQ(4, set.Capacity());

        res = set.Insert("Buzz");
        ASSERT_EQ(true, res.second);
        ASSERT_EQ("Buzz", *(res.first));
        ASSERT_EQ(4, set.Count());
        ASSERT_EQ(8, set.Capacity());

        res = set.Insert("Fuzz");
        ASSERT_EQ(true, res.second);
        ASSERT_EQ("Fuzz", *(res.first));
        ASSERT_EQ(5, set.Count());
        ASSERT_EQ(8, set.Capacity());
    }

    TEST(UnorderedSetTests, DeleteTest)
    {
        UnorderedSet<std::string> set(MemorySystem::GetUnmanagedAllocator());

        set.Insert("Foo");
        set.Insert("Bar");
        set.Insert("Biz");
        set.Insert("Baz");
        set.Insert("Foo2");
        set.Insert("Bar2");
        set.Insert("Biz2");
        set.Insert("Baz2");
        set.Insert("Foo3");
        set.Insert("Bar3");
        set.Insert("Biz3");
        set.Insert("Baz3");

        auto init_capacity = set.Capacity();
        auto count = set.Count();

        // Assert that the element can be erased
        bool res = set.Erase("Foo");
        ASSERT_TRUE(res);
        ASSERT_EQ(--count, set.Count());
        ASSERT_EQ(init_capacity, set.Capacity());

        // Assert it was actually removed the first time
        res = set.Erase("Foo");
        ASSERT_FALSE(res);
        ASSERT_EQ(count, set.Count());
        ASSERT_EQ(init_capacity, set.Capacity());

        res = set.Erase("Biz2");
        ASSERT_TRUE(res);
        ASSERT_EQ(--count, set.Count());
        ASSERT_EQ(init_capacity, set.Capacity());

        set.Erase("Foo3");
        ASSERT_TRUE(res);
        ASSERT_EQ(--count, set.Count());
        ASSERT_EQ(init_capacity, set.Capacity());

        auto insert_res = set.Insert("Biz2");
        ASSERT_EQ(true, insert_res.second);
    }

    TEST(UnorderedSetTests, FindTest)
    {
        UnorderedSet<std::string> set(MemorySystem::GetUnmanagedAllocator());

        set.Insert("Foo");
        set.Insert("Bar");
        set.Insert("Biz");
        set.Insert("Baz");

        auto res = set.Find("Foo");
        ASSERT_NE(set.end(), res);
        ASSERT_EQ("Foo", *res);

        set.Insert("Foo2");
        set.Insert("Bar2");
        set.Insert("Biz2");
        set.Insert("Baz2");
        set.Insert("Foo3");
        set.Insert("Bar3");
        set.Insert("Biz3");
        set.Insert("Baz3");

        res = set.Find("Foo");
        ASSERT_NE(set.end(), res);
        ASSERT_EQ("Foo", *res);

        res = set.Find("Biz");
        ASSERT_NE(set.end(), res);
        ASSERT_EQ("Biz", *res);

        res = set.Find("Bar2");
        ASSERT_NE(set.end(), res);
        ASSERT_EQ("Bar2", *res);

        res = set.Find("Foo3");
        ASSERT_NE(set.end(), res);
        ASSERT_EQ("Foo3", *res);

        set.Erase("Biz");
        set.Erase("Bar2");
        set.Erase("Foo3");

        res = set.Find("Biz");
        ASSERT_EQ(set.end(), res);

        res = set.Find("Bar2");
        ASSERT_EQ(set.end(), res);

        res = set.Find("Foo3");
        ASSERT_EQ(set.end(), res);
    }

    TEST(UnorderedSetTests, ContainsTest)
    {
        UnorderedSet<std::string> set(MemorySystem::GetUnmanagedAllocator());

        set.Insert("Foo");
        set.Insert("Bar");

        ASSERT_TRUE(set.Contains("Foo"));
        ASSERT_TRUE(set.Contains("Bar"));

        set.Erase("Foo");
        ASSERT_FALSE(set.Contains("Foo"));
        ASSERT_TRUE(set.Contains("Bar"));

        set.Erase("Bar");
        ASSERT_FALSE(set.Contains("Foo"));
        ASSERT_FALSE(set.Contains("Bar"));
    }

    TEST(UnorderedSetTests, ClearTest)
    {
        UnorderedSet<std::string> set(MemorySystem::GetUnmanagedAllocator());

        set.Insert("Foo");
        set.Insert("Bar");
        set.Insert("Biz");
        set.Insert("Baz");

        auto capacity = set.Capacity();

        set.Clear();
        ASSERT_EQ(0, set.Count());
        ASSERT_EQ(capacity, set.Capacity());
    }
} // namespace container_tests
