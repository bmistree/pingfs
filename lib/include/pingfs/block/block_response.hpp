#ifndef _BLOCK_RESPONSE_
#define _BLOCK_RESPONSE_

#include "block.hpp"

#include <memory>
#include <vector>

namespace pingfs {

class BlockResponse {
 public:
    explicit BlockResponse(
        const std::vector<std::shared_ptr<const Block>>& blocks);
    ~BlockResponse();

 private:
    const std::vector<std::shared_ptr<const Block>> blocks_;
};

}  // namespace pingfs

#endif
