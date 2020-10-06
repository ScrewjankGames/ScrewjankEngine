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

        set.Insert("Foo");

        // Try duplicate insertion
        // set.Insert("Foo");
        // set.Insert("Bar");
        // set.Insert("Biz");

        // auto foo = set.Find("Foo");
        // set.Insert({"Baz", "Boz"});
    }
} // namespace container_tests
