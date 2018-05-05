#include <pingfs/block/block_data/dir_file_block_data.hpp>
#include <pingfs/block/block_data/link_block_data.hpp>
#include <pingfs/fs/block_fuse.hpp>
#include <pingfs/fs/block_util.hpp>
#include <pingfs/fs/fs_util.hpp>
#include <pingfs/fs/fuse_wrapper.hpp>

#include <math.h>
#include <time.h>
#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <cstring>


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

void BlockFuse::free_block(BlockId block_id) {
    block_manager_->free_block(block_id);
}

int BlockFuse::getattr(const char* path, struct stat* stbuf) {
    BlockPtr resolved = resolve_inode(path);
    if (!resolved) {
        // No file/dir named by path
        return set_no_such_file_or_dir();
    }
    std::shared_ptr<const DirFileBlockData> resolved_data =
        block_util::try_cast_dir_file(resolved);
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
        block_util::try_cast_dir_file(parent_dir);
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
        block_util::replace_chain(blocks.rbegin(), blocks.rend(), {},
            {child_id_to_add}, block_manager_);

    // FIXME: Add lock guard to switch out root block
    set_root_block(last_block_replaced);

    // Free all blocks no longer in use
    for (auto iter = blocks.cbegin(); iter != blocks.cend(); ++iter) {
        free_block((*iter)->get_id());
    }
    return 0;
}


int BlockFuse::unlink(const char *path) {
    // Dirs and files are the same
    return rmdir(path);
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
            block_util::try_cast_link(blocks[i]);
        if (!link_data) {
            last_dir_file_index = i;
            found_last_dir_file_index = true;
            break;
        }
    }

    assert(found_last_dir_file_index);
    BlockId child_id_to_remove = blocks[last_dir_file_index + 1]->get_id();

    // Iterate end to /, replacing children directories on the way.
    BlockPtr last_replaced_block = block_util::replace_chain(
        // Should correspond to the block in blocks[last_dir_file_index]
        blocks.rbegin() + (blocks.size() - last_dir_file_index - 1),
        blocks.rend(), {child_id_to_remove}, {}, block_manager_);

    // Switch root block with last created block
    set_root_block(last_replaced_block);

    // Free all blocks no longer in use
    for (auto iter = blocks.cbegin(); iter != blocks.cend(); ++iter) {
        free_block((*iter)->get_id());
    }

    block_util::recursive_free_children_blocks(
        blocks.back(), block_manager_);
    return 0;
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
            block_util::try_cast_dir_file(*iter);
        if (dir_file_data) {
            if (dir_file_data->get_name() == target_name) {
                // exit loop and now search for path to block
                return *iter;
            }
        }
        std::shared_ptr<const LinkBlockData> link_data =
            block_util::try_cast_link(*iter);
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
        BlockPtr root = get_root_block();
        block_path->push_back(
            std::make_shared<Block>(
                root->get_id(), root->get_data()));
        return;
    }

    assert(block_util::try_cast_dir_file(from_block));
    // Those blocks that either could be the target or the ancestor of the
    // target (as a link).
    std::vector<BlockId> blocks_to_check =
        block_util::try_cast_dir_file(from_block)->get_children();
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
        block_util::find_path(
            retrieved_blocks, from_block, *target_inode, block_path);
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
                block_util::try_cast_dir_file(*iter);
            if (child) {
                // If it's a dir file block, do not trace any further.
                children->push_back(child);
                continue;
            }

            std::shared_ptr<const LinkBlockData> link_data =
                block_util::try_cast_link(*iter);
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
        block_util::try_cast_dir_file(blocks.back());
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


int BlockFuse::read(const char *path, char *buffer, size_t size,
    off_t offset, struct fuse_file_info *fi)  {

    std::vector<BlockPtr> blocks;
    get_path(path, &blocks);
    if (blocks.empty()) {
        // Path does not exist
        return set_no_such_file_or_dir();
    }

    std::shared_ptr<const DirFileBlockData> dir_file =
        block_util::try_cast_dir_file(blocks.back());

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
    std::string contents;
    block_util::read_file_contents(&contents, dir_file, block_manager_);

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
    set_root_block(
        block_util::replace_chain(
            blocks->rbegin(), blocks->rend(),
            {}, {new_block->get_id()}, block_manager_));

    // Free all blocks no longer in use
    for (auto iter = blocks->cbegin(); iter != blocks->cend(); ++iter) {
        free_block((*iter)->get_id());
    }

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
        block_util::try_cast_dir_file(file_inode);

    if (resolved_data->is_dir()) {
        return set_error_because_dir();
    }

    std::string file_contents;
    block_util::read_file_contents(&file_contents, resolved_data,
        block_manager_);

    if (static_cast<std::size_t>(offset) > file_contents.size()) {
        return -1;
    }
    file_contents.replace(offset, size, buffer, size);
    block_util::recursive_free_children_blocks(
        file_inode, block_manager_);

    write_file_starting_at_node(&blocks, file_contents);
    // Return number of bytes written
    return size;
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
        block_util::replace_blocks_for_contents(
            blocks,
            file_contents,
            &ids_to_add,
            BYTES_PER_BLOCK,
            BRANCHING_FACTOR,
            block_manager_);
    }
    std::shared_ptr<const DirFileBlockData> file_inode =
        block_util::try_cast_dir_file(blocks->back());

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
    set_root_block(
        block_util::replace_chain(blocks->rbegin(), blocks->rend(),
            file_inode->get_children(), ids_to_add,
            block_manager_));

    // free all blocks up to and including file's head block
    for (auto iter = blocks->cbegin(); iter != blocks->cend(); ++iter) {
        free_block((*iter)->get_id());
    }
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

const std::shared_ptr<const Block>& BlockFuse::get_root_block() const {
    return root_block_;
}

void BlockFuse::set_root_block(BlockPtr new_root) {
    root_block_ = new_root;
}

}  // namespace pingfs
