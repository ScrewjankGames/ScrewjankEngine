// STD Headers
#include <vector>

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include "containers/Vector.hpp"
#include "core/Log.hpp"

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

    TEST(VectorTests, ListInitializationTest)
    {
        Vector<int> vec1(MemorySystem::GetUnmanagedAllocator(), {1, 2, 3, 4, 5});
        ASSERT_EQ(5, vec1.Size());
        ASSERT_EQ(5, vec1.Capacity());

        for (size_t i = 0; i < vec1.Size(); i++) {
            ASSERT_EQ(i + 1, vec1[i]);
        }

        Vector<std::string> vec2(MemorySystem::GetUnmanagedAllocator());
        vec2 = {"Foo", "Bar", "Biz", "Baz"};
        ASSERT_EQ(4, vec2.Size());
        ASSERT_EQ(4, vec2.Capacity());
        ASSERT_EQ("Foo", vec2[0]);
        ASSERT_EQ("Bar", vec2[1]);
        ASSERT_EQ("Biz", vec2[2]);
        ASSERT_EQ("Baz", vec2[3]);
    }

    TEST(VectorTests, ElementInsertionTest)
    {
        Vector<VectorTestDummy> vec(MemorySystem::GetUnmanagedAllocator());
        VectorTestDummy dummy;

        // Copy construct dummy into vec
        vec.PushBack(dummy);
        ASSERT_EQ(-1, vec[0].Value());

        auto dummy2 = vec.EmplaceBack(100);

        ASSERT_EQ(dummy2, vec[1]);
        ASSERT_EQ(100, vec[1].Value());
    }

    TEST(VectorTests, ElementAccessTest)
    {
        Vector<int> vec1(MemorySystem::GetUnmanagedAllocator(), {1, 2, 3});

        ASSERT_EQ(1, vec1[0]);

        ASSERT_EQ(vec1[0], vec1.At(0));
        ASSERT_EQ(vec1[0], vec1.Front());
        ASSERT_EQ(vec1[2], vec1.Back());

#ifdef SJ_DEBUG
        ASSERT_DEATH(vec1.At(-1), ".*");
#endif // SJ_DEBUG
    }

    TEST(VectorTests, IterationTest)
    {
        Vector<int> vec(MemorySystem::GetUnmanagedAllocator());
        int i = 0;

        for (auto& element : vec) {
            i++;
        }
        ASSERT_EQ(0, i);

        vec.PushBack(1);
        vec.EmplaceBack(2);
        vec.PushBack(3);
        vec.EmplaceBack(4);
        vec.PushBack(5);
        vec.EmplaceBack(6);

        // Iterate through all the odd elements
        for (auto it = vec.begin(); it < vec.end(); it = it + 2) {
            ASSERT_NE(0, *it % 2);
        }

        // Iterate through all the even elements
        for (auto it = vec.begin() + 1; it < vec.end(); it += 2) {
            ASSERT_EQ(0, *it % 2);
        }

        // Iterate through all the odd elements backwards
        for (auto it = vec.end(); it > vec.begin(); it -= 2) {
            ASSERT_NE(0, *it % 2);
        }

        // Iterate through all the even elements backwards
        for (auto it = vec.end() - 1; it > vec.begin(); it -= 2) {
            ASSERT_EQ(0, *it % 2);
        }
    }

    TEST(VectorTests, SingleInsertionTest)
    {
        Vector<VectorTestDummy> vec(MemorySystem::GetUnmanagedAllocator());

        vec.Insert(0, VectorTestDummy(3));
        ASSERT_EQ(3, vec[0].Value());
        ASSERT_EQ(1, vec.Size());
        ASSERT_EQ(1, vec.Capacity());

        vec.Insert(0, VectorTestDummy(0));
        ASSERT_EQ(0, vec[0].Value());
        ASSERT_EQ(3, vec[1].Value());
        ASSERT_EQ(2, vec.Size());
        ASSERT_EQ(2, vec.Capacity());

        vec.Insert(1, VectorTestDummy(2));
        ASSERT_EQ(0, vec[0].Value());
        ASSERT_EQ(2, vec[1].Value());
        ASSERT_EQ(3, vec[2].Value());
        ASSERT_EQ(3, vec.Size());
        ASSERT_EQ(4, vec.Capacity());

        vec.Insert(1, VectorTestDummy(1));
        ASSERT_EQ(0, vec[0].Value());
        ASSERT_EQ(1, vec[1].Value());
        ASSERT_EQ(2, vec[2].Value());
        ASSERT_EQ(3, vec[3].Value());
        ASSERT_EQ(4, vec.Size());
        ASSERT_EQ(4, vec.Capacity());
    }

    TEST(VectorTests, VectorInsertionTest)
    {
        Vector<VectorTestDummy> vec1(MemorySystem::GetUnmanagedAllocator());

        Vector<VectorTestDummy> vec2(MemorySystem::GetUnmanagedAllocator(), {1, 6});
        Vector<VectorTestDummy> vec3(MemorySystem::GetUnmanagedAllocator(), {2, 5});
        Vector<VectorTestDummy> vec4(MemorySystem::GetUnmanagedAllocator(), {7, 8});

        // Vector insertion requires grow
        vec1.Insert(0, vec2);
        ASSERT_EQ(1, vec1[0]);
        ASSERT_EQ(6, vec1[1]);
        ASSERT_EQ(vec1.Size(), 2);
        ASSERT_EQ(vec1.Capacity(), 4);

        // Vector insertion does not require grow
        vec1.Insert(1, vec3);
        ASSERT_EQ(1, vec1[0]);
        ASSERT_EQ(2, vec1[1]);
        ASSERT_EQ(5, vec1[2]);
        ASSERT_EQ(6, vec1[3]);
        ASSERT_EQ(vec1.Size(), 4);
        ASSERT_EQ(vec1.Capacity(), 4);

        // PushBack forwards to Insert(vec1.Size(), vec4)
        vec1.PushBack(vec4);
        ASSERT_EQ(1, vec1[0]);
        ASSERT_EQ(2, vec1[1]);
        ASSERT_EQ(5, vec1[2]);
        ASSERT_EQ(6, vec1[3]);
        ASSERT_EQ(7, vec1[4]);
        ASSERT_EQ(8, vec1[5]);

        ASSERT_EQ(vec1.Size(), 6);
        ASSERT_EQ(vec1.Capacity(), 12);
    }

    TEST(VectorTests, VectorEraseElementTest)
    {
        Vector<int> vec(MemorySystem::GetUnmanagedAllocator());
        vec = {1, 2, 3, 4, 5, 6};

        // Erase 1
        auto it = vec.Erase(vec.begin());
        ASSERT_EQ(5, vec.Size());
        ASSERT_EQ(2, vec[0]);
        ASSERT_EQ(vec[0], *it);

        // Erase 6
        auto end_it = vec.end() - 1;
        end_it = vec.Erase(end_it);
        ASSERT_EQ(vec.end(), end_it);
        ASSERT_EQ(5, vec[3]);

        // Erase 3
        Vector<int>::iterator middle(&vec[1]);
        middle = vec.Erase(middle);
        ASSERT_EQ(3, vec.Size());
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
        Vector<VectorTestDummy> vec(MemorySystem::GetUnmanagedAllocator());

        vec.Emplace(0, 3);
        vec.Emplace(0, VectorTestDummy(0));
        vec.Emplace(1, 2);
        vec.Emplace(1, VectorTestDummy(1));

        ASSERT_EQ(4, vec.Size());
        ASSERT_EQ(4, vec.Capacity());

        for (size_t i = 0; i < vec.Size(); i++) {
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

        Vector<DtorDummy>* vec = new Vector<DtorDummy>(MemorySystem::GetUnmanagedAllocator());

        for (int i = 0; i < 10; i++) {
            vec->EmplaceBack(&destroyed);
        }

        delete vec;

        // std::vector<DtorDummy> vec;
        // for (int i = 0; i < 10; i++) {
        //    vec.emplace_back(&destroyed);
        //}

        ASSERT_EQ(10, destroyed);
    }

    TEST(VectorTests, CopyAssignmentOperatorTest)
    {
        Vector<std::string> vec1(MemorySystem::GetUnmanagedAllocator(), {"Foo", "Bar"});
        Vector<std::string> vec2(MemorySystem::GetUnmanagedAllocator(), {"Biz", "Baz"});

        // Copy Assignment
        vec1 = vec2;

        for (auto i = 0; i < vec1.Size(); i++) {
            ASSERT_EQ(vec1[i], vec2[i]);
        }
    }

    TEST(VectorTests, MoveAssignmentOperatorTest)
    {
        Vector<std::string> vec1(MemorySystem::GetUnmanagedAllocator(), {"Foo", "Bar"});
        Vector<std::string> vec2(MemorySystem::GetUnmanagedAllocator(), {"Biz", "Baz"});

        // Move Assign a vector temporary into vec2
        vec2 = Vector<std::string>(MemorySystem::GetUnmanagedAllocator(), {"One", "Two"});
        ASSERT_EQ("One", vec2[0]);
        ASSERT_EQ("Two", vec2[1]);
    }

    TEST(VectorTests, CopyContructorTest)
    {
        Vector<std::string> vec1(MemorySystem::GetUnmanagedAllocator(), {"Foo", "Bar"});
        Vector<std::string> vec2(vec1);

        ASSERT_EQ("Foo", vec2[0]);
        ASSERT_EQ("Bar", vec2[1]);
    }

    TEST(VectorTests, MoveContructorTest)
    {
        Vector<std::string> vec1(MemorySystem::GetUnmanagedAllocator(), {"Foo", "Bar"});
        Vector<std::string> vec2(std::move(vec1));
        ASSERT_EQ("Foo", vec2[0]);
        ASSERT_EQ("Bar", vec2[1]);

        Vector<std::string> vec3(
            Vector<std::string>(MemorySystem::GetUnmanagedAllocator(), {"Biz", "Baz"}));
        ASSERT_EQ("Biz", vec3[0]);
        ASSERT_EQ("Baz", vec3[1]);
    }

    TEST(VectorTests, ResizeTest)
    {
        Vector<int> vec(MemorySystem::GetUnmanagedAllocator(), 10);
        ASSERT_EQ(10, vec.Capacity());
        ASSERT_EQ(10, vec.Size());

        for (auto element : vec) {
            ASSERT_EQ(0, element);
        }

        vec.Resize(20, 1);
        ASSERT_EQ(20, vec.Capacity());
        ASSERT_EQ(20, vec.Size());

        auto sum = 0;
        for (auto element : vec) {
            sum += element;
        }
        ASSERT_EQ(sum, 10);

        // Attempt to reduce size
        vec.Resize(10);

        sum = 0;
        for (auto element : vec) {
            sum += element;
        }
        ASSERT_EQ(sum, 0);
    }

    TEST(VectorTests, ReserveTests)
    {
        Vector<int> vec(MemorySystem::GetUnmanagedAllocator());
        vec.Reserve(10);
        ASSERT_EQ(10, vec.Capacity());
        ASSERT_EQ(0, vec.Size());

        vec.PushBack(2);
        vec.PushBack(1);

        // Attempt to reduce capacity
        vec.Reserve(1);
        ASSERT_EQ(1, vec.Capacity());
        ASSERT_EQ(1, vec.Size());
        ASSERT_EQ(2, vec.Front());
    }

} // namespace container_tests
