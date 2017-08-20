#ifndef _FILE_START_BLOCK_DATA_TEST_
#define _FILE_START_BLOCK_DATA_TEST_

#include <block_data.pb.h>
#include <pingfs/block/block_data/file_start_block_data.hpp>

#include <vector>

TEST(FileStartBlockData, SerializeDesrialize) {
    std::vector<pingfs::BlockId> children;
    children.push_back(3);
    children.push_back(4);
    children.push_back(88);
    pingfs::FileStartBlockData original("test", children);

    pingfs::FileStartProto proto;
    original.gen_proto(&proto);
    ASSERT_EQ(pingfs::FileStartBlockData(proto), original);
}

#endif
