#include <pingfs/fs/fuse_factory.hpp>

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <memory>

/**
 * Tests building a dummy fuse filesystem.
 */


/**
 * Regardless of path put in, generate the same file information.
 *
 * @return error code or 0 if successful.
 */
static int test_getattr(const char *path, struct stat *stbuf) {
    // what physical device this is on
    stbuf->st_dev = 50;
    // unsigned data type representing file serial number.
    stbuf->st_ino = 1;
    // File permissions on file; for now, all can rwx.
    stbuf->st_mode = S_IRWXU | S_IRWXG | S_IRWXO | S_IFDIR;

    // Number of hard links to the file
    stbuf->st_nlink = 2;

    stbuf->st_uid = getuid();
    stbuf->st_gid = getgid();

    // Size of file or directory in bytes.
    stbuf->st_size = 100;

    // Time of last acccess; for now set to epoch
    stbuf->st_atime = time(NULL);
    // Time of last modification; for now, set to epoch
    stbuf->st_mtime = time(NULL);
    // Time of last status change; for now, set to epoch
    stbuf->st_ctime = time(NULL);
    return 0;
}


class DummyFactory : public pingfs::FuseFactory {
 public:
    DummyFactory() {
    }

    std::shared_ptr<struct fuse_operations> generate() override {
        std::shared_ptr<struct fuse_operations> ops =
            std::make_shared<struct fuse_operations>();
        ops->getattr = test_getattr;
        return ops;
    }
};

int main(int argc, char** argv) {
    DummyFactory factory;
    std::shared_ptr<struct fuse_operations> ops =
        factory.generate();
    return fuse_main(argc, argv, ops.get());
}
