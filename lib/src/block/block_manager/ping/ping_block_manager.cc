#include <pingfs/block/block_manager/ping/ping_block_manager.hpp>

namespace pingfs {

PingBlockManager::PingBlockManager(
    std::shared_ptr<IdSupplier> id_supplier,
    std::shared_ptr<PingBlockService> ping_block_service)
  : AsyncBlockManager(id_supplier),
    ping_block_service_(ping_block_service) {
    ping_block_service_->subscribe(this);
}

PingBlockManager::~PingBlockManager() {
    ping_block_service_->unsubscribe(this);
}

PingBlockManager::BlockPtr PingBlockManager::create_block(
    DataPtr data) {
    std::shared_ptr<const Block> block =
        std::make_shared<const Block>(next_id(), data);
    ping_block_service_->register_block(block);
    return block;
}

void PingBlockManager::free_block(BlockId block_id) {
    ping_block_service_->free_block(block_id);
}

}  // namespace pingfs
