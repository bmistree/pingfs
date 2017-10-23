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
 * Walks over a chain of connected blocks starting at {@code begin}
 * and ending at {@code end}, creating a parallel chain of blocks,
 * replacing children along the way.
 *
 * @param children_to_remove Remove these ids from the children of
 * the begin iterator.
 * @param children_to_add Add these ids to the children of the begin
 * iterator.
 */
BlockPtr replace_chain(
    std::vector<BlockPtr>::reverse_iterator begin,
    std::vector<BlockPtr>::reverse_iterator end,
    const std::vector<BlockId>& children_to_remove,
    const std::vector<BlockId>& children_to_add,
    std::shared_ptr<BlockManager> block_manager);

void recursive_free_children_blocks(
    BlockPtr block,
    std::shared_ptr<BlockManager> block_manager);

}  // namespace block_util

}  // namespace pingfs

#endif
