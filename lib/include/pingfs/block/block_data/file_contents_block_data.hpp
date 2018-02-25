#ifndef _FILE_CONTENTS_BLOCK_DATA_
#define _FILE_CONTENTS_BLOCK_DATA_

#include <block_data.pb.h>

#include <memory>
#include <string>
#include <vector>

#include "block_data.hpp"

namespace pingfs {

class FileContentsBlockData : public BlockData {
 public:
    explicit FileContentsBlockData(
        const proto::FileContentsProto& proto);
    explicit FileContentsBlockData(const std::string& data);
    explicit FileContentsBlockData(std::shared_ptr<const std::string> data);
    virtual ~FileContentsBlockData();
    void gen_proto(proto::FileContentsProto* proto) const;
    bool operator==(const BlockData &other) const;
    bool operator!=(const BlockData &other) const;

    void gen_block_data_proto(
        proto::BlockDataProto* proto) const override;

    std::string to_string() const override;
    
    std::shared_ptr<const std::string> get_data() const;

 private:
    const std::shared_ptr<const std::string> data_;
};

}  // namespace pingfs

#endif
