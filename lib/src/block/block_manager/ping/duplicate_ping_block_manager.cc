#include <pingfs/block/block_manager/ping/duplicate_ping_block_manager.hpp>

#include <thread>

namespace pingfs {

DuplicatePingBlockManager::DuplicatePingBlockManager(
    std::chrono::seconds delay,
    std::shared_ptr<IdSupplier> id_supplier,
    std::shared_ptr<PingBlockService> ping_block_service)
  : PingBlockManager(id_supplier, ping_block_service),
    delay_(delay) {
}

DuplicatePingBlockManager::~DuplicatePingBlockManager() {
}

DuplicatePingBlockManager::BlockPtr DuplicatePingBlockManager::create_block(
    DataPtr data) {
    BlockPtr ptr = PingBlockManager::create_block(data);

    // delay some time and then register the block again
    std::thread t(
        &DuplicatePingBlockManager::delayed_send,
        this,
        ptr);
    t.detach();

    return ptr;
}

void DuplicatePingBlockManager::delayed_send(BlockPtr to_send) {
    std::this_thread::sleep_for(delay_);
    ping_block_service_->register_block(to_send);
}


}  // namespace pingfs
