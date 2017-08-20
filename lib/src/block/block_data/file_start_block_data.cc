#include <pingfs/block/block_data/block_data.hpp>
#include <pingfs/block/block_data/file_start_block_data.hpp>

namespace pingfs {

FileStartBlockData::FileStartBlockData(const FileStartProto& proto)
  : FileStartBlockData(proto.filename(),
      BlockData::iters_to_vector(proto.children())) {
}

FileStartBlockData::FileStartBlockData(
    const std::string& filename, const std::vector<BlockId>& children)
  : filename_(filename),
    children_(children) {
}

FileStartBlockData::~FileStartBlockData() {
}

void FileStartBlockData::gen_proto(FileStartProto* proto) const {
    proto->set_filename(filename_);
    for (auto iter = children_.cbegin(); iter != children_.cend();
         ++iter) {
        proto->add_children(*iter);
    }
}

bool FileStartBlockData::operator==(const BlockData &o) const {
    const FileStartBlockData* other =
        dynamic_cast<const FileStartBlockData*>(&o);
    if (other == nullptr) {
        return false;
    }

    return ((filename_ == other->filename_) &&
        (children_ == other->children_));
}

bool FileStartBlockData::operator!=(const BlockData &other) const {
    return !(*this == other);
}


const std::string& FileStartBlockData::get_filename() const {
    return filename_;
}

const std::vector<BlockId>& FileStartBlockData::get_children() const {
    return children_;
}


}  // namespace pingfs
