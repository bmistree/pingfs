#ifndef _MODE_TEST_
#define _MODE_TEST_

#include <pingfs/block/block_data/mode.hpp>
#include <pingfs/block/block_data/read_write_execute.hpp>
#include <block_data.pb.h>


TEST(Mode, SerializeDesrialize) {
    pingfs::Mode m(pingfs::ReadWriteExecute::READ_WRITE_EXECUTE,
        pingfs::ReadWriteExecute::READ_EXECUTE,
        pingfs::ReadWriteExecute::READ);

    pingfs::proto::ModeProto proto;
    m.gen_proto(&proto);
    ASSERT_EQ(pingfs::Mode(proto), m);
}

#endif
