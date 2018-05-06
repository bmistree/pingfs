#ifndef _DISTRIBUTED_FREED_SERVICE_
#define _DISTRIBUTED_FREED_SERVICE_

#include <boost/thread/recursive_mutex.hpp>

#include <pingfs/block/block_manager/ping/block_ping_translator.hpp>
#include <pingfs/ping/ping.hpp>
#include <pingfs/ping/spoof/ping_spoof.hpp>

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "track_freed_service.hpp"

namespace pingfs {

class SpoofInfo {
 public:
    SpoofInfo(const boost::asio::ip::address_v4& spoofed_src,
        const boost::asio::ip::address_v4& bounce_target)
     : spoofed_src_(spoofed_src),
       bounce_target_(bounce_target) {
    }

    ~SpoofInfo() {
    }

    const boost::asio::ip::address_v4& get_spoofed_src() const {
        return spoofed_src_;
    }
    const boost::asio::ip::address_v4& get_bounce_target() const {
        return bounce_target_;
    }

 private:
    /**
     * The spoofed source of packets; these should
     * be the IP addrs of other servers running
     * distributed pingfs
     */
    const boost::asio::ip::address_v4 spoofed_src_;

    /**
     * The destination target of pings. We will
     * bounce pings off of this machine to reach
     * other nodes running distributed pingfs.
     */
    const boost::asio::ip::address_v4 bounce_target_;

};

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
        std::shared_ptr<PingSpoof> spoofer,
        const std::vector<std::shared_ptr<SpoofInfo>>& other_nodes);

    virtual ~DistributedFreedService();

    void register_block(std::shared_ptr<const Block> block) override;
    /**
     * Send the root block to all non-primary endpoints
     */
    void register_root(std::shared_ptr<const Block> root_block);

 private:
    void send_to_additional_endpts(std::shared_ptr<const Block> block);

 private:
    std::shared_ptr<PingSpoof> spoofer_;
    const std::vector<std::shared_ptr<SpoofInfo>> other_nodes_;
};

}  // namespace pingfs


#endif
