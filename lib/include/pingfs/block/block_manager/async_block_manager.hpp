#ifndef _ASYNC_BLOCK_MANAGER_
#define _ASYNC_BLOCK_MANAGER_

#include <boost/thread/mutex.hpp>

#include <pingfs/block/block_data/block_data.hpp>
#include <pingfs/block/block_request.hpp>
#include <pingfs/block/block_response.hpp>
#include <pingfs/block/block_manager/block_manager.hpp>
#include <pingfs/util/subscriber.hpp>

#include <unordered_map>
#include <vector>

#include "async_response.hpp"
#include "block_manager.hpp"

namespace pingfs {

class AsyncBlockManager :
        public BlockManager,
        public Subscriber<std::shared_ptr<const Block>> {
 public:
    AsyncBlockManager();
    virtual ~AsyncBlockManager();

 public:
    // BlockManager overrides
    const BlockResponse get_blocks(const BlockRequest& block_request) override;

 public:
    // Subscriber overrides
    void process(const std::shared_ptr<const Block>& block) override;

 private:
    boost::mutex map_mutex_;
    /**
     * A map from block ids to those responses that are waiting
     * on those blocks.
     */
    std::unordered_map<BlockId, std::vector<std::shared_ptr<AsyncResponse>>>
        waiting_;
};

}  // namespace pingfs

#endif
