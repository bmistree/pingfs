#ifndef _PASS_THROUGH_TRANSLATOR_
#define _PASS_THROUGH_TRANSLATOR_

#include <pingfs/block/block.hpp>
#include <pingfs/ping/echo_request.hpp>
#include <pingfs/ping/echo_response.hpp>

#include <memory>

#include "block_ping_translator.hpp"

namespace pingfs {

/**
 * A translator that directly puts the proto-ized version
 * of a block into the data payload of a ping request and
 * vice versa for a ping response.
 */
class PassThroughTranslator : public BlockPingTranslator {
 public:
    PassThroughTranslator();
    virtual ~PassThroughTranslator();
    std::shared_ptr<const Block> from_response(
        const EchoResponse& response) override;
    std::shared_ptr<const EchoRequest> to_request(
        std::shared_ptr<const Block> block,
        uint16_t identifier, uint16_t sequence_number) override;
};

}  // namespace pingfs

#endif
