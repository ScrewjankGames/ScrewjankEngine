// STD Headers
#include <memory_resource>
#include <unordered_set>

// Library Headers
#include <gtest/gtest.h>

import sj.shared.containers;

using namespace sj;

namespace container_tests
{
    struct TestSetId 
    {
        uint32_t sparseIndex;
        uint32_t generation;
    };

    struct TestComponentA
    {
        TestSetId ownerId;
        bool ownerEven;
    };

    struct TestComponentB
    {
        TestSetId ownerId;
        bool ownerOdd;
    };

    TEST(SparseSetTests, GenerationTest)
    {
        sparse_set<TestSetId> testSet(100, std::pmr::get_default_resource());

        std::vector<TestSetId> handles;

        for(int i = 0; i < 100; i++)
        {
            handles.emplace_back(testSet.create());
        }

        for(const TestSetId& id : handles)
        {
            testSet.release(id);
        }

        handles.clear();
        for(int i = 0; i < 100; i++)
        {
            handles.emplace_back(testSet.create());
        }
        
        for(const TestSetId& id : handles)
        {
            ASSERT_EQ(1, id.generation);
            testSet.release(id);
        }
    }

    TEST(SparseSetTests, MultiSetTest)
    {
        sparse_set<TestSetId> idSet(100, std::pmr::get_default_resource());
        sparse_set<TestSetId, TestComponentA> componentASet(100, std::pmr::get_default_resource());
        sparse_set<TestSetId, TestComponentB> componentBSet(100, std::pmr::get_default_resource());

        for(int i = 0; i < 100; i++)
        {
            TestSetId id = idSet.create();
            if(i % 2 == 0)
                componentASet.create(id, TestComponentA{id, id.sparseIndex % 2 == 0});
            if(i % 3 == 0)
                componentBSet.create(id, TestComponentB{id, id.sparseIndex % 2 != 0});
        }

        for(const TestSetId& id : idSet.get_set<TestSetId>())
        {
            if(id.sparseIndex % 2 == 0)
            {
                const TestComponentA& a = componentASet.get<TestComponentA>(id);
                ASSERT_EQ(id.sparseIndex, a.ownerId.sparseIndex);
                ASSERT_EQ(id.generation, a.ownerId.generation);
            }

            if(id.sparseIndex % 3 == 0 )
            {
                const TestComponentB& b = componentBSet.get<TestComponentB>(id);
                ASSERT_EQ(id.sparseIndex, b.ownerId.sparseIndex);
                ASSERT_EQ(id.generation, b.ownerId.generation);
            }
        }
    }

} // namespace container_tests