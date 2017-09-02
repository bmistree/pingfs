#ifndef _MODE_TEST_
#define _MODE_TEST_

#include <pingfs/block/block_data/file_type.hpp>
#include <pingfs/block/block_data/mode.hpp>
#include <pingfs/block/block_data/read_write_execute.hpp>

#include <block_data.pb.h>
#include <sys/stat.h>

static void test_convert_mode_t(mode_t mode) {
    pingfs::Mode m(mode);
    ASSERT_EQ(m.to_mode_t(), mode);
}


TEST(Mode, SerializeDesrialize) {
    pingfs::Mode m(pingfs::ReadWriteExecute::READ_WRITE_EXECUTE,
        pingfs::ReadWriteExecute::READ_EXECUTE,
        pingfs::ReadWriteExecute::READ,
        pingfs::FileType::DIR);

    pingfs::proto::ModeProto proto;
    m.gen_proto(&proto);
    ASSERT_EQ(pingfs::Mode(proto), m);
}

TEST(Mode, ModeT) {
    test_convert_mode_t(
         S_IRUSR | S_IWUSR | S_IXUSR |
         S_IWGRP |
         S_IROTH |
         S_IFDIR);

    test_convert_mode_t(
         S_IWUSR |
         S_IRGRP |
         S_IROTH | S_IXOTH |
         S_IFREG);
}

#endif
