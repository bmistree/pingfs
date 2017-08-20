#ifndef _FILE_START_BLOCK_DATA_
#define _FILE_START_BLOCK_DATA_

#include "block_data.pb.h"
#include "block_data.hpp"

#include <string>
#include <vector>


namespace pingfs {

class FileStartBlockData : BlockData {
 public:
    FileStartBlockData(const FileStartProto& proto);
    FileStartBlockData(const std::string& filename,
        const std::vector<BlockId>& children);
    virtual ~FileStartBlockData();

    void gen_proto(FileStartProto* proto) const;
    bool operator==(const FileStartBlockData &other) const;
    bool operator!=(const FileStartBlockData &other) const;
    
    const std::string& get_filename() const;
    const std::vector<BlockId>& get_children() const;

 private:
    const std::string filename_;
    const std::vector<BlockId> children_;
};


}  // namespace pingfs


#endif
