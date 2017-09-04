#include <pingfs/block/block_data/dir_file_block_data.hpp>
#include <pingfs/block/block_data/link_block_data.hpp>

#include <pingfs/fs/block_fuse.hpp>
#include <pingfs/fs/fs_util.hpp>
#include <pingfs/fs/fuse_wrapper.hpp>

#include <time.h>
#include <unistd.h>


namespace pingfs {

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

std::shared_ptr<const Block> BlockFuse::resolve_inode(
    std::vector<BlockId>* vec,
    const std::string& rel_file_dir_name) const {

    // special-case handling for resolving root block
    if (rel_file_dir_name == FsUtil::get_separator()) {
        return std::make_shared<Block>(
            root_block_.get_id(), root_block_.get_data());
    }

    // Rough algorithm: look through all blocks in vec. If
    // we find a block representing an inode (i.e., its data
    // are DirFileBlockData) that matches rel_file_dir_name,
    // then return it. If we find links, we look up their
    // children, looking for rel_file_dir_name.
    while (!vec->empty()) {
        std::shared_ptr<const BlockResponse> response =
            block_manager_->get_blocks(BlockRequest(*vec));
        vec->clear();

        const std::vector<std::shared_ptr<const Block>>& blocks =
            response->get_blocks();

        for (auto iter = blocks.cbegin(); iter != blocks.cend();
             ++iter) {
            std::shared_ptr<const BlockData> data =
                (*iter)->get_data();

            std::shared_ptr<const DirFileBlockData> dir_file_data =
                std::dynamic_pointer_cast<const DirFileBlockData>(data);
            if (dir_file_data.get()) {
                if (dir_file_data->get_name() == rel_file_dir_name) {
                    return *iter;
                }
            }
            std::shared_ptr<const LinkBlockData> link_data =
                std::dynamic_pointer_cast<const LinkBlockData>(data);
            if (link_data.get()) {
                const std::vector<BlockId>& children =
                    link_data->get_children();
                vec->insert(vec->end(), children.cbegin(),
                    children.cend());
            }
        }
    }
    return std::shared_ptr<Block>();
}

int BlockFuse::getattr(const char* path, struct stat* stbuf) {
    std::vector<std::string> parts = FsUtil::separate_path(path);
    std::vector<BlockId> to_resolve;
    std::shared_ptr<const Block> resolved;
    std::shared_ptr<const DirFileBlockData> resolved_data;

    for (std::size_t i = 0; i < parts.size(); ++i) {
        resolved = resolve_inode(&to_resolve, parts[i]);

        if (resolved.get() == nullptr) {
            // No file/dir named by path
            break;
        }

        resolved_data =
            std::static_pointer_cast<const DirFileBlockData>(
                resolved->get_data());
        to_resolve = resolved_data->get_children();
    }

    if (resolved.get() == nullptr) {
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

}  // namespace pingfs
