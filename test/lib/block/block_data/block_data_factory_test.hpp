#ifndef _BLOCK_DATA_FACTORY_TEST_
#define _BLOCK_DATA_FACTORY_TEST_

#include <pingfs/block/block_data/block_data_factory.hpp>
#include <pingfs/block/block_data/dir_file_block_data.hpp>
#include <pingfs/block/block_data/file_contents_block_data.hpp>
#include <pingfs/block/block_data/link_block_data.hpp>
#include <block_data.pb.h>

TEST(BlockDataFactory, DeserializeDirFile) {
    pingfs::DirFileBlockData block_data(
        "name",
        pingfs::Stat(
            pingfs::Mode(pingfs::ReadWriteExecute::READ,
                pingfs::ReadWriteExecute::NONE,
                pingfs::ReadWriteExecute::EXECUTE,
                pingfs::FileType::DIR),
            0 /* uid */,
            0 /* gid */,
            100 /* size */,
            52 /* access_time */,
            55 /* mod_time */,
            56 /* status_change_time */),
        {52u});

    pingfs::proto::BlockDataProto proto;
    block_data.gen_block_data_proto(&proto);
    ASSERT_EQ(
        *pingfs::BlockDataFactory::generate(proto),
        block_data);
}

TEST(BlockDataFactory, DeserializeLink) {
    pingfs::LinkBlockData block_data({3u, 4u, 5u });
    pingfs::proto::BlockDataProto proto;
    block_data.gen_block_data_proto(&proto);
    ASSERT_EQ(
        *pingfs::BlockDataFactory::generate(proto),
        block_data);
}

TEST(BlockDataFactory, DeserializeFileContents) {
    pingfs::FileContentsBlockData block_data("Example data");
    pingfs::proto::BlockDataProto proto;
    block_data.gen_block_data_proto(&proto);
    ASSERT_EQ(
        *pingfs::BlockDataFactory::generate(proto),
        block_data);
}

#endif
