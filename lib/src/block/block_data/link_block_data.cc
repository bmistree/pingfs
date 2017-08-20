#include <pingfs/block/block_data/block_data.hpp>
#include <pingfs/block/block_data/link_block_data.hpp>

namespace pingfs {

LinkBlockData::LinkBlockData(const LinkProto& proto)
  : LinkBlockData(BlockData::iters_to_vector(proto.children())) {
}

LinkBlockData::LinkBlockData(const std::vector<BlockId>& children)
  : children_(children) {
}

void LinkBlockData::gen_proto(LinkProto* proto) const {
    for (auto iter = children_.cbegin(); iter != children_.cend();
         ++iter) {
        proto->add_children(*iter);
    }
}

bool LinkBlockData::operator==(const BlockData &o) const {
    const LinkBlockData* other =
        dynamic_cast<const LinkBlockData*>(&o);
    if (other == nullptr) {
        return false;
    }
    return children_ == other->children_;
}

bool LinkBlockData::operator!=(const BlockData &other) const {
    return !(*this == other);
}

LinkBlockData::~LinkBlockData() {
}

const std::vector<BlockId>& LinkBlockData::get_children() const {
    return children_;
}

}  // namespace pingfs
