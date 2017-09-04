#ifndef _BLOCK_FUSE_TEST_
#define _BLOCK_FUSE_TEST_

#include <unistd.h>

#include <pingfs/fs/block_fuse.hpp>

#include <memory>

#include "gtest/gtest.h"


TEST(BlockFuse, GetAttrRoot) {
    std::shared_ptr<pingfs::MemoryBlockManager> block_manager =
        std::make_shared<pingfs::MemoryBlockManager>();

    pingfs::BlockFuse bf(block_manager, 55);
    struct stat stat;
    ASSERT_EQ(bf.getattr("/", &stat), 0);
    // dev should match dev bf was constructed with
    ASSERT_EQ(stat.st_dev, 55u);

    // should be a directory and u=rwx, g=rx, o=x
    ASSERT_EQ(stat.st_mode,
        static_cast<mode_t>(S_IFDIR | S_IRWXU | S_IRGRP |
            S_IXGRP | S_IXOTH));
}

#endif
