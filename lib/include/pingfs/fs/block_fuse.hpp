#ifndef _BLOCK_FUSE_
#define _BLOCK_FUSE_

#include <pingfs/block/block.hpp>
#include <pingfs/block/block_manager/block_manager.hpp>

#include "fuse_wrapper.hpp"

namespace pingfs {

/**
 * A class that uses a block manager to store file information.
 */
class BlockFuse : public FuseWrapper {
 public:
    explicit BlockFuse(std::shared_ptr<BlockManager> block_manager);
    virtual ~BlockFuse();

    int getattr(const char *path, struct stat *stbuf) override;
    int mkdir(const char *path, mode_t mode) override;
    int rmdir(const char *path) override;

 private:
    std::shared_ptr<BlockManager> block_manager_;
    Block root_block_;
};


}  // namespace pingfs


#endif
