#ifndef _PING_BLOCK_SERVICE_
#define _PING_BLOCK_SERVICE_

#include <boost/asio.hpp>
#include <pingfs/block/block.hpp>
#include <pingfs/ping/ping.hpp>
#include <pingfs/util/publisher.hpp>
#include <pingfs/util/subscriber.hpp>
#include <pingfs/ping/echo_response.hpp>

#include <atomic>
#include <memory>
#include <string>

#include "block_ping_translator.hpp"

namespace pingfs {

class PingBlockService :
        public Subscriber<EchoResponse>,
        public Publisher<std::shared_ptr<const Block>> {
 public:
    /**
     * @param remote_endpt A hostname to bounce messages
     * off of.
     */
    PingBlockService(
        uint16_t fs_id,
        std::shared_ptr<Ping> ping,
        std::shared_ptr<BlockPingTranslator> translator,
        const std::string& remote_endpt);

    virtual ~PingBlockService();

    /**
     * Start transmitting {@code block} across network.
     */
    void register_block(std::shared_ptr<const Block> block);

    /**
     * Subscriber override. Check if {@code resp}
     * has an identifier and sequence number that match
     * fs_id_ and id_, respectively. If they do, then
     * publish {@code resp} to subscribers.
     */
    void process(const EchoResponse& resp) override;

    /**
     * Indicates that if we receive this block, we should
     * discard it and not notify, instead of re-transmitting it.
     */
    virtual void free_block(BlockId block_id) = 0;

 protected:
    /**
     * Whether or not we should retransmit a received echo
     * request that contained a block with id {@code blockId}.
     */
    virtual bool should_recycle(BlockId blockId) = 0;

 private:
    /**
     * Used to generate unique ids across PingBlockServices.
     * Note that we don't track overflow here, so be wary.
     */
    static std::atomic<uint16_t> id_generator_;

    /**
     * All PingBlockService's for a common file system
     * should have the same id. Used in identifier field
     * of each ping packet. If this object receives a
     * ping that does not have fs_id_ as its identifier,
     * it ignores the packet.
     */
    const uint16_t fs_id_;

    /**
     * Assumed unique across all PingToBlock objects. Used
     * in the sequence field of each ping packet. If this
     * object receives a ping that does not have id_ as
     * its identifier, it ignores the packet.
     */
    const uint16_t id_;

    std::shared_ptr<Ping> ping_;
    std::shared_ptr<BlockPingTranslator> translator_;

    /**
     * The endpoint that we should retransmit received blocks to
     * (and presumably from which we receive echo responses from).
     */
    boost::asio::ip::icmp::endpoint endpoint_;
};

}  // namespace pingfs

#endif
