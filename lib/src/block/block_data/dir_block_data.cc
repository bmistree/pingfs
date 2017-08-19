#include <pingfs/block/block_data/block_data.hpp>
#include <pingfs/block/block_data/dir_block_data.hpp>

namespace pingfs {

DirBlockData::DirBlockData(const DirProto& proto)
  : DirBlockData(proto.dirname(),
      BlockData::iters_to_vector(proto.children())) {
}

DirBlockData::DirBlockData(
    const std::string& dirname, const std::vector<BlockId>& children)
  : dirname_(dirname),
    children_(children) {
}

DirBlockData::~DirBlockData() {
}

void DirBlockData::gen_proto(DirProto* proto) const {
    proto->set_dirname(dirname_);
    for (auto iter = children_.cbegin(); iter != children_.cend();
         ++iter) {
        proto->add_children(*iter);
    }
}

bool DirBlockData::operator==(const DirBlockData &other) const {
    return ((dirname_ == other.dirname_) &&
        (children_ == other.children_));
}

bool DirBlockData::operator!=(const DirBlockData &other) const {
    return !(*this == other);
}

const std::string& DirBlockData::get_dirname() const {
    return dirname_;
}

const std::vector<BlockId>& DirBlockData::get_children() const {
    return children_;
}

}  // namespace pingfs
