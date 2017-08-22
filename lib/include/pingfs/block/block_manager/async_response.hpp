#ifndef _ASYNC_RESPONSE_
#define _ASYNC_RESPONSE_

#include <boost/thread/mutex.hpp>

#include <pingfs/block/block_data/block_data.hpp>
#include <pingfs/block/block_request.hpp>
#include <pingfs/block/block_response.hpp>

#include <future>
#include <memory>
#include <thread>
#include <unordered_set>
#include <vector>

namespace pingfs {

class AsyncResponse {
 public:
    explicit AsyncResponse(const BlockRequest& request);
    ~AsyncResponse();

    std::future<std::shared_ptr<const BlockResponse>> get_future();
    void update(std::shared_ptr<const Block> block);

 private:
    // This method assumes that caller is already holding mutex_.
    void notify_if_complete();

 private:
    // The block ids that this class is still waiting on
    std::unordered_set<BlockId> waiting_on_;
    // The blocks that have already been added through update
    std::vector<std::shared_ptr<const Block>> blocks_;
    std::promise<std::shared_ptr<const BlockResponse>> promise_;

    boost::mutex mutex_;
};

}  // namespace pingfs

#endif
