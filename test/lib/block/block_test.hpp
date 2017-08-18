#ifndef _BLOCK_TEST_
#define _BLOCK_TEST_

#include <pingfs/block/block.hpp>

#include <string>

#include "gtest/gtest.h"

/**
 * Ensures that created blocks' equality overrides work.
 */
TEST(Block, EqualityOverride) {
    std::string test_data = "testing";
    pingfs::Block block_a(1u, test_data);
    pingfs::Block block_b(1u, test_data);
    ASSERT_EQ(block_a, block_b);
}

/**
 * Ensures that created blocks' inequality overrides work for
 * blocks with different data, but the same block ids.
 */
TEST(Block, InequalityOverrideSameBlockId) {
    std::string data_a = "a";
    std::string data_b = "b";
    pingfs::Block block_a(1u, data_a);
    pingfs::Block block_b(1u, data_b);
    ASSERT_NE(block_a, block_b);
}

/**
 * Ensures that created blocks' inequality overrides work for
 * blocks with the same ids, but different data.
 */
TEST(Block, InequalityOverrideSameData) {
    std::string data = "data";
    pingfs::Block block_a(1u, data);
    pingfs::Block block_b(2u, data);
    ASSERT_NE(block_a, block_b);
}


#endif
