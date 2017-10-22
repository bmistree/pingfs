#include <pingfs/block/block_data/dir_file_block_data.hpp>
#include <pingfs/block/block_data/link_block_data.hpp>
#include <pingfs/fs/block_fuse.hpp>
#include <pingfs/fs/fs_util.hpp>
#include <pingfs/fs/fuse_wrapper.hpp>

#include <math.h>
#include <time.h>
#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>

namespace pingfs {

using BlockPtr = std::shared_ptr<const Block>;

static int set_no_such_file_or_dir() {
    errno = ENOENT;
    return -ENOENT;
}

static int set_file_exists() {
    errno = EEXIST;
    return -EEXIST;
}

static int set_error_because_dir() {
    errno = EISDIR;
    return -EISDIR;
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
        return set_no_such_file_or_dir();
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

    Stat stat(
        // mode contains just the permission bits; additional
        // bits are implementation-dependent according to
        // mkdir documentation. therefore, we or with
        // S_IFDIR.
        Mode(mode | S_IFDIR),
        getuid(), getgid(),
        // FIXME: Choose a meaningful size
        1 /* size */,
        cur_time, cur_time, cur_time);

    std::vector<BlockId> empty;
    std::shared_ptr<const DirFileBlockData> new_dir_data_block =
        std::make_shared<const DirFileBlockData> (dir_to_make, stat, empty);

    BlockPtr new_dir_block = block_manager_->create_block(new_dir_data_block);
    BlockId child_id_to_add = new_dir_block->get_id();
    BlockPtr last_block_replaced =
        replace_chain(blocks.rbegin(), blocks.rend(), {},
            {child_id_to_add});

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


BlockPtr BlockFuse::replace_chain(
    std::vector<BlockPtr>::reverse_iterator begin,
    std::vector<BlockPtr>::reverse_iterator end,
    const std::vector<BlockId>& children_to_remove,
    const std::vector<BlockId>& children_to_add) {
    BlockPtr last_replaced_block;

    // child_id_to_remove only indicates the first
    // id to remove. Use this helper to replace the
    // pointer if nullptr was passed in.
    std::vector<BlockId> children_to_remove_helper =
        children_to_remove;

    for (auto iter = begin; iter != end; ++iter) {
        BlockPtr block_to_replace = *iter;
        std::vector<BlockId> block_to_replace_children;
        get_children(block_to_replace, &block_to_replace_children);

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
        blocks.rend(), {child_id_to_remove}, {});

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
        return set_no_such_file_or_dir();
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
        // Path does not exist
        return set_no_such_file_or_dir();
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
    if (static_cast<std::size_t>(offset) > contents.size()) {
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

void BlockFuse::read_file_contents(std::string* result,
    std::shared_ptr<const DirFileBlockData> file_inode) {
    assert(!file_inode->is_dir());
    std::vector<std::shared_ptr<const FileContentsBlockData>> file_blocks;
    get_contents_helper(
        file_inode->get_children(), &file_blocks, block_manager_);

    for (auto iter = file_blocks.cbegin(); iter != file_blocks.cend();
         ++iter) {
        *result += *((*iter)->get_data());
    }
}

static Stat gen_file_stat(const Mode& mode, std::size_t size) {
    time_t cur_time = time(NULL);
    return Stat(mode,
        getuid(),
        getgid(),
        size,
        cur_time /* access_time */,
        cur_time /* mod_time */,
        cur_time /* status_change_time */);
}

bool BlockFuse::create_file_block(const char* path,
    std::vector<BlockPtr>* blocks, std::size_t file_size,
    const Mode& mode) {

    get_path(path, blocks);
    // The file existed; do not try to create the file
    if (!blocks->empty()) {
        return false;
    }

    std::vector<std::string> separated_path = FsUtil::separate_path(path);
    std::vector<std::string> penultimate(
        separated_path.begin(), separated_path.end() - 1);

    get_path(FsUtil::join(penultimate).c_str(), blocks);
    if (blocks->empty()) {
        // The path up to the file did not exist; cannot
        // create the file itself.
        return false;
    }

    // The file did not exist, but the path to it did: create the
    // file.
    std::vector<BlockId> empty;
    std::shared_ptr<const DirFileBlockData> new_file_data =
        std::make_shared<const DirFileBlockData>(
            separated_path.back(),
            gen_file_stat(mode, file_size),
            empty);

    // Create a block associated with this file and add it to
    // this directory.
    BlockPtr new_block = block_manager_->create_block(new_file_data);
    // FIXME: add some concurrency checks here.
    root_block_ =
        replace_chain(blocks->rbegin(), blocks->rend(),
            {}, {new_block->get_id()});
    blocks->clear();

    // Get the updated path
    get_path(path, blocks);
    // This should exist: we just added it.
    assert(!blocks->empty());
    return true;
}


int BlockFuse::write(const char *path, const char *buffer,
    size_t size, off_t offset, struct fuse_file_info *fi) {

    std::vector<BlockPtr> blocks;
    get_path(path, &blocks);
    if (blocks.empty()) {
        return set_no_such_file_or_dir();
    }
    BlockPtr file_inode = blocks.back();
    std::shared_ptr<const DirFileBlockData> resolved_data =
        try_cast_dir_file(file_inode);

    if (resolved_data->is_dir()) {
        return set_error_because_dir();
    }

    std::string file_contents;
    read_file_contents(&file_contents, resolved_data);

    if (static_cast<std::size_t>(offset) > file_contents.size()) {
        return -1;
    }
    file_contents.replace(offset, size, buffer, size);
    recursive_free_children_blocks(file_inode);

    write_file_starting_at_node(&blocks, file_contents);
    // Return number of bytes written
    return size;
}

/**
 * @param blocks_to_link_to Blocks that a new link layer
 * should point to.
 * @param new_layer Initially empty; this method populates
 * this vector with link blocks to {@code blocks_to_link_to}.
 */
static void add_link_layer(
    const std::vector<BlockPtr>& blocks_to_link_to,
    std::vector<BlockPtr>* new_layer, std::size_t branching_factor,
    std::shared_ptr<BlockManager> block_manager) {

    std::size_t blocks_over_branching_factor =
        static_cast<std::size_t>(
            ceil(static_cast<double>(blocks_to_link_to.size()) /
                static_cast<double>(branching_factor)));

    for (std::size_t i = 0; i < blocks_over_branching_factor;
         ++i) {
        std::vector<BlockId> child_links;
        for (std::size_t j = 0; j < branching_factor; ++j) {
            std::size_t block_index = (i * branching_factor) + j;
            if (block_index >= blocks_to_link_to.size()) {
                break;
            }

            child_links.push_back(
                blocks_to_link_to[block_index]->get_id());
        }
        new_layer->push_back(
            block_manager->create_block(
                std::make_shared<const LinkBlockData>(child_links)));
    }
}

void BlockFuse::replace_blocks_for_contents(
    const std::vector<BlockPtr>* blocks,
    const std::string& file_contents,
    std::vector<BlockId>* ids_to_add) {

    assert(!blocks->empty());
    assert(!file_contents.empty());
    // Step 1: split file contents into leaf blocks
    std::size_t num_leaves = file_contents.size() / BYTES_PER_BLOCK;
    if ((num_leaves * BYTES_PER_BLOCK) != file_contents.size()) {
        ++num_leaves;
    }
    int depth = ceil(log(num_leaves) / log(BRANCHING_FACTOR));

    std::vector<std::shared_ptr<const FileContentsBlockData>> leaf_data;
    for (std::size_t i = 0; i < num_leaves; ++i) {
        std::size_t end =
            std::max((i + 1) * BYTES_PER_BLOCK,
                file_contents.size());
        leaf_data.push_back(
            std::make_shared<const FileContentsBlockData>(
                file_contents.substr(i * BYTES_PER_BLOCK, end)));
    }

    // Step 2: build a tree of links to those leaf blocks
    std::vector<BlockPtr> blocks_to_link_to;
    for (auto iter = leaf_data.cbegin(); iter != leaf_data.cend(); ++iter) {
        blocks_to_link_to.push_back(block_manager_->create_block(*iter));
    }

    for (std::size_t i = 0; i < static_cast<std::size_t>(depth); ++i) {
        std::vector<BlockPtr> links;
        add_link_layer(blocks_to_link_to, &links, BRANCHING_FACTOR,
            block_manager_);
        blocks_to_link_to = links;
    }

    for (auto iter = blocks_to_link_to.cbegin();
         iter != blocks_to_link_to.cend(); ++iter) {
        ids_to_add->push_back((*iter)->get_id());
    }
}


void BlockFuse::write_file_starting_at_node(
    std::vector<BlockPtr>* blocks,
    const std::string& file_contents) {
    // Rough algorithm:
    // 1. Split file_contents into leaf blocks based
    //    on the maximum number of bytes a block can
    //    hold.
    // 2. Build a tree of links to those leaf blocks.
    // 3. Connect the base of that tree to the last
    //    block in blocks.
    // 4. Update all blocks in blocks to reflect new
    //    file system.

    assert(blocks->size() != 0);

    std::vector<BlockId> ids_to_add;

    if (!file_contents.empty()) {
        replace_blocks_for_contents(
            blocks,
            file_contents,
            &ids_to_add);
    }
    std::shared_ptr<const DirFileBlockData> file_inode =
        try_cast_dir_file(blocks->back());


    // Steps 3 and 4: replace existing blocks

    // Switch out the last element in blocks. This way, When we write
    // blocks over when calling replace_chain, we will get the
    // new size and access/mod times.
    time_t cur_time = time(NULL);
    std::shared_ptr<const DirFileBlockData> updated_file =
        std::make_shared<const DirFileBlockData>(
            file_inode->get_name(),
            Stat(file_inode->get_stat(),
                file_contents.size(),
                cur_time,
                cur_time,
                cur_time),
            file_inode->get_children());

    blocks->back() = std::make_shared<const Block>(
        blocks->back()->get_id(), updated_file);

    // FIXME: add concurrency protection
    root_block_ =
        replace_chain(blocks->rbegin(), blocks->rend(),
            file_inode->get_children(), ids_to_add);
}

void BlockFuse::get_file_contents(
    std::shared_ptr<const DirFileBlockData> file_data,
    std::vector<std::shared_ptr<const FileContentsBlockData>>* file_blocks) {
    assert(!file_data->is_dir());
    get_contents_helper(
        file_data->get_children(), file_blocks, block_manager_);
}

int BlockFuse::create(const char *path, mode_t mode,
    struct fuse_file_info *fi) {
    std::vector<BlockPtr> blocks;
    get_path(path, &blocks);
    if (!blocks.empty()) {
        return set_file_exists();
    }
    bool succeeded =
        create_file_block(
            path, &blocks,
            0 /* file_size */,
            Mode(mode));
    if (!succeeded) {
        // FIXME: There may be other error conditions to
        // consider.
        // The path up to the file did not exist; cannot
        // create the file itself.
        return set_no_such_file_or_dir();
    }
    return 0;
}


}  // namespace pingfs
