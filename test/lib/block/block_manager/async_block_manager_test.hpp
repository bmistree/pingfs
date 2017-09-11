#ifndef _ASYNC_BLOCK_MANAGER_TEST_
#define _ASYNC_BLOCK_MANAGER_TEST_

#include <pingfs/block/block.hpp>
#include <pingfs/block/block_request.hpp>
#include <pingfs/block/block_response.hpp>
#include <pingfs/block/block_manager/async_block_manager.hpp>
#include <pingfs/block/block_data/file_contents_block_data.hpp>

#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "gtest/gtest.h"


class TestAsyncBlockManager : public pingfs::AsyncBlockManager {
 public:
    TestAsyncBlockManager()
      : pingfs::AsyncBlockManager(),
        next_block_id_(0) {
    }

    ~TestAsyncBlockManager() override {
    }

    void free_block(pingfs::BlockId block_id) override {
        throw "Should not get here";
    }

    std::shared_ptr<const pingfs::Block> create_block(
        std::shared_ptr<const pingfs::BlockData> data) override {
        return std::make_shared<const pingfs::Block>(next_block_id_++, data);
    }

 private:
    // Used for assigning unique ids when creating blocks
    pingfs::BlockId next_block_id_;
};


/**
 * Ensures that get get blocks work.
 */
TEST(AsyncBlockManager, GetBlock) {
    TestAsyncBlockManager manager;
    std::shared_ptr<const pingfs::FileContentsBlockData> data =
        std::make_shared<const pingfs::FileContentsBlockData>("abc");

    std::shared_ptr<const pingfs::Block> block =
        manager.create_block(data);

    // Delay some time and then post the block back to
    // manager.
    std::thread t(
        [&manager, &block, &data] () {
            std::this_thread::sleep_for(std::chrono::seconds{1});
            std::shared_ptr<const pingfs::Block> b_ptr =
                std::make_shared<const pingfs::Block>(
                    block->get_id(), data);
            manager.process(b_ptr);
        });
    t.detach();

    std::shared_ptr<const pingfs::BlockResponse> response =
        manager.get_blocks(
            pingfs::BlockRequest({block->get_id()}));
    const std::vector<std::shared_ptr<const pingfs::Block>>& blocks =
        response->get_blocks();
    ASSERT_EQ(blocks.size(), 1u);
    ASSERT_EQ(*blocks[0], *block);
}


#endif
