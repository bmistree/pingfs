#include <pingfs/block/block_data/block_data.hpp>
#include <pingfs/block/block_data/dir_file_block_data.hpp>

namespace pingfs {

DirFileBlockData::DirFileBlockData(const proto::DirFileProto& proto)
  : DirFileBlockData(proto.name(),
      Stat(proto.stat()),
      BlockData::iters_to_vector(proto.children())) {
}

DirFileBlockData::DirFileBlockData(const DirFileBlockData& other,
    const std::vector<BlockId>& new_children)
  : DirFileBlockData(other.name_, other.stat_, new_children) {
}

DirFileBlockData::DirFileBlockData(
    const std::string& name, const Stat& stat,
    const std::vector<BlockId>& children)
  : name_(name),
    stat_(stat),
    children_(children) {
}

DirFileBlockData::~DirFileBlockData() {
}

void DirFileBlockData::gen_proto(proto::DirFileProto* proto) const {
    proto->set_name(name_);
    stat_.gen_proto(proto->mutable_stat());
    for (auto iter = children_.cbegin(); iter != children_.cend();
         ++iter) {
        proto->add_children(*iter);
    }
}

void DirFileBlockData::gen_block_data_proto(
    proto::BlockDataProto* proto) const {
    gen_proto(proto->mutable_dir_file());
}

bool DirFileBlockData::operator==(const BlockData &o) const {
    const DirFileBlockData* other =
        dynamic_cast<const DirFileBlockData*>(&o);
    if (other == nullptr) {
        return false;
    }

    return ((name_ == other->name_) &&
        (children_ == other->children_) &&
        (stat_ == other->stat_));
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

const Stat& DirFileBlockData::get_stat() const {
    return stat_;
}

bool DirFileBlockData::is_dir() const {
    return stat_.is_dir();
}

}  // namespace pingfs
