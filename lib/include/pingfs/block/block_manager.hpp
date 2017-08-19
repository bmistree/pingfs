#ifndef _BLOCK_MANAGER_
#define _BLOCK_MANAGER_

#include "block_request.hpp"
#include "block_response.hpp"

#include "block_data.pb.h"

#include <string>

namespace pingfs {

class BlockManager {
 public:
    virtual const Block create_block(const BlockDataProto& data) = 0;
    virtual void free_block(BlockId block_id) = 0;
    virtual const BlockResponse get_blocks(
        const BlockRequest& block_request) = 0;
    virtual ~BlockManager();
};

}  // namespace pingfs

#endif
