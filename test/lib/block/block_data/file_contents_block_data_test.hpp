#ifndef _FILE_CONTENTS_BLOCK_DATA_TEST_
#define _FILE_CONTENTS_BLOCK_DATA_TEST_

#include "block_data.pb.h"
#include <memory>

#include <pingfs/block/block_data/file_contents_block_data.hpp>

TEST(FileContentsBlockData, SerializeDesrialize) {
    pingfs::FileContentsBlockData original(
        std::make_shared<const std::string>("some data"));
    pingfs::FileContentsProto proto;
    original.gen_proto(&proto);
    ASSERT_EQ(pingfs::FileContentsBlockData(proto), original);
}

#endif