#ifndef _DISTRIBUTED_FREED_SERVICE_
#define _DISTRIBUTED_FREED_SERVICE_

#include <boost/thread/recursive_mutex.hpp>

#include <pingfs/ping/ping.hpp>
#include <pingfs/block/block_manager/ping/block_ping_translator.hpp>

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "track_freed_service.hpp"

namespace pingfs {

/**
 * Sends pings to multiple nodes
 */
class DistributedFreedService : public TrackFreedService {
 public:
    /**
     * @param remote_endpt A hostname to bounce messages
     * off of.
     */
    DistributedFreedService(
        uint16_t fs_id,
        std::shared_ptr<Ping> ping,
        std::shared_ptr<BlockPingTranslator> translator,
        const std::string& primary_endpt,
        const std::vector<std::string>& other_endpts);

    virtual ~DistributedFreedService();

    void register_block(std::shared_ptr<const Block> block) override;
    /**
     * Send the root block to all non-primary endpoints
     */
    void register_root(std::shared_ptr<const Block> root_block);

 private:
    void send_to_additional_endpts(std::shared_ptr<const Block> block);

 private:
    std::vector<boost::asio::ip::icmp::endpoint> other_endpts_;
};

}  // namespace pingfs


#endif
