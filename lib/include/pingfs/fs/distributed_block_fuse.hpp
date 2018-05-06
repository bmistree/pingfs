#ifndef _DISTRIBUTED_BLOCK_FUSE_
#define _DISTRIBUTED_BLOCK_FUSE_

#include <pingfs/block/block.hpp>
#include <pingfs/block/block_manager/id_supplier/updating_id_supplier.hpp>
#include <pingfs/block/block_manager/ping/distributed_freed_service.hpp>

#include <pingfs/util/subscriber.hpp>

#include "block_fuse.hpp"

namespace pingfs {

class DistributedBlockFuse :
        public BlockFuse,
        public Subscriber<std::shared_ptr<const Block>> {

 public:
    DistributedBlockFuse(std::shared_ptr<BlockManager> block_manager,
        dev_t dev,
        std::shared_ptr<UpdatingIdSupplier> updating_id_supplier,
        std::shared_ptr<DistributedFreedService> distributed_ping_service);

    virtual ~DistributedBlockFuse();

    // Should issue ping to other host when generating root block
    void set_root_block(BlockPtr new_root) override;

    // Subscriber override
    void process(const BlockPtr& block) override;

 private:
    std::shared_ptr<UpdatingIdSupplier> updating_id_supplier_;
    std::shared_ptr<DistributedFreedService> distributed_ping_service_;
};


}  // namespace pingfs

#endif
