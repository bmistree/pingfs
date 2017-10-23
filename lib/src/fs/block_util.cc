#include <pingfs/block/block.hpp>
#include <pingfs/block/block_data/dir_file_block_data.hpp>
#include <pingfs/block/block_data/file_contents_block_data.hpp>
#include <pingfs/block/block_data/link_block_data.hpp>
#include <pingfs/fs/block_util.hpp>

#include <memory>
#include <unordered_map>
#include <vector>


namespace pingfs {

namespace block_util {

std::shared_ptr<const DirFileBlockData> try_cast_dir_file(
    BlockPtr block_ptr) {
    return std::dynamic_pointer_cast<const DirFileBlockData>(
        block_ptr->get_data());
}

std::shared_ptr<const LinkBlockData> try_cast_link(
    BlockPtr block_ptr) {
    return std::dynamic_pointer_cast<const LinkBlockData>(
        block_ptr->get_data());
}

std::shared_ptr<const FileContentsBlockData> try_cast_contents(
    BlockPtr block_ptr) {
    return std::dynamic_pointer_cast<const FileContentsBlockData>(
        block_ptr->get_data());
}

bool find_path(
    const std::unordered_map<BlockId, BlockPtr>& retrieved_blocks,
    BlockPtr from_block,
    const Block& target_block,
    std::vector<BlockPtr>* block_path) {

    if (from_block->get_id() == target_block.get_id()) {
        // Found target block
        return true;
    }

    // Expand dirs and links when looking for blocks.
    std::shared_ptr<const LinkBlockData> link = try_cast_link(from_block);
    std::shared_ptr<const DirFileBlockData> dir_file =
        try_cast_dir_file(from_block);
    if (!link && !dir_file) {
        // This is a terminal block that is not the block that we are looking
        // for. Therefore, we did not find a path to the target block,
        // and should just exit.
        return false;
    }

    const std::vector<BlockId>& children =
        link ? link->get_children() : dir_file->get_children();

    for (auto child_id_iter = children.cbegin();
         child_id_iter != children.cend(); ++child_id_iter) {
        auto entry = retrieved_blocks.find(*child_id_iter);
        if (entry == retrieved_blocks.end()) {
            // Child does not appear in retrieved_blocks; this
            // means that we did not need to expand this block
            // to find a path, and that we should therefore not
            // investigate the path further.
            continue;
        }
        const BlockPtr& child = entry->second;
        if (find_path(retrieved_blocks, child, target_block, block_path)) {
            block_path->push_back(child);
            return true;
        }
    }
    // we did not find a path to the target block
    return false;
}

bool get_children(BlockPtr block, std::vector<BlockId>* children) {
    std::shared_ptr<const LinkBlockData> link_data =
        block_util::try_cast_link(block);
    if (link_data) {
        *children = link_data->get_children();
        return true;
    }

    std::shared_ptr<const DirFileBlockData> dir_file =
        block_util::try_cast_dir_file(block);
    if (dir_file) {
        *children = dir_file->get_children();
        return true;
    }
    return false;
}


}  // namespace block_util

}  // namespace pingfs
