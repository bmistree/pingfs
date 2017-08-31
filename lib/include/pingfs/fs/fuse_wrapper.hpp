#ifndef _FUSE_WRAPPER_
#define _FUSE_WRAPPER_

#include "fuse_factory.hpp"

#include <memory>

namespace pingfs {

class FuseWrapper : public FuseFactory {
 public:
    FuseWrapper();
    void set_global_wrapper();
    std::shared_ptr<struct fuse_operations> generate() override;
    virtual ~FuseWrapper();

    virtual int getattr(const char *path, struct stat *stbuf) = 0;
    virtual int mkdir(const char *path, mode_t mode) = 0;
    virtual int rmdir(const char *path) = 0;
};

namespace fuse_wrapper {

static FuseWrapper* global_wrapper = nullptr;

int getattr(const char *path, struct stat *stbuf);
int mkdir(const char *path, mode_t mode);
int rmdir(const char *path);

}  // namespace fuse_wrapper


}  // namespace pingfs

#endif