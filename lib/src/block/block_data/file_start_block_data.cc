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

const std::string& FileStartBlockData::get_filename() const {
    return filename_;
}

const std::vector<BlockId>& FileStartBlockData::get_children() const {
    return children_;
}


}  // namespace pingfs
