// STD Headers
#include <unordered_set>
#include <string>

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include "containers/UnorderedSet.hpp"

using namespace sj;

namespace container_tests {

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
