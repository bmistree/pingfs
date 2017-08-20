#ifndef _DIR_BLOCK_DATA_TEST_
#define _DIR_BLOCK_DATA_TEST_

#include "block_data.pb.h"

#include <pingfs/block/block_data/dir_block_data.hpp>

TEST(DirBlockData, Construct) {
    std::vector<pingfs::BlockId> children;
    children.push_back(55);
    pingfs::DirBlockData data("name", children);
    ASSERT_EQ(data.get_dirname(), "name");
    ASSERT_EQ(data.get_children(), children);
}

TEST(DirBlockData, SerializeDesrialize) {
    std::vector<pingfs::BlockId> children;
    children.push_back(55);
    pingfs::DirBlockData original("name", children);

    pingfs::DirProto proto;
    original.gen_proto(&proto);
    ASSERT_EQ(pingfs::DirBlockData(proto), original);
}

#endif