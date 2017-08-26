#ifndef _FILE_START_BLOCK_DATA_TEST_
#define _FILE_START_BLOCK_DATA_TEST_

#include <block_data.pb.h>
#include <pingfs/block/block_data/file_start_block_data.hpp>

#include <vector>

TEST(FileStartBlockData, SerializeDesrialize) {
    pingfs::FileStartBlockData original("test", { 3u, 4u, 88u });

    pingfs::FileStartProto proto;
    original.gen_proto(&proto);
    ASSERT_EQ(pingfs::FileStartBlockData(proto), original);
}

#endif
