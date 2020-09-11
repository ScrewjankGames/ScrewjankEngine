// STD Headers
#include <vector>

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
        }

        VectorTestDummy& operator=(const VectorTestDummy&& other)
        {
            SJ_ENGINE_LOG_INFO("move assignment operator");
            m_Data = other.m_Data;
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
        Vector<int> vec1({1, 2, 3, 4, 5}, MemorySystem::GetDefaultUnmanagedAllocator());

        ASSERT_EQ(5, vec1.Size());
        ASSERT_EQ(5, vec1.Capacity());
    }

    TEST(VectorTests, ElementInsertionTest)
    {
        Vector<VectorTestDummy> vec(MemorySystem::GetDefaultUnmanagedAllocator());
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
        Vector<int> vec1({1, 2, 3}, MemorySystem::GetDefaultUnmanagedAllocator());

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
        Vector<int> vec(MemorySystem::GetDefaultUnmanagedAllocator());
        int i = 0;

        for (auto& element : vec) {
            i++;
        }
        ASSERT_EQ(0, i);

        vec.PushBack(2);
        vec.EmplaceBack(4);
    }

} // namespace container_tests
