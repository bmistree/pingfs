#include <pingfs/block/block_data/dir_file_block_data.hpp>

#include <pingfs/fs/block_fuse.hpp>
#include <pingfs/fs/fuse_wrapper.hpp>


namespace pingfs {

BlockFuse::BlockFuse(std::shared_ptr<BlockManager> block_manager)
  : block_manager_(block_manager),
    // Populate root of file system with /
    root_block_(block_manager->create_block(
            std::make_shared<const DirFileBlockData>("/",
                std::vector<BlockId>()))) {
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
