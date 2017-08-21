#ifndef _BLOCK_MANAGER_
#define _BLOCK_MANAGER_

#include <pingfs/block/block_request.hpp>
#include <pingfs/block/block_response.hpp>
#include <pingfs/block/block_data/block_data.hpp>

#include <memory>
#include <string>

namespace pingfs {

class BlockManager {
 public:
    virtual const Block create_block(std::shared_ptr<const BlockData> data) = 0;
    virtual void free_block(BlockId block_id) = 0;
    virtual const BlockResponse get_blocks(
        const BlockRequest& block_request) = 0;
    virtual ~BlockManager();
};

}  // namespace pingfs

#endif
