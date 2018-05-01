#include <pingfs/block/block.hpp>
#include <pingfs/fs/revertible_block_fuse.hpp>

#include <memory>

namespace pingfs {

using BlockPtr = std::shared_ptr<const Block>;

RevertibleBlockFuse::RevertibleBlockFuse(
    std::shared_ptr<BlockManager> block_manager,
    dev_t dev)
 : BlockFuse(block_manager, dev),
   checkpoint_requested_(false) {
}

RevertibleBlockFuse::~RevertibleBlockFuse() {
}

BlockPtr RevertibleBlockFuse::checkpoint() {
    checkpoint_requested_ = true;
    return root_block_;
}

void RevertibleBlockFuse::revert(BlockPtr reverted_root) {
    root_block_ = reverted_root;
}

void RevertibleBlockFuse::free_block(BlockId block_id) {
    if (checkpoint_requested_) {
        return;
    }
    block_manager_->free_block(block_id);
}

}  // namespace pingfs
