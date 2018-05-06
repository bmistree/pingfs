#include <boost/thread/recursive_mutex.hpp>

#include <pingfs/block/block_manager/ping/distributed_freed_service.hpp>
#include <pingfs/ping/ping.hpp>

#include <memory>

namespace pingfs {

static std::vector<boost::asio::ip::icmp::endpoint> get_endpts(
    const std::vector<std::string>& str_endpts,
    std::shared_ptr<Ping> ping) {
    std::vector<boost::asio::ip::icmp::endpoint> resolved;
    for (auto iter = str_endpts.cbegin(); iter != str_endpts.cend();
         ++iter) {
        resolved.push_back(ping->resolve(*iter));
    }
    return resolved;
}

DistributedFreedService::DistributedFreedService(
    uint16_t fs_id,
    std::shared_ptr<Ping> ping,
    std::shared_ptr<BlockPingTranslator> translator,
    const std::string& primary_endpt,
    const std::vector<std::string>& other_endpts)
 : TrackFreedService(fs_id, ping, translator, primary_endpt),
   other_endpts_(get_endpts(other_endpts, ping)) {
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
    for (auto endpt_iter = other_endpts_.cbegin();
         endpt_iter != other_endpts_.cend();
         ++endpt_iter) {
        send_to_endpt(*endpt_iter, request);
    }
}

}  // namespace pingfs
