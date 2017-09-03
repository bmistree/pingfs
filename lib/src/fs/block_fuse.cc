#include <pingfs/block/block_data/dir_file_block_data.hpp>

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

int BlockFuse::getattr(const char *path, struct stat *stbuf) {
    throw "Unsupported";
}

int BlockFuse::mkdir(const char *path, mode_t mode) {
    throw "Unsupported";
}

int BlockFuse::rmdir(const char *path) {
    throw "Unsupported";
}

}  // namespace pingfs
