#ifndef _FUSE_FACTORY_
#define _FUSE_FACTORY_

#include <fuse.h>

#include <memory>

namespace pingfs {

class FuseFactory {
 public:
    virtual std::shared_ptr<struct fuse_operations> generate() = 0;
    virtual ~FuseFactory() {
    }
};

}  // namespace pingfs

#endif
