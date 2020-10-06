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

        auto res = set.Insert("Foo");
        ASSERT_EQ(true, res.second);
        ASSERT_EQ("Foo", *(res.first));

        res = set.Insert("Foo");
        ASSERT_EQ(false, res.second);
        // ASSERT_EQ(set.end(), res.first);

        // Try duplicate insertion
        // set.Insert("Foo");
        // set.Insert("Bar");
        // set.Insert("Biz");

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
