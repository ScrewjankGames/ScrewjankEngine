// STD Headers
#include <span>

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include <ScrewjankEngine/containers/Vector.hpp>
#include <ScrewjankShared/utils/Log.hpp>

using namespace sj;

namespace container_tests {

    class VectorTestDummy
    {
      public:
        VectorTestDummy()
        {
            SJ_ENGINE_LOG_INFO("Default constructed");
            m_Data = -1;
        }

        VectorTestDummy(int val)
        {
            SJ_ENGINE_LOG_INFO("parameter constructed");
            m_Data = val;
        }

        VectorTestDummy(const VectorTestDummy& other)
        {
            SJ_ENGINE_LOG_INFO("copy constructed");
            m_Data = other.m_Data;
        }

        VectorTestDummy(const VectorTestDummy&& other) noexcept
        {
            SJ_ENGINE_LOG_INFO("move constructed");
            m_Data = other.m_Data;
        }

        VectorTestDummy& operator=(const VectorTestDummy& other)
        {
            SJ_ENGINE_LOG_INFO("copy assignment operator");
            m_Data = other.m_Data;

            return *this;
        }

        VectorTestDummy& operator=(const VectorTestDummy&& other)
        {
            SJ_ENGINE_LOG_INFO("move assignment operator");
            m_Data = other.m_Data;

            return *this;
        }

        int Value()
        {
            return m_Data;
        }

        bool operator==(const VectorTestDummy& other) const
        {
            return m_Data == other.m_Data;
        }

      private:
        int m_Data;
    };

    TEST(StaticVectorTests, AddRemoveTest)
    {
        static_vector<int, 3> vec;
        ASSERT_TRUE(vec.empty());

        vec.push_back(1);
        vec.push_back(2);
        vec.push_back(3);
        ASSERT_FALSE(vec.empty());

        ASSERT_EQ(1, vec[0]);
        ASSERT_EQ(2, vec[1]);
        ASSERT_EQ(3, vec[2]);

        vec.erase(1);
        ASSERT_EQ(1, vec[0]);
        ASSERT_NE(2, vec[1]);

        vec.erase(0);
        ASSERT_NE(1, vec[0]);
        
        vec.erase(0);
        ASSERT_TRUE(vec.empty());
    }

    TEST(StaticVectorTests, SwappableTests)
    {
        static_vector<int, 3> a;
        static_vector<int, 3> b;

        ;

        static_assert(std::is_swappable<decltype(a)>::value, "Not swappable?");
        static_assert(noexcept(swap(a, b)), "");
        static_assert(std::is_nothrow_swappable<decltype(a)>::value, "Not nothrow swappable?");
    }

    TEST(StaticVectorTests, EmplaceTests)
    {
        static_vector<int, 4> testVec = {0, 2, 3};
        static_vector<int, 4, VectorOptions {.kStableOperations = false}> testVec2 = {0, 2, 3};

        testVec.emplace(&testVec[1], 1);
        ASSERT_EQ(testVec.size(), 4);
        for(int i = 0; i < testVec.size(); i++)
        {
            ASSERT_EQ(i, testVec[i]);
        }

        testVec2.emplace(&testVec2[1], 1);

        ASSERT_EQ(testVec2.size(), 4);
        ASSERT_EQ(testVec2[0], 0);
        ASSERT_EQ(testVec2[1], 1);
        ASSERT_EQ(testVec2[2], 3);
        ASSERT_EQ(testVec2[3], 2);
    }

    TEST(VectorTests, ListInitializationTest)
    {
        dynamic_vector<int> vec1({1, 2, 3, 4, 5});
        ASSERT_EQ(5, vec1.size());
        ASSERT_EQ(5, vec1.capacity());

        for (size_t i = 0; i < vec1.size(); i++) {
            ASSERT_EQ(i + 1, vec1[i]);
        }

        dynamic_vector<std::string> vec2;
        vec2 = {"Foo", "Bar", "Biz", "Baz"};
        ASSERT_EQ(4, vec2.size());
        ASSERT_EQ(4, vec2.capacity());
        ASSERT_EQ("Foo", vec2[0]);
        ASSERT_EQ("Bar", vec2[1]);
        ASSERT_EQ("Biz", vec2[2]);
        ASSERT_EQ("Baz", vec2[3]);
    }

    TEST(VectorTests, ElementInsertionTest)
    {
        dynamic_vector<VectorTestDummy> vec;
        VectorTestDummy dummy;

        // Copy construct dummy into vec
        vec.push_back(dummy);
        ASSERT_EQ(-1, vec[0].Value());

        auto dummy2 = vec.emplace_back(100);

        ASSERT_EQ(dummy2, vec[1]);
        ASSERT_EQ(100, vec[1].Value());
    }

    TEST(VectorTests, ElementAccessTest)
    {
        dynamic_vector<int> vec1(MemorySystem::GetRootMemSpace(), {1, 2, 3});

        ASSERT_EQ(1, vec1[0]);

        ASSERT_EQ(vec1[0], vec1.at(0));
        ASSERT_EQ(vec1[0], vec1.front());
        ASSERT_EQ(vec1[2], vec1.back());
        
#ifndef SJ_GOLD
        ASSERT_DEATH(vec1.at(-1), ".*");
#endif // SJ_GOLD
    }

    TEST(VectorTests, IterationTest)
    {
        dynamic_vector<int> vec;
        int i = 0;

        for (auto& element : vec) {
            i++;
        }
        ASSERT_EQ(0, i);

        vec.push_back(1);
        vec.emplace_back(2);
        vec.push_back(3);
        vec.emplace_back(4);
        vec.push_back(5);
        vec.emplace_back(6);

        // Iterate through all the odd elements
        for (auto it = vec.begin(); it < vec.end(); it = it + 2) {
            ASSERT_NE(0, *it % 2);
        }

        // Iterate through all the even elements
        for (auto it = vec.begin() + 1; it < vec.end(); it += 2) {
            ASSERT_EQ(0, *it % 2);
        }

        // Iterate through all the odd elements backwards
        for (auto it = vec.end() - 2; it > vec.begin(); it -= 2) {
            ASSERT_NE(0, *it % 2);
        }

        // Iterate through all the even elements backwards
        for (auto it = vec.end() - 1; it > vec.begin(); it -= 2) {
            ASSERT_EQ(0, *it % 2);
        }
    }

    TEST(VectorTests, SingleInsertionTest)
    {
        dynamic_vector<VectorTestDummy> vec;

        vec.insert(0, VectorTestDummy(3));
        ASSERT_EQ(3, vec[0].Value());
        ASSERT_EQ(1, vec.size());
        ASSERT_EQ(1, vec.capacity());

        vec.insert(0, VectorTestDummy(0));
        ASSERT_EQ(0, vec[0].Value());
        ASSERT_EQ(3, vec[1].Value());
        ASSERT_EQ(2, vec.size());
        ASSERT_EQ(2, vec.capacity());

        vec.insert(1, VectorTestDummy(2));
        ASSERT_EQ(0, vec[0].Value());
        ASSERT_EQ(2, vec[1].Value());
        ASSERT_EQ(3, vec[2].Value());
        ASSERT_EQ(3, vec.size());
        ASSERT_EQ(4, vec.capacity());

        vec.insert(1, VectorTestDummy(1));
        ASSERT_EQ(0, vec[0].Value());
        ASSERT_EQ(1, vec[1].Value());
        ASSERT_EQ(2, vec[2].Value());
        ASSERT_EQ(3, vec[3].Value());
        ASSERT_EQ(4, vec.size());
        ASSERT_EQ(4, vec.capacity());
    }

    TEST(VectorTests, VectorInsertionTest)
    {
        dynamic_vector<VectorTestDummy> vec1;

        dynamic_vector<VectorTestDummy> vec2 = {1, 6};
        dynamic_vector<VectorTestDummy> vec3 = {2, 5};
        dynamic_vector<VectorTestDummy> vec4({7, 8});

        // Vector insertion requires grow
        vec1.insert(0, vec2);
        ASSERT_EQ(1, vec1[0]);
        ASSERT_EQ(6, vec1[1]);
        ASSERT_EQ(vec1.size(), 2);
        ASSERT_EQ(vec1.capacity(), 4);

        // Vector insertion does not require grow
        vec1.insert(1, vec3);
        ASSERT_EQ(1, vec1[0]);
        ASSERT_EQ(2, vec1[1]);
        ASSERT_EQ(5, vec1[2]);
        ASSERT_EQ(6, vec1[3]);
        ASSERT_EQ(vec1.size(), 4);
        ASSERT_EQ(vec1.capacity(), 4);

        // push_back forwards to insert(vec1.size(), vec4)
        vec1.push_back(vec4);
        ASSERT_EQ(1, vec1[0]);
        ASSERT_EQ(2, vec1[1]);
        ASSERT_EQ(5, vec1[2]);
        ASSERT_EQ(6, vec1[3]);
        ASSERT_EQ(7, vec1[4]);
        ASSERT_EQ(8, vec1[5]);

        ASSERT_EQ(vec1.size(), 6);
        ASSERT_EQ(vec1.capacity(), 12);
    }

    TEST(VectorTests, VectorEraseElementTest)
    {
        dynamic_vector<int> vec;
        vec = {1, 2, 3, 4, 5, 6};

        // Erase 1
        auto it = vec.erase(vec.begin());
        ASSERT_EQ(5, vec.size());
        ASSERT_EQ(2, vec[0]);
        ASSERT_EQ(vec[0], *it);

        // Erase 6
        auto end_it = vec.end() - 1;
        end_it = vec.erase(end_it);
        ASSERT_EQ(vec.end(), end_it);
        ASSERT_EQ(5, vec[3]);

        // Erase 3
        dynamic_vector<int>::iterator middle(&vec[1]);
        middle = vec.erase(middle);
        ASSERT_EQ(3, vec.size());
        ASSERT_EQ(4, *middle);

        // Iterate
        size_t sum = 0;
        for (auto& element : vec) {
            sum += element;
        }
        ASSERT_EQ(11, sum);
    }

    TEST(VectorTests, EmplaceTest)
    {
        dynamic_vector<VectorTestDummy> vec;

        vec.emplace(0, 3);
        vec.emplace(0, VectorTestDummy(0));
        vec.emplace(1, 2);
        vec.emplace(1, VectorTestDummy(1));

        ASSERT_EQ(4, vec.size());
        ASSERT_EQ(4, vec.capacity());

        for (size_t i = 0; i < vec.size(); i++) {
            ASSERT_EQ(i, vec[i].Value());
        }
    }

    TEST(VectorTests, ElementDestructionTest)
    {
        int destroyed = 0;
        struct DtorDummy
        {
            DtorDummy(int* counter)
            {
                m_Counter = counter;
            }

            ~DtorDummy()
            {
                (*m_Counter)++;
            }

            int* m_Counter;
        };

        dynamic_vector<DtorDummy>* vec = new dynamic_vector<DtorDummy>;

        for (int i = 0; i < 10; i++) {
            vec->emplace_back(&destroyed);
        }

        delete vec;

        ASSERT_EQ(10, destroyed);
    }

    TEST(VectorTests, CopyAssignmentOperatorTest)
    {
        dynamic_vector<std::string> vec1(MemorySystem::GetRootMemSpace(), {"Foo", "Bar"});
        dynamic_vector<std::string> vec2(MemorySystem::GetRootMemSpace(), {"Biz", "Baz"});

        // Copy Assignment
        vec1 = vec2;

        for (auto i = 0; i < vec1.size(); i++) {
            ASSERT_EQ(vec1[i], vec2[i]);
        }
    }

    TEST(VectorTests, MoveAssignmentOperatorTest)
    {
        dynamic_vector<std::string> vec1(MemorySystem::GetRootMemSpace(), {"Foo", "Bar"});
        dynamic_vector<std::string> vec2(MemorySystem::GetRootMemSpace(), {"Biz", "Baz"});

        // Move Assign a vector temporary into vec2
        vec2 = dynamic_vector<std::string>(MemorySystem::GetRootMemSpace(), {"One", "Two"});
        ASSERT_EQ("One", vec2[0]);
        ASSERT_EQ("Two", vec2[1]);
    }

    TEST(VectorTests, CopyContructorTest)
    {
        dynamic_vector<std::string> vec1(MemorySystem::GetRootMemSpace(), {"Foo", "Bar"});
        dynamic_vector<std::string> vec2(vec1);

        ASSERT_EQ("Foo", vec2[0]);
        ASSERT_EQ("Bar", vec2[1]);
    }

    TEST(VectorTests, MoveContructorTest)
    {
        dynamic_vector<std::string> vec1(MemorySystem::GetRootMemSpace(), {"Foo", "Bar"});
        dynamic_vector<std::string> vec2(std::move(vec1));
        
        ASSERT_EQ(0, vec1.size());

        ASSERT_EQ("Foo", vec2[0]);
        ASSERT_EQ("Bar", vec2[1]);

        vec1.push_back("Buzz");
        ASSERT_EQ(1, vec1.size());

        dynamic_vector<std::string> vec3(
            dynamic_vector<std::string>(MemorySystem::GetRootMemSpace(), {"Biz", "Baz"}));
        ASSERT_EQ("Biz", vec3[0]);
        ASSERT_EQ("Baz", vec3[1]);
    }

    TEST(VectorTests, ResizeTest)
    {
        dynamic_vector<int> vec(MemorySystem::GetRootMemSpace(), 10);
        ASSERT_EQ(10, vec.capacity());
        ASSERT_EQ(10, vec.size());

        for (auto element : vec) {
            ASSERT_EQ(0, element);
        }

        vec.resize(20, 1);
        ASSERT_EQ(20, vec.capacity());
        ASSERT_EQ(20, vec.size());

        auto sum = 0;
        for (auto element : vec) {
            sum += element;
        }
        ASSERT_EQ(sum, 10);

        // Attempt to reduce size
        vec.resize(10);

        sum = 0;
        for (auto element : vec) {
            sum += element;
        }
        ASSERT_EQ(sum, 0);

        vec.resize(0);
        ASSERT_EQ(0, vec.size());
        ASSERT_EQ(0, vec.capacity());
    }

    TEST(VectorTests, ReserveTests)
    {
        dynamic_vector<int> vec;
        vec.reserve(10);
        ASSERT_EQ(10, vec.capacity());
        ASSERT_EQ(0, vec.size());

        vec.push_back(2);
        vec.push_back(1);

        // Attempt to reduce capacity
        vec.reserve(1);
        ASSERT_EQ(1, vec.capacity());
        ASSERT_EQ(1, vec.size());
        ASSERT_EQ(2, vec.front());

        // Attempt to eliminate capacity
        vec.reserve(0);
        ASSERT_EQ(0, vec.capacity());
        ASSERT_EQ(0, vec.size());
    }

    TEST(VectorTests, ValueInitializingConstructorTest)
    {
        dynamic_vector<std::string> vec1(MemorySystem::GetRootMemSpace(), 10, "Foo");

        for (size_t i = 0; i < vec1.size(); i++) {
            ASSERT_EQ("Foo", vec1[i]);
        }
    }

    TEST(VectorTests, StdSpanConstruction)
    {
        dynamic_vector<std::string> vec1(MemorySystem::GetRootMemSpace(), 10, "Foo");

        std::span testSpan(vec1);

        for(size_t i = 0; i < vec1.size(); i++)
        {
            ASSERT_EQ(testSpan[i], vec1[i]);
        }
    }


} // namespace container_tests
