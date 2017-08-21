#include <pingfs/block/block_manager/async_block_manager.hpp>

namespace pingfs {

AsyncBlockManager::AsyncBlockManager() {
}

AsyncBlockManager::~AsyncBlockManager() {
}

const BlockResponse AsyncBlockManager::get_blocks(
    const BlockRequest& block_request) {
    // FIXME: Must finish
    throw "Unsupported operation";
}

}  // namespace pingfs
