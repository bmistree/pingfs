#ifndef _REVERTIBLE_BLOCK_FUSE_
#define _REVERTIBLE_BLOCK_FUSE_

#include <pingfs/block/block.hpp>
#include <pingfs/block/block_data/dir_file_block_data.hpp>
#include <pingfs/block/block_data/file_contents_block_data.hpp>
#include <pingfs/block/block_data/link_block_data.hpp>
#include <pingfs/block/block_manager/block_manager.hpp>

#include <unistd.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "block_fuse.hpp"
#include "fuse_wrapper.hpp"

namespace pingfs {

class RevertibleBlockFuse : public BlockFuse {
 public:
    explicit RevertibleBlockFuse(std::shared_ptr<BlockManager> block_manager,
        dev_t dev);
    virtual ~RevertibleBlockFuse();

    /**
     * Returns the root block at this time, and sets
     * checkpoint_requested_.
     */
    BlockPtr checkpoint();

    /**
     * Resets the root block to reverted_root, which
     * reverts the filesystem to a previous state.
     */
    void revert(BlockPtr reverted_root);

    int mkdir(const char *path, mode_t mode) override;
    int rmdir(const char *path) override;

 protected:
    /**
     * Asks block_manager_ to free a block if
     * checkpoint_requested_ is not set.
     */
    void free_block(BlockId block_id) override;

 private:

    /**
     * If a directory is prefixed with this, then
     * take a restore point when it's created. Similarly
     * revert when the dir is deleted.
     */
    static constexpr const char* REVERT_DIR_SENTINEL =
        "/__checkpoint_";

    /**
     * If true, then do not free any block ids. This is an
     * overly-conservative approach. However, it guarantees
     * that we can always recover checkpointed state.
     */
    bool checkpoint_requested_;

    /**
     * Maps from filenames to the associated restore point.
     */
    std::unordered_map<std::string, BlockPtr> checkpoints_;
};


}  // namespace pingfs


#endif
