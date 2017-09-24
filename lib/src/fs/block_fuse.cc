#include <pingfs/block/block_data/dir_file_block_data.hpp>
#include <pingfs/block/block_data/link_block_data.hpp>
#include <pingfs/fs/block_fuse.hpp>
#include <pingfs/fs/fs_util.hpp>
#include <pingfs/fs/fuse_wrapper.hpp>

#include <time.h>
#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>

namespace pingfs {

using BlockPtr = std::shared_ptr<const Block>;

static void set_no_such_file_or_dir() {
    errno = ENOENT;
}

static std::shared_ptr<const DirFileBlockData> try_cast_dir_file(
    BlockPtr block_ptr) {
    return std::dynamic_pointer_cast<const DirFileBlockData>(
        block_ptr->get_data());
}

static std::shared_ptr<const LinkBlockData> try_cast_link(
    BlockPtr block_ptr) {
    return std::dynamic_pointer_cast<const LinkBlockData>(
        block_ptr->get_data());
}

static std::shared_ptr<const FileContentsBlockData> try_cast_contents(
    BlockPtr block_ptr) {
    return std::dynamic_pointer_cast<const FileContentsBlockData>(
        block_ptr->get_data());
}

static bool find_path(
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

/**
 * Returns BlockData initializing parameters for file system.
 */
static std::shared_ptr<const BlockData> make_root_block_data() {
    time_t cur_time = time(NULL);
    return std::make_shared<const DirFileBlockData>(
        FsUtil::get_separator(),
        Stat(Mode(ReadWriteExecute::READ_WRITE_EXECUTE,
                ReadWriteExecute::READ_EXECUTE,
                ReadWriteExecute::EXECUTE,
                FileType::DIR),
            getuid(),
            getgid(),
            0 /* size */,
            cur_time /* access_time */,
            cur_time /* mod_time */,
            cur_time /* status_change_time */),
        std::vector<BlockId>());
}

BlockFuse::BlockFuse(std::shared_ptr<BlockManager> block_manager,
    dev_t dev)
  : block_manager_(block_manager),
    // Populate root of file system with /
    root_block_(block_manager->create_block(make_root_block_data())),
    dev_(dev) {
}

BlockFuse::~BlockFuse() {
}

BlockPtr BlockFuse::resolve_inode(const std::string& path) const {
    std::vector<std::shared_ptr<const Block>> blocks;
    get_path(path.c_str(), &blocks);
    if (blocks.empty()) {
        return std::shared_ptr<Block>();
    }
    return blocks.back();
}

int BlockFuse::getattr(const char* path, struct stat* stbuf) {
    BlockPtr resolved = resolve_inode(path);
    if (!resolved) {
        // No file/dir named by path
        set_no_such_file_or_dir();
        return -1;
    }
    std::shared_ptr<const DirFileBlockData> resolved_data =
        try_cast_dir_file(resolved);
    // resolve_inode should only return dir file block data.
    assert(resolved_data);

    // File/dir found; populate stbuf with its inode info.
    resolved_data->get_stat().update_stat(
            dev_,
            1 /* ino; FIXME don't hard code */,
            stbuf);
    return 0;
}

/**
 * Returns true if {@code path} is a valid path for making a directory
 * for; false otherwise.
 */
bool BlockFuse::mkdir_valid(const char* path,
    std::vector<BlockPtr>* blocks, std::string* dir_to_make) {
    std::vector<std::string> parts = FsUtil::separate_path(path);
    if (parts.size() <= 1) {
        return false;
    }

    // Check if the directory already exists
    struct stat stbuf;
    if (getattr(path, &stbuf) == 0) {
        // The directory/file already exists. Do not continue.
        return false;
    }

    // Get the parent directory
    *dir_to_make = parts.back();
    parts.pop_back();

    get_path(FsUtil::join(parts).c_str(), blocks);
    if (blocks->empty()) {
        // No path exists to parent directory
        return false;
    }

    // Check that the parent is a directory
    BlockPtr parent_dir = (*blocks)[blocks->size() - 1];
    std::shared_ptr<const DirFileBlockData> parent =
        try_cast_dir_file(parent_dir);
    if (!parent) {
        // The parent isn't a directory. Can't call mkdir on a file.
        return false;
    }
    return true;
}


int BlockFuse::mkdir(const char *path, mode_t mode) {
    std::vector<BlockPtr> blocks;
    std::string dir_to_make;
    if (!mkdir_valid(path, &blocks, &dir_to_make)) {
        return 1;
    }

    // Replace blocks making up path to parent directory
    // Note that we have to grow blocks backwards from the created
    // directory down to the root.

    time_t cur_time = time(NULL);
    Stat stat(Mode(mode), getuid(), getgid(),
        // FIXME: Choose a meaningful size
        1 /* size */,
        cur_time, cur_time, cur_time);

    std::vector<BlockId> empty;
    std::shared_ptr<const DirFileBlockData> new_dir_data_block =
        std::make_shared<const DirFileBlockData> (dir_to_make, stat, empty);

    BlockPtr new_dir_block = block_manager_->create_block(new_dir_data_block);
    BlockId child_id_to_add = new_dir_block->get_id();
    BlockPtr last_block_replaced =
        replace_chain(blocks.rbegin(), blocks.rend(), nullptr,
            &child_id_to_add);

    // FIXME: Add lock guard to switch out root block
    root_block_ = last_block_replaced;

    // Free all blocks no longer in use
    for (auto iter = blocks.cbegin(); iter != blocks.cend(); ++iter) {
        block_manager_->free_block((*iter)->get_id());
    }
    return 0;
}

/**
 * Returns true if {@code block} can be cast to a link block or dir
 * file block, and false otherwise. If {@code block} can be cast to
 * a link/dir file block, then populate {@code children} with that
 * block's children.
 */
static bool get_children(BlockPtr block, std::vector<BlockId>* children) {
    std::shared_ptr<const LinkBlockData> link_data =
        try_cast_link(block);
    if (link_data) {
        *children = link_data->get_children();
        return true;
    }

    std::shared_ptr<const DirFileBlockData> dir_file =
        try_cast_dir_file(block);
    if (dir_file) {
        *children = dir_file->get_children();
        return true;
    }
    return false;
}


BlockPtr BlockFuse::replace_chain(
    std::vector<BlockPtr>::reverse_iterator begin,
    std::vector<BlockPtr>::reverse_iterator end,
    BlockId* child_id_to_remove,
    BlockId* child_id_to_add) {
    BlockPtr last_replaced_block;

    // child_id_to_remove only indicates the first
    // id to remove. Use this helper to replace the
    // pointer if nullptr was passed in.
    BlockId child_id_to_remove_helper;

    for (auto iter = begin; iter != end; ++iter) {
        BlockPtr block_to_replace = *iter;

        std::vector<BlockId> block_to_replace_children;
        get_children(block_to_replace, &block_to_replace_children);

        // child_id_to_remove contains the id of either the last
        // block that we replaced or of the directory/link
        // that we're removing. Erase it.
        if (child_id_to_remove != nullptr) {
            auto remove_iter = std::find(
                block_to_replace_children.begin(),
                block_to_replace_children.end(),
                *child_id_to_remove);
            assert(remove_iter != block_to_replace_children.end());
            block_to_replace_children.erase(remove_iter);
        } else {
            child_id_to_remove = &child_id_to_remove_helper;
        }
        *child_id_to_remove = block_to_replace->get_id();

        if (last_replaced_block) {
            // Add the id of the last replaced block
            block_to_replace_children.push_back(
                last_replaced_block->get_id());
        } else if (child_id_to_add) {
            block_to_replace_children.push_back(*child_id_to_add);
        }
        // FIXME: Set access/changed time on each directory.
        last_replaced_block = replace_block_with_diff_children(
            block_to_replace, block_to_replace_children);
    }
    return last_replaced_block;
}

int BlockFuse::rmdir(const char *path) {
    std::vector<BlockPtr> blocks;
    get_path(path, &blocks);
    if (blocks.empty()) {
        // Cannot remove directory: directory
        // does not exist.
        return 1;
    }

    if (blocks.size() == 1) {
        // Cannot delete /
        return 1;
    }

    std::size_t last_dir_file_index;
    bool found_last_dir_file_index = false;
    for (std::size_t i = blocks.size() - 2; i >= 0; --i) {
        std::shared_ptr<const LinkBlockData> link_data =
            try_cast_link(blocks[i]);
        if (!link_data) {
            last_dir_file_index = i;
            found_last_dir_file_index = true;
            break;
        }
    }

    assert(found_last_dir_file_index);
    BlockId child_id_to_remove = blocks[last_dir_file_index + 1]->get_id();

    // Iterate end to /, replacing children directories on the way.
    BlockPtr last_replaced_block = replace_chain(
        // Should correspond to the block in blocks[last_dir_file_index]
        blocks.rbegin() + (blocks.size() - last_dir_file_index - 1),
        blocks.rend(), &child_id_to_remove, nullptr);

    // Switch root block with last created block
    root_block_ = last_replaced_block;

    // Free all blocks no longer in use
    for (auto iter = blocks.cbegin(); iter != blocks.cend(); ++iter) {
        block_manager_->free_block((*iter)->get_id());
    }

    recursive_free_children_blocks(blocks.back());
    return 0;
}

void BlockFuse::recursive_free_children_blocks(BlockPtr block) {
    std::vector<BlockId> children;
    if (!get_children(block, &children)) {
        // This block cannot have children. Skip it.
        return;
    }

    // Fetch children blocks so that we can delete their children
    std::shared_ptr<const BlockResponse> response =
        block_manager_->get_blocks(BlockRequest(children));

    // Free children blocks
    for (auto iter = children.cbegin(); iter != children.cend();
         ++iter) {
        block_manager_->free_block(*iter);
    }

    // Try deleting children of children
    const std::vector<BlockPtr>& child_blocks = response->get_blocks();
    for (auto iter = child_blocks.cbegin();
         iter != child_blocks.cend(); ++iter) {
        recursive_free_children_blocks(*iter);
    }
}

BlockPtr BlockFuse::replace_block_with_diff_children(
    BlockPtr block_to_replace, const std::vector<BlockId>& new_children) {

    std::shared_ptr<const LinkBlockData> link_data =
        try_cast_link(block_to_replace);
    if (link_data) {
        return block_manager_->create_block(
            std::make_shared<const LinkBlockData>(new_children));
    }

    std::shared_ptr<const DirFileBlockData> dir_file =
        try_cast_dir_file(block_to_replace);
    if (dir_file) {
        return block_manager_->create_block(
            std::make_shared<const DirFileBlockData>(*dir_file, new_children));
    }

    throw "Unexpected block type";
}

void BlockFuse::get_path(const char* path,
    std::vector<BlockPtr>* blocks) const {
    std::vector<std::string> parts = FsUtil::separate_path(path);

    // Rough algorithm: break the path into parts. Find the blocks to go
    // from the first part to the second part. On top of that, find
    // the blocks to go from the second part to the third. Etc.
    BlockPtr from_block;
    for (auto iter = parts.cbegin(); iter != parts.cend(); ++iter) {
        std::vector<std::shared_ptr<const Block>> block_path;
        get_path_part(*iter, from_block, &block_path);
        if (block_path.empty()) {
            // There was no path. Return an empty block vector.
            blocks->clear();
            return;
        }

        blocks->insert(blocks->end(), block_path.cbegin(), block_path.cend());
        from_block = block_path.back();
    }
}

/**
 * Returns a non-empty pointer if a dir_file block in {@code resp_blocks}
 * exists with name {@code target_name}. Otherwise, returns an empty
 * pointer.
 *
 * Appends children of link blocks in {@code resp_blocks} to
 * {@code blocks_to_check} and to {@code retrieved_blocks}.
 */
static BlockPtr get_path_part_helper(std::vector<BlockId>* blocks_to_check,
    std::unordered_map<BlockId, BlockPtr>* retrieved_blocks,
    const std::vector<BlockPtr>& resp_blocks,
    const std::string& target_name) {

    for (auto iter = resp_blocks.cbegin(); iter != resp_blocks.cend();
         ++iter) {
        // Add this block to all of our retrieved blocks
        // in case we need it to reconstruct the path to
        // the target block.
        // FIXME: it is a little inefficient to do this
        // here. Really, we only need to track link
        // blocks and the final block. We could consider
        // just updating for those two cases instead.
        (*retrieved_blocks)[(*iter)->get_id()] = *iter;

        std::shared_ptr<const DirFileBlockData> dir_file_data =
            try_cast_dir_file(*iter);
        if (dir_file_data) {
            if (dir_file_data->get_name() == target_name) {
                // exit loop and now search for path to block
                return *iter;
            }
        }
        std::shared_ptr<const LinkBlockData> link_data =
            try_cast_link(*iter);
        if (link_data) {
            // We must check all children of a link to see
            // if they or their children are the target block.
            const std::vector<BlockId>& children =
                link_data->get_children();
            blocks_to_check->insert(blocks_to_check->end(),
                children.cbegin(),
                children.cend());
        }
    }
    BlockPtr empty;
    return empty;
}

/**
 * Rough algorithm: use the block manager to perform a
 * breadth-first search until we find the target block.
 * Store each block touched in the breadth-first search
 * that is a link or along the path. Once we have found the
 * terminal block, do a depth-first search on the touched
 * blocks to find the path.
 */
void BlockFuse::get_path_part(
    const std::string& rel_file_dir_name,
    BlockPtr from_block,
    std::vector<BlockPtr>* block_path) const {

    // special-case handling for resolving root block
    if (rel_file_dir_name == FsUtil::get_separator()) {
        block_path->push_back(
            std::make_shared<Block>(
                root_block_->get_id(), root_block_->get_data()));
        return;
    }

    assert(try_cast_dir_file(from_block));
    // Those blocks that either could be the target or the ancestor of the
    // target (as a link).
    std::vector<BlockId> blocks_to_check =
        try_cast_dir_file(from_block)->get_children();
    // All link blocks that we encountered while searching for target.
    std::unordered_map<BlockId, BlockPtr> retrieved_blocks;
    // This gets set when we encounter the target in the loop below.
    // If it is not set, that means we could not find the target block.
    BlockPtr target_inode;
    while (!blocks_to_check.empty() && !target_inode) {
        std::shared_ptr<const BlockResponse> response =
            block_manager_->get_blocks(BlockRequest(blocks_to_check));
        blocks_to_check.clear();
        const std::vector<BlockPtr>& resp_blocks = response->get_blocks();

        target_inode = get_path_part_helper(&blocks_to_check,
            &retrieved_blocks, resp_blocks, rel_file_dir_name);
    }

    if (target_inode) {
        // The target block was reachable. Reconstruct the path to
        // it by calling find_path.
        find_path(retrieved_blocks, from_block, *target_inode, block_path);
    }
}

void BlockFuse::get_dir_files_from_dir(
    std::shared_ptr<const DirFileBlockData> dir_file,
    std::vector<std::shared_ptr<const DirFileBlockData>>* children) {

    std::vector<BlockId> blocks_to_trace = dir_file->get_children();

    while (!blocks_to_trace.empty()) {
        std::shared_ptr<const BlockResponse> resp =
            block_manager_->get_blocks(BlockRequest(blocks_to_trace));
        blocks_to_trace.clear();

        for (auto iter = resp->get_blocks().cbegin();
             iter !=resp->get_blocks().cend(); ++iter) {
            std::shared_ptr<const DirFileBlockData> child =
                try_cast_dir_file(*iter);
            if (child) {
                // If it's a dir file block, do not trace any further.
                children->push_back(child);
                continue;
            }

            std::shared_ptr<const LinkBlockData> link_data =
                try_cast_link(*iter);
            if (link_data) {
                // Keep tracing through links.
                blocks_to_trace.insert(blocks_to_trace.end(),
                    link_data->get_children().cbegin(),
                    link_data->get_children().cend());
            }
        }
    }
}

int BlockFuse::readdir(const char *path, void *buf,
    fuse_fill_dir_t filler, off_t offset,
    struct fuse_file_info *fi) {

    std::vector<BlockPtr> blocks;
    get_path(path, &blocks);
    if (blocks.empty()) {
        // path doesn't exist
        set_no_such_file_or_dir();
        return -1;
    }

    std::shared_ptr<const DirFileBlockData> dir_file =
        try_cast_dir_file(blocks.back());
    if (!dir_file) {
        // Searched for path is not a directory.
        return 1;
    }

    struct stat stbuf;
    dir_file->get_stat().update_stat(
        dev_,
        1 /* ino; FIXME don't hard code */,
        &stbuf);
    int filler_result = filler(buf, ".", &stbuf, 0);
    if (filler_result != 0) {
        return filler_result;
    }

    // FIXME: Currently using NULL instead of stat struct for
    // parent.
    filler_result = filler(buf, "..", NULL, 0);
    if (filler_result != 0) {
        return filler_result;
    }

    std::vector<std::shared_ptr<const DirFileBlockData>> children;
    get_dir_files_from_dir(dir_file, &children);

    for (auto iter = children.cbegin(); iter != children.cend();
         ++iter) {
        (*iter)->get_stat().update_stat(
            dev_,
            1 /* ino; FIXME don't hard code */,
            &stbuf);
        filler(buf, (*iter)->get_name().c_str(), &stbuf, 0);
    }
    return 0;
}

static void file_blocks_to_contents(
    const std::vector<
      std::shared_ptr<const FileContentsBlockData>>& file_blocks,
    std::string* contents) {

    for (auto iter = file_blocks.cbegin(); iter != file_blocks.cend();
         ++iter) {
        const std::string& data = *((*iter)->get_data());
        contents->insert(contents->end(), data.cbegin(), data.cend());
    }
}


int BlockFuse::read(const char *path, char *buffer, size_t size,
    off_t offset, struct fuse_file_info *fi)  {

    std::vector<BlockPtr> blocks;
    get_path(path, &blocks);
    if (blocks.empty()) {
        return -1;
    }

    std::shared_ptr<const DirFileBlockData> dir_file =
        try_cast_dir_file(blocks.back());

    if (!dir_file) {
        // Wrong type of block data
        return -1;
    }

    if (dir_file->is_dir()) {
        // Trying to read a directory instead of a file
        return -1;
    }

    // FIXME: For now, constructing entire file in memory and then just
    // returning the relevant portion. Would be much more efficient to just find
    // the targeted portion of the file.
    std::vector<std::shared_ptr<const FileContentsBlockData>> file_blocks;
    get_file_contents(dir_file, &file_blocks);
    std::string contents;
    file_blocks_to_contents(file_blocks, &contents);

    // FIXME: skipping populating fuse_file_info struct

    if (offset > contents.size()) {
        return 0;
    }

    // FIXME: should I be using int64_t here?
    int64_t actual_size =
        ((offset + size) <= contents.size()) ? size
                                             : (contents.size() - offset);
    std::memcpy(buffer, contents.c_str() + offset, actual_size);
    return actual_size;
}

/**
 * Performs a depth-first left-to-right search to populate
 * file_blocks from block tree.
 */
static void get_contents_helper(
    const std::vector<BlockId>& blocks_to_check,
    std::vector<std::shared_ptr<const FileContentsBlockData>>* file_blocks,
    std::shared_ptr<BlockManager> block_manager) {

    std::shared_ptr<const BlockResponse> response =
        block_manager->get_blocks(BlockRequest(blocks_to_check));
    for (auto iter = response->get_blocks().cbegin();
         iter != response->get_blocks().cend(); ++iter) {
        std::shared_ptr<const FileContentsBlockData> contents =
            try_cast_contents(*iter);
        if (contents) {
            file_blocks->push_back(contents);
            continue;
        }

        std::shared_ptr<const LinkBlockData> link_data =
            try_cast_link(*iter);
        if (link_data) {
            get_contents_helper(
                link_data->get_children(), file_blocks, block_manager);
        }
    }
}

void BlockFuse::get_file_contents(
    std::shared_ptr<const DirFileBlockData> file_data,
    std::vector<std::shared_ptr<const FileContentsBlockData>>* file_blocks) {
    assert(!file_data->is_dir());
    get_contents_helper(
        file_data->get_children(), file_blocks, block_manager_);
}

}  // namespace pingfs
