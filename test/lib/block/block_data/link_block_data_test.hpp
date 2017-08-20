#ifndef _LINK_BLOCK_DATA_TEST_
#define _LINK_BLOCK_DATA_TEST_

#include <block_data.pb.h>
#include <pingfs/block/block_data/link_block_data.hpp>
#include <vector>

TEST(LinkBlockData, SerializeDesrialize) {
    std::vector<pingfs::BlockId> children;
    children.push_back(3);
    children.push_back(4);
    children.push_back(88);
    pingfs::LinkBlockData original(children);

    pingfs::LinkProto proto;
    original.gen_proto(&proto);
    ASSERT_EQ(pingfs::LinkBlockData(proto), original);
}

#endif
