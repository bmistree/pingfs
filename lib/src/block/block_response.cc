#include <pingfs/block/block_response.hpp>

namespace pingfs {

BlockResponse::BlockResponse(
    const std::vector<std::shared_ptr<const Block>>& blocks)
  : blocks_(blocks) {
}

const std::vector<std::shared_ptr<const Block>>& BlockResponse::get_blocks()
    const {
    return blocks_;
}

BlockResponse::~BlockResponse() {
}

}  // namespace pingfs
