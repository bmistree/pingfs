#ifndef _LINK_BLOCK_DATA_
#define _LINK_BLOCK_DATA_

#include <block_data.pb.h>
#include <string>
#include <vector>

#include "block_data.hpp"

namespace pingfs {


class LinkBlockData : public BlockData {
 public:
    explicit LinkBlockData(const proto::LinkProto& proto);
    explicit LinkBlockData(const std::vector<BlockId>& children);
    virtual ~LinkBlockData();

    void gen_proto(proto::LinkProto* proto) const;
    bool operator==(const BlockData &other) const;
    bool operator!=(const BlockData &other) const;

    const std::vector<BlockId>& get_children() const;

    void gen_block_data_proto(
        proto::BlockDataProto* proto) const override;
    std::string to_string() const override;

 private:
    const std::vector<BlockId> children_;
};

}  // namespace pingfs

#endif
