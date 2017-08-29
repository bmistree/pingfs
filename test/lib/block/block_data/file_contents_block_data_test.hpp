#ifndef _FILE_CONTENTS_BLOCK_DATA_TEST_
#define _FILE_CONTENTS_BLOCK_DATA_TEST_

#include <block_data.pb.h>
#include <pingfs/block/block_data/file_contents_block_data.hpp>

#include <string>

TEST(FileContentsBlockData, SerializeDesrialize) {
    pingfs::FileContentsBlockData original("some data");
    pingfs::proto::FileContentsProto proto;
    original.gen_proto(&proto);
    ASSERT_EQ(pingfs::FileContentsBlockData(proto), original);
}

#endif
