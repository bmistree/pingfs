#ifndef _BLOCK_TEST_
#define _BLOCK_TEST_

#include <pingfs/block/block.hpp>

#include <string>

#include "block_data.pb.h"
#include "gtest/gtest.h"


pingfs::BlockDataProto test_dir_proto(const std::string& dirname) {
    pingfs::BlockDataProto data_proto;
    pingfs::DirProto* dir_proto = data_proto.mutable_dir();
    dir_proto->set_dirname(dirname);
    return data_proto;
}

/**
 * Ensures that created blocks' equality overrides work.
 */
TEST(Block, EqualityOverride) {
    pingfs::BlockDataProto data_proto = test_dir_proto("dirname");
    pingfs::Block block_a(1u, data_proto);
    pingfs::Block block_b(1u, data_proto);
    ASSERT_EQ(block_a, block_b);
}

/**
 * Ensures that created blocks' inequality overrides work for
 * blocks with different data, but the same block ids.
 */
TEST(Block, InequalityOverrideSameBlockId) {
    pingfs::BlockDataProto data_a = test_dir_proto("a");
    pingfs::BlockDataProto data_b = test_dir_proto("b");
    pingfs::Block block_a(1u, data_a);
    pingfs::Block block_b(1u, data_b);
    ASSERT_NE(block_a, block_b);
}

/**
 * Ensures that created blocks' inequality overrides work for
 * blocks with the same ids, but different data.
 */
TEST(Block, InequalityOverrideSameData) {
    pingfs::BlockDataProto data = test_dir_proto("data");
    pingfs::Block block_a(1u, data);
    pingfs::Block block_b(2u, data);
    ASSERT_NE(block_a, block_b);
}


#endif
