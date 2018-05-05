#ifndef _DUPLICATE_PING_BLOCK_MANAGER_
#define _DUPLICATE_PING_BLOCK_MANAGER_

#include <pingfs/block/block_data/block_data.hpp>

#include <chrono>
#include <memory>

#include "ping_block_service.hpp"
#include "ping_block_manager.hpp"

namespace pingfs {

class DuplicatePingBlockManager : PingBlockManager {
 private:
    using BlockPtr = std::shared_ptr<const Block>;
    using DataPtr = std::shared_ptr<const BlockData>;

 public:
    DuplicatePingBlockManager(
        std::chrono::seconds delay,
        std::shared_ptr<IdSupplier> id_supplier,
        std::shared_ptr<PingBlockService> ping_block_service);

    virtual ~DuplicatePingBlockManager();
    BlockPtr create_block(DataPtr data) override;

 private:
    void delayed_send(BlockPtr to_send);

 private:
    std::chrono::seconds delay_;
};

}  // namespace pingfs

#endif
