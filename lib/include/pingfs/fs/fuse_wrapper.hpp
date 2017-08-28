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
    virtual int getattr(const char *path, struct stat *stbuf) = 0;
    virtual ~FuseWrapper();
};

namespace fuse_wrapper {

static FuseWrapper* global_wrapper = nullptr;
int getattr(const char *path, struct stat *stbuf);

}  // namespace fuse_wrapper


}  // namespace pingfs

#endif
