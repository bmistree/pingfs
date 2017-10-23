#include <pingfs/block/block_manager/block_manager.hpp>

namespace pingfs {

BlockManager::BlockManager(
    std::shared_ptr<IdSupplier> id_supplier)
  : id_supplier_(id_supplier) {
}

BlockManager::~BlockManager() {
}

BlockId BlockManager::next_id() {
    return id_supplier_->next_id();
}
void BlockManager::free_id(BlockId block_id) {
    id_supplier_->free_id(block_id);
}

}  // namespace pingfs
