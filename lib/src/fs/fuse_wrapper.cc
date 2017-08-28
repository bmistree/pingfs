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
    return ops;
}


namespace fuse_wrapper {

int getattr(const char *path, struct stat *stbuf) {
    return global_wrapper->getattr(path, stbuf);
}

}  // namespace fuse_wrapper


}  // namespace pingfs
