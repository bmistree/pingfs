#include <pingfs/block/block_data/dir_file_block_data.hpp>
#include <pingfs/block/block_data/link_block_data.hpp>
#include <pingfs/fs/block_fuse.hpp>
#include <pingfs/fs/fs_util.hpp>
#include <pingfs/fs/fuse_wrapper.hpp>

#include <time.h>
#include <unistd.h>

#include <cassert>


namespace pingfs {

using BlockPtr = std::shared_ptr<const Block>;

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

static bool find_path(
    const std::unordered_map<BlockId, BlockPtr>& retrieved_blocks,
    BlockPtr from_block,
    const Block& target_block,
    std::vector<BlockPtr>* block_path) {

    // try to find a path from one root block to the target;
    // if that doesn't work go onto the next root block, etc.
    if (from_block->get_id() == target_block.get_id()) {
        // Found target block
        return true;
    }

    std::shared_ptr<const LinkBlockData> link = try_cast_link(from_block);
    if (!link) {
        // we did not find a path to the target block
        return false;
    }

    const std::vector<BlockId>& link_children = link->get_children();
    for (auto child_id_iter = link_children.cbegin();
         child_id_iter != link_children.cend(); ++child_id_iter) {
        const std::shared_ptr<const Block>& child =
            retrieved_blocks.find(*child_id_iter)->second;
        // All children should appear in retrieved_blocks
        assert(child.get() != nullptr);

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

BlockPtr BlockFuse::resolve_inode(
    std::vector<BlockId>* vec,
    const std::string& rel_file_dir_name) const {

    std::vector<std::shared_ptr<const Block>> blocks;
    get_path(rel_file_dir_name.c_str(), &blocks);

    if (blocks.empty()) {
        return std::shared_ptr<Block>();
    }
    return blocks.back();
}

int BlockFuse::getattr(const char* path, struct stat* stbuf) {
    std::vector<std::string> parts = FsUtil::separate_path(path);
    std::vector<BlockId> to_resolve;
    BlockPtr resolved;
    std::shared_ptr<const DirFileBlockData> resolved_data;

    for (std::size_t i = 0; i < parts.size(); ++i) {
        resolved = resolve_inode(&to_resolve, parts[i]);

        if (!resolved) {
            // No file/dir named by path
            break;
        }

        resolved_data =
            std::static_pointer_cast<const DirFileBlockData>(
                resolved->get_data());
        to_resolve = resolved_data->get_children();
    }

    if (!resolved) {
        // No file/dir named by path
        return 1;
    }

    // File/dir found; populate stbuf with its inode info.
    resolved_data->get_stat().update_stat(
            dev_,
            1 /* ino; FIXME don't hard code */,
            stbuf);
    return 0;
}


int BlockFuse::mkdir(const char *path, mode_t mode) {
    throw "Unsupported";
}

int BlockFuse::rmdir(const char *path) {
    throw "Unsupported";
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

    // Those blocks that either could be the target or the ancestor of the
    // target.
    std::vector<BlockId> blocks_to_check({ from_block->get_id() });
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

        for (auto iter = resp_blocks.cbegin(); iter != resp_blocks.cend();
             ++iter) {
            std::shared_ptr<const DirFileBlockData> dir_file_data =
                try_cast_dir_file(*iter);
            if (dir_file_data) {
                if (dir_file_data->get_name() == rel_file_dir_name) {
                    // exit loop and now search for path to block
                    target_inode = *iter;
                    break;
                }
            }
            std::shared_ptr<const LinkBlockData> link_data =
                try_cast_link(*iter);
            if (link_data) {
                // We must check all children of a link to see
                // if they or their children are the target block.
                const std::vector<BlockId>& children =
                    link_data->get_children();
                blocks_to_check.insert(blocks_to_check.end(),
                    children.cbegin(),
                    children.cend());
                // Add this link block to all of our retrieved blocks
                // in case we need it to reconstruct the path to
                // the target block.
                retrieved_blocks[(*iter)->get_id()] = *iter;
            }
        }
    }

    if (target_inode) {
        // The target block was reachable. Reconstruct the path to
        // it by calling find_path.
        find_path(retrieved_blocks, from_block, *target_inode, block_path);
    }
}

}  // namespace pingfs
