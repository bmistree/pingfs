#ifndef _BLOCK_FUSE_
#define _BLOCK_FUSE_

#include <pingfs/block/block.hpp>
#include <pingfs/block/block_manager/block_manager.hpp>

#include <unistd.h>

#include <string>
#include <vector>

#include "fuse_wrapper.hpp"

namespace pingfs {

/**
 * A class that uses a block manager to store file information.
 */
class BlockFuse : public FuseWrapper {
 public:
    explicit BlockFuse(std::shared_ptr<BlockManager> block_manager,
        dev_t dev);
    virtual ~BlockFuse();

    int getattr(const char *path, struct stat *stbuf) override;
    int mkdir(const char *path, mode_t mode) override;
    int rmdir(const char *path) override;

 private:
    /**
     * Returns the Block corresponding to the inode for
     * {@code rel_file_dir_name}, or a wrapped nullptr, if none exists.
     *
     * Starts search from the BlockIds in {@code vec} and runs through
     * all file links until found.
     *
     * Note that this method special-cases handling for the root block.
     * If {@code rel_file_dir_name} is /, then this method returns
     * it directly.
     */
    std::shared_ptr<const Block> resolve_inode(
        std::vector<BlockId>* vec,
        const std::string& rel_file_dir_name) const;

 private:
    std::shared_ptr<BlockManager> block_manager_;
    Block root_block_;
    const dev_t dev_;
};


}  // namespace pingfs


#endif
