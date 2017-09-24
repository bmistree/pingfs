#ifndef _DIR_FILE_BLOCK_DATA_TEST_
#define _DIR_FILE_BLOCK_DATA_TEST_

#include <pingfs/block/block_data/dir_file_block_data.hpp>
#include <block_data.pb.h>

#include <cassert>
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

TEST(DirFileBlockData, SwapChildren) {
    std::vector<pingfs::BlockId> original_children({ 55u });
    pingfs::DirFileBlockData original(
        "name", get_test_stat(), original_children);

    std::vector<pingfs::BlockId> diff_children({ 56u });
    pingfs::DirFileBlockData diff(original, diff_children);
    assert(original_children != diff_children);

    ASSERT_NE(diff, original);
    ASSERT_EQ(diff.get_name(), original.get_name());
    ASSERT_EQ(diff.get_stat(), original.get_stat());

    ASSERT_EQ(original.get_children(), original_children);
    ASSERT_EQ(diff.get_children(), diff_children);
}

TEST(DirFileBlockData, IsDir) {
    std::vector<pingfs::BlockId> original_children({ 55u });
    pingfs::DirFileBlockData data(
        "name", get_test_stat(), original_children);
    ASSERT_EQ(data.is_dir(), true);
}

TEST(DirFileBlockData, IsNotDir) {
    std::vector<pingfs::BlockId> original_children({ 55u });

    pingfs::Stat stat(
        pingfs::Mode(pingfs::ReadWriteExecute::READ,
            pingfs::ReadWriteExecute::NONE,
            pingfs::ReadWriteExecute::EXECUTE,
            pingfs::FileType::REGULAR),
        0 /* uid */,
        0 /* gid */,
        100 /* size */,
        52 /* access_time */,
        55 /* mod_time */,
        56 /* status_change_time */);

    pingfs::DirFileBlockData data(
        "name", stat, original_children);
    ASSERT_EQ(data.is_dir(), false);
}


#endif
