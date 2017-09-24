#ifndef _BLOCK_FUSE_TEST_
#define _BLOCK_FUSE_TEST_

#include <errno.h>
#include <unistd.h>

#include <pingfs/fs/block_fuse.hpp>

#include <memory>
#include <string>

#include "gtest/gtest.h"

static pingfs::Mode gen_test_mode() {
    return pingfs::Mode(
        pingfs::ReadWriteExecute::READ_WRITE_EXECUTE,
        pingfs::ReadWriteExecute::READ_WRITE_EXECUTE,
        pingfs::ReadWriteExecute::READ_WRITE_EXECUTE,
        pingfs::FileType::DIR);
}

static void verify_path_dne(int return_code) {
    ASSERT_EQ(return_code, -1);
    ASSERT_EQ(errno, ENOENT);
}

static void verify_mkdir_fails(const std::string& dir_to_make) {
    std::shared_ptr<pingfs::MemoryBlockManager> block_manager =
        std::make_shared<pingfs::MemoryBlockManager>();

    pingfs::BlockFuse block_fuse(block_manager, 55);
    ASSERT_NE(block_fuse.mkdir(dir_to_make.c_str(),
            gen_test_mode().to_mode_t()), 0);
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

TEST(BlockFuse, GetAttrDne) {
    std::shared_ptr<pingfs::MemoryBlockManager> block_manager =
        std::make_shared<pingfs::MemoryBlockManager>();

    pingfs::BlockFuse bf(block_manager, 55);
    struct stat stat;
    verify_path_dne(bf.getattr("/a", &stat));
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

    ASSERT_EQ(block_manager->num_blocks(),
        // 1 becuase of root block
        1u);

    ASSERT_EQ(block_fuse.mkdir("/test", gen_test_mode().to_mode_t()), 0);

    struct stat stbuf;
    ASSERT_EQ(block_fuse.getattr("/test", &stbuf), 0);

    ASSERT_EQ(block_manager->num_blocks(),
        // root block + new block
        2u);
}


TEST(BlockFuse, MkdirNestedSucceeds) {
    std::shared_ptr<pingfs::MemoryBlockManager> block_manager =
        std::make_shared<pingfs::MemoryBlockManager>();

    pingfs::BlockFuse block_fuse(block_manager, 55);
    ASSERT_EQ(block_fuse.mkdir("/a", gen_test_mode().to_mode_t()), 0);
    struct stat stbuf;
    ASSERT_EQ(block_fuse.getattr("/a", &stbuf), 0);

    ASSERT_EQ(block_manager->num_blocks(),
        // 2: 1 for root block and 1 for a
        2u);

    ASSERT_EQ(block_fuse.mkdir("/a/b", gen_test_mode().to_mode_t()), 0);
    ASSERT_EQ(block_fuse.getattr("/a/b", &stbuf), 0);
    ASSERT_EQ(block_manager->num_blocks(),
        // 3: 1 for root block, 1 for a, and 1 for b.
        3u);

    ASSERT_EQ(block_fuse.mkdir("/a/b/c", gen_test_mode().to_mode_t()), 0);
    ASSERT_EQ(block_fuse.getattr("/a/b/c", &stbuf), 0);
    ASSERT_EQ(block_manager->num_blocks(),
        // 4: 1 for root block, 1 for a, 1 for b, and 1 for c
        4u);
}

/**
 * Generates a chain of nested directories; deletes one of those
 * directories in the chain and checks that all subsequent
 * directories are also killed.
 */
TEST(BlockFuse, MkdirRmDirIncludingChildren) {
    std::shared_ptr<pingfs::MemoryBlockManager> block_manager =
        std::make_shared<pingfs::MemoryBlockManager>();

    pingfs::BlockFuse block_fuse(block_manager, 55);
    ASSERT_EQ(block_fuse.mkdir("/a", gen_test_mode().to_mode_t()), 0);
    ASSERT_EQ(block_fuse.mkdir("/a/b", gen_test_mode().to_mode_t()), 0);
    ASSERT_EQ(block_fuse.mkdir("/a/b/c", gen_test_mode().to_mode_t()), 0);
    ASSERT_EQ(block_fuse.mkdir("/a/b/c/d", gen_test_mode().to_mode_t()), 0);

    ASSERT_EQ(block_manager->num_blocks(), 5u);

    // Actually remove a directory
    ASSERT_EQ(block_fuse.rmdir("/a/b"), 0);

    ASSERT_EQ(block_manager->num_blocks(), 2u);

    // Checks that deletes all child directories
    struct stat stbuf;
    verify_path_dne(block_fuse.getattr("/a/b/c/d", &stbuf));
    verify_path_dne(block_fuse.getattr("/a/b/c", &stbuf));
    verify_path_dne(block_fuse.getattr("/a/b", &stbuf));

    // Checks that does not delete parent directory
    ASSERT_EQ(block_fuse.getattr("/a", &stbuf), 0);
}

TEST(BlockFuse, RmChildDir) {
    std::shared_ptr<pingfs::MemoryBlockManager> block_manager =
        std::make_shared<pingfs::MemoryBlockManager>();

    pingfs::BlockFuse block_fuse(block_manager, 55);
    ASSERT_EQ(block_fuse.mkdir("/a", gen_test_mode().to_mode_t()), 0);
    ASSERT_EQ(block_fuse.mkdir("/a/b", gen_test_mode().to_mode_t()), 0);

    ASSERT_EQ(block_manager->num_blocks(), 3u);

    // Actually remove a directory
    ASSERT_EQ(block_fuse.rmdir("/a/b"), 0);

    // Checks that deletes target directories
    struct stat stbuf;
    verify_path_dne(block_fuse.getattr("/a/b", &stbuf));

    // Checks that does not delete parent directory
    ASSERT_EQ(block_fuse.getattr("/a", &stbuf), 0);

    // Ensures that we are not leaking blocks and that the
    // right number of blocks are left
    ASSERT_EQ(block_manager->num_blocks(), 2u);
}

TEST(BlockFuse, FailRmRoot) {
    std::shared_ptr<pingfs::MemoryBlockManager> block_manager =
        std::make_shared<pingfs::MemoryBlockManager>();
    pingfs::BlockFuse block_fuse(block_manager, 55);
    ASSERT_NE(block_fuse.rmdir("/"), 0);
}

TEST(BlockFuse, FailRmNonExistent) {
    std::shared_ptr<pingfs::MemoryBlockManager> block_manager =
        std::make_shared<pingfs::MemoryBlockManager>();
    pingfs::BlockFuse block_fuse(block_manager, 55);
    ASSERT_NE(block_fuse.rmdir("/a/b"), 0);
}

TEST(BlockFuse, ReadPathDoesNotExist) {
    std::shared_ptr<pingfs::MemoryBlockManager> block_manager =
        std::make_shared<pingfs::MemoryBlockManager>();
    pingfs::BlockFuse block_fuse(block_manager, 55);
    char buffer;
    struct fuse_file_info info;
    ASSERT_LT(
        block_fuse.read(
            "/a", &buffer, 0 /* size */, 0 /* offset */,
            &info),
        0);
}

TEST(BlockFuse, ReadDirFails) {
    std::shared_ptr<pingfs::MemoryBlockManager> block_manager =
        std::make_shared<pingfs::MemoryBlockManager>();
    pingfs::BlockFuse block_fuse(block_manager, 55);
    char buffer;
    struct fuse_file_info info;
    ASSERT_LT(
        block_fuse.read(
            "/", &buffer, 0 /* size */, 0 /* offset */,
            &info),
        0);
}

#endif
