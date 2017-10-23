#ifndef _BLOCK_FUSE_HELPER_
#define _BLOCK_FUSE_HELPER_

#include <pingfs/block/block.hpp>
#include <pingfs/block/block_data/dir_file_block_data.hpp>
#include <pingfs/block/block_data/file_contents_block_data.hpp>
#include <pingfs/block/block_data/link_block_data.hpp>
#include <pingfs/block/block_manager/block_manager.hpp>

#include <memory>
#include <string>
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

/**
 * Returns true if a there is a path from {@code from_block} to
 * {@code target_block}. Also populates {@code block_path} with
 * that path.
 *
 * @param retrieved_blocks Contains all blocks that we may
 * need when trying to build a path from {@code from_block} to
 * {@code target_block}.
 */
bool find_path(
    const std::unordered_map<BlockId, BlockPtr>& retrieved_blocks,
    BlockPtr from_block,
    const Block& target_block,
    std::vector<BlockPtr>* block_path);

/**
 * Returns true if {@code block} can be cast to a link block or dir
 * file block, and false otherwise. If {@code block} can be cast to
 * a link/dir file block, then populate {@code children} with that
 * block's children.
 */
bool get_children(BlockPtr block, std::vector<BlockId>* children);

/**
 * Composes all data in {@code file_blocks} into a single string,
 * which this method stores in {@code contents} before returning.
 */
void file_blocks_to_contents(
    const std::vector<
      std::shared_ptr<const FileContentsBlockData>>& file_blocks,
    std::string* contents);

/**
 * Read the contents of a file referenced by {@code file_inode}
 * into {@code result}.
 */
void read_file_contents(std::string* result,
    std::shared_ptr<const DirFileBlockData> file_inode,
    std::shared_ptr<BlockManager> block_manager);

/**
 * Return all FileContentsBlockData blocks for the file associated with
 * {@code file_data}. Note that {@code file_data} must be associated with
 * a file and not a directory. Also note that {@code file_blocks} will be
 * in the order that the block appears in the file.
 */
void get_file_contents(
    std::shared_ptr<const DirFileBlockData> file_data,
    std::vector<std::shared_ptr<const FileContentsBlockData>>* file_blocks,
    std::shared_ptr<BlockManager> block_manager);

/**
 * Performs a depth-first left-to-right search to populate
 * file_blocks from block tree.
 */
void get_file_contents(
    const std::vector<BlockId>& blocks_to_check,
    std::vector<std::shared_ptr<const FileContentsBlockData>>* file_blocks,
    std::shared_ptr<BlockManager> block_manager);


}  // namespace block_util

}  // namespace pingfs

#endif
