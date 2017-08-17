#ifndef _BLOCK_REQUEST_
#define _BLOCK_REQUEST_

#include <vector>
#include "block.hpp"

namespace pingfs {

class BlockRequest {
 public:
    // FIXME: Consider an option where we do not copy these data
    explicit BlockRequest(const std::vector<BlockId>& blocks);
    ~BlockRequest();

    const std::vector<BlockId>& get_blocks() const;

 private:
    const std::vector<BlockId> blocks_;
};

}  // namespace pingfs

#endif
