#ifndef _STAT_TEST_
#define _STAT_TEST_

#include <pingfs/block/block_data/mode.hpp>
#include <pingfs/block/block_data/read_write_execute.hpp>
#include <pingfs/block/block_data/stat.hpp>
#include <block_data.pb.h>


TEST(Stat, SerializeDesrialize) {
    pingfs::Stat stat(
        pingfs::Mode(pingfs::ReadWriteExecute::READ_WRITE_EXECUTE,
            pingfs::ReadWriteExecute::READ_EXECUTE,
            pingfs::ReadWriteExecute::READ),
        33 /* uid */, 55 /* gid */, 100 /* size */,
        1011 /* access_time */, 1023 /* mod_time */,
        355 /* status_change_time */);

    pingfs::proto::StatProto proto;
    stat.gen_proto(&proto);
    ASSERT_EQ(pingfs::Stat(proto), stat);
}

#endif
