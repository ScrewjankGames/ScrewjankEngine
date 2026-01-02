// STD Headers
#include <glaze/core/reflect.hpp>
#include <string>
#include <memory>

// Library Headers
#include <memory_resource>
#include <gtest/gtest.h>
#include <glaze/glaze.hpp>
#include <glaze/core/meta.hpp>

// SJ Headers
#include <ScrewjankStd/Log.hpp>

import sj.engine.ecs.Archetype;
import sj.std.type_info;

using namespace sj;

namespace ecs_tests
{

struct DummyComponentA
{
    float a = 1234.4321f;
    int b = 50501312;
};

struct DummyComponentB
{
    std::string name = "BBoy";
};

struct DummyComponentC
{
    std::string name = "CBoy";
    std::vector<int> nums = {5, 0, 5, 0, 1, 3, 1, 2};
};

TEST(ArchetypeTest, ConstructionTest)
{
    const type_info& infoA = sj::type_info_of<DummyComponentA>;
    const type_info& infoB = sj::type_info_of<DummyComponentB>;
    const type_info& infoC = sj::type_info_of<DummyComponentC>;

    Archetype archetype(std::array {&infoA, &infoB, &infoC}, std::pmr::get_default_resource());
    size_t row0 = archetype.AddRow();

    {
        DummyComponentA& a0 = archetype.GetComponent<DummyComponentA>(row0);
        ASSERT_EQ(a0.a, 1234.4321f);
        a0.a = 1.0f;
    }

    for(int i = 0; i < 100; i++)
        archetype.AddRow();

    {
        DummyComponentA& a0 = archetype.GetComponent<DummyComponentA>(row0);
        ASSERT_EQ(a0.a, 1.0f);
    }

    ASSERT_TRUE(false);
}

// TEST(ArchetypeTest, HasComponentsTest)
// {
//     std::array query1 = {DummyComponentA::kTypeId};
//     std::array query2 = {DummyComponentA::kTypeId, DummyComponentB::kTypeId};
//     std::array query3 = {DummyComponentB::kTypeId, DummyComponentA::kTypeId};

//     std::array query4 = {DummyComponentA::kTypeId,
//                          DummyComponentB::kTypeId,
//                          DummyComponentC::kTypeId};
//     std::array query5 = {DummyComponentA::kTypeId,
//                          DummyComponentC::kTypeId,
//                          DummyComponentB::kTypeId};

//     std::array query6 = {DummyComponentC::kTypeId,
//                          DummyComponentA::kTypeId,
//                          DummyComponentB::kTypeId};
//     std::array query7 = {DummyComponentC::kTypeId,
//                          DummyComponentB::kTypeId,
//                          DummyComponentA::kTypeId};

//     std::array<std::span<TypeId>, 7> allQueries =
//         {query1, query2, query3, query4, query5, query6, query7};

//     {
//         Archetype emptyTest(std::pmr::get_default_resource());

//         for(auto query : allQueries)
//         {
//             ASSERT_FALSE(emptyTest.HasAllComponents(query));
//         }
//     }

//     {
//         Archetype<DummyComponentA, DummyComponentB, DummyComponentC> completeTest(
//             std::pmr::get_default_resource());

//         for(auto query : allQueries)
//         {
//             ASSERT_TRUE(completeTest.HasAllComponents(query));
//         }
//     }

//     // Partial test
//     {
//         Archetype<DummyComponentA, DummyComponentB> ab(std::pmr::get_default_resource());

//         ASSERT_TRUE(ab.HasAllComponents(query1));
//         ASSERT_TRUE(ab.HasAllComponents(query2));
//         ASSERT_TRUE(ab.HasAllComponents(query3));
//         ASSERT_FALSE(ab.HasAllComponents(query4));
//         ASSERT_FALSE(ab.HasAllComponents(query5));
//         ASSERT_FALSE(ab.HasAllComponents(query6));
//         ASSERT_FALSE(ab.HasAllComponents(query7));
//     }

//     // out of order test
//     {
//         Archetype<DummyComponentB, DummyComponentC, DummyComponentA> ordered(
//             std::pmr::get_default_resource());
//         for(auto query : allQueries)
//         {
//             ASSERT_TRUE(ordered.HasAllComponents(query));
//         }
//     }
// }

// TEST(ArchetypeTest, TypeErasedComponentAcessTest)
// {
//     IArchetype* interface = new Archetype<DummyComponentA, DummyComponentB, DummyComponentC>(
//         std::pmr::get_default_resource());

//     GameObjectId id0 {.sparseIndex=0};
//     GameObjectId id1 {.sparseIndex=1};

//     interface.NewGameObject(id0);

//     delete interface;
// }

} // namespace ecs_tests
