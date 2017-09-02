#ifndef _FILE_TYPE_TEST_
#define _FILE_TYPE_TEST_

#include <pingfs/block/block_data/file_type.hpp>
#include <block_data.pb.h>

static void test_serialize_deserialize(
    const pingfs::FileType& file_type) {
    pingfs::proto::FileTypeProto proto;
    pingfs::FileTypeFactory::gen_proto(&proto, file_type);
    ASSERT_EQ(pingfs::FileTypeFactory::from_proto(proto), file_type);
}


TEST(FileType, SerializeDesrialize) {
    test_serialize_deserialize(pingfs::FileType::DIR);
    test_serialize_deserialize(pingfs::FileType::REGULAR);
}

#endif
