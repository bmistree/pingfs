
#include <pingfs/block/block_manager/ping/ping_block_service.hpp>

#include <pingfs/ping/echo_request.hpp>
#include <pingfs/ping/echo_response.hpp>

#include <memory>
#include <string>

namespace pingfs {

std::atomic<uint16_t> PingBlockService::id_generator_{0};

PingBlockService::PingBlockService(
    uint16_t fs_id,
    std::shared_ptr<Ping> ping,
    std::shared_ptr<BlockPingTranslator> translator,
    const std::string& remote_endpt)
  : translator_(translator),
    fs_id_(fs_id),
    id_(id_generator_++),
    ping_(ping),
    endpoint_(ping->resolve(remote_endpt)) {
    ping_->subscribe(this);
}

PingBlockService::~PingBlockService() {
    ping_->unsubscribe(this);
}

void PingBlockService::register_block(
    std::shared_ptr<const Block> block) {
    send_to_endpt(endpoint_, translate(block));
}

std::shared_ptr<const EchoRequest> PingBlockService::translate(
    std::shared_ptr<const Block> block) {
    return translator_->to_request(block, fs_id_, id_);
}

void PingBlockService::send_to_endpt(
    boost::asio::ip::icmp::endpoint endpt,
    std::shared_ptr<const EchoRequest> request) {
   ping_->ping(*request, endpoint_);
}

void PingBlockService::process(const EchoResponse& resp) {
    if (resp.get_identifier() != fs_id_) {
        return;
    }
    if (resp.get_sequence_number() != id_) {
        return;
    }

    std::shared_ptr<const Block> block =
        translator_->from_response(resp);

    notify(block);

    if (should_recycle(block->get_id())) {
        // send transformed ping
        ping_->ping(EchoRequest(resp), endpoint_);
    }
}


}  // namespace pingfs
