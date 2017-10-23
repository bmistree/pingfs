#include <pingfs/block/block_manager/ping/ping_block_manager.hpp>

namespace pingfs {

PingBlockManager::PingBlockManager(
    std::shared_ptr<IdSupplier> id_supplier,
    boost::asio::io_service* io_service,
    const std::string& remote_endpt)
  : AsyncBlockManager(id_supplier),
    ping_(io_service),
    endpoint_(ping_.resolve(remote_endpt)) {
}

PingBlockManager::~PingBlockManager() {
}

PingBlockManager::BlockPtr PingBlockManager::create_block(DataPtr data) {
    throw "Unsupported operation";
}

void PingBlockManager::free_block(BlockId block_id) {
    throw "Unsupported operation";
}

}  // namespace pingfs
