// STD Headers
#include <unordered_set>
#include <string>

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include <ScrewjankEngine/containers/UnorderedSet.hpp>

#include <ScrewjankEngine/containers/Array.hpp>
#include <ScrewjankEngine/containers/Vector.hpp>

using namespace sj;

struct DummyStruct
{
    int value;

    bool operator==(const DummyStruct& other) const
    {
        return value == other.value;
    }

    DummyStruct()
    {
        value = 0;
        CtorCount++;
    }

    DummyStruct(int v)
    {
        value = v;
        CtorCount++;
    }

    ~DummyStruct()
    {
        value = -1234;
        DtorCount++;
    }

    static int CtorCount;
    static int DtorCount;
};
int DummyStruct::CtorCount = 0;
int DummyStruct::DtorCount = 0;

template <>
struct std::hash<DummyStruct>
{
    std::size_t operator()(const DummyStruct& s) const noexcept
    {
        return std::hash<int> {}(s.value);
    }
};

namespace container_tests {

    TEST(UnorderedSetTests, IterationTest)
    {
        dynamic_unordered_set<std::string> set(MemorySystem::GetRootHeapZone());
        set = {"Foo", "Bar", "Biz", "Baz"};

        // Assert that iterator to begin actually points to an element
        ASSERT_TRUE(set.contains(*(set.begin())));

        size_t i = 0;
        for (auto it = set.begin(); it != set.end(); it++, i++) {
            auto element = *it;
            ASSERT_TRUE(set.contains(element));
        }
        ASSERT_EQ(i, set.count());

        i = 0;
        for (auto& element : set) {
            ASSERT_TRUE(set.contains(element));
            i++;
        }
        ASSERT_EQ(i, set.count());
    }

    TEST(UnorderedSetTests, InitializerListTest)
    {
        dynamic_unordered_set<std::string> set(MemorySystem::GetRootHeapZone());
        set = {"Foo", "Bar", "Biz", "Baz"};

        ASSERT_EQ(4, set.count());

        ASSERT_TRUE(set.contains("Foo"));
        ASSERT_TRUE(set.contains("Bar"));
        ASSERT_TRUE(set.contains("Biz"));
        ASSERT_TRUE(set.contains("Baz"));
        ASSERT_FALSE(set.contains(""));
    }

    TEST(UnorderedSetTests, RangeConstructionTest)
    {
        dynamic_vector<std::string> vec(MemorySystem::GetRootHeapZone());
        vec = {"FOO", "BAR", "BIZ", "BAZ", "BUZZ"};

        dynamic_unordered_set<std::string> set(MemorySystem::GetRootHeapZone(),
                                      vec.begin(),
                                      vec.end());

        ASSERT_EQ(vec.size(), set.count());

        for (auto& element : vec)
        {
            ASSERT_TRUE(set.contains(element));
        }

        auto var = *vec.begin();
        vec.erase(vec.begin());

        ASSERT_TRUE(set.contains(var));
    }

    TEST(UnorderedSetTests, CopyConstructorTest)
    {
        dynamic_unordered_set<std::string> set1(MemorySystem::GetRootHeapZone());
        set1 = {"Foo", "Bar", "Biz", "Baz"};

        dynamic_unordered_set<std::string> set2(set1);

        ASSERT_EQ(set1, set2);

        set1.erase("Bar");
        ASSERT_FALSE(set1.contains("Bar"));
        ASSERT_TRUE(set2.contains("Bar"));

        ASSERT_TRUE(set2.erase("Bar"));

        // Ensure elements are deeply copied
        auto foo1 = set1.find("Foo");
        auto foo2 = set2.find("Foo");
        ASSERT_NE(foo1, foo2);
    }

    TEST(UnorderedSetTests, MoveConstructorTest)
    {
        dynamic_unordered_set<std::string> set1(MemorySystem::GetRootHeapZone(),
                                       {"Foo", "Bar", "Biz", "Baz"});

        dynamic_unordered_set<std::string> set2(std::move(set1));
        ASSERT_FALSE(set1.contains("Foo"));
        ASSERT_EQ(set1.count(), 0);

        ASSERT_EQ(4, set2.count());
        ASSERT_TRUE(set2.contains("Foo"));
        ASSERT_TRUE(set2.contains("Bar"));
        ASSERT_TRUE(set2.contains("Biz"));
        ASSERT_TRUE(set2.contains("Baz"));
    }

    TEST(UnorderedSetTests, CopyAssignmentTest)
    {
        dynamic_unordered_set<std::string> set1(MemorySystem::GetRootHeapZone(),
                                       {"Foo", "Bar", "Biz", "Baz"});

        auto set2 = set1;
        ASSERT_EQ(set1.count(), set2.count());
        ASSERT_EQ(set1.capacity(), set2.capacity());

        auto foo1 = set1.find("Foo");
        auto foo2 = set2.find("Foo");
        ASSERT_NE(foo1, foo2);
    }

    TEST(UnorderedSetTests, MoveAssignmentTest)
    {
        dynamic_unordered_set<std::string> set1(MemorySystem::GetRootHeapZone(),
                                       {"Foo", "Bar", "Biz", "Baz"});

        auto set2 = std::move(set1);
        ASSERT_EQ(4, set2.count());

        auto foo1 = set1.find("Foo");
        ASSERT_EQ(set1.end(), foo1);

        ASSERT_TRUE(set2.contains("Foo"));

        set1.insert("Foo");
        ASSERT_EQ(1, set1.count());
    }

    TEST(UnorderedSetTests, InsertTest)
    {
        dynamic_unordered_set<std::string> set(MemorySystem::GetRootHeapZone());

        auto res = set.insert("Foo");
        ASSERT_EQ(true, res.second);
        ASSERT_EQ("Foo", *(res.first));
        ASSERT_EQ(1, set.count());
        ASSERT_EQ(2, set.capacity());

        // Try duplicate insertion
        res = set.insert("Foo");
        ASSERT_EQ(false, res.second);
        ASSERT_EQ(1, set.count());
        ASSERT_EQ(2, set.capacity());

        res = set.insert("Bar");
        ASSERT_EQ(true, res.second);
        ASSERT_EQ("Bar", *(res.first));
        ASSERT_EQ(2, set.count());
        ASSERT_EQ(4, set.capacity());

        res = set.insert("Biz");
        ASSERT_EQ(true, res.second);
        ASSERT_EQ("Biz", *(res.first));
        ASSERT_EQ(3, set.count());
        ASSERT_EQ(4, set.capacity());

        res = set.insert("Buzz");
        ASSERT_EQ(true, res.second);
        ASSERT_EQ("Buzz", *(res.first));
        ASSERT_EQ(4, set.count());
        ASSERT_EQ(8, set.capacity());

        res = set.insert("Fuzz");
        ASSERT_EQ(true, res.second);
        ASSERT_EQ("Fuzz", *(res.first));
        ASSERT_EQ(5, set.count());
        ASSERT_EQ(8, set.capacity());
    }

    TEST(UnorderedSetTests, DeleteTest)
    {
        dynamic_unordered_set<std::string> set(MemorySystem::GetRootHeapZone());

        set.insert("Foo");
        set.insert("Bar");
        set.insert("Biz");
        set.insert("Baz");
        set.insert("Foo2");
        set.insert("Bar2");
        set.insert("Biz2");
        set.insert("Baz2");
        set.insert("Foo3");
        set.insert("Bar3");
        set.insert("Biz3");
        set.insert("Baz3");

        auto init_capacity = set.capacity();
        auto count = set.count();

        // Assert that the element can be erased
        bool res = set.erase("Foo");
        ASSERT_TRUE(res);
        ASSERT_EQ(--count, set.count());
        ASSERT_EQ(init_capacity, set.capacity());

        // Assert it was actually removed the first time
        res = set.erase("Foo");
        ASSERT_FALSE(res);
        ASSERT_EQ(count, set.count());
        ASSERT_EQ(init_capacity, set.capacity());

        res = set.erase("Biz2");
        ASSERT_TRUE(res);
        ASSERT_EQ(--count, set.count());
        ASSERT_EQ(init_capacity, set.capacity());

        set.erase("Foo3");
        ASSERT_TRUE(res);
        ASSERT_EQ(--count, set.count());
        ASSERT_EQ(init_capacity, set.capacity());

        auto insert_res = set.insert("Biz2");
        ASSERT_EQ(true, insert_res.second);

        // Degenerate Case: After Range Insertion?
        array<const char*, 1> arr = {"VK_KHR_swapchain"};
        dynamic_unordered_set<const char*> set2( MemorySystem::GetRootHeapZone(), arr.begin(), arr.end() );

        ASSERT_TRUE(set2.contains(arr[0]));
        ASSERT_TRUE(set2.erase(arr[0]));
        ASSERT_EQ(0, set2.count());
    }

    TEST(UnorderedSetTests, FindTest)
    {
        dynamic_unordered_set<std::string> set(MemorySystem::GetRootHeapZone());

        set.insert("Foo");
        set.insert("Bar");
        set.insert("Biz");
        set.insert("Baz");

        auto res = set.find("Foo");
        ASSERT_NE(set.end(), res);
        ASSERT_EQ("Foo", *res);

        set.insert("Foo2");
        set.insert("Bar2");
        set.insert("Biz2");
        set.insert("Baz2");
        set.insert("Foo3");
        set.insert("Bar3");
        set.insert("Biz3");
        set.insert("Baz3");

        res = set.find("Foo");
        ASSERT_NE(set.end(), res);
        ASSERT_EQ("Foo", *res);

        res = set.find("Biz");
        ASSERT_NE(set.end(), res);
        ASSERT_EQ("Biz", *res);

        res = set.find("Bar2");
        ASSERT_NE(set.end(), res);
        ASSERT_EQ("Bar2", *res);

        res = set.find("Foo3");
        ASSERT_NE(set.end(), res);
        ASSERT_EQ("Foo3", *res);

        set.erase("Biz");
        set.erase("Bar2");
        set.erase("Foo3");

        res = set.find("Biz");
        ASSERT_EQ(set.end(), res);

        res = set.find("Bar2");
        ASSERT_EQ(set.end(), res);

        res = set.find("Foo3");
        ASSERT_EQ(set.end(), res);
    }

    TEST(UnorderedSetTests, CountTest)
    {
        int seed = 324123871324890;
        std::srand(seed);

        dynamic_unordered_set<std::string> testset(MemorySystem::GetRootHeapZone());
        std::unordered_set<std::string> stableset;
        std::vector<std::string> goodvec;


        char buff[256];
        for(int i = 0; i < 25; i++)
        {
            if(i == 8)
            {
                __nop();
            }

            bool insert = std::rand() % 2 || testset.count() == 0;
            if(insert)
            {
                snprintf(buff, sizeof(buff), "%d", rand());
                stableset.insert(buff);
                testset.insert(buff);
                goodvec.push_back(buff);
            }
            else
            {
                int toRemove = rand() % goodvec.size();
                testset.erase(goodvec[toRemove]);
                stableset.erase(goodvec[toRemove]);
                goodvec.erase(goodvec.begin() + toRemove);
            }
            
            std::cout << "i " << i << std::endl;
            
            ASSERT_EQ(stableset.size(), testset.count());
        }

    }

    TEST(UnorderedSetTests, ContainsTest)
    {
        dynamic_unordered_set<std::string> set(MemorySystem::GetRootHeapZone());

        set.insert("Foo");
        set.insert("Bar");

        int seed = 324123871324890;
        std::srand(seed);

        char buff[256];
        for(int i = 0; i < 50; i++)
        {
            snprintf(buff, sizeof(buff), "%d", rand());
            set.insert(buff);
        }

        ASSERT_TRUE(set.contains("Foo"));
        ASSERT_TRUE(set.contains("Bar"));

        set.erase("Foo");
        ASSERT_FALSE(set.contains("Foo"));
        ASSERT_TRUE(set.contains("Bar"));

        set.erase("Bar");
        ASSERT_FALSE(set.contains("Foo"));
        ASSERT_FALSE(set.contains("Bar"));
    }

    TEST(UnorderedSetTests, ClearTest)
    {
        dynamic_unordered_set<std::string> set(MemorySystem::GetRootHeapZone());

        set.insert("Foo");
        set.insert("Bar");
        set.insert("Biz");
        set.insert("Baz");

        auto capacity = set.capacity();

        set.clear();
        ASSERT_EQ(0, set.count());
        ASSERT_EQ(capacity, set.capacity());
    }

    TEST(UnorderedSetTests, DestructorTest)
    {
        HeapZoneScope scope(MemorySystem::GetRootHeapZone());

        ASSERT_EQ(0, DummyStruct::CtorCount);
        ASSERT_EQ(0, DummyStruct::DtorCount);

        sj::dynamic_unordered_set<DummyStruct> set(MemorySystem::GetRootHeapZone());
        set.emplace( 1 );
        ASSERT_EQ(1, DummyStruct::CtorCount);
        ASSERT_EQ(0, DummyStruct::DtorCount); // 1 for the death of the temporary
    }
} // namespace container_tests
