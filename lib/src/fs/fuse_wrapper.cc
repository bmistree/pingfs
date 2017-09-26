#include <pingfs/fs/fuse_wrapper.hpp>

namespace pingfs {

FuseWrapper::FuseWrapper() {
}

FuseWrapper::~FuseWrapper() {
}

void FuseWrapper::set_global_wrapper() {
    fuse_wrapper::global_wrapper = this;
}

std::shared_ptr<struct fuse_operations> FuseWrapper::generate() {
    std::shared_ptr<struct fuse_operations> ops =
        std::make_shared<struct fuse_operations>();
    ops->getattr = fuse_wrapper::getattr;
    ops->mkdir = fuse_wrapper::mkdir;
    ops->rmdir = fuse_wrapper::rmdir;
    ops->readdir = fuse_wrapper::readdir;
    ops->read = fuse_wrapper::read;
    return ops;
}


namespace fuse_wrapper {

int getattr(const char *path, struct stat *stbuf) {
    return global_wrapper->getattr(path, stbuf);
}

int mkdir(const char *path, mode_t mode) {
    return global_wrapper->mkdir(path, mode);
}

int rmdir(const char *path) {
    return global_wrapper->rmdir(path);
}

int readdir(const char *path, void *buf,
    fuse_fill_dir_t filler, off_t offset,
    struct fuse_file_info *fi) {
    return global_wrapper->readdir(path, buf, filler, offset, fi);
}

int read(const char *path, char *buffer, size_t size,
    off_t offset, struct fuse_file_info *fi) {
    return global_wrapper->read(path, buffer, size, offset, fi);
}

}  // namespace fuse_wrapper


}  // namespace pingfs
