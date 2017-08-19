#ifndef _DIR_BLOCK_DATA_
#define _DIR_BLOCK_DATA_

#include "block_data.pb.h"
#include "block_data.hpp"

#include <string>
#include <vector>

namespace pingfs {

class DirBlockData : BlockData {
 public:
    DirBlockData(const DirProto& proto);
    DirBlockData(const std::string& dirname,
        const std::vector<BlockId>& children);
    virtual ~DirBlockData();

    const std::string& get_dirname() const;
    const std::vector<BlockId>& get_children() const;

    bool operator==(const DirBlockData &other) const;
    bool operator!=(const DirBlockData &other) const;
    
    void gen_proto(DirProto* proto) const;

 private:
    const std::string dirname_;
    const std::vector<BlockId> children_;
};


}  // namespace pingfs

#endif
