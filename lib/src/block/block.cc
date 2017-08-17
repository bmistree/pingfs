
#include <pingfs/block/block.hpp>

#include <memory>
#include <string>

namespace pingfs {

Block::Block(BlockId block_id, const std::string& data)
  : Block(block_id, std::make_shared<const std::string>(data)) {
}

Block::Block(BlockId block_id, std::shared_ptr<const std::string> data)
  : block_id_(block_id),
    data_(data) {
}

Block::~Block() {
}

std::shared_ptr<const std::string> Block::get_data() const {
    return data_;
}



}  // namespace pingfs
