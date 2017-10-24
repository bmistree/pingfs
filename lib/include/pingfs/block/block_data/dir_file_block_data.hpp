#ifndef _DIR_FILE_BLOCK_DATA_
#define _DIR_FILE_BLOCK_DATA_

#include <block_data.pb.h>

#include <string>
#include <vector>

#include "block_data.hpp"
#include "stat.hpp"

namespace pingfs {

/**
 * Either the inode for a file or an inode for a directory.
 */
class DirFileBlockData : public BlockData {
 public:
    explicit DirFileBlockData(const proto::DirFileProto& proto);
    DirFileBlockData(const DirFileBlockData& other,
        const std::vector<BlockId>& new_children);

    DirFileBlockData(const std::string& name,
        const Stat& stat,
        const std::vector<BlockId>& children);
    virtual ~DirFileBlockData();

    const std::string& get_name() const;
    const std::vector<BlockId>& get_children() const;
    const Stat& get_stat() const;

    bool operator==(const BlockData &other) const;
    bool operator!=(const BlockData &other) const;

    void gen_proto(proto::DirFileProto* proto) const;
    void gen_block_data_proto(
        proto::BlockDataProto* proto) const override;

    bool is_dir() const;

 private:
    /**
     * The name of the directory or file that this corresponds to.
     */
    const std::string name_;
    const Stat stat_;
    const std::vector<BlockId> children_;
};


}  // namespace pingfs

#endif
