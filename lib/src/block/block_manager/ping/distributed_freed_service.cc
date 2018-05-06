#include <boost/thread/recursive_mutex.hpp>

#include <pingfs/block/block_manager/ping/distributed_freed_service.hpp>
#include <pingfs/ping/ping.hpp>

#include <memory>

namespace pingfs {

DistributedFreedService::DistributedFreedService(
    uint16_t fs_id,
    std::shared_ptr<Ping> ping,
    std::shared_ptr<BlockPingTranslator> translator,
    const std::string& primary_endpt,
    std::shared_ptr<PingSpoof> spoofer,
    const std::vector<std::shared_ptr<SpoofInfo>>& other_nodes)
 : TrackFreedService(fs_id, ping, translator, primary_endpt),
   spoofer_(spoofer),
   other_nodes_(other_nodes) {
}

DistributedFreedService::~DistributedFreedService() {
}

void DistributedFreedService::register_block(
    std::shared_ptr<const Block> block) {
    // Sends to primary endpoint
    TrackFreedService::register_block(block);
    send_to_additional_endpts(block);
}

void DistributedFreedService::register_root(
    std::shared_ptr<const Block> root_block) {
    // We don't need to send this to our primary endpoint; just all
    // additioanl endpoints
    send_to_additional_endpts(root_block);
}

void DistributedFreedService::send_to_additional_endpts(
    std::shared_ptr<const Block> block) {

    std::shared_ptr<const EchoRequest> request = translate(block);
    for (auto other_iter = other_nodes_.cbegin();
         other_iter != other_nodes_.cend(); ++other_iter) {
        spoofer_->send(request, (*other_iter)->get_spoofed_src(),
            (*other_iter)->get_bounce_target());
    }
}

}  // namespace pingfs
