#ifndef _BLOCK_FUSE_TEST_
#define _BLOCK_FUSE_TEST_

#include <unistd.h>

#include <pingfs/fs/block_fuse.hpp>

#include <memory>
#include <string>

#include "gtest/gtest.h"

static void verify_mkdir_fails(const std::string& dir_to_make) {
    std::shared_ptr<pingfs::MemoryBlockManager> block_manager =
        std::make_shared<pingfs::MemoryBlockManager>();

    pingfs::BlockFuse block_fuse(block_manager, 55);
    pingfs::Mode mode(
        pingfs::ReadWriteExecute::READ_WRITE_EXECUTE,
        pingfs::ReadWriteExecute::READ_WRITE_EXECUTE,
        pingfs::ReadWriteExecute::READ_WRITE_EXECUTE,
        pingfs::FileType::DIR);
    ASSERT_NE(block_fuse.mkdir(dir_to_make.c_str(), mode.to_mode_t()), 0);
}

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

TEST(BlockFuse, MkdirFailsPathDoesNotExist) {
    verify_mkdir_fails("/test/dne");
}

TEST(BlockFuse, MkdirFailsPathAlreadyExists) {
    verify_mkdir_fails("/");
}

TEST(BlockFuse, MkdirSucceeds) {
    std::shared_ptr<pingfs::MemoryBlockManager> block_manager =
        std::make_shared<pingfs::MemoryBlockManager>();

    pingfs::BlockFuse block_fuse(block_manager, 55);
    pingfs::Mode mode(
        pingfs::ReadWriteExecute::READ_WRITE_EXECUTE,
        pingfs::ReadWriteExecute::READ_WRITE_EXECUTE,
        pingfs::ReadWriteExecute::READ_WRITE_EXECUTE,
        pingfs::FileType::DIR);
    ASSERT_EQ(block_fuse.mkdir("/test", mode.to_mode_t()), 0);

    struct stat stbuf;
    ASSERT_EQ(block_fuse.getattr("/test", &stbuf), 0);
}

#endif
