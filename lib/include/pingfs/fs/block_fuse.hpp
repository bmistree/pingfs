#ifndef _BLOCK_FUSE_
#define _BLOCK_FUSE_

#include <pingfs/block/block.hpp>
#include <pingfs/block/block_data/dir_file_block_data.hpp>
#include <pingfs/block/block_data/file_contents_block_data.hpp>
#include <pingfs/block/block_data/link_block_data.hpp>
#include <pingfs/block/block_manager/block_manager.hpp>

#include <unistd.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "fuse_wrapper.hpp"

namespace pingfs {

/**
 * A class that uses a block manager to store file information.
 */
class BlockFuse : public FuseWrapper {
 private:
    using BlockPtr = std::shared_ptr<const Block>;

 public:
    explicit BlockFuse(std::shared_ptr<BlockManager> block_manager,
        dev_t dev);
    virtual ~BlockFuse();

    int getattr(const char *path, struct stat *stbuf) override;
    int mkdir(const char *path, mode_t mode) override;
    int rmdir(const char *path) override;
    int readdir(const char *path, void *buf,
        fuse_fill_dir_t filler, off_t offset,
        struct fuse_file_info *fi) override;
    int read(const char *path, char *buffer, size_t size,
        off_t offset, struct fuse_file_info *fi) override;
    int write(const char *path, const char *buffer,
        size_t size, off_t offset, struct fuse_file_info *fi) override;
    int create(const char *path, mode_t mode,
        struct fuse_file_info *fi) override;

 private:
    static const std::size_t BYTES_PER_BLOCK = 1000;
    static const  std::size_t BRANCHING_FACTOR = 4;

    /**
     * Returns the Block corresponding to the inode for
     * {@code path}, or a wrapped nullptr, if none exists.
     */
    BlockPtr resolve_inode(const std::string& path) const;

    /**
     * Adds all blocks between the root block and the final inode
     * block for path to blocks (inclusive for both). The order of
     * blocks added to blocks matches the order that  blocks were encountered
     * in the tree. Specifically, the root block will be in position 0
     * and the final inode will be at the end. If no path exists
     * (i.e., path doesn't exist), does not modify the blocks vector.
     *
     * @param blocks Should be empty when input. If empty when this function
     * exits, there are no sequences of blocks that represent path (i.e.,
     * path is invalid, there's some corruption, or operations have changed
     * the block structure since path was created).
     */
    void get_path(const char* path,
        std::vector<BlockPtr>* blocks) const;

    void get_path_part(const std::string& rel_file_dir_name,
        BlockPtr from_block,
        std::vector<BlockPtr>* block_path) const;

    /**
     * Returns true if {@code path} is a valid path for making a directory
     * for; false otherwise.
     */
    bool mkdir_valid(const char* path,
        std::vector<BlockPtr>* blocks, std::string* dir_to_make);

    /**
     * Returns a created block that is identical to {@code block_to_replace},
     * except its children are {@code new_children}.
     *
     * @param block_to_replace Must be a block containing either
     * DirFileBlockData or LinkBlockData.
     */
    BlockPtr replace_block_with_diff_children(
        BlockPtr block_to_replace, const std::vector<BlockId>& new_children);

    void recursive_free_children_blocks(BlockPtr block);

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
        const std::vector<BlockId>& children_to_add);

    /**
     * Populates {@code children} with the immediate files and dirs of {@code
     * dir_file}.
     */
    void get_dir_files_from_dir(
        std::shared_ptr<const DirFileBlockData> dir_file,
        std::vector<std::shared_ptr<const DirFileBlockData>>* children);

    /**
     * Return all FileContentsBlockData blocks for the file associated with
     * {@code file_data}. Note that {@code file_data} must be associated with
     * a file and not a directory. Also note that {@code file_blocks} will be
     * in the order that the block appears in the file.
     */
    void get_file_contents(std::shared_ptr<const DirFileBlockData> file_data,
        std::vector<std::shared_ptr<const FileContentsBlockData>>* file_blocks);

    /**
     * Write the entire string in {@code file_contents} to the file
     * system starting at the inode in the final position in {@code blockes}.
     *
     * @param blocks The path leading up to the file to replace. The final
     * value should be an inode for the target file.
     * @param file_contents The string to write for the file.
     */
    void write_file_starting_at_node(
        const std::vector<BlockPtr>* blocks,
        const std::string& file_contents);

    /**
     * Read the contents of a file referenced by {@code file_inode}
     * into {@code result}.
     */
    void read_file_contents(std::string* result,
        std::shared_ptr<const DirFileBlockData> file_inode);

    void write_file_starting_at_node(
        std::vector<BlockPtr>* blocks,
        const std::string& file_contents);

    /**
     * Either get the blocks leading up to the inode
     * for a given path, or creates a file for the given path.
     */
    void get_write_blocks(const char* path,
        std::vector<BlockPtr>* blocks);

    /**
     * Create a file block associated with path. Returns true
     * and the path to the new file in {@code blocks}, if it
     * can create the file. If it cannot (e.g., if the file
     * already exists or the directory doesn't exist), then
     * return false and do not change {@code blocks}.
     */
    bool create_file_block(const char* path,
        std::vector<BlockPtr>* blocks, std::size_t file_size,
        const Mode& mode);

    /**
     * Add blocks for {@code file_contents} to the end of {@code blocks}.
     * Update {@code ids_to_add} with the ids of new blocks to add.
     * Note that {@code file_contents} must be non-empty.
     */
    void replace_blocks_for_contents(
        const std::vector<BlockPtr>* blocks,
        const std::string& file_contents,
        std::vector<BlockId>* ids_to_add);


 private:
    std::shared_ptr<BlockManager> block_manager_;
    BlockPtr root_block_;
    const dev_t dev_;
};


}  // namespace pingfs


#endif
