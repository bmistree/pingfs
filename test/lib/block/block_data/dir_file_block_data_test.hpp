#ifndef _DIR_FILE_BLOCK_DATA_TEST_
#define _DIR_FILE_BLOCK_DATA_TEST_

#include <pingfs/block/block_data/dir_file_block_data.hpp>
#include <block_data.pb.h>
#include <vector>

pingfs::Stat get_test_stat() {
    return pingfs::Stat(
        pingfs::Mode(pingfs::ReadWriteExecute::READ,
            pingfs::ReadWriteExecute::NONE,
            pingfs::ReadWriteExecute::EXECUTE,
            pingfs::FileType::DIR),
        0 /* uid */,
        0 /* gid */,
        100 /* size */,
        52 /* access_time */,
        55 /* mod_time */,
        56 /* status_change_time */);
}

TEST(DirFileBlockData, Construct) {
    std::vector<pingfs::BlockId> children({55u});
    pingfs::Stat stat = get_test_stat();
    pingfs::DirFileBlockData data("name", stat, children);
    ASSERT_EQ(data.get_name(), "name");
    ASSERT_EQ(data.get_stat(), stat);
    ASSERT_EQ(data.get_children(), children);
}

TEST(DirFileBlockData, SerializeDesrialize) {
    pingfs::DirFileBlockData original("name", get_test_stat(), { 55u });

    pingfs::proto::DirFileProto proto;
    original.gen_proto(&proto);
    ASSERT_EQ(pingfs::DirFileBlockData(proto), original);
}

#endif
