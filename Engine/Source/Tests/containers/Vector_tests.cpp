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

    TEST(VectorTests, ElementAccessTest)
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

} // namespace container_tests
