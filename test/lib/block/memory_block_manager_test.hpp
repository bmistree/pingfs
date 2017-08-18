#ifndef _MEMORY_BLOCK_MANAGER_TEST_
#define _MEMORY_BLOCK_MANAGER_TEST_

#include <pingfs/block/block.hpp>
#include <pingfs/block/memory_block_manager.hpp>

#include <string>

#include "gtest/gtest.h"

/**
 * Ensures that created blocks maintain data.
 */
TEST(MemoryBlockManager, CreateBlock) {
    pingfs::MemoryBlockManager manager;

    std::string test_data = "testing";

    const pingfs::Block block = manager.create_block(test_data);
    ASSERT_EQ(test_data, *block.get_data());
}

#endif
