#ifndef _MEMORY_BLOCK_MANAGER_
#define _MEMORY_BLOCK_MANAGER_

#include <boost/thread/mutex.hpp>

#include <pingfs/block/block_response.hpp>
#include <pingfs/block/block_data/block_data.hpp>

#include <memory>
#include <string>
#include <unordered_map>

#include "block_manager.hpp"


namespace pingfs {

class MemoryBlockManager : public BlockManager {
 public:
    explicit MemoryBlockManager(std::shared_ptr<IdSupplier> id_supplier);

    std::shared_ptr<const Block> create_block(
        std::shared_ptr<const BlockData> data) override;
    void free_block(BlockId block_id) override;
    std::shared_ptr<const BlockResponse> get_blocks(
        const BlockRequest& block_request) override;

    std::size_t num_blocks();

    ~MemoryBlockManager() override;

 private:
    std::unordered_map<BlockId, std::shared_ptr<const Block>> map_;
    boost::mutex map_mutex_;
};

}  // namespace pingfs

#endif
