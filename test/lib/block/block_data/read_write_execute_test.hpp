#ifndef _READ_WRITE_EXECUTE_TEST_
#define _READ_WRITE_EXECUTE_TEST_

#include <pingfs/block/block_data/read_write_execute.hpp>
#include <block_data.pb.h>
#include <vector>

static void test_serialize_deserialize(
    const pingfs::ReadWriteExecute& read_write_execute) {

    pingfs::proto::ReadWriteExecuteProto proto;
    pingfs::ReadWriteExecuteFactory::gen_proto(
        &proto, read_write_execute);

    ASSERT_EQ(
        pingfs::ReadWriteExecuteFactory::from_proto(proto),
        read_write_execute);
}

TEST(ReadWriteExecute, SerializeDesrialize) {
    test_serialize_deserialize(pingfs::ReadWriteExecute::READ);
    test_serialize_deserialize(pingfs::ReadWriteExecute::READ_WRITE);
    test_serialize_deserialize(pingfs::ReadWriteExecute::READ_WRITE_EXECUTE);
    test_serialize_deserialize(pingfs::ReadWriteExecute::WRITE);
    test_serialize_deserialize(pingfs::ReadWriteExecute::WRITE_EXECUTE);
    test_serialize_deserialize(pingfs::ReadWriteExecute::EXECUTE);
}

#endif
