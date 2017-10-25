#ifndef _TRACK_FREED_SERVICE_
#define _TRACK_FREED_SERVICE_

#include <boost/thread/recursive_mutex.hpp>

#include <pingfs/ping/ping.hpp>
#include <pingfs/block/block_manager/ping/block_ping_translator.hpp>

#include <memory>
#include <string>
#include <unordered_set>

#include "ping_block_service.hpp"

namespace pingfs {

/**
 * Keeps track of all block ids that have been requested
 * to be freed.
 */
class TrackFreedService : public PingBlockService {
 public:
    /**
     * @param remote_endpt A hostname to bounce messages
     * off of.
     */
    TrackFreedService(
        uint16_t fs_id,
        std::shared_ptr<Ping> ping,
        std::shared_ptr<BlockPingTranslator> translator,
        const std::string& remote_endpt);

    virtual ~TrackFreedService();

    void free_block(BlockId block_id) override;

 protected:
    bool should_recycle(BlockId block_id) override;

 private:
    boost::recursive_mutex ids_mutex_;
    // Ids for blocks that we should garbage collect
    std::unordered_set<BlockId> freed_ids_;
};

}  // namespace pingfs


#endif
