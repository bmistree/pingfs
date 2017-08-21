#include <pingfs/block/block_manager/memory_block_manager.hpp>
#include <boost/thread/locks.hpp>

#include <memory>

namespace pingfs {

MemoryBlockManager::MemoryBlockManager()
  : next_block_id_(1),
    next_id_mutex_(),
    map_(),
    map_mutex_() {
}

MemoryBlockManager::~MemoryBlockManager() {
}

BlockId MemoryBlockManager::get_next_block_id() {
    boost::mutex::scoped_lock locker(next_id_mutex_);
    return next_block_id_++;
}

const Block MemoryBlockManager::create_block(
    std::shared_ptr<const BlockData> data) {
    std::shared_ptr<const Block> block =
        std::make_shared<const Block>(Block(get_next_block_id(), data));
    {
        boost::mutex::scoped_lock map_lock(map_mutex_);
        map_[block->get_block_id()] = block;
    }
    return *block;
}

void MemoryBlockManager::free_block(BlockId block_id) {
    boost::mutex::scoped_lock map_lock(map_mutex_);
    map_.erase(block_id);
}

const BlockResponse MemoryBlockManager::get_blocks(
    const BlockRequest& block_request) {
    std::vector<std::shared_ptr<const Block>> retrieved_blocks;
    const std::vector<BlockId>& request_blocks = block_request.get_blocks();
    for (auto iter = request_blocks.cbegin();
         iter != request_blocks.cend(); ++iter) {
        retrieved_blocks.push_back(map_[*iter]);
    }
    return BlockResponse(retrieved_blocks);
}

}  // namespace pingfs
