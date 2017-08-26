#include <pingfs/block/block_data/block_data.hpp>
#include <pingfs/block/block_data/file_contents_block_data.hpp>

namespace pingfs {

FileContentsBlockData::FileContentsBlockData(const FileContentsProto& proto)
  : FileContentsBlockData(std::make_shared<const std::string>(proto.data())) {
}

FileContentsBlockData::FileContentsBlockData(const std::string& data)
  : FileContentsBlockData(std::make_shared<const std::string>(data)) {
}

FileContentsBlockData::FileContentsBlockData(
    std::shared_ptr<const std::string> data)
  : data_(data) {
}

FileContentsBlockData::~FileContentsBlockData() {
}

bool FileContentsBlockData::operator==(const BlockData &o) const {
    const FileContentsBlockData* other =
        dynamic_cast<const FileContentsBlockData*>(&o);
    if (other == nullptr) {
        return false;
    }
    return *data_ == *(other->data_);
}

bool FileContentsBlockData::operator!=(const BlockData &other) const {
    return !(*this == other);
}


void FileContentsBlockData::gen_proto(FileContentsProto* proto) const {
    proto->set_data(*data_);
}

std::shared_ptr<const std::string> FileContentsBlockData::get_data() const {
    return data_;
}


}  // namespace pingfs
