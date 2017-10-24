#ifndef _BLOCK_DATA_
#define _BLOCK_DATA_

#include <block_data.pb.h>
#include <google/protobuf/repeated_field.h>

#include <memory>
#include <vector>

namespace pingfs {

typedef uint64_t BlockId;

class BlockData {
 public:
    virtual bool operator==(const BlockData &other) const = 0;
    virtual bool operator!=(const BlockData &other) const = 0;
    /**
     * Return a proto-ized version of these data in {@code proto}.
     */
    virtual void gen_block_data_proto(
        proto::BlockDataProto* proto) const = 0;

    virtual ~BlockData() {
    }
 protected:
    static std::vector<BlockId> iters_to_vector(
        const google::protobuf::RepeatedField<BlockId>& repeated_field);
};


}  // namespace pingfs

#endif
