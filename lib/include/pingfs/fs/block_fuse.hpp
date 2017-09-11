#ifndef _BLOCK_FUSE_
#define _BLOCK_FUSE_

#include <pingfs/block/block.hpp>
#include <pingfs/block/block_manager/block_manager.hpp>

#include <unistd.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "fuse_wrapper.hpp"

namespace pingfs {

/**
 * A class that uses a block manager to store file information.
 */
class BlockFuse : public FuseWrapper {
 private:
    using BlockPtr = std::shared_ptr<const Block>;

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
     * {@code path}, or a wrapped nullptr, if none exists.
     */
    BlockPtr resolve_inode(const std::string& path) const;

    /**
     * Adds all blocks between the root block and the final inode
     * block for path to blocks (inclusive for both). The order of
     * blocks added to blocks matches the order that  blocks were encountered
     * in the tree. Specifically, the root block will be in position 0
     * and the final inode will be at the end. If no path exists
     * (i.e., path doesn't exist), does not modify the blocks vector.
     *
     * @param blocks Should be empty when input. If empty when this function
     * exits, there are no sequences of blocks that represent path (i.e.,
     * path is invalid, there's some corruption, or operations have changed
     * the block structure since path was created).
     */
    void get_path(const char* path,
        std::vector<BlockPtr>* blocks) const;

    void get_path_part(const std::string& rel_file_dir_name,
        BlockPtr from_block,
        std::vector<BlockPtr>* block_path) const;

    /**
     * Returns true if {@code path} is a valid path for making a directory
     * for; false otherwise.
     */
    bool mkdir_valid(const char* path,
        std::vector<BlockPtr>* blocks, std::string* dir_to_make);

 private:
    std::shared_ptr<BlockManager> block_manager_;
    BlockPtr root_block_;
    const dev_t dev_;
};


}  // namespace pingfs


#endif
