#ifndef _DIR_BLOCK_DATA_
#define _DIR_BLOCK_DATA_

#include <block_data.pb.h>

#include <string>
#include <vector>

#include "block_data.hpp"

namespace pingfs {

class DirBlockData : public BlockData {
 public:
    explicit DirBlockData(const DirProto& proto);
    DirBlockData(const std::string& dirname,
        const std::vector<BlockId>& children);
    virtual ~DirBlockData();

    const std::string& get_dirname() const;
    const std::vector<BlockId>& get_children() const;

    bool operator==(const BlockData &other) const;
    bool operator!=(const BlockData &other) const;

    void gen_proto(DirProto* proto) const;

 private:
    const std::string dirname_;
    const std::vector<BlockId> children_;
};


}  // namespace pingfs

#endif
