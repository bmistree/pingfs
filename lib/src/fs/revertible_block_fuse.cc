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

int RevertibleBlockFuse::mkdir(const char *path, mode_t mode) {
    // FIXME: there's probably a better way to do
    // this than creating a string directly.
    std::string path_str(path);
    std::string sentinel(REVERT_DIR_SENTINEL);

    auto res = std::mismatch(
        sentinel.begin(),
        sentinel.end(),
        path_str.begin());
    if (res.first == sentinel.end()) {
        // Create a checkpoint.
        checkpoints_[path_str] = checkpoint();
    }
    return BlockFuse::mkdir(path, mode);
}

int RevertibleBlockFuse::rmdir(const char *path) {
    auto iter = checkpoints_.find(std::string(path));
    if (iter == checkpoints_.end()) {
        return BlockFuse::rmdir(path);
    }
    revert(iter->second);
    checkpoints_.erase(iter);
}


}  // namespace pingfs
