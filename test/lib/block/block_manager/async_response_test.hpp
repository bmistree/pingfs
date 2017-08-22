#ifndef _ASYNC_RESPONSE_TEST_
#define _ASYNC_RESPONSE_TEST_

#include <pingfs/block/block_data/block_data.hpp>
#include <pingfs/block/block_data/file_contents_block_data.hpp>
#include <pingfs/block/block_manager/async_response.hpp>

#include <future>
#include <memory>
#include <string>
#include <vector>

#include "gtest/gtest.h"


TEST(AsyncResponse, TestEmpty) {
    std::vector<pingfs::BlockId> wanted_blocks;
    pingfs::BlockRequest request(wanted_blocks);
    pingfs::AsyncResponse response(request);
    std::shared_ptr<const pingfs::BlockResponse> block_response =
        response.get_future().get();
    ASSERT_TRUE(block_response->get_blocks().empty());
}

TEST(AsyncResponse, TestUpdate) {
    std::vector<pingfs::BlockId> wanted_blocks;
    wanted_blocks.push_back(5);
    pingfs::BlockRequest request(wanted_blocks);
    pingfs::AsyncResponse response(request);

    std::shared_ptr<const pingfs::FileContentsBlockData> block_data =
        std::make_shared<const pingfs::FileContentsBlockData>(
            pingfs::FileContentsBlockData(
                std::make_shared<const std::string>("some data")));

    std::shared_ptr<const pingfs::Block> block =
        std::make_shared<const pingfs::Block>(pingfs::Block(5, block_data));

    response.update(block);
    const std::vector<std::shared_ptr<const pingfs::Block>> response_blocks =
        response.get_future().get()->get_blocks();

    ASSERT_EQ(response_blocks.size(), 1u);
    ASSERT_EQ(*response_blocks[0], *block);
}

#endif
