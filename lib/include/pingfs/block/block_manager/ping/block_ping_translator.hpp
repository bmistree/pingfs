#ifndef _BLOCK_PING_TRANSLATOR_
#define _BLOCK_PING_TRANSLATOR_

#include <pingfs/block/block.hpp>
#include <pingfs/ping/echo_request.hpp>
#include <pingfs/ping/echo_response.hpp>

#include <memory>

namespace pingfs {

/**
 * Interface for generating blocks from echo responses
 * and echo requests from blocks.
 */
class BlockPingTranslator {
 public:
    virtual std::shared_ptr<const Block> from_response(
        const EchoResponse& response) = 0;
    virtual std::shared_ptr<const EchoRequest> to_request(
        std::shared_ptr<const Block> block,
        uint16_t identifier, uint16_t sequence_number) = 0;
    virtual ~BlockPingTranslator() {
    }
};

}  // namespace pingfs

#endif
