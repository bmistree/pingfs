
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

Block::~Block() {
}

std::shared_ptr<const BlockData> Block::get_data() const {
    return data_;
}

}  // namespace pingfs
