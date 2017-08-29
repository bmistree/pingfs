#ifndef _DIR_BLOCK_DATA_TEST_
#define _DIR_BLOCK_DATA_TEST_

#include <pingfs/block/block_data/dir_block_data.hpp>
#include <block_data.pb.h>
#include <vector>

TEST(DirBlockData, Construct) {
    std::vector<pingfs::BlockId> children({55u});
    pingfs::DirBlockData data("name", children);
    ASSERT_EQ(data.get_dirname(), "name");
    ASSERT_EQ(data.get_children(), children);
}

TEST(DirBlockData, SerializeDesrialize) {
    pingfs::DirBlockData original("name", { 55u });

    pingfs::proto::DirProto proto;
    original.gen_proto(&proto);
    ASSERT_EQ(pingfs::DirBlockData(proto), original);
}

#endif
