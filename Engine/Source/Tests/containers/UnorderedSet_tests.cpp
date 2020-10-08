// STD Headers
#include <unordered_set>
#include <string>

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include "containers/UnorderedSet.hpp"

using namespace sj;

namespace container_tests {

    TEST(UnorderedSetTests, InsertDeleteTest)
    {
        UnorderedSet<std::string> set(MemorySystem::GetUnmanagedAllocator());

        auto res1 = set.Insert("Foo");
        ASSERT_EQ(true, res1.second);
        ASSERT_EQ("Foo", *(res1.first));

        // Try duplicate insertion
        auto res2 = set.Insert("Foo");
        ASSERT_EQ(false, res2.second);
        ASSERT_EQ(res1.first, res2.first);

        set.Insert("Bar");
        set.Insert("Biz");
        set.Insert("Buzz");
        // auto foo = set.Find("Foo");
        // set.Insert({"Baz", "Boz"});
    }

    TEST(UnorderedSetTests, ElementUniquenessTest)
    {
        auto terrible_hasher = [](int x) {
            return 0;
        };

        UnorderedSet<int, decltype(terrible_hasher)> set(MemorySystem::GetUnmanagedAllocator());
        set.Insert(2);
        set.Insert(1);
        set.Erase(2);
        auto res = set.Insert(1);
        ASSERT_EQ(false, res.second);
    }
} // namespace container_tests
