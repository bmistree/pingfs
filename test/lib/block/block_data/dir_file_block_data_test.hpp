#ifndef _DIR_FILE_BLOCK_DATA_TEST_
#define _DIR_FILE_BLOCK_DATA_TEST_

#include <pingfs/block/block_data/dir_file_block_data.hpp>
#include <block_data.pb.h>
#include <vector>

TEST(DirFileBlockData, Construct) {
    std::vector<pingfs::BlockId> children({55u});
    pingfs::DirFileBlockData data("name", children);
    ASSERT_EQ(data.get_name(), "name");
    ASSERT_EQ(data.get_children(), children);
}

TEST(DirFileBlockData, SerializeDesrialize) {
    pingfs::DirFileBlockData original("name", { 55u });

    pingfs::proto::DirFileProto proto;
    original.gen_proto(&proto);
    ASSERT_EQ(pingfs::DirFileBlockData(proto), original);
}

#endif
