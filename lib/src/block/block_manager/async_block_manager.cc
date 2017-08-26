#include <pingfs/block/block_manager/async_block_manager.hpp>

namespace pingfs {

AsyncBlockManager::AsyncBlockManager()
  : map_mutex_(),
    waiting_() {
}

AsyncBlockManager::~AsyncBlockManager() {
}

void AsyncBlockManager::process(const std::shared_ptr<const Block>& block) {
    boost::mutex::scoped_lock scope(map_mutex_);
    if (waiting_.find(block->get_block_id()) == waiting_.end()) {
        return;
    }

    std::vector<std::shared_ptr<AsyncResponse>>& waiting_vec =
        waiting_[block->get_block_id()];

    for (auto iter = waiting_vec.cbegin();
         iter != waiting_vec.cend(); ++iter) {
        (*iter)->update(block);
    }

    waiting_.erase(block->get_block_id());
}

std::shared_ptr<const BlockResponse> AsyncBlockManager::get_blocks(
    const BlockRequest& block_request) {

    const std::vector<BlockId>& blocks = block_request.get_blocks();
    std::shared_ptr<AsyncResponse> resp_ptr =
        std::make_shared<AsyncResponse>(block_request);

    {
        boost::mutex::scoped_lock scope(map_mutex_);
        for (auto block_iter = blocks.cbegin();
             block_iter != blocks.cend(); ++block_iter) {
            std::vector<std::shared_ptr<AsyncResponse>>& resp_vec =
                waiting_[*block_iter];
            resp_vec.push_back(resp_ptr);
        }
    }
    return resp_ptr->get_future().get();
}

}  // namespace pingfs
