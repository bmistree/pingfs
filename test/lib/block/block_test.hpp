#ifndef _BLOCK_TEST_
#define _BLOCK_TEST_

#include <block_data.pb.h>

#include <pingfs/block/block.hpp>
#include <pingfs/block/block_data/dir_block_data.hpp>

#include <string>
#include <vector>

#include "gtest/gtest.h"


std::shared_ptr<const pingfs::BlockData> test_dir_data(
    const std::string& dirname) {
    std::vector<pingfs::BlockId> children;
    return std::make_shared<const pingfs::DirBlockData>(
        pingfs::DirBlockData(dirname, children));
}

/**
 * Ensures that created blocks' equality overrides work.
 */
TEST(Block, EqualityOverride) {
    std::shared_ptr<const pingfs::BlockData> data = test_dir_data("dirname");
    pingfs::Block block_a(1u, data);
    pingfs::Block block_b(1u, data);
    ASSERT_EQ(block_a, block_b);
}

/**
 * Ensures that created blocks' inequality overrides work for
 * blocks with different data, but the same block ids.
 */
TEST(Block, InequalityOverrideSameBlockId) {
    std::shared_ptr<const pingfs::BlockData> data_a = test_dir_data("a");
    std::shared_ptr<const pingfs::BlockData> data_b = test_dir_data("b");
    pingfs::Block block_a(1u, data_a);
    pingfs::Block block_b(1u, data_b);
    ASSERT_NE(block_a, block_b);
}

/**
 * Ensures that created blocks' inequality overrides work for
 * blocks with the same ids, but different data.
 */
TEST(Block, InequalityOverrideSameData) {
    std::shared_ptr<const pingfs::BlockData> data = test_dir_data("data");
    pingfs::Block block_a(1u, data);
    pingfs::Block block_b(2u, data);
    ASSERT_NE(block_a, block_b);
}


#endif
