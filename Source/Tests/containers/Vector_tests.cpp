// STD Headers
#include <span>

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include <ScrewjankShared/utils/Log.hpp>

import sj.shared.containers;

using namespace sj;

namespace container_tests
{

    class VectorTestDummy
    {
    public:
        VectorTestDummy() : m_Data(-1)
        {
            SJ_ENGINE_LOG_INFO("Default constructed");
        }

        VectorTestDummy(int val) : m_Data(val)
        {
            SJ_ENGINE_LOG_INFO("parameter constructed");
        }

        VectorTestDummy(const VectorTestDummy& other) : m_Data(other.m_Data)
        {
            SJ_ENGINE_LOG_INFO("copy constructed");
        }

        VectorTestDummy(const VectorTestDummy&& other) noexcept : m_Data(other.m_Data)
        {
            SJ_ENGINE_LOG_INFO("move constructed");
        }

        VectorTestDummy& operator=(const VectorTestDummy& other)
        {
            SJ_ENGINE_LOG_INFO("copy assignment operator");
            m_Data = other.m_Data;

            return *this;
        }

        VectorTestDummy& operator=(VectorTestDummy&& other) noexcept
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

        vec.emplace_back(1);
        vec.emplace_back(2);
        vec.emplace_back(3);
        ASSERT_FALSE(vec.empty());

        ASSERT_EQ(1, vec[0]);
        ASSERT_EQ(2, vec[1]);
        ASSERT_EQ(3, vec[2]);

        vec.erase(vec.begin() + 1);
        ASSERT_EQ(1, vec[0]);
        ASSERT_NE(2, vec[1]);

        vec.erase(vec.begin());
        ASSERT_NE(1, vec[0]);

        vec.erase(vec.begin());
        ASSERT_TRUE(vec.empty());
    }

    TEST(StaticVectorTests, SwappableTests)
    {
        static_vector<int, 3> a;
        static_vector<int, 3> b;

        ;

        static_assert(std::is_swappable_v<decltype(a)>, "Not swappable?");
        static_assert(noexcept(swap(a, b)));
        static_assert(std::is_nothrow_swappable_v<decltype(a)>, "Not nothrow swappable?");
    }

    TEST(StaticVectorTests, EmplaceTests)
    {
        static_vector<int, 4> testVec = {0, 2, 3};

        testVec.emplace(&testVec[1], 1);
        ASSERT_EQ(testVec.size(), 4);
        for(size_t i = 0; i < testVec.size(); i++)
        {
            ASSERT_EQ(i, testVec[i]);
        }
    }

    TEST(VectorTests, ListInitializationTest)
    {
        dynamic_vector<int> vec1({1, 2, 3, 4, 5});
        ASSERT_EQ(5, vec1.size());
        ASSERT_EQ(5, vec1.capacity());

        for(size_t i = 0; i < vec1.size(); i++)
        {
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
        vec.emplace_back(dummy);
        ASSERT_EQ(-1, vec[0].Value());

        auto dummy2 = vec.emplace_back(100);

        ASSERT_EQ(dummy2, vec[1]);
        ASSERT_EQ(100, vec[1].Value());
    }

    TEST(VectorTests, ElementAccessTest)
    {
        dynamic_vector<int> vec1({1, 2, 3});

        ASSERT_EQ(1, vec1[0]);

        ASSERT_EQ(vec1[0], vec1.at(0));
        ASSERT_EQ(vec1[0], vec1.front());
        ASSERT_EQ(vec1[2], vec1.back());
    }

    TEST(VectorTests, IterationTest)
    {
        dynamic_vector<int> vec;
        size_t i = 0;

        for(auto& _ : vec)
        {
            i++;
        }
        ASSERT_EQ(0, i);

        vec.emplace_back(1);
        vec.emplace_back(2);
        vec.emplace_back(3);
        vec.emplace_back(4);
        vec.emplace_back(5);
        vec.emplace_back(6);

        // Iterate through all the odd elements
        for(auto it = vec.begin(); it < vec.end(); it = it + 2)
        {
            ASSERT_NE(0, *it % 2);
        }

        // Iterate through all the even elements
        for(auto it = vec.begin() + 1; it < vec.end(); it += 2)
        {
            ASSERT_EQ(0, *it % 2);
        }

        // Iterate through all the odd elements backwards
        for(auto it = vec.end() - 2; it > vec.begin(); it -= 2)
        {
            ASSERT_NE(0, *it % 2);
        }

        // Iterate through all the even elements backwards
        for(auto it = vec.end() - 1; it > vec.begin(); it -= 2)
        {
            ASSERT_EQ(0, *it % 2);
        }
    }

    TEST(VectorTests, SingleInsertionTest)
    {
        dynamic_vector<VectorTestDummy> vec;

        vec.emplace(vec.begin(), VectorTestDummy(3));
        ASSERT_EQ(3, vec[0].Value());
        ASSERT_EQ(1, vec.size());
        ASSERT_EQ(1, vec.capacity());

        vec.emplace(vec.begin(), VectorTestDummy(0));
        ASSERT_EQ(0, vec[0].Value());
        ASSERT_EQ(3, vec[1].Value());
        ASSERT_EQ(2, vec.size());
        ASSERT_EQ(2, vec.capacity());

        vec.emplace(vec.begin() + 1, VectorTestDummy(2)); // NOLINT
        ASSERT_EQ(0, vec[0].Value());
        ASSERT_EQ(2, vec[1].Value());
        ASSERT_EQ(3, vec[2].Value());
        ASSERT_EQ(3, vec.size());
        ASSERT_EQ(4, vec.capacity());

        vec.emplace(vec.begin() + 1, VectorTestDummy(1)); // NOLINT
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

        vec1.insert(vec1.begin(), vec2.begin(), vec2.end());
        ASSERT_EQ(1, vec1[0]);
        ASSERT_EQ(6, vec1[1]);
        ASSERT_EQ(vec1.size(), 2);

        vec1.insert(vec1.begin() + 1, vec3.begin(), vec3.end());
        ASSERT_EQ(1, vec1[0]);
        ASSERT_EQ(2, vec1[1]);
        ASSERT_EQ(5, vec1[2]);
        ASSERT_EQ(6, vec1[3]);
        ASSERT_EQ(vec1.size(), 4);

        // push_back forwards to insert(vec1.size(), vec4)
        vec1.insert(vec1.end(), vec4.begin(), vec4.end());
        ASSERT_EQ(1, vec1[0]);
        ASSERT_EQ(2, vec1[1]);
        ASSERT_EQ(5, vec1[2]);
        ASSERT_EQ(6, vec1[3]);
        ASSERT_EQ(7, vec1[4]);
        ASSERT_EQ(8, vec1[5]);

        ASSERT_EQ(vec1.size(), 6);
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
        for(auto& element : vec)
        {
            sum += element;
        }
        ASSERT_EQ(11, sum);
    }

    TEST(VectorTests, RangeEraseTest)
    {
        dynamic_vector<int> test1 = {0,1,2,3,4,5,6};

        auto res = test1.erase(test1.begin() + 2, test1.begin() + 4); // erase 2,3
        ASSERT_EQ(5, test1.size());
        ASSERT_EQ(*res, 4);
        
        dynamic_vector<int> test2 = {0,1,2,3,4,5,6};
        auto res2 = test2.erase(test2.begin() + 1, test2.end());
        ASSERT_EQ(1, test2.size());
        ASSERT_EQ(res2, test2.end());
    }

    TEST(VectorTests, EmplaceTest)
    {
        dynamic_vector<VectorTestDummy> vec;

        vec.emplace(vec.begin(), 3);
        vec.emplace(vec.begin(), VectorTestDummy(0));
        vec.emplace(vec.begin() + 1, 2);
        vec.emplace(vec.begin() + 1, VectorTestDummy(1));

        ASSERT_EQ(4, vec.size());
        ASSERT_EQ(4, vec.capacity());

        for(size_t i = 0; i < vec.size(); i++)
        {
            ASSERT_EQ(i, vec[i].Value());
        }
    }

    TEST(VectorTests, ElementDestructionTest)
    {
        int destroyed = 0;
        struct DtorDummy
        {
            DtorDummy(int* counter) : m_Counter(counter)
            {
            }

            ~DtorDummy()
            {
                (*m_Counter)++;
            }

            int* m_Counter;
        };

        dynamic_vector<DtorDummy>* vec = new dynamic_vector<DtorDummy>;

        for(size_t i = 0; i < 10; i++)
        {
            vec->emplace_back(&destroyed);
        }

        delete vec;

        ASSERT_EQ(10, destroyed);
    }

    TEST(VectorTests, CopyAssignmentOperatorTest)
    {
        dynamic_vector<std::string> vec1({"Foo", "Bar"});
        dynamic_vector<std::string> vec2({"Biz", "Baz"});

        // Copy Assignment
        vec1 = vec2;

        for(size_t i = 0; i < vec1.size(); i++)
        {
            ASSERT_EQ(vec1[i], vec2[i]);
        }
    }

    TEST(VectorTests, MoveAssignmentOperatorTest)
    {
        dynamic_vector<std::string> vec1({"Foo", "Bar"});
        dynamic_vector<std::string> vec2({"Biz", "Baz"});

        // Move Assign a vector temporary into vec2
        vec2 = dynamic_vector<std::string>({"One", "Two"});
        ASSERT_EQ("One", vec2[0]);
        ASSERT_EQ("Two", vec2[1]);
    }

    TEST(VectorTests, CopyContructorTest)
    {
        dynamic_vector<std::string> vec1({"Foo", "Bar"});
        dynamic_vector<std::string> vec2(vec1);

        ASSERT_EQ("Foo", vec2[0]);
        ASSERT_EQ("Bar", vec2[1]);
    }

    TEST(VectorTests, MoveContructorTest)
    {
        dynamic_vector<std::string> vec1({"Foo", "Bar"});
        dynamic_vector<std::string> vec2(std::move(vec1));

        ASSERT_EQ(0, vec1.size()); // NOLINT(clang-analyzer-cplusplus.Move)

        ASSERT_EQ("Foo", vec2[0]);
        ASSERT_EQ("Bar", vec2[1]);

        vec1.emplace_back("Buzz");
        ASSERT_EQ(1, vec1.size());

        dynamic_vector<std::string> vec3(dynamic_vector<std::string>({"Biz", "Baz"}));
        ASSERT_EQ("Biz", vec3[0]);
        ASSERT_EQ("Baz", vec3[1]);
    }

    TEST(VectorTests, ResizeTest)
    {
        dynamic_vector<int> vec(10);
        ASSERT_EQ(10, vec.capacity());
        ASSERT_EQ(10, vec.size());

        for(auto element : vec)
        {
            ASSERT_EQ(0, element);
        }

        vec.resize(20, 1);
        ASSERT_EQ(20, vec.capacity());
        ASSERT_EQ(20, vec.size());

        auto sum = 0;
        for(auto element : vec)
        {
            sum += element;
        }
        ASSERT_EQ(sum, 10);

        // Attempt to reduce size
        vec.resize(10);

        sum = 0;
        for(auto element : vec)
        {
            sum += element;
        }
        ASSERT_EQ(sum, 0);

        vec.resize(0);
        ASSERT_EQ(0, vec.size());
    }

    TEST(VectorTests, ReserveTests)
    {
        dynamic_vector<int> vec;
        vec.reserve(10);
        ASSERT_EQ(10, vec.capacity());
        ASSERT_EQ(0, vec.size());

        vec.emplace_back(2);
        vec.emplace_back(1);
    }

    TEST(VectorTests, ValueInitializingConstructorTest)
    {
        dynamic_vector<std::string> vec1(10, "Foo");

        for(size_t i = 0; i < vec1.size(); i++)
        {
            ASSERT_EQ("Foo", vec1[i]);
        }
    }

    TEST(VectorTests, StdSpanConstruction)
    {
        dynamic_vector<std::string> vec1(10, "Foo");

        std::span testSpan(vec1);

        for(size_t i = 0; i < vec1.size(); i++)
        {
            ASSERT_EQ(testSpan[i], vec1[i]);
        }
    }

} // namespace container_tests
