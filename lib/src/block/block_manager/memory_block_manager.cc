#include <pingfs/block/block_manager/memory_block_manager.hpp>
#include <boost/thread/locks.hpp>

#include <memory>

namespace pingfs {

MemoryBlockManager::MemoryBlockManager(
    std::shared_ptr<IdSupplier> id_supplier)
  : BlockManager(id_supplier),
    map_(),
    map_mutex_() {
}

MemoryBlockManager::~MemoryBlockManager() {
}

std::shared_ptr<const Block> MemoryBlockManager::create_block(
    std::shared_ptr<const BlockData> data) {
    std::shared_ptr<const Block> block =
        std::make_shared<const Block>(next_id(), data);
    {
        boost::mutex::scoped_lock map_lock(map_mutex_);
        map_[block->get_id()] = block;
    }
    return block;
}

void MemoryBlockManager::free_block(BlockId block_id) {
    boost::mutex::scoped_lock map_lock(map_mutex_);
    free_id(block_id);
    map_.erase(block_id);
}

std::shared_ptr<const BlockResponse> MemoryBlockManager::get_blocks(
    const BlockRequest& block_request) {
    std::vector<std::shared_ptr<const Block>> retrieved_blocks;
    const std::vector<BlockId>& request_blocks = block_request.get_blocks();
    for (auto iter = request_blocks.cbegin();
         iter != request_blocks.cend(); ++iter) {
        retrieved_blocks.push_back(map_[*iter]);
    }
    return std::make_shared<const BlockResponse>(retrieved_blocks);
}

std::size_t MemoryBlockManager::num_blocks() {
    boost::mutex::scoped_lock locker(map_mutex_);
    return map_.size();
}

std::string MemoryBlockManager::to_string() {
    bool first_entry = true;
    std::string stringified = "[";
    boost::mutex::scoped_lock locker(map_mutex_);
    for (auto iter = map_.cbegin(); iter != map_.cend();
         ++iter) {
        if (!first_entry) {
            stringified += ", ";
        }
        first_entry = false;
        stringified += iter->second->to_string();
    }
    stringified += "]";
    return stringified;
}

}  // namespace pingfs
