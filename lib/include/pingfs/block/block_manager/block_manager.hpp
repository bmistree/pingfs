#ifndef _BLOCK_MANAGER_
#define _BLOCK_MANAGER_

#include <pingfs/block/block_request.hpp>
#include <pingfs/block/block_response.hpp>
#include <pingfs/block/block_data/block_data.hpp>
#include <pingfs/block/block_manager/id_supplier/id_supplier.hpp>

#include <memory>
#include <string>

namespace pingfs {

class BlockManager {
 public:
    virtual std::shared_ptr<const Block> create_block(
        std::shared_ptr<const BlockData> data) = 0;
    virtual void free_block(BlockId block_id) = 0;
    virtual std::shared_ptr<const BlockResponse> get_blocks(
        const BlockRequest& block_request) = 0;
    virtual ~BlockManager();

 protected:
    explicit BlockManager(std::shared_ptr<IdSupplier> id_supplier);

    BlockId next_id();
    void free_id(BlockId block_id);

 private:
    std::shared_ptr<IdSupplier> id_supplier_;
};

}  // namespace pingfs

#endif
