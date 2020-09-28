// STD Headers

// Library Headers
#include "gtest/gtest.h"

// Void Engine Headers
#include "containers/Vector.hpp"
#include "core/Log.hpp"

using namespace Screwjank;

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

        VectorTestDummy(const VectorTestDummy&& other)
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
        Vector<int> vec1({1, 2, 3, 4, 5}, MemorySystem::GetUnmanagedAllocator());

        ASSERT_EQ(5, vec1.Size());
        ASSERT_EQ(5, vec1.Capacity());
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
        Vector<int> vec1({1, 2, 3}, MemorySystem::GetUnmanagedAllocator());

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

        vec.PushBack(2);
        vec.EmplaceBack(4);
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

        Vector<VectorTestDummy> vec2({1, 6}, MemorySystem::GetUnmanagedAllocator());
        Vector<VectorTestDummy> vec3({2, 5}, MemorySystem::GetUnmanagedAllocator());
        Vector<VectorTestDummy> vec4({7, 8}, MemorySystem::GetUnmanagedAllocator());

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

} // namespace container_tests
