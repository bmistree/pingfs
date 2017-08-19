#ifndef _FILE_CONTENTS_BLOCK_DATA_
#define _FILE_CONTENTS_BLOCK_DATA_

#include "block_data.pb.h"
#include "block_data.hpp"

#include <memory>
#include <string>
#include <vector>

namespace pingfs {

class FileContentsBlockData : BlockData {
 public:
    FileContentsBlockData(const FileContentsProto& proto);
    FileContentsBlockData(std::shared_ptr<const std::string> data);
    virtual ~FileContentsBlockData();

    std::shared_ptr<const std::string> get_data() const;

 private:
    const std::shared_ptr<const std::string> data_;
};

}  // namespace pingfs

#endif
