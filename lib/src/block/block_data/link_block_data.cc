#include <pingfs/block/block_data/block_data.hpp>
#include <pingfs/block/block_data/link_block_data.hpp>

namespace pingfs {

LinkBlockData::LinkBlockData(const LinkProto& proto)
 : LinkBlockData(BlockData::iters_to_vector(proto.children())) {
}

LinkBlockData::LinkBlockData(const std::vector<BlockId>& children)
  : children_(children) {
}

LinkBlockData::~LinkBlockData() {
}

const std::vector<BlockId>& LinkBlockData::get_children() const {
    return children_;
}

}  // namespace pingfs
