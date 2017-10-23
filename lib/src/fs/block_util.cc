#include <pingfs/block/block.hpp>
#include <pingfs/block/block_data/dir_file_block_data.hpp>
#include <pingfs/block/block_data/file_contents_block_data.hpp>
#include <pingfs/block/block_data/link_block_data.hpp>
#include <pingfs/fs/block_util.hpp>

#include <algorithm>
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


void file_blocks_to_contents(
    const std::vector<
      std::shared_ptr<const FileContentsBlockData>>& file_blocks,
    std::string* contents) {

    for (auto iter = file_blocks.cbegin(); iter != file_blocks.cend();
         ++iter) {
        const std::string& data = *((*iter)->get_data());
        contents->insert(contents->end(), data.cbegin(), data.cend());
    }
}

/**
 * Performs a depth-first left-to-right search to populate
 * file_blocks from block tree.
 */
static void get_file_contents(
    const std::vector<BlockId>& blocks_to_check,
    std::vector<std::shared_ptr<const FileContentsBlockData>>* file_blocks,
    std::shared_ptr<BlockManager> block_manager) {

    std::shared_ptr<const BlockResponse> response =
        block_manager->get_blocks(BlockRequest(blocks_to_check));
    for (auto iter = response->get_blocks().cbegin();
         iter != response->get_blocks().cend(); ++iter) {
        std::shared_ptr<const FileContentsBlockData> contents =
            block_util::try_cast_contents(*iter);
        if (contents) {
            file_blocks->push_back(contents);
            continue;
        }

        std::shared_ptr<const LinkBlockData> link_data =
            block_util::try_cast_link(*iter);
        if (link_data) {
            get_file_contents(
                link_data->get_children(), file_blocks, block_manager);
        }
    }
}


void read_file_contents(std::string* result,
    std::shared_ptr<const DirFileBlockData> file_inode,
    std::shared_ptr<BlockManager> block_manager) {
    assert(!file_inode->is_dir());
    std::vector<std::shared_ptr<const FileContentsBlockData>> file_blocks;
    block_util::get_file_contents(
        file_inode->get_children(), &file_blocks, block_manager);

    for (auto iter = file_blocks.cbegin(); iter != file_blocks.cend();
         ++iter) {
        *result += *((*iter)->get_data());
    }
}

static void remove_ids(
    std::vector<BlockId>* to_remove_from,
    const std::vector<BlockId>& to_remove) {

    for (auto to_remove_iter = to_remove.cbegin();
         to_remove_iter != to_remove.cend();
         ++to_remove_iter) {
        auto remove_iter = std::find(
            to_remove_from->begin(),
            to_remove_from->end(),
            *to_remove_iter);
        assert(remove_iter != to_remove_from->end());
        to_remove_from->erase(remove_iter);
    }
}

/**
 * Returns a created block that is identical to {@code block_to_replace},
 * except its children are {@code new_children}.
 *
 * @param block_to_replace Must be a block containing either
 * DirFileBlockData or LinkBlockData.
 */
static BlockPtr replace_block_with_diff_children(
    BlockPtr block_to_replace,
    const std::vector<BlockId>& new_children,
    std::shared_ptr<BlockManager> block_manager) {

    std::shared_ptr<const LinkBlockData> link_data =
        try_cast_link(block_to_replace);
    if (link_data) {
        return block_manager->create_block(
            std::make_shared<const LinkBlockData>(new_children));
    }

    std::shared_ptr<const DirFileBlockData> dir_file =
        try_cast_dir_file(block_to_replace);
    if (dir_file) {
        return block_manager->create_block(
            std::make_shared<const DirFileBlockData>(*dir_file, new_children));
    }

    throw "Unexpected block type";
}


BlockPtr replace_chain(
    std::vector<BlockPtr>::reverse_iterator begin,
    std::vector<BlockPtr>::reverse_iterator end,
    const std::vector<BlockId>& children_to_remove,
    const std::vector<BlockId>& children_to_add,
    std::shared_ptr<BlockManager> block_manager) {
    BlockPtr last_replaced_block;

    // child_id_to_remove only indicates the first
    // id to remove. Use this helper to replace the
    // pointer if nullptr was passed in.
    std::vector<BlockId> children_to_remove_helper =
        children_to_remove;

    for (auto iter = begin; iter != end; ++iter) {
        BlockPtr block_to_replace = *iter;
        std::vector<BlockId> block_to_replace_children;
        block_util::get_children(block_to_replace, &block_to_replace_children);

        // children_to_remove_helper contains the id of either the last
        // block that we replaced or of the directory/link
        // that we're removing. Erase it.
        remove_ids(&block_to_replace_children, children_to_remove_helper);

        children_to_remove_helper = {block_to_replace->get_id()};
        if (last_replaced_block) {
            // Add the id of the last replaced block
            block_to_replace_children.push_back(
                last_replaced_block->get_id());
        } else {
            // This is the first block that we are replacing;
            // add the requested passed-in blocks to it.
            block_to_replace_children.insert(
                block_to_replace_children.end(),
                children_to_add.begin(),
                children_to_add.end());
        }
        // FIXME: Set access/changed time on each directory.
        last_replaced_block = replace_block_with_diff_children(
            block_to_replace, block_to_replace_children, block_manager);
    }
    return last_replaced_block;
}


void recursive_free_children_blocks(
    BlockPtr block,
    std::shared_ptr<BlockManager> block_manager) {
    std::vector<BlockId> children;
    if (!get_children(block, &children)) {
        // This block cannot have children. Skip it.
        return;
    }

    // Fetch children blocks so that we can delete their children
    std::shared_ptr<const BlockResponse> response =
        block_manager->get_blocks(BlockRequest(children));

    // Free children blocks
    for (auto iter = children.cbegin(); iter != children.cend();
         ++iter) {
        block_manager->free_block(*iter);
    }

    // Try deleting children of children
    const std::vector<BlockPtr>& child_blocks = response->get_blocks();
    for (auto iter = child_blocks.cbegin();
         iter != child_blocks.cend(); ++iter) {
        recursive_free_children_blocks(*iter, block_manager);
    }
}

}  // namespace block_util

}  // namespace pingfs
