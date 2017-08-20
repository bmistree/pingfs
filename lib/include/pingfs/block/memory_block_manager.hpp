#ifndef _MEMORY_BLOCK_MANAGER_
#define _MEMORY_BLOCK_MANAGER_

#include <boost/thread/mutex.hpp>

#include "block_data/block_data.hpp"

#include <memory>
#include <string>
#include <unordered_map>

#include "block_manager.hpp"
#include "block_response.hpp"

namespace pingfs {

class MemoryBlockManager : public BlockManager {
 public:
    MemoryBlockManager();

    const Block create_block(std::shared_ptr<const BlockData> data) override;
    void free_block(BlockId block_id) override;
    const BlockResponse get_blocks(const BlockRequest& block_request) override;
    ~MemoryBlockManager() override;

 private:
    BlockId get_next_block_id();

 private:
    BlockId next_block_id_;
    boost::mutex next_id_mutex_;
    std::unordered_map<BlockId, std::shared_ptr<const Block>> map_;
    boost::mutex map_mutex_;
};

}  // namespace pingfs

#endif
