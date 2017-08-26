#ifndef _LINK_BLOCK_DATA_TEST_
#define _LINK_BLOCK_DATA_TEST_

#include <block_data.pb.h>
#include <pingfs/block/block_data/link_block_data.hpp>
#include <vector>

TEST(LinkBlockData, SerializeDesrialize) {
    pingfs::LinkBlockData original({ 3u, 4u, 88u });

    pingfs::LinkProto proto;
    original.gen_proto(&proto);
    ASSERT_EQ(pingfs::LinkBlockData(proto), original);
}

#endif
