#ifndef _LINK_BLOCK_DATA_
#define _LINK_BLOCK_DATA_

#include "block_data.pb.h"
#include "block_data.hpp"

#include <string>
#include <vector>


namespace pingfs {


class LinkBlockData : BlockData {
 public:
    LinkBlockData(const LinkProto& proto);
    LinkBlockData(const std::vector<BlockId>& children);
    virtual ~LinkBlockData();
    const std::vector<BlockId>& get_children() const;

 private:
    const std::vector<BlockId> children_;
};

}  // namespace pingfs

#endif
