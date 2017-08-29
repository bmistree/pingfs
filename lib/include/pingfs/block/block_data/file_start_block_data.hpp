#ifndef _FILE_START_BLOCK_DATA_
#define _FILE_START_BLOCK_DATA_

#include <block_data.pb.h>

#include <string>
#include <vector>

#include "block_data.hpp"

namespace pingfs {

class FileStartBlockData : public BlockData {
 public:
    explicit FileStartBlockData(
        const proto::FileStartProto& proto);
    FileStartBlockData(const std::string& filename,
        const std::vector<BlockId>& children);
    virtual ~FileStartBlockData();

    void gen_proto(proto::FileStartProto* proto) const;
    bool operator==(const BlockData &other) const;
    bool operator!=(const BlockData &other) const;

    const std::string& get_filename() const;
    const std::vector<BlockId>& get_children() const;

 private:
    const std::string filename_;
    const std::vector<BlockId> children_;
};


}  // namespace pingfs


#endif
