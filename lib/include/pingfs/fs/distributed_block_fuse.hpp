#ifndef _DISTRIBUTED_BLOCK_FUSE_
#define _DISTRIBUTED_BLOCK_FUSE_

#include <pingfs/block/block.hpp>
#include <pingfs/block/block_manager/id_supplier/updating_id_supplier.hpp>
#include <pingfs/util/subscriber.hpp>

#include "block_fuse.hpp"

namespace pingfs {

class DistributedBlockFuse :
        public BlockFuse,
        public Subscriber<std::shared_ptr<const Block>> {

 public:
    DistributedBlockFuse(std::shared_ptr<BlockManager> block_manager,
        dev_t dev,
        std::shared_ptr<UpdatingIdSupplier> updating_id_supplier);

    virtual ~DistributedBlockFuse();

    // Subscriber overrides
    void process(const BlockPtr& block) override;

 private:
    std::shared_ptr<UpdatingIdSupplier> updating_id_supplier_;
};


}  // namespace pingfs

#endif
