#include <pingfs/block/block.hpp>
#include <pingfs/block/block_request.hpp>


namespace pingfs {

BlockRequest::BlockRequest(const std::vector<BlockId>& blocks)
  : blocks_(blocks) {
}

BlockRequest::~BlockRequest() {
}

const std::vector<BlockId>& BlockRequest::get_blocks() const {
    return blocks_;
}

}  // namespace pingfs
