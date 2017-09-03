#ifndef _DIR_FILE_BLOCK_DATA_
#define _DIR_FILE_BLOCK_DATA_

#include <block_data.pb.h>

#include <string>
#include <vector>

#include "block_data.hpp"

namespace pingfs {

/**
 * Either the inode for a file or an inode for a directory.
 */
class DirFileBlockData : public BlockData {
 public:
    explicit DirFileBlockData(const proto::DirFileProto& proto);
    DirFileBlockData(const std::string& name,
        const std::vector<BlockId>& children);
    virtual ~DirFileBlockData();

    const std::string& get_name() const;
    const std::vector<BlockId>& get_children() const;

    bool operator==(const BlockData &other) const;
    bool operator!=(const BlockData &other) const;

    void gen_proto(proto::DirFileProto* proto) const;

 private:
    /**
     * The name of the directory or file that this corresponds to.
     */
    const std::string name_;
    const std::vector<BlockId> children_;
};


}  // namespace pingfs

#endif
