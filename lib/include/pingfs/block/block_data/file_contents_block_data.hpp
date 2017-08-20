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
    explicit FileContentsBlockData(const FileContentsProto& proto);
    explicit FileContentsBlockData(std::shared_ptr<const std::string> data);
    virtual ~FileContentsBlockData();
    void gen_proto(FileContentsProto* proto) const;
    bool operator==(const BlockData &other) const;
    bool operator!=(const BlockData &other) const;

    std::shared_ptr<const std::string> get_data() const;

 private:
    const std::shared_ptr<const std::string> data_;
};

}  // namespace pingfs

#endif
