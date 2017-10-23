#ifndef _BLOCK_FUSE_HELPER_
#define _BLOCK_FUSE_HELPER_

#include <pingfs/block/block.hpp>
#include <pingfs/block/block_data/dir_file_block_data.hpp>
#include <pingfs/block/block_data/file_contents_block_data.hpp>
#include <pingfs/block/block_data/link_block_data.hpp>

#include <memory>
#include <unordered_map>
#include <vector>

namespace pingfs {

namespace block_util {

using BlockPtr = std::shared_ptr<const Block>;

std::shared_ptr<const DirFileBlockData> try_cast_dir_file(
    BlockPtr block_ptr);

std::shared_ptr<const LinkBlockData> try_cast_link(
    BlockPtr block_ptr);

std::shared_ptr<const FileContentsBlockData> try_cast_contents(
    BlockPtr block_ptr);

bool find_path(
    const std::unordered_map<BlockId, BlockPtr>& retrieved_blocks,
    BlockPtr from_block,
    const Block& target_block,
    std::vector<BlockPtr>* block_path);


}  // namespace block_util

}  // namespace pingfs

#endif
