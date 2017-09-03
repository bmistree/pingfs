#include <pingfs/block/block_data/block_data.hpp>
#include <pingfs/block/block_data/dir_file_block_data.hpp>

namespace pingfs {

DirFileBlockData::DirFileBlockData(const proto::DirFileProto& proto)
  : DirFileBlockData(proto.name(),
      BlockData::iters_to_vector(proto.children())) {
}

DirFileBlockData::DirFileBlockData(
    const std::string& name, const std::vector<BlockId>& children)
  : name_(name),
    children_(children) {
}

DirFileBlockData::~DirFileBlockData() {
}

void DirFileBlockData::gen_proto(proto::DirFileProto* proto) const {
    proto->set_name(name_);
    for (auto iter = children_.cbegin(); iter != children_.cend();
         ++iter) {
        proto->add_children(*iter);
    }
}

bool DirFileBlockData::operator==(const BlockData &o) const {
    const DirFileBlockData* other =
        dynamic_cast<const DirFileBlockData*>(&o);
    if (other == nullptr) {
        return false;
    }

    return ((name_ == other->name_) &&
        (children_ == other->children_));
}

bool DirFileBlockData::operator!=(const BlockData &other) const {
    return !(*this == other);
}

const std::string& DirFileBlockData::get_name() const {
    return name_;
}

const std::vector<BlockId>& DirFileBlockData::get_children() const {
    return children_;
}

}  // namespace pingfs
