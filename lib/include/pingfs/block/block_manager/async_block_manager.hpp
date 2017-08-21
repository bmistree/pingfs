#ifndef _ASYNC_BLOCK_MANAGER_
#define _ASYNC_BLOCK_MANAGER_

#include <pingfs/block/block_data/block_data.hpp>
#include <pingfs/block/block_request.hpp>
#include <pingfs/block/block_response.hpp>
#include <pingfs/block/block_manager/block_manager.hpp>

#include <unordered_map>

#include "block_manager.hpp"

namespace pingfs {

class AsyncBlockManager : public BlockManager {
 public:
    AsyncBlockManager();
    virtual ~AsyncBlockManager();

 public:
    const BlockResponse get_blocks(const BlockRequest& block_request) override;
};

}  // namespace pingfs

#endif
