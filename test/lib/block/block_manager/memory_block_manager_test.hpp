#ifndef _MEMORY_BLOCK_MANAGER_TEST_
#define _MEMORY_BLOCK_MANAGER_TEST_

#include <pingfs/block/block.hpp>
#include <pingfs/block/block_request.hpp>
#include <pingfs/block/block_response.hpp>
#include <pingfs/block/block_manager/memory_block_manager.hpp>
#include <pingfs/block/block_data/file_contents_block_data.hpp>

#include <memory>
#include <string>
#include <vector>

#include "gtest/gtest.h"


/**
 * Ensures that created blocks maintain data.
 */
TEST(MemoryBlockManager, CreateBlock) {
    pingfs::MemoryBlockManager manager;
    std::shared_ptr<const pingfs::FileContentsBlockData> test_data =
        std::make_shared<const pingfs::FileContentsBlockData>("testing");
    std::shared_ptr<const pingfs::Block> block =
        manager.create_block(test_data);
    ASSERT_EQ(*test_data, *block->get_data());
}

/**
 * Ensures that we can retrieve a block after creating it.
 */
TEST(MemoryBlockManager, RetrieveBlock) {
    pingfs::MemoryBlockManager manager;

    std::shared_ptr<const pingfs::FileContentsBlockData> test_data =
        std::make_shared<const pingfs::FileContentsBlockData>("testing");
    std::shared_ptr<const pingfs::Block> created_block =
        manager.create_block(test_data);
    pingfs::BlockRequest block_request({created_block->get_id() });

    std::shared_ptr<const pingfs::BlockResponse> response =
        manager.get_blocks(block_request);

    const std::vector<std::shared_ptr<const pingfs::Block>>& retrieved_blocks =
        response->get_blocks();

    ASSERT_EQ(retrieved_blocks.size(), 1u);
    ASSERT_EQ(*(retrieved_blocks[0]), *created_block);
}

#endif
