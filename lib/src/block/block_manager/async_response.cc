#include <boost/thread/locks.hpp>
#include <pingfs/block/block_manager/async_response.hpp>

namespace pingfs {

static std::unordered_set<BlockId> vec_to_set(
    const std::vector<BlockId>& blocks) {
    return std::unordered_set<BlockId>(blocks.cbegin(), blocks.cend());
}

AsyncResponse::AsyncResponse(const BlockRequest& request)
  : waiting_on_(vec_to_set(request.get_blocks())),
    blocks_(),
    promise_(),
    mutex_() {
    boost::mutex::scoped_lock scoped_lock(mutex_);
    // Test if the request was empty and we're ready to fulfill response
    notify_if_complete();
}

AsyncResponse::~AsyncResponse() {
}

std::future<std::shared_ptr<const BlockResponse>> AsyncResponse::get_future() {
    return promise_.get_future();
}

/**
 * Note this method assumes that the caller is already holding mutex_.
 * Additionally, caller must ensure that set_value only gets called once.
 */
void AsyncResponse::notify_if_complete() {
    if (waiting_on_.empty()) {
        promise_.set_value(std::make_shared<const BlockResponse>(blocks_));
    }
}

void AsyncResponse::update(std::shared_ptr<const Block> block) {
    boost::mutex::scoped_lock scoped_lock(mutex_);
    if (waiting_on_.find(block->get_id()) == waiting_on_.end()) {
        return;
    }
    waiting_on_.erase(block->get_id());
    blocks_.push_back(block);
    notify_if_complete();
}

}  // namespace pingfs
