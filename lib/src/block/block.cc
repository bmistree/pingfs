
#include <pingfs/block/block.hpp>

#include <memory>
#include <string>

namespace pingfs {

Block::Block(const BlockProto& proto)
 : Block(proto.id(), proto.data()) {
}

Block::Block(BlockId block_id, const BlockDataProto& data_proto)
  : Block(block_id, std::make_shared<const BlockDataProto>(data_proto)) {
}

Block::Block(BlockId block_id, std::shared_ptr<const BlockDataProto> data)
  : block_id_(block_id),
    data_(data) {
}

BlockId Block::get_block_id() const {
    return block_id_;
}

bool Block::operator==(const Block &other) const {
    return block_id_ == other.block_id_;
    // return ((block_id_ == other.block_id_) &&
    //     (*data_ == *(other.data_)));
}

bool Block::operator!=(const Block &other) const {
    return !(*this == other);
}

Block::~Block() {
}

std::shared_ptr<const BlockDataProto> Block::get_data() const {
    return data_;
}

}  // namespace pingfs
