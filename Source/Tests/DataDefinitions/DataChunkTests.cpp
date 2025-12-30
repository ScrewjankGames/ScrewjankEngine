#include <glaze/json/write.hpp>
#include <gtest/gtest.h>

#include <glaze/glaze.hpp>

import sj.datadefs;
import sj.std;

using namespace sj;

namespace data_chunk_tests
{
struct DummyStruct
{
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
};

TEST(DataChunkTests, JSONReadWriteTest)
{
    DataChunk test {.type = sj::type_id_of<DummyStruct>};
    test.data = DummyStruct {};
    auto res = glz::write_json(test);
    ASSERT_TRUE(res);

    DataChunk test2;
    auto _ = glz::read_json(test2, *res);
    ASSERT_EQ(test.type, test2.type);
}

TEST(DataChunkTests, BEVEReadWriteTest)
{
    DataChunk test {.type = sj::type_id_of<DummyStruct>};
    test.data = DummyStruct {};
    auto res = glz::write_beve(test);
    ASSERT_TRUE(res);

    DataChunk test2;
    auto _ = glz::read_beve(test2, *res);
    ASSERT_EQ(test.type, test2.type);
}

} // namespace data_chunk_tests