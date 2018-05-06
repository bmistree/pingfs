#include <pingfs/fs/distributed_block_fuse.hpp>

#include <pingfs/fs/block_util.hpp>
#include <pingfs/fs/fs_util.hpp>

namespace pingfs {

using BlockPtr = std::shared_ptr<const Block>;


DistributedBlockFuse::DistributedBlockFuse(std::shared_ptr<BlockManager> block_manager,
    dev_t dev,
    std::shared_ptr<UpdatingIdSupplier> updating_id_supplier)
 : BlockFuse(block_manager, dev),
   updating_id_supplier_(updating_id_supplier) {
}

DistributedBlockFuse::~DistributedBlockFuse() {
}

/**
 * Return true if {@code block} is the root of an entire
 * filesystem.
 */
static bool is_root_block(const BlockPtr& block) {
    std::shared_ptr<const DirFileBlockData> resolved_data =
        block_util::try_cast_dir_file(block);
    if (resolved_data) {
        return false;
    }
    if (!resolved_data->is_dir()) {
        return false;
    }

    return resolved_data->get_name() == FsUtil::get_separator();
}

void DistributedBlockFuse::process(const BlockPtr& block) {
    updating_id_supplier_->update_if_less_than(block->get_id());
    if (!is_root_block(block)) {
        return;
    }

    BlockPtr root = get_root_block();
    if (root->get_id() > block->get_id()) {
        return;
    }
    set_root_block(block);
}

}  // namespace pingfs
