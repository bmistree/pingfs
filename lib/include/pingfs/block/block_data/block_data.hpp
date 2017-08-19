#ifndef _BLOCK_DATA_
#define _BLOCK_DATA_

#include "block_data.pb.h"
#include <google/protobuf/repeated_field.h>

#include <memory>
#include <vector>

namespace pingfs {

typedef uint64_t BlockId;

class BlockData {
 public:
    virtual ~BlockData() {
    }
 protected:
    static std::vector<BlockId> iters_to_vector(
        const google::protobuf::RepeatedField<BlockId>& repeated_field);
};


}  // namespace pingfs

#endif
