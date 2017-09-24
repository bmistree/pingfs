#include <pingfs/fs/fuse_wrapper.hpp>

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <memory>

/**
 * Tests building a dummy fuse filesystem.
 */

class DummyFactory : public pingfs::FuseWrapper {
 public:
    DummyFactory()
      : FuseWrapper() {
    }

    /**
     * Regardless of path put in, generate the same file
     * information.
     *
     * @return error code or 0 if successful.
     */
    int getattr(const char *path, struct stat *stbuf) override {
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

    int mkdir(const char *path, mode_t mode) override {
        return 0;
    }
    int rmdir(const char *path) override {
        return 0;
    }

    int readdir(const char *path, void *buf,
        fuse_fill_dir_t filler, off_t offset,
        struct fuse_file_info *fi) override {
        throw "Unsupported";
    }

    int read(const char *path, char *buffer, size_t size,
        off_t offset, struct fuse_file_info *fi) override {
        throw "Unsupported";
    }
};

int main(int argc, char** argv) {
    DummyFactory factory;
    factory.set_global_wrapper();
    std::shared_ptr<struct fuse_operations> ops =
        factory.generate();
    return fuse_main(argc, argv, ops.get());
}
