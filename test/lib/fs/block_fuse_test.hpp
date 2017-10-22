#ifndef _BLOCK_FUSE_TEST_
#define _BLOCK_FUSE_TEST_

#include <errno.h>
#include <unistd.h>

#include <pingfs/fs/block_fuse.hpp>

#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "gtest/gtest.h"

static pingfs::Mode gen_test_mode() {
    return pingfs::Mode(
        pingfs::ReadWriteExecute::READ_WRITE_EXECUTE,
        pingfs::ReadWriteExecute::READ_WRITE_EXECUTE,
        pingfs::ReadWriteExecute::READ_WRITE_EXECUTE,
        pingfs::FileType::DIR);
}

static void verify_path_dne(int return_code) {
    ASSERT_EQ(return_code, -ENOENT);
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
    verify_path_dne(
        block_fuse.read(
            "/a", &buffer, 0 /* size */, 0 /* offset */,
            &info));
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

TEST(BlockFuse, WriteFailsDne) {
    std::shared_ptr<pingfs::MemoryBlockManager> block_manager =
        std::make_shared<pingfs::MemoryBlockManager>();
    pingfs::BlockFuse block_fuse(block_manager, 55);
    const char* test_buffer = "nothing";
    struct fuse_file_info info;
    verify_path_dne(
        block_fuse.write(
            "/a/b/c/d.txt", test_buffer, 0 /* size */, 0 /* offset */,
            &info));
}

static void verify_read(
    std::shared_ptr<pingfs::BlockFuse> block_fuse,
    const std::string& filename, off_t offset,
    const std::string& expected) {

    struct fuse_file_info info;
    char* buffer = new char[expected.size()];
    block_fuse->read(filename.c_str(),
        buffer,
        expected.size(),
        offset,
        &info);

    // Compare to ensure that the read string was the same
    // as what was written.
    for (std::size_t i = 0; i < expected.size(); ++i) {
        ASSERT_EQ(expected[i], buffer[i]);
    }
    delete buffer;
}

static void test_write_read(const std::string& filename,
    const std::string& data_to_write,
    std::shared_ptr<pingfs::BlockFuse> block_fuse) {
    struct fuse_file_info info;

    // Testing that initial write works
    ASSERT_EQ(
        block_fuse->write(
            filename.c_str(), data_to_write.c_str(),
            data_to_write.size(), 0 /* offset */,
            &info),
        static_cast<int>(data_to_write.size()));

    // Ensure that getattr shows that the file exists
    struct stat stbuf;
    ASSERT_EQ(block_fuse->getattr(filename.c_str(), &stbuf), 0);

    // Ensure that read same data out
    verify_read(block_fuse, filename, 0 /* offset */,
        data_to_write);
}

static void create_file(
    const std::string& filename,
    std::shared_ptr<pingfs::BlockFuse>* block_fuse) {
    std::shared_ptr<pingfs::MemoryBlockManager> block_manager =
        std::make_shared<pingfs::MemoryBlockManager>();
    *block_fuse =
        std::make_shared<pingfs::BlockFuse>(
            pingfs::BlockFuse(block_manager, 55));
    struct fuse_file_info info;

    ASSERT_EQ(
        (*block_fuse)->create(filename.c_str(),
            pingfs::Mode(pingfs::ReadWriteExecute::READ_WRITE,
                pingfs::ReadWriteExecute::READ_WRITE,
                pingfs::ReadWriteExecute::READ_WRITE,
                pingfs::FileType::REGULAR).to_mode_t(),
            &info),
        0);
}

static void test_create_write_read(
    const std::vector<std::string>& to_write_vec) {
    std::string filename = "/a.txt";
    std::shared_ptr<pingfs::BlockFuse> block_fuse;
    create_file(filename, &block_fuse);
    for (auto iter = to_write_vec.cbegin();
         iter != to_write_vec.cend(); ++iter) {
        test_write_read(filename, *iter, block_fuse);
    }
}

TEST(BlockFuse, WriteReadSucceeds) {
    std::vector<std::string> to_write_vec;
    to_write_vec.push_back("test");
    test_create_write_read(to_write_vec);
}

TEST(BlockFuse, WriteEmptySucceeds) {
    std::vector<std::string> to_write_vec;
    to_write_vec.push_back("");
    test_create_write_read(to_write_vec);
}

TEST(BlockFuse, MultipleWrites) {
    std::vector<std::string> to_write_vec;
    to_write_vec.push_back("a");
    to_write_vec.push_back("b");
    to_write_vec.push_back("c");
    to_write_vec.push_back("");
    test_create_write_read(to_write_vec);
}

static void verify_create_write_read_to_file(
    std::shared_ptr<pingfs::BlockFuse>* block_fuse,
    const std::string& filename,
    const std::string& content) {

    create_file(filename, block_fuse);
    struct fuse_file_info info;

    // Writerite content into file and ensure that it's there
    ASSERT_EQ((*block_fuse)->write(filename.c_str(),
            content.c_str(),
            content.size(),
            0 /* offset */,
            &info),
        static_cast<int>(content.size()));
    verify_read(*block_fuse, filename, 0 /* offset */,
        content);
}

TEST(BlockFuse, WriteOffsetMiddle) {
    std::string filename = "/a.txt";
    std::shared_ptr<pingfs::BlockFuse> block_fuse;

    // Initially write content into file and ensure
    // that it's there
    verify_create_write_read_to_file(
        &block_fuse, filename, "abcdefgh");

    // Write to offset of file and ensure content
    // is updated
    struct fuse_file_info info;
    std::string expected_update = "axydefgh";
    ASSERT_EQ(block_fuse->write(filename.c_str(),
            "xy",
            2 /* size to write */,
            1 /* offset */,
            &info),
        2 /* num bytes written */);

    verify_read(block_fuse, filename, 0 /* offset */,
        expected_update);
}

TEST(BlockFuse, WriteOffsetEnd) {
    std::string filename = "/a.txt";
    std::shared_ptr<pingfs::BlockFuse> block_fuse;

    // Initially write content into file and ensure
    // that it's there
    verify_create_write_read_to_file(
        &block_fuse, filename, "abcdefgh");

    // Write to offset of file and ensure content
    // is updated
    struct fuse_file_info info;
    std::string expected_update = "abcdefgxy";
    ASSERT_EQ(block_fuse->write(filename.c_str(),
            "xy",
            2 /* size to write */,
            7 /* offset */,
            &info),
        2 /* num bytes written */);

    verify_read(block_fuse, filename, 0 /* offset */,
        expected_update);
}

TEST(BlockFuse, ReadPastEnd) {
    std::string filename = "/a.txt";
    std::shared_ptr<pingfs::BlockFuse> block_fuse;

    // Initially write content into file and ensure
    // that it's there
    std::string actual_contents = "abcdefgh";
    verify_create_write_read_to_file(
        &block_fuse, filename, actual_contents);

    struct fuse_file_info info;
    char read_buffer[128];
    ASSERT_EQ(block_fuse->read(filename.c_str(),
            read_buffer,
            128 /* size */,
            0 /* offset */,
            &info),
        static_cast<int>(actual_contents.size()));

    std::string read_contents(read_buffer, actual_contents.size());
    ASSERT_EQ(read_contents, actual_contents);
}

// FIXME: Still must test reads with offsets

// FIXME: Still must test that handle large files (want to
// test that we handle tree branches correctly).


#endif
