
#include <pingfs/block/block.hpp>
#include <pingfs/block/block_data/block_data_factory.hpp>

#include <memory>
#include <string>

namespace pingfs {

Block::Block(const proto::BlockProto& proto)
  : Block(proto.id(), BlockDataFactory::generate(proto.data())) {
}

Block::Block(BlockId block_id, std::shared_ptr<const BlockData> data)
  : block_id_(block_id),
    data_(data) {
}

BlockId Block::get_id() const {
    return block_id_;
}

bool Block::operator==(const Block &other) const {
    return ((block_id_ == other.block_id_) &&
        (*data_ == *other.data_));
}

bool Block::operator!=(const Block &other) const {
    return !(*this == other);
}

void Block::gen_proto(proto::BlockProto* proto) const {
    proto->set_id(block_id_);
    data_->gen_block_data_proto(proto->mutable_data());
}

Block::~Block() {
}

std::shared_ptr<const BlockData> Block::get_data() const {
    return data_;
}

std::string Block::to_string() const {
    return "[block_id=" + std::to_string(block_id_) + ", data="
        + data_->to_string() + "]";
}

}  // namespace pingfs
