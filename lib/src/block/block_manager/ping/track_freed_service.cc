#include <boost/thread/recursive_mutex.hpp>

#include <pingfs/block/block_manager/ping/track_freed_service.hpp>

namespace pingfs {

TrackFreedService::TrackFreedService(
    uint16_t fs_id,
    std::shared_ptr<Ping> ping,
    std::shared_ptr<BlockPingTranslator> translator,
    const std::string& remote_endpt)
  : PingBlockService(fs_id, ping, translator, remote_endpt) {
}

TrackFreedService::~TrackFreedService() {
}

void TrackFreedService::free_block(BlockId block_id) {
    boost::recursive_mutex::scoped_lock locker(ids_mutex_);
    freed_ids_.insert(block_id);
}

bool TrackFreedService::should_recycle(BlockId block_id) {
    boost::recursive_mutex::scoped_lock locker(ids_mutex_);
    auto iter = freed_ids_.find(block_id);
    if (iter == freed_ids_.end()) {
        return true;
    }

    freed_ids_.erase(iter);
    return false;
}


}  // namespace pingfs
